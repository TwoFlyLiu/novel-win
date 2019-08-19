// test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>

int main()
{
	const char name[] = "�й�";
	char buf[100];

	printf("%s\n", name);
	printf("len(name): %d\n", strlen(name));

	for (size_t i = 0; i < sizeof(name) / sizeof(char); i++)
	{
		unsigned char ch = (unsigned char)name[i];
		itoa(ch, buf, 2);
		printf("%s ", buf);
	}
	printf("\n");

	for (size_t i = 0; i < sizeof(name) / sizeof(char); i++)
	{
		printf("%02x ", (unsigned char)name[i]);
	}
	printf("\n");

	// �ȶ����ֽڣ��ڶ����ֽ�
	// fe ff, ���ֽ���ǰ��С��
	short num = 0xfffe;
	char *pnum = (char*)&num;
	for (size_t i = 0; i < 2; i++)
	{
		printf("%02x ", (unsigned char)*pnum);
		++pnum;
	}
	printf("\n");

	const wchar_t *welcome = L"Hello, �й�";
	const char *u16 = (const char*)welcome;

	for (size_t i = 0; i < wcslen(welcome) * 2; i++) {
		printf("\\x%02x ", (unsigned char)u16[i]);
	}
	printf("\n");

	getchar();
    return 0;
}

