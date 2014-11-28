// LogTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../LogLib/Log.h"

int _tmain(int argc, _TCHAR* argv[])
{
	LOGW(LOG_TYPE_EXAMPLE, LOG_LEVEL_DEBUG, L"%s", L"nihao");
	LOGW(LOG_TYPE_EXAMPLE, LOG_LEVEL_INFO, L"%s %d", L"ÌìÏÂ", 78);

	getchar();
	return 0;
}

