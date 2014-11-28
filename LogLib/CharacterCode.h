// Author: WangZhan -> wangzhan.1985@gmail.com
#pragma once
#include <iostream>


namespace utility
{
	std::string ConvertUnicodeToUTF8(LPCWSTR lpszString);
	std::wstring ConvertUTF8ToUnicode(LPCSTR lpszString);
	std::string ConvertUnicodeToAnsi(LPCWSTR lpszString);
	std::wstring ConvertAnsiToUnicode(LPCSTR lpszString);

	std::wstring TranslateSqlStringW(const std::wstring &strSqlString);
	std::string TranslateSqlStringA(const std::string &strSqlString);
}

