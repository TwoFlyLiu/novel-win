// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>

int main()
{
	const char name[] = "中国";
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

	// 先读低字节，在读高字节
	// fe ff, 低字节在前，小端
	short num = 0xfffe;
	char *pnum = (char*)&num;
	for (size_t i = 0; i < 2; i++)
	{
		printf("%02x ", (unsigned char)*pnum);
		++pnum;
	}
	printf("\n");

	const wchar_t *welcome = L"Hello, 中国";
	const char *u16 = (const char*)welcome;

	for (size_t i = 0; i < wcslen(welcome) * 2; i++) {
		printf("\\x%02x ", (unsigned char)u16[i]);
	}
	printf("\n");

	getchar();
    return 0;
}

