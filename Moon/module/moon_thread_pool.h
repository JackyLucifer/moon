/************************************************************************/
/* ��ģ��Ϊ�̳߳�ʵ��                                                   */
/************************************************************************/
#pragma once
#ifndef _MOON_THREAD_POOL_H
#define _MOON_THREAD_POOL_H
#ifdef MS_WINDOWS
#include <Windows.h>
#endif
#include "../collection/array_list.h"

#ifdef __cplusplus
extern "C" {
#endif

	/************************************************************************/
	/* �̶߳���                                                             */
	/************************************************************************/
	typedef struct _Moon_Thread{
#ifdef MS_WINDOWS
		HANDLE m_handle;//windows���߳̾��
		DWORD m_dThreadId;//win32�߳�ID
#endif
	}Moon_Thread;
	
	/************************************************************************/
	/* �����̳߳�                                                           */
	/*    ����:size   �̳߳ش�С											*/
	/*		   pFunc  �߳̿�ʼ����											*/
	/*	  ����ֵ�������ɹ������߳������ַ������ʧ�ܷ���NULL    			*/
	/************************************************************************/
	Array_List* moon_create_thread_pool(int size,
#ifdef MS_WINDOWS
		LPTHREAD_START_ROUTINE pFunc,
#endif
		void* param
		);

	/************************************************************************/
	/* ��ȡ��ʹ�õ��߳�                                                     */
	/*	������pThread  �̳߳ص�ָ��											*/
	/*	����ֵ���ɹ�����һ�����õ��߳̽ṹ��ʧ�ܷ���NULL					*/
	/************************************************************************/
	Moon_Thread moon_get_usable_thread(Array_List* pThread);

	/************************************************************************/
	/* �ͷ��̳߳�                                                           */
	/*	������pThread  �̳߳�ָ��											*/
	/************************************************************************/
	void moon_destory_thread_pool(Array_List* pThread);
#ifdef __cplusplus
}
#endif

#endif
