#include "Stack.h"
#include <stdlib.h>
#include <malloc.h>

/**
 * ���ܣ���ʼ��ջ
 * ����ֵ������ɹ����򷵻�ջ�ĵ�ַ�����ʧ�ܷ���NULL
 */
Stack* Stack_Init()
{
	long i = 0;
	Stack* stack = NULL;
	stack = (Stack*) malloc(sizeof(Stack));
	if(stack == NULL)
	{
		return NULL;
	}
	//����ջ�ռ�
	stack->node = (Stack_Node*)malloc(STACK_INIT_SIZE * sizeof(Stack_Node));
	if(stack->node == NULL)
	{
		free(stack);
		return NULL;
	}
	stack->base = 0;
	stack->top = stack->base;
	stack->size = STACK_INIT_SIZE;
	//����������Ϊ��
	for(i = 0;i < stack->size;i++)
	{
		stack->node[i].data = NULL;
	}
	return stack;
}

/**
 * ���ܣ�������ѹջ
 * ������
 *		stack��ջָ��
 *		data����ѹ��ջ�����ݵ�ַ
 */
void Stack_Push(Stack* stack,void* data)
{
	int i = 0;
	if(stack == NULL || stack->node == NULL)
	{
		return;
	}
	//�ж�ջ�ռ��Ƿ����
	if(stack->top - stack->base + 1 == stack->size)
	{
		//���·���ռ�ռ�
		stack->node = (Stack_Node*)realloc(stack->node,(stack->size + STACK_INCREASE_SIZE) * sizeof(Stack_Node));
		if(stack->node == NULL)
		{
			return;
		}
		stack->size += STACK_INCREASE_SIZE;
		//�����·������������ΪNULL
		for(i = stack->top + 1;i < stack->size;i++)
		{
			stack->node[i].data = NULL;
		}
	}
	//��ʼѹջ
	stack->node[stack->top].data = data;
	stack->top++;
}

/**
 * ���ܣ������ݵ���ջ
 * ������
 *		stack��ջָ��
 * ����ֵ�����ص����ݵĵ�ַ�����ջû�������򷵻�NULL
 */
void* Stack_Pop(Stack* stack)
{
	void* data = NULL;
	if(stack == NULL || stack->node == NULL)
	{
		return NULL;
	}
	if(stack->base == stack->top)//û������
		return NULL;
	data = stack->node[stack->top - 1].data;//����ջ������
	stack->node[stack->top].data = NULL;//ջ���������ÿ�
	stack->top--;//ջ����1
	return data;
}

/**
 * ���ܣ����ջ
 * ������
 *		stack��ջָ��
 */
void Stack_Clear(Stack* stack)
{
	long i = 0;
	if(stack == NULL || stack->node == NULL)
	{
		return;
	}
	for(i= stack->top - 1;i >= stack->base;i--)
	{
		if(i < 0)//��������С��0
			break;
		stack->node[i].data = NULL;
	}
	stack->top = stack->base;
}

/**
 * ���ܣ��ͷ�ջ
 * ������
 *		stack��ջָ��
 */
void Stack_Free(Stack* stack)
{
	if(stack == NULL)
		return;
	if(stack->node != NULL)
	{
		free(stack->node);
	}
	free(stack);
}

/**
 * ���ܣ�ջ����
 */
void Stack_Test()
{
	int *p = NULL;
	Stack* stack = Stack_Init();
	int i = 10,j = 11,k = 12,l = 13;
	Stack_Push(stack,&i);
	Stack_Push(stack,&j);
	Stack_Push(stack,&k);
	Stack_Push(stack,&l);
	p = (int*)Stack_Pop(stack);
	//���ջ
	Stack_Clear(stack);

	//ѭ��ѹ��1000������
	for(i = 0;i < 1000;i++)
	{
		p = (int*)malloc(sizeof(int));
		*p = i;
		Stack_Push(stack,p);
	}
	p = (int*)Stack_Pop(stack);
	Stack_Clear(stack);
	//�ͷ������ڴ�
	for(i = 0;i < 1000;i++)
	{
		free((int*)Stack_Pop(stack));
	}
	//���ջ
	Stack_Clear(stack);
	//10����ת8���Ʋ���
	i = 2386;
	while(i != 0 )
	{
		p = (int*)malloc(sizeof(int));
		*p = i % 8;
		Stack_Push(stack,p);
		i = i / 8;
	}
	//���ε���ջ�����־��ǰ˽���
	while(stack->base != stack->top)
	{
		p = (int*)Stack_Pop(stack);
		if((stack->top - stack->base + 1) == 4)
		{
			i += (*p) * 1000;
		}
		else if((stack->top - stack->base + 1) == 3)
		{
			i += (*p) * 100;
		}
		else if((stack->top - stack->base + 1) == 2)
		{
			i += (*p) * 10;
		}
		else if((stack->top - stack->base + 1) == 1)
		{
			i += (*p);
		}
		free(p);
	}
	//i��ֵ����2386�İ˽���
	Stack_Free(stack);
}