// Author: WangZhan -> wangzhan.1985@gmail.com
#pragma once
#include <windows.h>
#include <iostream>

// 1. 日志格式 LOGW(UILOG, DEBUG, L"open successfully, errcode: %d", 0)
// 2. 日志开关 HKCU/logid/logtype
// 3. 日志文件存储位置 %appdata%/logid/

// 日志类型
enum LOG_TYPE
{
	LOG_TYPE_EXAMPLE,	// 例子

	LOG_TYPE_COUNT
};

struct LogTypeData
{
	LOG_TYPE type;
	LPCWSTR lpszLogName;
	LPCWSTR lpszRegSubKey;
	LPCWSTR lpszRegKey;
};

__declspec(selectany) LogTypeData g_LogTypeData[LOG_TYPE_COUNT] =
{
	{ LOG_TYPE_EXAMPLE, L"example.txt", L"SOFTWARE\\example", L"example" }
};


// 日志错误级别
enum LOG_LEVEL
{
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_FATAL,

	LOG_LEVEL_COUNT
};

struct LogLevelData
{
	LOG_LEVEL level;
	LPCWSTR lpszValue;
};

__declspec(selectany) LogLevelData g_LogLevelData[LOG_LEVEL_COUNT] =
{
	{ LOG_LEVEL_DEBUG, L"DEBUG" },
	{ LOG_LEVEL_INFO, L"INFO" },
	{ LOG_LEVEL_WARN, L"WARN" },
	{ LOG_LEVEL_FATAL, L"FATAL" }
};

namespace nslog
{
	void LogW(LOG_TYPE type, LOG_LEVEL level, LPCWSTR lpszFunction, LPCWSTR lpszFile, int iLine, LPCWSTR lpszFormat, ...);
}


// 调用宏接口
#define LOG_IMPL_W(type, level, function, file, line, format, ...)	nslog::LogW(type, level, function, file, line, format, __VA_ARGS__)
#define LOGW(type, level, format, ...)	LOG_IMPL_W(type, level, __FUNCTIONW__, __FILEW__, __LINE__, format, __VA_ARGS__)

