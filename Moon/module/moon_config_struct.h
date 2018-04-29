/************************************************************************/
/* ��ģ��ϵͳʹ�����õĽṹ                                             */
/************************************************************************/
#pragma once
#ifndef _MOON_CONFIG_STRUCT_H
#define _MOON_CONFIG_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* ������������õĽṹ                                                 */
/************************************************************************/
typedef struct _Moon_Server_Config{
	char moon_manager_domain[1024];//MoonManager������ַ
	unsigned int client_count;//�ͻ��˵���������̨�������������ͻ�������������
	char server_ip[16];//������IP
	unsigned int server_port;//�������˿�
	unsigned int http_port;//http�������ö˿�
	char server_node_name[255];//�������ڵ�����
	char log_level_debug;//�Ƿ�����debug��־
	char log_level_info;//�Ƿ�����info��־
	char log_level_warnning;//�Ƿ�����warnning��־
	char log_level_error;//�Ƿ�����error��־
}Moon_Server_Config;

#ifdef __cplusplus
}
#endif

#endif