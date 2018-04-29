#include "link_list.h"
#include <stdlib.h>
#include <malloc.h>

#ifdef _MSC_VER/* only support win32 and greater. */
#include <windows.h>
#define MS_WINDOWS
#define LINK_LIST_MUTEX "LINK_LIST_MUTEX"
static HANDLE g_hMutex;
#endif

/**
 * ���ܣ���ʼ������
 * ����ֵ������ɹ����򷵻�����ĵ�ַ�����ʧ�ܷ���NULL
 */
Link_List* link_list_init()
{
	Link_List* pList = NULL;
	pList = (Link_List*)malloc(sizeof(Link_List));
	if(pList == NULL)
	{
		return NULL;
	}
	pList->length = 0;
	pList->head = NULL;
	pList->trail = NULL;
	//��ʼ��������
#ifdef MS_WINDOWS
	g_hMutex = CreateMutex(NULL, FALSE,  TEXT(LINK_LIST_MUTEX));
	if (g_hMutex == NULL)
	{
		link_list_free(pList);
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
int link_list_insert(Link_List* pList,void* pData,long index)
{
	long i = 0;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(LINK_LIST_MUTEX));
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
	//�ж��Ƿ����״β���
	if(pList->length == 0)
	{
		Link_Node* pNode = (Link_Node*)malloc(sizeof(Link_Node));
		if(pNode == NULL)
			return -1;
		pNode->data = pData;
		pNode->priorNode = NULL;
		pNode->nextNode = NULL;
		pList->head = pNode;
		pList->trail = pNode;
		pList->length++;
		return 0;
	}
	else
	{
		if(-1 == index)//��ĩβ������
		{
			//�����ڵ�
			Link_Node* pNode = (Link_Node*)malloc(sizeof(Link_Node));
			if(pNode == NULL)
				return -1;
			pNode->data = pData;
			pNode->nextNode = NULL;
			pNode->priorNode = pList->trail;
			//���ڵ�ӵ�ĩβ��
			pList->trail->nextNode = pNode;
			pList->trail = pNode;
			pList->length++;
		}
		else if(0 == index) //�ӿ�ʼ������
		{
			//�����ڵ�
			Link_Node* pNode = (Link_Node*)malloc(sizeof(Link_Node));
			if(pNode == NULL)
				return -1;
			pNode->data = pData;
			pNode->nextNode = pList->head;
			pNode->priorNode = NULL;
			//���ڵ���ص�ͷ��
			pList->head->priorNode = pNode;
			pList->head = pNode;
			pList->length++;
			return 0;
		}
		else//ָ��λ�ò���
		{
			//���ڿ���ʹ�ÿ��ٲ����㷨�Ż�
			Link_Node* pNode = pList->head;
			i=0;
			while(pNode != NULL)
			{
				if(index == i)//���ҵ�Ҫ�����λ�ýڵ�
				{
					//�����ڵ�
					Link_Node* pCurrentNode = (Link_Node*)malloc(sizeof(Link_Node));
					if(pCurrentNode == NULL)
						return -1;
					pCurrentNode->nextNode = pNode;
					pCurrentNode->priorNode = pNode->priorNode;
					pNode->priorNode->nextNode = pCurrentNode;
					pNode->priorNode = pCurrentNode;
					pList->length++;
					return 0;
				}
				pNode = pNode->nextNode;
				i++;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
	return 0;
}

/**
 * ���ܣ���ĳ��λ��ȡ��Ԫ��
 * ������
 *		pList�������ַ
 *		index��λ��
 * ����ֵ������������ָ��
 */
void* link_list_getAt(Link_List* pList,unsigned long index)
{
	int i = 0;
	Link_Node* pNode = NULL;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(LINK_LIST_MUTEX));
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
	i = 0;
	pNode = pList->head;
	while(pNode != NULL)
	{
		if(i == index)
		{
			return pNode->data;
		}
		pNode = pNode->nextNode;
		i++;
	}
	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
	return NULL;
}

/**
 * ���ܣ�ͨ������ɾ��Ԫ�أ�ɾ��Ԫ��ֻ�ǽ�data����ΪNULL���������ͷ�dataָ�룬�ɵ������ͷ�
 * ������
 *		pList�������ַ
 *		index��λ��
 */
void link_list_removeAt(Link_List* pList,unsigned long index)
{
	int i = 0;
	Link_Node* pNode = NULL;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(LINK_LIST_MUTEX));
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
	i = 0;
	pNode = pList->head;
	while(pNode != NULL)
	{
		if(i == index)
		{
			//�ҵ��ڵ㣬��ʼɾ��
			if(pNode->priorNode == NULL) //��ʾ����ͷ���ڵ�
			{
				Link_Node* head = pList->head;
				pList->head = head->nextNode;
				pList->head->priorNode = NULL;
				free(head);//�ͷŽڵ�
				pList->length--;
			}
			else if(pNode->nextNode == NULL)//��ʾ����β���ڵ�
			{
				Link_Node* trial = pList->trail;
				pList->trail = trial->priorNode;
				pList->trail->nextNode = NULL;
				free(trial);//�ͷŽڵ�
				pList->length--;
			}
			else
			{
				//��ǰ�ڵ����һ���ڵ����һ���ڵ�ָ��ǰ�ڵ����һ���ڵ�
				pNode->priorNode->nextNode = pNode->nextNode;
				//��ǰ�ڵ����һ���ڵ����һ���ڵ�ָ��ǰ�ڵ����һ���ڵ�
				pNode->nextNode->priorNode = pNode->priorNode;
				free(pNode);
				pList->length--;
			}
			return;
		}
		pNode = pNode->nextNode;
		i++;
	}
	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
}

/**
 * ���ܣ��������
 * ������
 *	pList�������ַ
 */
void link_list_clear(Link_List* pList)
{
	//����������Ϊ��
	Link_Node* pNode = NULL;
	//��Ҫ���߳�ͬ��
#ifdef MS_WINDOWS
	HANDLE hMutex = OpenMutex(SYNCHRONIZE , TRUE, TEXT(LINK_LIST_MUTEX));
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
	//��β����ʼ�ͷ�
	pNode = pList->trail;
	while(pNode != NULL)
	{
		pList->trail = pNode->priorNode;
		free(pNode);
		pNode = pList->trail;
	}
	pList->length = 0;
	pList->head = NULL;
	pList->trail = NULL;

	//////////////////////////////////////////////////////////////////////////
#ifdef MS_WINDOWS
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif
}

/**
 * ���ܣ��ͷ�����ռ�
 * ������
 *	pList�������ַ
 */
void link_list_free(Link_List* pList)
{
	Link_Node* pNode = NULL;
	if(pList == NULL)
	{
		return;
	}
	//��β����ʼ�ͷ�
	pNode = pList->trail;
	while(pNode != NULL)
	{
		pList->trail = pNode->priorNode;
		free(pNode);
		pNode = pList->trail;
	}
	pList->length = 0;
	pList->head = NULL;
	pList->trail = NULL;
	free(pList);
	pList = NULL;
	if (g_hMutex != NULL)
	{
		CloseHandle(g_hMutex);
		g_hMutex = NULL;
	}
}


/**
 * ���ܣ�Link_List����
 */
void link_list_test()
{
	int i = 0;
	int *p = NULL;
	Link_List* list = link_list_init();
	//��̬���1000��ֵ
	for(i = 0;i < 1000;i++)
	{
		p = (int*) malloc(sizeof(int));
		*p = i;
		link_list_insert(list,p,-1);
	}
	//ȡ����500��Ԫ��
	p = (int*)link_list_getAt(list,499);
	//ɾ����500��Ԫ�أ�ɾ��֮��ǵ��ͷ�
	link_list_removeAt(list,499);
	free(p);
	//���»�ȡ��500��Ԫ��
	p = (int*)link_list_getAt(list,499);
	//��ʼ�ͷſռ�
	for(i = 0;i<list->length;i++)
	{
		p = (int*)link_list_getAt(list,i);
		free(p);
	}
	link_list_clear(list);
	link_list_free(list);
}