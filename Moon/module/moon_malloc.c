#include "../cfg/environment.h"
#include <malloc.h>
#ifdef __cplusplus
extern "C" {
#endif

void* moon_malloc(unsigned int num_bytes)/*allocate memory*/
{
	/*�Ժ���Ҫ�����ڴ�صĹ���*/
	void* pMemory = malloc(num_bytes);
	memset(pMemory,0,num_bytes);//��ʼ��Ϊ0
	return pMemory;
}

void moon_free(void* memory)/*free point*/
{
	if(memory != NULL)
	{
		free(memory);
	}
}

#ifdef __cplusplus
}
#endif