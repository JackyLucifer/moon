/****************************************************************************
���ߣ�����Ȼ
ʱ�䣺2017-8-2
���ļ�����Ϊ����������ص�ʵ��
������������ԣ�
	1�����ڴ��еĴ�ŵ�ַ��������
�ŵ㣺
	1�����ڵ�ַ�������ԣ�����CPU��pcָ��Ѱַ�ĵ�ַ�ռ�ķ�Χ����̫�������������������ٶȷǳ��죬�������ٶȷǳ��죬�ͷſռ�Ҳ�ȽϿ�
ȱ�㣺
	2��Ҳ���ǵ�ַ�������ԣ����Ե�ÿ����������ʱ��Ҫ��������ʣ��ռ�ļ�飬����ռ䲻����Ȼ����Ҫ���·����ڴ棬
	   �����������λ�ú�����Ԫ�أ���Ҫ�Ѹ�λ�õ�Ԫ�ض�Ҫ��������Ųһλ��Ȼ���ڵ�ǰλ�ý��в��룬�������������
	   �ٶȷǳ���
****************************************************************************/
#pragma once
#ifndef _ARRAY_LIST_H
#define _ARRAY_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

//�����ʼ����С
#define ARRAY_LIST_INIT_SIZE 100
//�������ÿ�����ӵĴ�С
#define ARRAY_LIST_INCREASE_SIZE 200

//����ڵ�
typedef struct _Array_Node{
	void* data;//������
}Array_Node;

//��������ṹ
typedef struct _Array_List{
	Array_Node *node;//������
	unsigned long length;//����ĵ�ǰ����
	unsigned long size;//����������С
}Array_List;

/**
 * ���ܣ���ʼ������
 * ����ֵ������ɹ����򷵻�����ĵ�ַ�����ʧ�ܷ���NULL
 */
Array_List* array_list_init();

/**
 * ���ܣ������������
 * ������
 *		pList�������ַ
 *		pData����������ݽڵ�
 *		index��Ҫ�����λ�ã����Ϊ0����Ĭ�ϴ�����Ŀ�ʼ�����룬���Ϊ-1����Ĭ�ϴ������������
 * ����ֵ���ɹ�����0��ʧ�ܷ���-1
 */
int array_list_insert(Array_List* pList,void* pData,long index);

/**
 * ���ܣ���ĳ��λ��ȡ��Ԫ��
 * ������
 *		pList�������ַ
 *		index��λ��
 * ����ֵ������������ָ��
 */
void* array_list_getAt(Array_List* pList,unsigned long index);

/**
 * ���ܣ�ͨ������ɾ��Ԫ�أ�ɾ��Ԫ��ֻ�ǽ�data����ΪNULL���������ͷ�dataָ�룬�ɵ������ͷ�
 * ������
 *		pList�������ַ
 *		index��λ��
 */
void array_list_removeAt(Array_List* pList,unsigned long index);

/**
 * ���ܣ��Ƴ�ĳ��Ԫ��
 * ������
 *     pList�������ַ
 *	   pData��Ԫ��ָ��
 */
void array_list_remove(Array_List* pList,void* pData);

/**
 * ���ܣ��������
 * ������
 *	pList�������ַ
 */
void array_list_clear(Array_List* pList);

/**
 * ���ܣ��ͷ�����ռ�
 * ������
 *	pList�������ַ
 */
void array_list_free(Array_List* pList);

/**
 * ���ܣ�Array����
 */
void array_list_test();

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif
