/************************************************************************/
/* ʹ��΢��IOCP�˿ڸ���ͨ��ģ��                                         */
/************************************************************************/
#pragma once
#ifndef _MS_NT_IOCP
#define _MS_NT_IOCP
#include "../cfg/environment.h"
#include "../module/moon_config_struct.h"
#include "../module/module_log.h"

#ifdef MS_WINDOWS
// winsock 2 ��ͷ�ļ��Ϳ�
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")


#ifdef __cplusplus
extern "C" {
#endif

bool ms_iocp_server_start();/*����IOCP����*/

void ms_iocp_server_stop();/*ֹͣIOCP����*/

#ifdef __cplusplus
}
#endif

#endif
#endif