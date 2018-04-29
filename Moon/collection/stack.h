/****************************************************************************
���ߣ�����Ȼ
ʱ�䣺2017-8-3
���ļ�����Ϊջ�����ʵ��
ջ�����ԣ�
	1���Ƚ����������ȳ�
****************************************************************************/
#pragma once
#ifndef _STACK_H
#define _STACK_H
#ifdef __cplusplus
extern "C" {
#endif

//����ջ�ĳ�ʼ����С
#define STACK_INIT_SIZE 100
//ջ�ռ���ֻ�У�ÿ�ε����Ĵ�С
#define STACK_INCREASE_SIZE 200

/**
 * ����ջ�ڵ�
 */
typedef struct _Stack_Node{
	void* data;//������
}Stack_Node;

/**
 * ����ջ�ṹ
 */
typedef struct _Stack{
	Stack_Node* node;//ջ��������
	unsigned long base;//ջ��ָ��
	unsigned long top;//ջ��ָ��
	unsigned long size;//ջ��С
}Stack;

/**
 * ���ܣ���ʼ��ջ
 * ����ֵ������ɹ����򷵻�ջ�ĵ�ַ�����ʧ�ܷ���NULL
 */
Stack* Stack_Init();

/**
 * ���ܣ�������ѹջ
 * ������
 *		stack��ջָ��
 *		data����ѹ��ջ�����ݵ�ַ
 */
void Stack_Push(Stack* stack,void* data);

/**
 * ���ܣ������ݵ���ջ
 * ������
 *		stack��ջָ��
 * ����ֵ�����ص����ݵĵ�ַ�����ջû�������򷵻�NULL
 */
void* Stack_Pop(Stack* stack);

/**
 * ���ܣ����ջ
 * ������
 *		stack��ջָ��
 */
void Stack_Clear(Stack* stack);

/**
 * ���ܣ��ͷ�ջ
 * ������
 *		stack��ջָ��
 */
void Stack_Free(Stack* stack);

/**
 * ���ܣ�ջ����
 */
void Stack_Test();

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
#endif