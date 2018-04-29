#include "../cfg/environment.h"
#include "moon_http_service.h"
#include "moon_config_struct.h"
#include "moon_thread_pool.h"
#include <Strsafe.h>
#include "module_log.h"
#include "moon_string.h"
#include "moon_malloc.h"
#include <direct.h>
#include <stdio.h>
#ifdef MS_WINDOWS
#include <winbase.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define RECV_BUF_LENGTH 1024 //����������ݰ��Ļ�������С
#define MIN_BUF 128 //��С��������С
#define RESPONSE_BUF_LENGTH 1024*10 //��Ӧ��������С
#define SEND_FILE_SIZE 1024 //ÿ�η����ļ��Ĵ�С

	extern Moon_Server_Config* p_global_server_config;//��������ȫ�ֱ���

	static Array_List* pThreads = NULL;//�̳߳�����

	static bool g_bEndListen = false;//�Ƿ�ֹͣ����

	
	/************************************************************************/
	/* ��ȡ����ɹ���Э��ͷ�ַ���                                           */
	/*	������pOutHead ������ַ���											*/
	/*		  sendContextLength �����ı���С								*/
	/*	����ֵ����������ַ����Ĵ�С										*/
	/************************************************************************/
	int moon_http_request_success_200(char* pOutHead,long sendContextLength)
	{
		sprintf(pOutHead,"HTTP/1.1 200 OK\nConnection: keep-alive\nServer: Moon_Http_Service/1.1\nContent-Length: %ld\nContent-Type: text/html\n\n",sendContextLength);
		return moon_get_string_length(pOutHead);
	}

	/************************************************************************/
	/* ��ȡ����ɹ���Э��ͷ������ͼƬ����                                           */
	/*	������pOutHead ������ַ���											*/
	/*		  sendContextLength �����ı���С								*/
	/*	����ֵ����������ַ����Ĵ�С										*/
	/************************************************************************/
	int moon_http_response_head_image(char* pOutHead,long sendContextLength,char* extension)
	{
		char tmp[20] = {0};
		moon_to_capital(extension,tmp);
		if (strcmp("JPG",tmp) == 0 || strcmp("JPEG",tmp) == 0)
		{
			sprintf(pOutHead,"HTTP/1.1 200 OK\nConnection: keep-alive\nServer: Moon_Http_Service/1.1\nContent-Transfer-Encoding: binary\nContent-Length: %ld\nContent-Type: image/jpeg\n\n",sendContextLength);
		}
		else if (strcmp("PNG",tmp) == 0)
		{
			sprintf(pOutHead,"HTTP/1.1 200 OK\nConnection: keep-alive\nServer: Moon_Http_Service/1.1\nContent-Transfer-Encoding: binary\nContent-Length: %ld\nContent-Type: image/PNG\n\n",sendContextLength);
		}
		else if (strcmp("BMP",tmp) == 0)
		{
			sprintf(pOutHead,"HTTP/1.1 200 OK\nConnection: keep-alive\nServer: Moon_Http_Service/1.1\nContent-Transfer-Encoding: binary\nContent-Length: %ld\nContent-Type: image/BMP\n\n",sendContextLength);
		}
		return moon_get_string_length(pOutHead);
	}



/*********************************************************************************************************
	 �����ǹ���web�������ҳ������
**********************************************************************************************************/

	/************************************************************************/
	/*���������http����������                                            */
	/*	pOutContent:�������												*/
	/*	errmsg����������													*/
	/************************************************************************/
	int moon_http_browser_request_error(char* pOutContent,char* errmsg)
	{
		char tmpHtml[1024] = {0};
		sprintf(pOutContent,"<html xmlns=http://www.w3.org/1999/xhtml>");
		strcat(pOutContent,"<head><title>moon http server</title></head>");
		strcat(pOutContent,"<body>");
		strcat(pOutContent,"<div align='center'><h1>moon server has error</h1></div>");
		sprintf(tmpHtml,"<div align='center'>%s</div>",errmsg);
		strcat(pOutContent,tmpHtml);
		strcat(pOutContent,"</body>");
		strcat(pOutContent,"</html>");
		return moon_get_string_length(pOutContent);
	}

	/************************************************************************/
	/* �������ҳ����                                                       */
	/*	������pOutContent �������											*/
	/*	����ֵ������������ݳ���											*/
	/************************************************************************/
	int moon_http_browser_request_index(char* pOutContent)
	{
		char tmpHtml[1024] = {0};
		sprintf(pOutContent,"<html xmlns=http://www.w3.org/1999/xhtml>");
		strcat(pOutContent,"<head><title>moon http server</title></head>");
		strcat(pOutContent,"<body bgcolor='#3299CC'>");
		strcat(pOutContent,"<div align='center'><h1>Moon Server Info</h1></div>");
		memset(tmpHtml,0,1024);
		sprintf(tmpHtml,"<div><image style='width:100px;height:100px;' src='http://%s:%d/image/748x578.jpg?platform=b'/></div>",p_global_server_config->server_ip,p_global_server_config->http_port);
		strcat(pOutContent,tmpHtml);
		sprintf(tmpHtml,"<div><label>Moon Server IP:</label><label>%s</label><label style='margin-left:100px;'>Moon Port:</label><label>%d</label></div>",p_global_server_config->server_ip,p_global_server_config->server_port);
		strcat(pOutContent,tmpHtml);
		memset(tmpHtml,0,1024);
		strcat(pOutContent,"</body>");
		strcat(pOutContent,"</html>");
		return moon_get_string_length(pOutContent);
	}

	/************************************************************************/
	/* ����������ļ���ͼƬ����Ϣ��                                         */
	/*	������pOutContent:���ͼƬ��										*/
	/*		  path:�����ͼƬ·��(���·��)  								*/
	/*		  clientSock:�������ݰ�			  								*/
	/************************************************************************/
	long moon_http_browser_request_file(char* pOutContent,char* path,SOCKET clientSock)
	{
		int responseHeadLength = 0;
		char responseHead[1024] = {0};
		long imageLength = 0;
		char imagePath[1024] = {0};
		FILE *resource = NULL;
		char errmsg[126] = {0};
		char tmpPath[1024] = {0};//��Ŵ�д·��������Ѱ�Һ�׺
		char imageExt[20] = {0};//ͼƬ��չ��
		char send_buf[SEND_FILE_SIZE] = {0};//ÿ�η����ļ��Ĵ�С
		// �������url·��ת��Ϊ����·��
		_getcwd(imagePath,_MAX_PATH);
		strcat(imagePath,path);
		// �򿪱���·���µ��ļ������紫������r�ı���ʽ�򿪻����
		resource = fopen(imagePath,"rb");
		//���ͼƬ�����ڣ��򷵻ش�����Ϣ
		if(resource==NULL)
		{
			return moon_http_browser_request_error(pOutContent,"request file not found");
		}
		//���ļ�����
		fseek(resource,0,SEEK_SET);
		fseek(resource,0,SEEK_END);
		imageLength = ftell(resource);
		fseek(resource,0,SEEK_SET);//�����ļ�ָ��
		//�ж�ͼƬ��ʽ
		moon_to_capital(imagePath,tmpPath);
		if (strstr(imagePath,"JPG") != NULL || strstr(imagePath,"JPEG") != NULL)
		{
			strcpy(imageExt,"JPG");
		}
		else if (strstr(imagePath,"PNG") != NULL)
		{
			strcpy(imageExt,"PNG");
		}
		else if (strstr(imagePath,"BMP") != NULL)
		{
			strcpy(imageExt,"BMP");
		}
		responseHeadLength = moon_http_response_head_image(responseHead,imageLength,imageExt);//���ͳɹ�����Ӧͷ��
		send(clientSock,responseHead,responseHeadLength,0);//ͷ��
		//��ʼ����ͼƬ�ļ�
		while (1)
		{
			memset(send_buf,0,SEND_FILE_SIZE); //������0
			imageLength = fread(send_buf,1,SEND_FILE_SIZE,resource);
			if (SOCKET_ERROR == send(clientSock, send_buf, imageLength, 0))
			{
				sprintf(errmsg,"send() Failed:%d",WSAGetLastError());
				moon_write_error_log(errmsg);
				break;
			}
			if(feof(resource))
				break;
		}   
		if (resource != NULL)
		{
			fclose(resource);
		}
		return 0;
	}


/*****************************************************************************************************************/


	/************************************************************************/
	/* �������url��·���Ͳ����ָ���                                        */
	/*   ������srcUrl:�����url�����ַ���									*/
	/*		   pOutUrlPath:�����·����Ϣ									*/
	/*		   pOutParam������Ĳ�����Ϣ									*/
	/************************************************************************/
	void moon_http_splite_url_and_param(char* srcUrl,char* pOutUrlPath,char* pOutParam)
	{
		int i = 0;
		int iParam = 0;
		if (srcUrl == NULL)
		{
			return;
		}
		while (srcUrl[i] != '\0' && srcUrl[i] != '?')//��ȡurlpath
		{
			pOutUrlPath[i] = srcUrl[i];
			i++;
		}
		//���\0
		pOutUrlPath[i] = '\0';
		while (srcUrl[i] != '\0')//��ȡ����
		{
			if (srcUrl[i] != '?')
			{
				pOutParam[iParam] = srcUrl[i];
				iParam++;
			}
			i++;
		}
		pOutParam[iParam] = '\0';
	}

	/************************************************************************/
	/* ����http��Ӧ                                                         */
	/*	������url �����url�ַ���											*/
	/*		  pOutContent ��Ӧ������										*/
	/*		  clientSock ���ڷ����ļ��ȴ�����								*/
	/*	����ֵ��������Ӧ�����ַ����ĳ���									*/
	/************************************************************************/
	long moon_http_response(char* url,char* pOutContent,SOCKET clientSock)
	{
		char requestPath[1024] = {0};//����·��
		char requestParam[1024] = {0};//�������
		//��urlת�ɴ�д
		char urlTmp[MIN_BUF] = {0};
		char logMsg[1200] = {0};//��־��Ϣ
		sprintf(logMsg,"current request url path:%s",url);
		moon_write_debug_log(logMsg);
		if (strcmp("\\",url) == 0)//��ʾ�����������ҳ
		{
			return moon_http_browser_request_index(pOutContent);
		}
		else //�������������
		{
			moon_http_splite_url_and_param(url,requestPath,requestParam);
			memset(urlTmp,0,MIN_BUF);
			moon_to_capital(requestParam,urlTmp);//�������url·��ת�ɴ�д�������ж�
			//�жϲ����Ƿ�Ϊ�գ���������ƽ̨Ϊ��
			if(stringIsEmpty(requestParam) || strstr(urlTmp,"PLATFORM=") == NULL)
			{
				return moon_http_browser_request_error(pOutContent,"request url platform is null");
			}
			memset(urlTmp,0,MIN_BUF);
			moon_to_capital(requestPath,urlTmp);//�������url·��ת�ɴ�д�������ж�
			if(strstr(urlTmp,".JPG") != NULL || strstr(urlTmp,".PNG") != NULL || strstr(urlTmp,".BMP") != NULL|| strstr(urlTmp,".JPEG") != NULL)//�������ͼƬ
			{
				return moon_http_browser_request_file(pOutContent,urlTmp,clientSock);
			}
		}
		return moon_get_string_length(pOutContent);
	}

#ifdef MS_WINDOWS

	static HANDLE g_hListener = NULL;//�����߳̾��

	static DWORD g_dListenerId = 0;//�����߳�ID

	static SOCKET g_sListener=NULL;//�����׽���

	//������
	VOID CALLBACK moon_win32_http_request(PTP_CALLBACK_INSTANCE instance,PVOID context,PTP_WORK work)  
	{  
		moon_http_context* pContext = (moon_http_context*)context;
		SOCKET sAccept = pContext->m_socket;
		char recv_buf[RECV_BUF_LENGTH] = {0}; 
		char method[MIN_BUF] = {0};
		char url[MIN_BUF] = {0};
		char strMsg[256] = {0};
		int i, j;
		char responseHead[1024] = {0};//��Ӧͷ��
		int responseHeadLength = 0;//��Ӧͷ������
		char responseError[1024] = {0};//��Ӧ����
		int responseErrorLength = 0;//��Ӧ���󳤶�
		char* pResponseContent = NULL;//��Ӧ����
		long responseLength = 0;//��Ӧ���ݳ���
		if (recv(sAccept,recv_buf,sizeof(recv_buf),0) == SOCKET_ERROR)   //���մ���
		{
			sprintf(strMsg,"http service request at recv() Failed:%d",WSAGetLastError());
			moon_write_error_log(strMsg);
			return;
		}
		i = 0; j = 0;
		// ȡ����һ�����ʣ�һ��ΪHEAD��GET��POST
		while (!(' ' == recv_buf[j]) && (i < sizeof(method) - 1))
		{
			method[i] = recv_buf[j];
			i++; j++;
		}
		method[i] = '\0';   // ������������Ҳ�ǳ�ѧ�ߺ����׺��ӵĵط�
		// �������GET��HEAD��������ֱ�ӶϿ���������
		// ��������Ĺ淶Щ���Է��������һ��501δʵ�ֵı�ͷ��ҳ��
		if (stricmp(method, "GET") && stricmp(method, "HEAD"))
		{
			if (sAccept != NULL)
			{
				closesocket(sAccept); //�ͷ������׽��֣�������ÿͻ���ͨ��
				sAccept = NULL;
			}
			moon_write_debug_log("http request is not get or head method.close ok.");
			return;
		}

		// ��ȡ���ڶ�������(url�ļ�·�����ո����)������'/'��Ϊwindows�µ�·���ָ���'\'
		// ����ֻ���Ǿ�̬����(����url�г���'?'��ʾ�Ǿ�̬����Ҫ����CGI�ű���'?'������ַ�����ʾ���������������'+'����
		// ���磺www.csr.com/cgi_bin/cgi?arg1+arg2 �÷�����ʱҲ�в�ѯ�����ڳ���������)
		i = 0;
		while ((' ' == recv_buf[j]) && (j < sizeof(recv_buf)))
			j++;
		while (!(' ' == recv_buf[j]) && (i < sizeof(recv_buf) - 1) && (j < sizeof(recv_buf)))
		{
			if (recv_buf[j] == '/')
				url[i] = '\\';
			else if(recv_buf[j] == ' ')
				break;
			else
				url[i] = recv_buf[j];
			i++; j++;
		}
		url[i] = '\0';
		
		if (pResponseContent == NULL)
		{
			pResponseContent = (char*)moon_malloc(RESPONSE_BUF_LENGTH * sizeof(char));
			if (pResponseContent == NULL)
			{
				moon_write_error_log("moon_malloc falied");
				//��ͻ��˷��ͷ������������Ϣ
				//�ж��������������moonManager����
			}
		}
		responseLength = moon_http_response(url,pResponseContent,sAccept);//�������Ӧ��Ϣ�ŷ������ݰ�
		if (responseLength > 0)
		{
			responseHeadLength = moon_http_request_success_200(responseHead,responseLength);
			send(sAccept,responseHead,responseHeadLength,0);//����200������ɹ���ͷ��
			send(sAccept,pResponseContent,responseLength, 0);//������Ӧ����
		}
		closesocket(sAccept); //�ͷ������׽��֣�������ÿͻ���ͨ��
		sAccept = NULL;
		moon_free(pResponseContent);
		pResponseContent = NULL;
	} 


	//�����߳�
	DWORD WINAPI moon_win32_http_listen(LPVOID lpParameter)
	{
		char strMsg[256] = {0};
		WSADATA wsaData;
		SOCKET sAccept;        //�����������׽��֣������׽���
		moon_http_context context;//http������
		int serverport=p_global_server_config->http_port;   //�������˿ں�
		struct sockaddr_in ser,cli;   //��������ַ���ͻ��˵�ַ
		int iLen;
		PTP_POOL tPool;
		TP_CALLBACK_ENVIRON pcbe;
		//��һ��������Э��ջ
		if (WSAStartup(MAKEWORD(2,2),&wsaData) !=0)
		{
			moon_write_error_log("Failed to load Winsock.");
			return -1;
		}
		//�ڶ��������������׽��֣����ڼ����ͻ�����
		g_sListener =socket(AF_INET,SOCK_STREAM,0);
		if (g_sListener == INVALID_SOCKET)
		{
			sprintf(strMsg,"http service socket() Failed:%d",WSAGetLastError());
			moon_write_error_log(strMsg);
			return -1;
		}
		//������������ַ��IP+�˿ں�
		ser.sin_family=AF_INET;
		ser.sin_port=htons(serverport);               //�������˿ں�
		ser.sin_addr.s_addr=inet_addr(p_global_server_config->server_ip);   //������IP��ַ

		//���������󶨼����׽��ֺͷ�������ַ
		if (bind(g_sListener,(LPSOCKADDR)&ser,sizeof(ser))==SOCKET_ERROR)
		{
			sprintf(strMsg,"http service blind() Failed:%d",WSAGetLastError());
			moon_write_error_log(strMsg);
			return -1;
		}
		//���岽��ͨ�������׽��ֽ��м���
		if (listen(g_sListener,5)==SOCKET_ERROR)
		{
			sprintf(strMsg,"http service listen() Failed:%d",WSAGetLastError());
			moon_write_error_log(strMsg);
			return -1;
		}
		//�����̳߳�
		tPool = CreateThreadpool(NULL);
		//�����̳߳������С���߳�����
		SetThreadpoolThreadMinimum(tPool,5);
		SetThreadpoolThreadMaximum(tPool,10);
		//��ʼ���̳߳ػ�������
		InitializeThreadpoolEnvironment(&pcbe);
		//Ϊ�̳߳������̳߳ػ�������
		SetThreadpoolCallbackPool(&pcbe,tPool);
		while (!g_bEndListen)  //ѭ���ȴ��ͻ�������
		{
			//�����������ܿͻ��˵��������󣬷�����ÿͻ������������׽���
			iLen=sizeof(cli);
			sAccept=accept(g_sListener,(struct sockaddr*)&cli,&iLen);
			if (sAccept==INVALID_SOCKET)
			{
				sprintf(strMsg,"http service accept() Failed:%d",WSAGetLastError());
				moon_write_error_log(strMsg);
				break;
			}
			//���߲�����������������������
			//DWORD ThreadID;
			//CreateThread(NULL,0,SimpleHTTPServer,(LPVOID)sAccept,0,&ThreadID);
			//���ι����ύ
			context.m_socket = sAccept;
			TrySubmitThreadpoolCallback(moon_win32_http_request,&context,&pcbe);
		}
		//�����̳߳صĻ�������
		DestroyThreadpoolEnvironment(&pcbe);
		//�ر��̳߳�
		CloseThreadpool(tPool);
		return 0;
	}
#endif

	/************************************************************************/
	/* ����http����                                                         */
	/************************************************************************/
	bool lauch_http_service()
	{
		//�����̳߳�
#ifdef MS_WINDOWS
		//���������߳�
		g_bEndListen = false;
		g_hListener = CreateThread(NULL,0,moon_win32_http_listen,NULL,0,&g_dListenerId);
		if (g_hListener == NULL)
		{
			moon_write_error_log("create http_listenning_server thread failed");
			return false;
		}
		//�رռ����߳̾��
		CloseHandle(g_hListener);
		//pThreads = moon_create_thread_pool(10,moon_win32_http_request,NULL);
#endif
		moon_write_info_log("moon http service started");
	}

	/************************************************************************/
	/* ֹͣhttp����                                                         */
	/************************************************************************/
	bool end_http_service()
	{
		moon_write_info_log("close http server");
		g_bEndListen = true;
		//�ȴ������߳��˳�
#ifdef MS_WINDOWS
		WaitForSingleObject(g_hListener,1000);
		closesocket(g_sListener);
		WSACleanup();
#endif
	}

#ifdef __cplusplus
}
#endif