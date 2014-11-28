#include "stdafx.h"
#include <windows.h>
#include <vector>
#include "CharacterCode.h"

namespace utility
{
	std::string ConvertUnicodeToUTF8(LPCWSTR lpszString)
	{
		std::string strString;
		int iLen = WideCharToMultiByte(CP_UTF8, 0, lpszString, -1, 0, 0, 0, 0);
		if (iLen > 0)
		{
			++iLen;
			std::vector<char> vecString(iLen, 0);
			WideCharToMultiByte(CP_UTF8, 0, lpszString, -1, &vecString[0], iLen, 0, 0);
			strString = &vecString[0];
		}
		return strString;
	}

	std::string ConvertUnicodeToAnsi(LPCWSTR lpszString)
	{
		std::string strString;
		int iLen = WideCharToMultiByte(CP_ACP, 0, lpszString, -1, 0, 0, 0, 0);
		if (iLen > 0)
		{
			++iLen;
			std::vector<char> vecString(iLen, 0);
			WideCharToMultiByte(CP_ACP, 0, lpszString, -1, &vecString[0], iLen, 0, 0);
			strString = &vecString[0];
		}
		return strString;
	}

	std::wstring ConvertUTF8ToUnicode(LPCSTR lpszString)
	{
		std::wstring strString;
		int iLen = MultiByteToWideChar(CP_UTF8, 0, lpszString, -1, 0, 0);
		if (iLen > 0)
		{
			++iLen;
			std::vector<wchar_t> vecString(iLen, 0);
			MultiByteToWideChar(CP_UTF8, 0, lpszString, -1, &vecString[0], iLen);
			strString = &vecString[0];
		}
		return strString;
	}

	std::wstring ConvertAnsiToUnicode(LPCSTR lpszString)
	{
		std::wstring strString;
		int iLen = MultiByteToWideChar(CP_ACP, 0, lpszString, -1, 0, 0);
		if (iLen > 0)
		{
			++iLen;
			std::vector<wchar_t> vecString(iLen, 0);
			MultiByteToWideChar(CP_ACP, 0, lpszString, -1, &vecString[0], iLen);
			strString = &vecString[0];
		}
		return strString;
	}

	template<class char_t>
	std::basic_string<char_t> TranslateSqlString(const std::basic_string<char_t> &strSqlString,
		char_t ch, const std::basic_string<char_t> newValue)
	{
		std::basic_string<char_t> strTranslated;
		int iStart = 0;
		do 
		{
			int iPos = strSqlString.find_first_of(ch, iStart);
			if (iPos == std::basic_string<char_t>::npos)
			{
				strTranslated += strSqlString.substr(iStart);
				break;
			}

			strTranslated += strSqlString.substr(iStart, iPos - iStart + 1);
			strTranslated += newValue;
			if (iPos == strSqlString.length() - 1)
			{
				break;
			}
			iStart = iPos + 1;
		} while (true);

		return strTranslated;
	}

	std::wstring TranslateSqlStringW(const std::wstring &strSqlString)
	{
		return TranslateSqlString(strSqlString, L'\'', std::wstring(L"''"));
	}

	std::string TranslateSqlStringA(const std::string &strSqlString)
	{
		return TranslateSqlString(strSqlString, '\'', std::string("''"));
	}

} // end namespace utility
