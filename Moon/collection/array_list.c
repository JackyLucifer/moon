#include "array_List.h"
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>


#ifdef _MSC_VER/* only support win32 and greater. */
#include <windows.h>
#define MS_WINDOWS
#define ARRAY_LIST_MUTEX "ARRAY_LIST_MUTEX"
static HANDLE g_hMutex;
#endif

/**
 * ���ܣ���ʼ������
 * ����ֵ������ɹ����򷵻�����ĵ�ַ�����ʧ�ܷ���NULL
 */
Array_List* array_list_init()
{
	int i = 0;
	Array_List* pList = (Array_List*)malloc(sizeof(Array_List));//��������ռ�
	if(pList == NULL)
	{
		return NULL;
	}
	//����Node�ڵ�
	pList->node = (Array_Node*)malloc(ARRAY_LIST_INIT_SIZE * sizeof(Array_Node));//������������Ŀռ�
	if(pList->node == NULL)
	{
		free(pList);
		return NULL;
	}
	//��Node��data����Ϊ����
	for(i = 0;i < ARRAY_LIST_INIT_SIZE;i++)
	{
		pList->node[i].data = NULL;
	}
	pList->length = 0;
	pList->size = ARRAY_LIST_INIT_SIZE;

	//��ʼ��������
#ifdef MS_WINDOWS
	g_hMutex = CreateMutex(NULL, FALSE,  TEXT(ARRAY_LIST_MUTEX));
	if (g_hMutex == NULL)
	{
		array_list_free(pList);
		return NULL;
	}
#endif
	return pList;
}

/**
 * ���ܣ������������
 * ������
 *		pList�������ַ
 *		pData����������ݽڵ�
 *		index��Ҫ�����λ�ã����Ϊ0����Ĭ�ϴ�����Ŀ�ʼ�����룬���Ϊ-1����Ĭ�ϴ������������
 * ����ֵ���ɹ�����0��ʧ�ܷ���-1
 */
int array_list_insert(Array_List* pList,void* pData,long index)
{
	long i = 0;
	unsigned long reallocSize = 0;//���·���ռ�Ĵ�С
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(ARRAY_LIST_MUTEX));
	if(hMutex == NULL)
	{
		return -1;
	}
	//WaitforsingleObject���ȴ�ָ����һ��mutex��ֱ����ȡ��ӵ��Ȩ
	//ͨ����������֤�����������ȫ����ɣ����������߳��޷������
	WaitForSingleObject(hMutex, 1000);
#endif
	//////////////////////////////////////////////////////////////////////////
	//�ٽ���
	if(pList == NULL)
		return -1;
	if(index < -1 || (index > pList->length && index != -1))
	{
		return -1;
	}
	//�ж�����ռ乻������
	if(pList->length == pList->size - 1)
	{
		//���·���ռ�
		reallocSize = pList->size + ARRAY_LIST_INCREASE_SIZE;
		pList->node = (Array_Node*)realloc(pList->node,reallocSize * sizeof(Array_Node));
		if(pList->node == NULL)
		{
			return -1;
		}
		//���ҽ������ڵ��data����Ϊ��
		for(i = pList->length;i < reallocSize;i++)
		{
			pList->node[i].data = NULL;
		}
		pList->size = reallocSize;
	}
	//��ʼ����
	if(index == -1)//��ĩβ������
	{
		pList->node[pList->length].data = pData;
		pList->length++;//��������1
		return 0;
	}
	else if(index == 0) //�ӿ�ʼ������
	{
		for(i = pList->length;i >0;i--)
		{
			pList->node[i] = pList->node[i - 1];
		}
		pList->node[0].data = pData;
		pList->length++;//��������1
		return 0;
	}
	else//ָ��λ�ò���
	{
		for(i = pList->length;i > index;i--)
		{
			pList->node[i] = pList->node[i - 1];
		}
		pList->node[i].data = pData;
		pList->length++;//��������1
		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
	return 0;
}

/**
 * ���ܣ�ͨ������ɾ��Ԫ�أ�ɾ��Ԫ��ֻ�ǽ�data����ΪNULL���������ͷ�dataָ�룬�ɵ������ͷ�
 * ������
 *		pList�������ַ
 *		index��λ��
 */
void array_list_removeAt(Array_List* pList,unsigned long index)
{
	int i = 0;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(ARRAY_LIST_MUTEX));
	if(hMutex == NULL)
	{
		return;
	}
	//WaitforsingleObject���ȴ�ָ����һ��mutex��ֱ����ȡ��ӵ��Ȩ
	//ͨ����������֤�����������ȫ����ɣ����������߳��޷������
	WaitForSingleObject(hMutex, 1000);
#endif
	//////////////////////////////////////////////////////////////////////////
	//�ٽ���
	if(pList == NULL)
		return;
	if(index < 0 || index >= pList->length)
		return;
	for(i = index; i < pList->length;i++)//��ɾ�����������Ԫ��������ǰ��
	{
		pList->node[i] = pList->node[i + 1];
	}
	//�����һλ�ÿ�
	pList->node[pList->length - 1].data = NULL;
	//���ȼ�1
	pList->length--;
	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
}

/**
 * ���ܣ��Ƴ�ĳ��Ԫ��
 * ������
 *     pList�������ַ
 *	   pData��Ԫ��ָ��
 */
void array_list_remove(Array_List* pList,void* pData)
{
	int i = 0;
	int removeIndex = -1;//��Ҫ��ɾ����Ԫ������
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(ARRAY_LIST_MUTEX));
	if(hMutex == NULL)
	{
		return;
	}
	//WaitforsingleObject���ȴ�ָ����һ��mutex��ֱ����ȡ��ӵ��Ȩ
	//ͨ����������֤�����������ȫ����ɣ����������߳��޷������
	WaitForSingleObject(hMutex, 1000);
#endif
	//////////////////////////////////////////////////////////////////////////
	//�ٽ���
	for (i = 0;i < pList->length;i++)
	{
		if(array_list_getAt(pList,i) == pData)
		{
			removeIndex = i;
			break;
		}
	}
	if(removeIndex != -1)
	{
		if(pList == NULL)
			return;
		for(i = removeIndex; i < pList->length;i++)//��ɾ�����������Ԫ��������ǰ��
		{
			pList->node[i] = pList->node[i + 1];
		}
		//�����һλ�ÿ�
		pList->node[pList->length - 1].data = NULL;
		//���ȼ�1
		pList->length--;
	}
	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
}

/**
 * ���ܣ���ĳ��λ��ȡ��Ԫ��
 * ������
 *		pList�������ַ
 *		index��λ��
 */
void* array_list_getAt(Array_List* pList,unsigned long index)
{
	void* pData = NULL;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(ARRAY_LIST_MUTEX));
	if(hMutex == NULL)
	{
		return NULL;
	}
	//WaitforsingleObject���ȴ�ָ����һ��mutex��ֱ����ȡ��ӵ��Ȩ
	//ͨ����������֤�����������ȫ����ɣ����������߳��޷������
	WaitForSingleObject(hMutex, 1000);
#endif
	//////////////////////////////////////////////////////////////////////////
	//�ٽ���
	if(pList == NULL)
	{
		return NULL;
	}
	if(index < 0 || index >= pList->length)
	{
		return NULL;
	}
	pData = pList->node[index].data;
	
	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
	return pData;
}

/**
 * ���ܣ��������
 * ������
 *	pList�������ַ
 */
void array_list_clear(Array_List* pList)
{
	int i = 0;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(ARRAY_LIST_MUTEX));
	if(hMutex == NULL)
	{
		return NULL;
	}
	//WaitforsingleObject���ȴ�ָ����һ��mutex��ֱ����ȡ��ӵ��Ȩ
	//ͨ����������֤�����������ȫ����ɣ����������߳��޷������
	WaitForSingleObject(hMutex, 1000);
#endif
	//////////////////////////////////////////////////////////////////////////
	//�ٽ���
	if(pList == NULL)
	{
		return;
	}
	//����������Ϊ��
	for(i = 0;i < pList->length;i++)
	{
		if(pList->node[i].data != NULL)
		{
			pList->node[i].data = NULL;
		}
	}
	pList->length = 0;

	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
}

/**
 * ���ܣ��ͷ�����ռ䣬�ɴ������߳��ͷ�
 * ������
 *	pList�������ַ
 */
void array_list_free(Array_List* pList)
{
	if(pList == NULL)
		return;
	//�ͷŽڵ�
	if(pList->node != NULL)
	{
		free(pList->node);
		pList->node = NULL;
	}
	if(pList != NULL)
	{
		free(pList);
		pList = NULL;
	}
	if (g_hMutex != NULL)
	{
		CloseHandle(g_hMutex);
		g_hMutex = NULL;
	}
}

/**
 * ���ܣ�Array_List����
 */
void array_list_test()
{
	int i = 0;
	int *p = NULL;
	Array_List* list = array_list_init();
	//��̬���1000��ֵ
	for(i = 0;i < 1000;i++)
	{
		p = (int*) malloc(sizeof(int));
		*p = i;
		array_list_insert(list,p,-1);
	}
	//ȡ����500��Ԫ��
	p = (int*)array_list_getAt(list,499);
	//ɾ����500��Ԫ�أ�ɾ��֮��ǵ��ͷ�
	array_list_removeAt(list,499);
	free(p);
	//���»�ȡ��500��Ԫ��
	p = (int*)array_list_getAt(list,499);
	//��ʼ�ͷſռ�
	for(i = 0;i<list->length;i++)
	{
		p = (int*)array_list_getAt(list,i);
		free(p);
	}
	array_list_clear(list);
	array_list_free(list);
}