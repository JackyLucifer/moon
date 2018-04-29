#include "ms_nt__iocp.h"
#include "ms_socket_context.h"
#include "../module/moon_char.h"
#include <stdio.h>
#include "../module/moon_string.h"

#ifdef MS_WINDOWS
#include <wchar.h>
#endif

#ifdef MS_WINDOWS
#ifdef __cplusplus
extern "C" {
#endif

// ÿһ���������ϲ������ٸ��߳�(Ϊ������޶ȵ��������������ܣ���������ĵ�)
#define WORKER_THREADS_PER_PROCESSOR 2
// ͬʱͶ�ݵ�Accept���������(���Ҫ����ʵ�ʵ�����������)
#define MAX_POST_ACCEPT              10
// ���ݸ�Worker�̵߳��˳��ź�
#define EXIT_CODE                    NULL


// �ͷ�ָ��;����Դ�ĺ�

// �ͷ�ָ���
#define RELEASE(x)                      {if(x != NULL ){free(x);x=NULL;}}
// �ͷž����
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
// �ͷ�Socket��
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}
//�����ٽ���
#define _EnterCriticalSection(x) {if( x != NULL){EnterCriticalSection(&x->SockCritSec);}}
//�˳��ٽ���
#define _LeaveCriticalSection(x) {if( x != NULL){LeaveCriticalSection(&x->SockCritSec);}}

static HANDLE	g_hShutdownEvent;// ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�
static HANDLE g_hIOCompletionPort;//��ɶ˿ڵľ��
static unsigned int g_nThreads;//�����̵߳�����
static HANDLE* g_phWorkerThreads;// �������̵߳ľ��ָ��
static PMS_SOCKET_CONTEXT g_pListenContext = NULL;//������������
static LPFN_ACCEPTEX                g_lpfnAcceptEx;                // AcceptEx �� GetAcceptExSockaddrs �ĺ���ָ�룬���ڵ�����������չ����
static LPFN_GETACCEPTEXSOCKADDRS    g_lpfnGetAcceptExSockAddrs; 
static Array_List* g_pListMSClientSocketContext;//�ͻ���Socket��Context��Ϣ
static HANDLE g_hAliveThread;//��������߳�



/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��

bool isSocketAlive(SOCKET s)
{
	int nByteSent=send(s,"",0,0);//����һ��������
	if (-1 == nByteSent) return false;
	return true;
}

///////////////////////////////////////////////////////////////////
// ��ʾ��������ɶ˿��ϵĴ���
bool handleError( MS_SOCKET_CONTEXT *pContext,const DWORD dwErr )
{
	char strMsg[256] = {0};
	// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
	if(WAIT_TIMEOUT == dwErr)  
	{  	
		// ȷ�Ͽͻ����Ƿ񻹻���...
		if( !isSocketAlive( pContext->m_socket) )
		{
			array_list_remove(g_pListMSClientSocketContext,pContext);
			free_socket_context(pContext);
			pContext = NULL;
			return true;
		}
		else
		{
			return true;
		}
	}
	// �����ǿͻ����쳣�˳���
	else if( ERROR_NETNAME_DELETED==dwErr )
	{
		array_list_remove(g_pListMSClientSocketContext,pContext);
		return true;
	}
	else
	{
		sprintf(strMsg,"completion port error,error code:%d",dwErr);
		moon_write_error_log(strMsg);
		return false;
	}
}

/////////////////////////////////////////////////////
// �����(Socket)�󶨵���ɶ˿���
bool associateWithIOCP( PMS_SOCKET_CONTEXT pContext )
{
	char strMsg[256] = {0};
	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->m_socket, g_hIOCompletionPort, (DWORD)pContext, 0);

	if (NULL == hTemp)
	{
		sprintf(strMsg,"execute CreateIoCompletionPort() function error,error code:%d",GetLastError());
		moon_write_error_log(strMsg);
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////
// Ͷ�ݽ�����������
bool postRecv( PMS_IO_CONTEXT pIoContext )
{
	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	int nBytesRecv = 0;
	char strMsg[256] = {0};
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	memset(pIoContext->m_szBuffer,0,MAX_BUFFER_LEN);
	pIoContext->m_OpType = RECV_POSTED;

	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	nBytesRecv = WSARecv( pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL );

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		sprintf(strMsg,"post first message error,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg);
		return false;
	}
	return true;
}

//====================================================================================
//
//				    Ͷ����ɶ˿�����
//
//====================================================================================
//////////////////////////////////////////////////////////////////
// Ͷ��Accept����
bool post_accept( PMS_IO_CONTEXT pAcceptIoContext )
{
	// ׼������
	DWORD dwBytes = 0;  
	char strMsg[256] = {0};
	WSABUF *p_wbuf   = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->m_Overlapped;
	pAcceptIoContext->m_OpType = ACCEPT_POSTED;

	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� ) 
	pAcceptIoContext->m_sockAccept  = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET==pAcceptIoContext->m_sockAccept)  
	{  
		sprintf(strMsg,"create accept socket failed,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg);
		return false;
	} 

	// Ͷ��AcceptEx
	if(FALSE == g_lpfnAcceptEx( g_pListenContext->m_socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2),   
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))  
	{  
		if(WSA_IO_PENDING != WSAGetLastError())  
		{  
			sprintf(strMsg,"post accept request failed,error code:%d",WSAGetLastError());
			moon_write_error_log(strMsg);
			return false;
		}  
	}
	return true;
}

////////////////////////////////////////////////////////////
// ���пͻ��������ʱ�򣬽��д���
// �����е㸴�ӣ���Ҫ�ǿ������Ļ����Ϳ����׵��ĵ���....
// ������������Ļ�����ɶ˿ڵĻ������������һ�����

// ��֮��Ҫ֪�����������ListenSocket��Context��������Ҫ����һ�ݳ������������Socket��
// ԭ����Context����Ҫ���������Ͷ����һ��Accept����
//
bool doAccpet( PMS_SOCKET_CONTEXT pSocketContext, PMS_IO_CONTEXT pIoContext )
{
	
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;  
	PMS_SOCKET_CONTEXT pNewSocketContext = NULL;
	PMS_IO_CONTEXT pNewIoContext = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
	moon_char clientMsg[MAX_BUFFER_LEN] = {0};
	int len = 0;
	
	///////////////////////////////////////////////////////////////////////////
	// 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	// ��� m_lpfnGetAcceptExSockAddrs �����˰�~~~~~~
	// ��������ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������˳��ȡ���ͻ��˷����ĵ�һ�����ݣ���ǿ����...
	g_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2),  
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);
	//�����ͻ��˵�һ�δ�������Ϣ
	len = moon_ms_windows_utf8_to_unicode(pIoContext->m_szBuffer,clientMsg);//���յ���utf-8���ֽ���ת��Ϊunicode

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. ������Ҫע�⣬���ﴫ��������ListenSocket�ϵ�Context�����Context���ǻ���Ҫ���ڼ�����һ������
	// �����һ���Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext

	pNewSocketContext = create_new_socket_context();
	pNewSocketContext->m_socket           = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_client_addr), ClientAddr, sizeof(SOCKADDR_IN));

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	if( false==associateWithIOCP( pNewSocketContext ) )
	{
		RELEASE(pNewSocketContext);
		return false;
	} 

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	pNewIoContext = create_new_io_context();
	pNewIoContext->m_OpType       = RECV_POSTED;
	pNewIoContext->m_sockAccept   = pNewSocketContext->m_socket;
	// ���Buffer��Ҫ���������Լ�����һ�ݳ���
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

	// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
	if( false==postRecv( pNewIoContext) )
	{
		array_list_remove(pNewSocketContext->m_pListIOContext,pNewIoContext);
		free_io_context(pNewIoContext);
		free_socket_context(pNewSocketContext);
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
	array_list_insert(g_pListMSClientSocketContext,pNewSocketContext,-1);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 5. ʹ�����֮�󣬰�Listen Socket���Ǹ�IoContext���ã�Ȼ��׼��Ͷ���µ�AcceptEx
	memset(pIoContext->m_szBuffer,0,MAX_BUFFER_LEN);
	return post_accept( pIoContext );
}

/////////////////////////////////////////////////////////////////
// ���н��յ����ݵ����ʱ�򣬽��д���
bool doRecv( PMS_SOCKET_CONTEXT pSocketContext, PMS_IO_CONTEXT pIoContext )
{
	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	//SOCKADDR_IN* ClientAddr = &pSocketContext->m_client_addr;
	//this->_ShowMessage( _T("�յ�  %s:%d ��Ϣ��%s"),inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf );

	// Ȼ��ʼͶ����һ��WSARecv����
	return postRecv( pIoContext );
}

//////////////////////////////////////////////////////////////////////////
// Ͷ�ݷ������ݵ�����
bool postSend(PMS_IO_CONTEXT pIoContext)
{
	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	int nBytesRecv = 0;
	char strMsg[256] = {0};
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;
	pIoContext->m_OpType = SEND_POSTED;
	//memset(pIoContext->m_szBuffer,0,MAX_BUFFER_LEN);
	p_wbuf->len = strlen(p_wbuf->buf);
	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	nBytesRecv = WSASend( pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL );

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		//this->_ShowMessage("Ͷ�ݵ�һ��WSARecvʧ�ܣ�");
		sprintf(strMsg,"post first message error,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg);
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//���з��͵����ݵ�ʱ����д���
bool doSend(PMS_SOCKET_CONTEXT pSocketContext, PMS_IO_CONTEXT pIoContext)
{
	return postSend(pIoContext);
}

///////////////////////////////////////////////////////////////////
// �������̣߳�  ΪIOCP�������Ĺ������߳�
//         Ҳ����ÿ����ɶ˿��ϳ�����������ݰ����ͽ�֮ȡ�������д�����߳�
///////////////////////////////////////////////////////////////////
DWORD static WINAPI worker_thread(LPVOID lpParam)
{
	OVERLAPPED           *pOverlapped = NULL;
	PMS_SOCKET_CONTEXT   pSocketContext = NULL;
	DWORD                dwBytesTransfered = 0;

	// ѭ����������֪�����յ�Shutdown��ϢΪֹ
	while (WAIT_OBJECT_0 != WaitForSingleObject(g_hShutdownEvent, 0))
	{
		BOOL bReturn = GetQueuedCompletionStatus(
			g_hIOCompletionPort,
			&dwBytesTransfered,
			(PULONG_PTR)&pSocketContext,
			&pOverlapped,
			INFINITE);
		if (pSocketContext != NULL)
		{
			_EnterCriticalSection(pSocketContext);
		}
		// ����յ������˳���־����ֱ���˳�
		if ( EXIT_CODE==(DWORD)pSocketContext )
		{
			if (pSocketContext != NULL)
			{
				_LeaveCriticalSection(pSocketContext);
			}
			break;
		}

		// �ж��Ƿ�����˴���
		if( !bReturn )  
		{  
			DWORD dwErr = GetLastError();

			// ��ʾһ����ʾ��Ϣ
			if( !handleError( pSocketContext,dwErr ) )
			{
				_LeaveCriticalSection(pSocketContext);
				break;
			}
			_LeaveCriticalSection(pSocketContext);
			continue;
		}  
		else  
		{  
			// ��ȡ����Ĳ���
			MS_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, MS_IO_CONTEXT, m_Overlapped);  

			// �ж��Ƿ��пͻ��˶Ͽ���
			if((0 == dwBytesTransfered) && ( RECV_POSTED==pIoContext->m_OpType || SEND_POSTED==pIoContext->m_OpType))  
			{  
				// �ͷŵ���Ӧ����Դ
				array_list_remove(g_pListMSClientSocketContext,pSocketContext);
				free_socket_context(pSocketContext);
				pSocketContext = NULL;
				_LeaveCriticalSection(pSocketContext);
				continue;  
			}  
			else
			{
				switch( pIoContext->m_OpType )  
				{  
					// Accept  
				case ACCEPT_POSTED:
					{ 
						// Ϊ�����Ӵ���ɶ��ԣ�������ר�ŵ�_DoAccept�������д�����������
						doAccpet( pSocketContext, pIoContext );
					}
					break;

					// RECV
				case RECV_POSTED:
					{
						// Ϊ�����Ӵ���ɶ��ԣ�������ר�ŵ�_DoRecv�������д����������
						doRecv( pSocketContext,pIoContext );
						//���յ���Ϣ֮����ͻ��˻ش���Ϣ
						strcpy(pIoContext->m_wsaBuf.buf,"server");
						doSend(pSocketContext,pIoContext);
					}
					break;

					// SEND
				case SEND_POSTED:
					{
						//moon_write_info_log("��ʼ������Ϣ:");
						//moon_write_info_log(pIoContext->m_szBuffer);
					}
					break;
				default:
					// ��Ӧ��ִ�е�����
					//TRACE(_T("_WorkThread�е� pIoContext->m_OpType �����쳣.\n"));
					break;
				} //switch
			}//if
		}//if
		_LeaveCriticalSection(pSocketContext);
	}//while
	moon_write_info_log("the worker thread exit");
	return 0;
}

/*****************************************************************
 *  ����������߳�
 *****************************************************************/
DWORD static WINAPI alive_thread(LPVOID lpParam)
{
	int i = 0;
	MS_SOCKET_CONTEXT* pSocketContext = NULL;
	while (WAIT_OBJECT_0 != WaitForSingleObject(g_hShutdownEvent, 0))
	{
		for (i = 0;i < g_pListMSClientSocketContext->length;i++)
		{
			pSocketContext = (MS_SOCKET_CONTEXT*)array_list_getAt(g_pListMSClientSocketContext,i);
			if(!isSocketAlive(pSocketContext->m_socket))
			{
				array_list_remove(g_pListMSClientSocketContext,pSocketContext);
				pSocketContext = NULL;
				i--;
			}
		}
		Sleep(2000);
	}
	moon_write_info_log("alive thread exit");
	return 0;
}

////////////////////////////////////////////////////////////////////
// ��ʼ��WinSock 2.2
bool static load_socket_lib()
{    
	WSADATA wsaData;
	int nResult;
	char strMsg[256] = {0};
	nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != nResult)
	{
		sprintf(strMsg,"init winsock 2.2 falied,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg);
		return false; 
	}

	return true;
}

///////////////////////////////////////////////////////////////////
// ��ñ����д�����������
int getNoOfProcessors()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

////////////////////////////////
// ��ʼ����ɶ˿�
bool ms_iocp_init()
{
	int i = 0;
	DWORD nThreadID;
	char strMsg[256] = {0};
	// ������һ����ɶ˿�
	g_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );

	if ( NULL == g_hIOCompletionPort)
	{
		sprintf(strMsg,"create Completion port falied,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg);
		return false;
	}

	// ���ݱ����еĴ�����������������Ӧ���߳���
	g_nThreads = WORKER_THREADS_PER_PROCESSOR * getNoOfProcessors();

	// Ϊ�������̳߳�ʼ�����
	g_phWorkerThreads = (HANDLE*)malloc(sizeof(HANDLE) * g_nThreads);

	// ���ݼ�����������������������߳�
	for (i=0; i < g_nThreads; i++)
	{
		g_phWorkerThreads[i] = CreateThread(0, 0, worker_thread, NULL, 0, &nThreadID);
	}

	return true;
}

////////////////////////////////////////////////////////////
//	����ͷŵ�������Դ
void dispose()
{
	int i = 0;
	// �ر�ϵͳ�˳��¼����
	RELEASE_HANDLE(g_hShutdownEvent);

	// �ͷŹ������߳̾��ָ��
	for(i=0;i<g_nThreads;i++ )
	{
		RELEASE_HANDLE(g_phWorkerThreads[i]);
	}

	RELEASE(g_phWorkerThreads);

	//�ͷ����������߳̾��
	RELEASE_HANDLE(g_hAliveThread);

	// �ر�IOCP���
	RELEASE_HANDLE(g_hIOCompletionPort);

	// �رռ���Socket
	free_socket_context(g_pListenContext);

	//�ͷſͻ��������ļ���
	for (i = 0; i < g_pListMSClientSocketContext->length;)
	{
		free_socket_context((PMS_SOCKET_CONTEXT)array_list_getAt(g_pListMSClientSocketContext,i));
		i = 0;
	}
	array_list_free(g_pListMSClientSocketContext);
}

/////////////////////////////////////////////////////////////////
// ��ʼ��Socket
bool ms_init_listen_socket(const Moon_Server_Config* p_global_server_config)
{
	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	char strMsg[256] = {0};
	PMS_SOCKET_CONTEXT pNewSocketContext = NULL;
	PMS_IO_CONTEXT pNewIoContext = NULL;
	int i = 0 ;
	int addrlen = 0;
	DWORD dwBytes = 0; 
	DWORD	Flags = 0;
	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in ServerAddress;

	// �������ڼ�����Socket����Ϣ
	g_pListenContext = create_new_socket_context();

	// ��Ҫʹ���ص�IO�������ʹ��WSASocket������Socket���ſ���֧���ص�IO����
	g_pListenContext->m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_pListenContext->m_socket) 
	{
		sprintf(strMsg,"init socket failed,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg);
		return false;
	}
	else
	{
		sprintf(strMsg,"init socket success");
		moon_write_info_log(strMsg);
		memset(strMsg,0,256);
	}

	// ��Listen Socket������ɶ˿���
	if( NULL== CreateIoCompletionPort( (HANDLE)g_pListenContext->m_socket, g_hIOCompletionPort,(DWORD)g_pListenContext, 0))  
	{  
		sprintf(strMsg,"bind listen socket to io completion port failed,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg); 
		RELEASE_SOCKET( g_pListenContext->m_socket );
		return false;
	}
	else
	{
		sprintf(strMsg,"listen socket binding success");
		moon_write_info_log(strMsg);
		memset(strMsg,0,256);
	}

	// ����ַ��Ϣ
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// ������԰��κο��õ�IP��ַ�����߰�һ��ָ����IP��ַ 
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                      
	ServerAddress.sin_addr.s_addr = inet_addr(p_global_server_config->server_ip);         
	ServerAddress.sin_port = htons(p_global_server_config->server_port);                          

	// �󶨵�ַ�Ͷ˿�
	if (SOCKET_ERROR == bind(g_pListenContext->m_socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
	{
		sprintf(strMsg,"bind() execute failed,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg); 
		RELEASE_SOCKET( g_pListenContext->m_socket );
		return false;
	}

	// ��ʼ���м���
	if (SOCKET_ERROR == listen(g_pListenContext->m_socket,SOMAXCONN))
	{
		sprintf(strMsg,"listen() execute failed,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg); 
		RELEASE_SOCKET( g_pListenContext->m_socket );
		return false;
	}

	// ʹ��AcceptEx��������Ϊ���������WinSock2�淶֮���΢�������ṩ����չ����
	// ������Ҫ�����ȡһ�º�����ָ�룬
	// ��ȡAcceptEx����ָ��
	if(SOCKET_ERROR == WSAIoctl(
		g_pListenContext->m_socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx), 
		&g_lpfnAcceptEx, 
		sizeof(g_lpfnAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		sprintf(strMsg,"WSAIoctl can not get AcceptEx function point,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg); 
		dispose();
		return false;  
	}  

	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
	if(SOCKET_ERROR == WSAIoctl(
		g_pListenContext->m_socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), 
		&g_lpfnGetAcceptExSockAddrs, 
		sizeof(g_lpfnGetAcceptExSockAddrs),   
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		sprintf(strMsg,"WSAIoctl can not get GuidGetAcceptExSockAddrs function point,error code:%d",WSAGetLastError());
		moon_write_error_log(strMsg); 
		dispose();
		return false;
	}  


	// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����
	for(i=0;i < MAX_POST_ACCEPT;i++ )
	{
		// �½�һ��IO_CONTEXT
		PMS_IO_CONTEXT pAcceptIoContext = create_new_io_context();
		if( false==post_accept( pAcceptIoContext ) )
		{
			free_io_context(pAcceptIoContext);
			return false;
		}
		//�����յ�IO�����ķ�������б�
		array_list_insert(g_pListenContext->m_pListIOContext,pAcceptIoContext,-1);
	}

	//ʹ�õڶ��ֽ��շ�ʽ(����)
	/*
	while(WAIT_TIMEOUT == WaitForSingleObject(g_hShutdownEvent, 0))	//when m_killevent is set , server shutdown
	{
		pNewSocketContext = create_new_socket_context();
		addrlen = sizeof(pNewSocketContext->m_client_addr);
		if ((pNewSocketContext->m_socket = WSAAccept(g_pListenContext->m_socket,(SOCKADDR *)&(pNewSocketContext->m_client_addr), &addrlen, NULL, 0)) == SOCKET_ERROR)
		{
			free_socket_context(pNewSocketContext);
			pNewSocketContext = NULL;
			sprintf(strMsg,"WSAAccept() failed with error %d", WSAGetLastError());
			moon_write_error_log(strMsg); 
			return false;
		}
		if(!associateWithIOCP(pNewSocketContext))//���ͻ���socket�󶨵���ɶ˿�
		{
			free_socket_context(pNewSocketContext);
			pNewSocketContext = NULL;
			sprintf(strMsg,"WSAAccept() failed with error %d", WSAGetLastError());
			moon_write_error_log(strMsg); 
			return false;
		}
		array_list_insert(g_pListMSClientSocketContext,pNewSocketContext,-1);
		pNewIoContext = create_new_io_context();
		
		pNewIoContext->m_OpType = RECV_POSTED;
		pNewIoContext->m_sockAccept = pNewSocketContext->m_socket;

		//start receive ...
		if (WSARecv(pNewIoContext->m_sockAccept,&(pNewIoContext->m_wsaBuf),1, &dwBytes, &Flags,
			&(pNewIoContext->m_Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				free_io_context(pNewIoContext);
				pNewIoContext = NULL;
				sprintf(strMsg,"WSARecv() failed with error %d", WSAGetLastError());
				moon_write_error_log(strMsg); 
				return false;
			}
		}
		free_io_context(pNewIoContext);
		pNewIoContext = NULL;
	}*/
	return true;
}

bool ms_iocp_server_start(const Moon_Server_Config* p_global_server_config)/*����IOCP����*/
{
	DWORD nThreadID;
	if(!load_socket_lib())
	{
		return false;
	}
	// ����ϵͳ�˳����¼�֪ͨ
	g_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//��ʼ���ͻ��˼���
	g_pListMSClientSocketContext = array_list_init();
	// ��ʼ��IOCP
	if (false == ms_iocp_init())
	{
		moon_write_error_log("init ms_nt_iocp falied");
		return false;
	}
	else
	{
		moon_write_info_log("init ms_nt_iocp finished");
	}
	if (false == ms_init_listen_socket(p_global_server_config))
	{
		moon_write_error_log("init ms_init_listen_socket falied");
		return false;
	}
	else
	{
		moon_write_info_log("init ms_init_listen_socket finished");
	}
	//������������߳�
	g_hAliveThread = CreateThread(0, 0, alive_thread, NULL, 0, &nThreadID);
	if (g_hAliveThread == NULL)
	{
		moon_write_error_log("create alive thread failed");
		ms_iocp_server_stop();
		return false;
	}
	return true;
}

void ms_iocp_server_stop()/*ֹͣIOCP����*/
{
	int i = 0;
	if( g_pListenContext!=NULL && g_pListenContext->m_socket!=INVALID_SOCKET )
	{
		// ����ر���Ϣ֪ͨ
		SetEvent(g_hShutdownEvent);
		for (i = 0; i < g_nThreads; i++)
		{
			// ֪ͨ���е���ɶ˿ڲ����˳�
			PostQueuedCompletionStatus(g_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}
		// �ȴ����еĿͻ�����Դ�˳�
		WaitForMultipleObjects(g_nThreads, g_phWorkerThreads, TRUE, INFINITE);
		// �ȴ���������߳��˳�
		WaitForSingleObject(g_hAliveThread,INFINITE);
		// �ͷ�������Դ
		dispose();
	}	
}
#ifdef __cplusplus
}
#endif
#endif