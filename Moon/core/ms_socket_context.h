/************************************************************************/
/* ʹ��΢��IOCP�˿ڸ���ͨ��ģ��                                         */
/************************************************************************/
#pragma once
#ifndef _MS_SOCKET_CONTEXT
#define _MS_SOCKET_CONTEXT
#include "../collection/array_list.h"
#include "../cfg/environment.h"

#ifdef MS_WINDOWS
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
#define MAX_BUFFER_LEN        8192

//////////////////////////////////////////////////////////////////
// ����ɶ˿���Ͷ�ݵ�I/O����������
typedef enum _OPERATION_TYPE  
{  
	ACCEPT_POSTED,                     // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,                       // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,                       // ��־Ͷ�ݵ��ǽ��ղ���
	NULL_POSTED                        // ���ڳ�ʼ����������

}OPERATION_TYPE;

//====================================================================================
//
//				��IO���ݽṹ�嶨��(����ÿһ���ص������Ĳ���)
//
//====================================================================================
typedef struct _MS_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)              
	SOCKET         m_sockAccept;                               // ������������ʹ�õ�Socket
	WSABUF         m_wsaBuf;                                   // WSA���͵Ļ����������ڸ��ص�������������
	char           m_szBuffer[MAX_BUFFER_LEN];                 // �����WSABUF�������ַ��Ļ�����
	OPERATION_TYPE m_OpType;                                   // ��ʶ�������������(��Ӧ�����ö��)

} MS_IO_CONTEXT, *PMS_IO_CONTEXT;

/*�����µ�IO������*/
PMS_IO_CONTEXT create_new_io_context();
/*�ͷ�IO������*/
void free_io_context(PMS_IO_CONTEXT context);

/*��������ݽṹ�嶨��(����ÿһ����ɶ˿ڣ�Ҳ����ÿһ��Socket�Ĳ���)*/
typedef struct _MS_SOCKET_CONTEXT
{
	CRITICAL_SECTION     SockCritSec;//socketͬ���ٽ�������
	SOCKET      m_socket;      // ÿһ���ͻ������ӵ�Socket
	SOCKADDR_IN m_client_addr; // �ͻ��˵ĵ�ַ
	unsigned int m_client_port;// ÿ���ͻ��˵Ķ˿�								
	char m_client_id[255];//�ͻ���ID(Ψһ��ʶ)
	Array_List* m_pListIOContext;//// �ͻ���������������������ݣ�Ҳ����˵����ÿһ���ͻ���Socket���ǿ���������ͬʱͶ�ݶ��IO�����
}MS_SOCKET_CONTEXT,*PMS_SOCKET_CONTEXT;


/*����һ���µ�socket������*/
PMS_SOCKET_CONTEXT create_new_socket_context();

/*�ͷ�socket������*/
void free_socket_context(PMS_SOCKET_CONTEXT context);

#ifdef __cplusplus
}
#endif
#endif