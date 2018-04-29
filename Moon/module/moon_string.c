#include "../cfg/environment.h"
#include "moon_string.h"
#include <math.h>
#ifdef MS_WINDOWS
#include <windows.h>
#endif
#include "moon_malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long moon_get_string_length(const char* str)
{
	unsigned long length = 0;
	if (NULL == str)
		return length;
	while (str[length++] != '\0');
	return --length;
}

void moon_trim(char *strIn, char *strOut)//ȥ���ַ�����λ�ո�
{
	char *start, *end, *temp;//����ȥ���ո���ַ�����ͷβָ��ͱ���ָ��
	temp = strIn;
	while (*temp == ' '){
	        ++temp;
	}
	start = temp; //���ͷָ��
	temp = strIn + moon_get_string_length(strIn) - 1; //�õ�ԭ�ַ������һ���ַ���ָ��(����'\0')
	while (*temp == ' '){
	    --temp;
	}
	end = temp; //���βָ��
	for(strIn = start; strIn <= end; ){
	    *strOut++ = *strIn++;
	}
	*strOut = '\0';
}

void moon_trim_line(char *strIn, char *strOut)//ȥ����β����
{
	char *start, *end, *temp;//����ȥ���ո���ַ�����ͷβָ��ͱ���ָ��
	temp = strIn;
	while (*temp == '\n'){
		++temp;
	}
	start = temp; //���ͷָ��
	temp = strIn + moon_get_string_length(strIn) - 1; //�õ�ԭ�ַ������һ���ַ���ָ��(����'\0')
	while (*temp == '\n'){
		--temp;
	}
	end = temp; //���βָ��
	for(strIn = start; strIn <= end; ){
		*strOut++ = *strIn++;
	}
	*strOut = '\0';
}

bool moon_string_parse_to_int(const char* str,unsigned int* outInt)/*���ַ���ת��unsigned int,���ʧ�ܷ���false���ɹ�����true*/
{
	bool bFlag = true;
	int length = 0;
	int i = 0;
	if (str == NULL)
	{
		return false;
	}
	length = moon_get_string_length(str);//��ȡ�ַ�������
	for (i = 0;i < length;i++)
	{
		if (str[i] < 48 || str[i] > 57) //�������ַ�ֱ�ӷ���false
		{
			return false;
		}
		if(48 == str[i]) //0
		{
			*outInt += 0 * pow(10,(length - i - 1));
		}
		if(49 == str[i]) //1
		{
			*outInt += 1 * pow(10,(length - i - 1));
		}
		if(50 == str[i]) //2
		{
			*outInt += 2 * pow(10,(length - i - 1));
		}
		if(51 == str[i]) //3
		{
			*outInt += 3 * pow(10,(length - i - 1));
		}
		if(52 == str[i]) //4
		{
			*outInt += 4 * pow(10,(length - i - 1));
		}
		if(53 == str[i]) //5
		{
			*outInt += 5 * pow(10,(length - i - 1));
		}
		if(54 == str[i]) //6
		{
			*outInt += 6 * pow(10,(length - i - 1));
		}
		if(55 == str[i]) //7
		{
			*outInt += 7 * pow(10,(length - i - 1));
		}
		if(56 == str[i]) //8
		{
			*outInt += 8 * pow(10,(length - i - 1));
		}
		if(57 == str[i]) //9
		{
			*outInt += 9 * pow(10,(length - i - 1));
		}
	}
	return bFlag;
}

bool stringIsEmpty(char* str)//�ж��ַ����Ƿ�ΪNULL
{
	if(str == NULL)
		return true;
	if(str[0] == '\0')
		return true;
	else
		return false;
}
#ifdef MS_WINDOWS
int moon_ms_windows_ascii_to_utf8(const char* asciiStr,wchar_t* outUTF8)//��asciiת��utf8,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
{
	int convertSize = 0;
	wchar_t* unicodeStr = NULL;
	int widesize = MultiByteToWideChar(CP_ACP,0,asciiStr,-1,NULL,0);
	if (asciiStr == NULL)
	{
		return 0;
	}
	if (widesize == 0)
	{
		return 0;
	}
	unicodeStr = moon_malloc(widesize);
	//��asciiת��unicode
	convertSize = moon_ms_windows_ascii_to_unicode(asciiStr,unicodeStr);
	if (convertSize == -1)
	{
		return convertSize;
	}
	//�ٽ�unicodeת��utf8
	convertSize = moon_ms_windows_unicode_to_utf8(unicodeStr,outUTF8);
	moon_free(unicodeStr);
	return convertSize;
}

int moon_ms_windows_ascii_to_unicode(const char* asciiStr,wchar_t* outUnicode)//��asciiת��unicode,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
{
	int widesize = MultiByteToWideChar(CP_ACP,0,asciiStr,-1,NULL,0);
	int convertlength = 0;
	if (widesize == ERROR_NO_UNICODE_TRANSLATION)
	{
		return -1;
	}
	if (widesize == 0)
	{
		return 0;
	}
	convertlength = MultiByteToWideChar(CP_ACP,0,asciiStr,-1,outUnicode,widesize);
	if (widesize != convertlength)
	{
		memset(outUnicode,0,widesize);
		return -1;
	}
	return convertlength;
}

int moon_ms_windows_unicode_to_utf8(const wchar_t* unicodeStr,wchar_t* outUTF8)//��unicodeת��utf8,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
{
	int utf8size = WideCharToMultiByte(CP_UTF8,0,unicodeStr,-1,NULL,0,NULL,NULL);
	int convertSize = 0;
	if (utf8size == 0)
	{
		return -1;
	}
	convertSize = WideCharToMultiByte(CP_UTF8,0,unicodeStr,-1,outUTF8,utf8size,NULL,NULL);
	if (convertSize != utf8size)
	{
		memset(outUTF8,0,utf8size);
		return -1;
	}
	return convertSize;
}

int moon_ms_windows_unicode_to_ascii(const moon_char* unicodeStr,char* outAscii)//��unicodeת��ascii,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
{
	int convertSize = 0;
	int asciiSize = WideCharToMultiByte(CP_ACP,0,unicodeStr,-1,NULL,0,NULL,NULL);
	if (asciiSize == 0)
	{
		return -1;
	}
	convertSize = WideCharToMultiByte(CP_ACP,0,unicodeStr,-1,outAscii,asciiSize,NULL,NULL);
	if (convertSize != asciiSize)
	{
		memset(outAscii,0,asciiSize);
		return -1;
	}
	return convertSize;
}
int moon_ms_windows_utf8_to_unicode(const moon_char* utf8Str,moon_char* outUnicode)//��utf8ת��unicode,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
{
	int convertSize = 0;
	int unicodeSize = MultiByteToWideChar(CP_UTF8,0,utf8Str,-1,NULL,0);
	if (unicodeSize == 0)
	{
		return -1;
	}
	convertSize = MultiByteToWideChar(CP_UTF8,0,utf8Str,-1,outUnicode,unicodeSize);
	if (convertSize != unicodeSize)
	{
		memset(outUnicode,0,unicodeSize);
		return -1;
	}
	return convertSize;
}

int moon_ms_windows_utf8_to_ascii(const moon_char* utf8Str,char* outAscii)//��utf8ת��unicode,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
{
	int convertSize = 0;
	wchar_t* unicodeStr = NULL;
	int unicodeSize = MultiByteToWideChar(CP_UTF8,0,utf8Str,-1,NULL,0);
	if (utf8Str == NULL)
	{
		return 0;
	}
	if (unicodeSize == 0)
	{
		return 0;
	}
	unicodeStr = moon_malloc(unicodeSize);
	convertSize = moon_ms_windows_utf8_to_unicode(utf8Str,unicodeStr);
	if (convertSize == -1)
	{
		return convertSize;
	}
	convertSize = moon_ms_windows_unicode_to_ascii(unicodeStr,outAscii);
	moon_free(unicodeStr);
	return convertSize;
}

#endif

/************************************************************************/
/* ���ַ���Сдת���ɴ�д�����                                         */
/*   ������srcStr:ԭ�ַ���												*/
/*		   pOutCapital:ת������ַ���									*/
/*	����ֵ������ʵ��ת����ĸ�ĸ���										*/
/************************************************************************/
int moon_to_capital(char* srcStr,char* pOutCapital)
{
	int count = 0;
	int i = 0;
	int length = moon_get_string_length(srcStr);
	for (i = 0;i < length;i++)
	{
		if (srcStr[i] >= 'a' && srcStr[i] <= 'z')
		{
			pOutCapital[i] = srcStr[i] - 32;
			count++;
		}
		else
		{
			pOutCapital[i] = srcStr[i];
		}
	}
	return count;
}

#ifdef __cplusplus
}
#endif