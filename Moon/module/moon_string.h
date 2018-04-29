/************************************************************************/
/* ��ģ��Ϊ�ַ�������������											*/
/************************************************************************/
#pragma once
#ifndef _MOON_STRING_H
#define _MOON_STRING_H
#include "../cfg/environment.h"
#ifdef MS_WINDOWS
#include <wchar.h>
#endif
#include "moon_char.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long moon_get_string_length(const char* str);/*get string length,with out '\0'*/

void moon_trim(char *strIn, char *strOut);//ȥ���ַ�����β�ո�

void moon_trim_line(char *strIn, char *strOut);//ȥ����β����

bool moon_string_parse_to_int(const char* str,unsigned int* outInt);/*���ַ���ת��unsigned int,���ʧ�ܷ���false���ɹ�����true*/

bool stringIsEmpty(char* str);//�ж��ַ����Ƿ�ΪNULL

/************************************************************************/
/* ���ַ���Сдת���ɴ�д�����                                         */
/*   ������srcStr:ԭ�ַ���												*/
/*		   pOutCapital:ת������ַ���									*/
/*	����ֵ������ʵ��ת����ĸ�ĸ���										*/
/************************************************************************/
int moon_to_capital(char* srcStr,char* pOutCapital);

#ifdef MS_WINDOWS
int moon_ms_windows_ascii_to_utf8(const char* asciiStr,wchar_t* outUTF8);//��asciiת��utf8,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
int moon_ms_windows_ascii_to_unicode(const char* asciiStr,wchar_t* outUnicode);//��asciiת��unicode,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
int moon_ms_windows_unicode_to_utf8(const wchar_t* unicodeStr,wchar_t* outUTF8);//��unicodeת��utf8,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
int moon_ms_windows_unicode_to_ascii(const moon_char* unicodeStr,char* outAscii);//��unicodeת��ascii,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
int moon_ms_windows_utf8_to_unicode(const moon_char* utf8Str,moon_char* outUnicode);//��utf8ת��unicode,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
int moon_ms_windows_utf8_to_ascii(const moon_char* utf8Str,char* outAscii);//��utf8ת��unicode,�ɹ�����ʵ��ת�����ֽ�����ʧ�ܷ���-1
#endif

#ifdef __cplusplus
}
#endif

#endif