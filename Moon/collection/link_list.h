/****************************************************************************
���ߣ�����Ȼ
ʱ�䣺2017-8-2
���ļ�����Ϊ������������ص�ʵ��
������������ԣ�
	1�����ڴ��еĴ�ŵ�ַ�Ƿ������ģ��������
�ŵ㣺
	1�����ڵ�ַ�ķ������ԣ�������������������ʱ��ֻ��Ҫ�ҵ������λ�ã�Ȼ�󽫸�λ�õ�ǰ��ָ���λ��ֻ���Ԫ�صĵ�ַ���У�����Ҫ���·����ڴ�
	   ��һ˲�������ɣ��ٶȷǳ���
ȱ�㣺
	2�����ڵ�ַ�ķ������ԣ�����Ԫ�����ڴ��еĴ��λ���Ƿǹ̶��ģ����Ծ�����˵�һ��Ԫ�غ͵ڶ���Ԫ�ص��ڴ�λ�ÿ�ȷǳ������Ϊ4GB������CPU��PC
	   ָ��Ѱַ�ǳ�����������������������Ч�ʵ��£��ͷſռ�Ч�ʵ���
****************************************************************************/
#pragma once
#ifndef _Link_LIST_H
#define _Link_LIST_H
#ifdef __cplusplus
extern "C" {
#endif

//����ڵ㣬˫������
typedef struct _Link_Node{
	void* data;//������
	struct _Link_Node* priorNode;//ָ����һ���ڵ��ָ��
	struct _Link_Node* nextNode;//ָ����һ���ڵ��ָ��
}Link_Node;

//��������ṹ
typedef struct _Link_List{
	Link_Node *head;//�����ͷָ��
	Link_Node *trail;//�����β��ָ��
	unsigned long length;//����ĵ�ǰ����
}Link_List;

/**
 * ���ܣ���ʼ������
 * ����ֵ������ɹ����򷵻�����ĵ�ַ�����ʧ�ܷ���NULL
 */
Link_List* link_list_init();

/**
 * ���ܣ������������
 * ������
 *		pList�������ַ
 *		pData����������ݽڵ�
 *		index��Ҫ�����λ�ã����Ϊ0����Ĭ�ϴ�����Ŀ�ʼ�����룬���Ϊ-1����Ĭ�ϴ������������
 * ����ֵ���ɹ�����0��ʧ�ܷ���-1
 */
int link_list_insert(Link_List* pList,void* pData,long index);

/**
 * ���ܣ���ĳ��λ��ȡ��Ԫ��
 * ������
 *		pList�������ַ
 *		index��λ��
 * ����ֵ������������ָ��
 */
void* link_list_getAt(Link_List* pList,unsigned long index);

/**
 * ���ܣ�ͨ������ɾ��Ԫ�أ�ɾ��Ԫ��ֻ�ǽ�data����ΪNULL���������ͷ�dataָ�룬�ɵ������ͷ�
 * ������
 *		pList�������ַ
 *		index��λ��
 */
void link_list_removeAt(Link_List* pList,unsigned long index);


/**
 * ���ܣ��������
 * ������
 *	pList�������ַ
 */
void link_list_clear(Link_List* pList);

/**
 * ���ܣ��ͷ�����ռ�
 * ������
 *	pList�������ַ
 */
void link_list_free(Link_List* pList);

/**
 * ���ܣ�Link_List����
 */
void link_list_test();

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
#endif