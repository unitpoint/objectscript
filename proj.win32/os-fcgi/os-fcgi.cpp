// os-insight.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	printf("This project is moved to https://github.com/unitpoint/os-fcgi \npress enter to exit\n");
	char buf[10];
	fgets(buf, 9, stdin);
	return 0;
}

