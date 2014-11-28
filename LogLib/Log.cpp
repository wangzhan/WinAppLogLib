#include "stdafx.h"
#include "Log.h"
#include <map>
#include <vector>
#include <process.h>
#include <ShlObj.h>
#include <assert.h>
#include <Winreg.h>
#include "AutoLock.h"
#include "CharacterCode.h"


static wchar_t *g_lpszLogID = L"example";		// need to update
static int g_iLogLimitSize = 4 * 1024 * 1024; // 4M
static unsigned int WM_MSG_LOG_WRITE = WM_USER + 185;


class CLog
{
public:
	typedef std::map<std::wstring, HANDLE> LogFilesMap_t;
	typedef LogFilesMap_t::iterator LogFilesMapIter_t;

	
	CLog() : m_iThreadID(0), m_hThread(NULL) {}
	~CLog()
	{
		if (m_hThread != NULL)
		{
			::PostQuitMessage(0);
			WaitForSingleObject(m_hThread, 5 * 1000);
			CloseHandle(m_hThread);

			for (LogFilesMapIter_t iter = m_mapLogFiles.begin(); iter != m_mapLogFiles.end(); ++iter)
				CloseHandle(iter->second);

			m_mapLogFiles.clear();
		}
	}

	void Init()
	{
		AutoLock autoLock(m_lock);

		if (m_iThreadID == 0)
		{
			HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			std::pair<CLog*, HANDLE> threadParam = std::make_pair(this, hEvent);
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, &threadParam, 0, &m_iThreadID);
			if (hThread)
				WaitForSingleObject(hEvent, INFINITE);

			CloseHandle(hEvent);
		}
	}

	void Write(LPCWSTR lpszLogPath, LPCWSTR lpszLogs)
	{
		if (m_iThreadID != 0)
			PostThreadMessage(m_iThreadID, WM_MSG_LOG_WRITE, (WPARAM)lpszLogPath, (LPARAM)lpszLogs);
	}

	static unsigned int __stdcall ThreadFunc(void *pParam)
	{
		std::pair<CLog*, HANDLE> *pThreadParam = (std::pair<CLog*, HANDLE>*)pParam;
		if (!pThreadParam)
			return 1;

		CLog *pLog = pThreadParam->first;
		MSG msg;
		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		SetEvent(pThreadParam->second);

		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (msg.message == WM_MSG_LOG_WRITE)
			{
				LPCWSTR lpszFilePath = (LPCWSTR)msg.wParam;
				LPCWSTR lpszLog = (LPCWSTR)msg.lParam;
				pLog->WriteLog(lpszFilePath, lpszLog);
				free((void*)lpszFilePath);
				free((void*)lpszLog);
			}
		}

		return 0;
	}

private:
	HANDLE OpenLog(LPCWSTR lpszLogPath)
	{
		LogFilesMapIter_t iter = m_mapLogFiles.find(lpszLogPath);
		if (iter != m_mapLogFiles.end())
			return iter->second;

		// 打开日志文件
		HANDLE hLogFile = INVALID_HANDLE_VALUE;
		static std::wstring strDir;
		if (strDir.empty())
		{
			wchar_t szDir[MAX_PATH] = { 0 };
			if (FAILED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szDir)))
				return INVALID_HANDLE_VALUE;

			strDir = szDir;
			strDir += L"\\";
			strDir += g_lpszLogID;
			strDir += L"\\";

			CreateDirectory(strDir.c_str(), NULL);
		}

		std::wstring strFilePath = strDir + lpszLogPath;
		hLogFile = CreateFile(strFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
		if (hLogFile == INVALID_HANDLE_VALUE)
			return INVALID_HANDLE_VALUE;

		// 文件过大时，把原日志文件备份下，重新创建文件
		LARGE_INTEGER logSize;
		if (!GetFileSizeEx(hLogFile, &logSize))
			return INVALID_HANDLE_VALUE;

		if (logSize.LowPart > (unsigned int)g_iLogLimitSize)
		{
			int iSubLen = strFilePath.rfind(L'.');
			std::wstring strBackupFilePath =
				strFilePath.substr(0, iSubLen == std::wstring::npos ? strFilePath.length() : iSubLen) + L"_backup.txt";
			DeleteFile(strBackupFilePath.c_str());

			CloseHandle(hLogFile);
			MoveFile(strFilePath.c_str(), strBackupFilePath.c_str());

			hLogFile = CreateFile(strFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
			if (hLogFile == INVALID_HANDLE_VALUE)
				return INVALID_HANDLE_VALUE;
		}

		SetFilePointer(hLogFile, 0, NULL, FILE_END);

		m_mapLogFiles.insert(std::make_pair(lpszLogPath, hLogFile));
		return hLogFile;
	}

	void WriteLog(LPCWSTR lpszLogPath, LPCWSTR lpszLogs)
	{
		HANDLE hLogFile = OpenLog(lpszLogPath);
		if (hLogFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwWrite = 0;
			std::string strContent = utility::ConvertUnicodeToUTF8(lpszLogs);
			WriteFile(hLogFile, strContent.c_str(), strContent.length(), &dwWrite, NULL);
		}
	}

private:
	LockGuard m_lock;

	unsigned int m_iThreadID;
	HANDLE m_hThread;

	// <filepath, filehandle>
	LogFilesMap_t m_mapLogFiles;
};

CLog g_Log;

static void LogImplW(LOG_TYPE type, LOG_LEVEL level, LPCWSTR lpszFunction, LPCWSTR lpszFile, int iLine, LPCWSTR lpszFormat, va_list args)
{
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	
	assert(level < LOG_LEVEL_COUNT);
	wchar_t lpszHead[MAX_PATH] = { 0 };
	swprintf_s(lpszHead, MAX_PATH, L"%d-%02d-%02d %02d:%02d:%02d'%d\" %d\t[%s] ",
		localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute,
		localTime.wSecond, localTime.wMilliseconds, GetCurrentThreadId(), g_LogLevelData[level].lpszValue);

	wchar_t lpszTail[MAX_PATH] = { 0 };
	swprintf_s(lpszTail, MAX_PATH, L" Function: %s; File: %s; Line: %d.\n", lpszFunction, lpszFile, iLine);
	
	int iLen = _vscwprintf(lpszFormat, args);
	std::vector<wchar_t> vecContent(iLen + 1, 0);
	_vsnwprintf_s(&vecContent[0], iLen + 1, _TRUNCATE, lpszFormat, args);

	std::wstring strContent = lpszHead;
	strContent += std::wstring(&vecContent[0], iLen);
	strContent += lpszTail;

	assert(type < LOG_TYPE_COUNT);
	g_Log.Init();
	g_Log.Write(_wcsdup(g_LogTypeData[type].lpszLogName), _wcsdup(strContent.c_str()));
}


// <type, level>
typedef std::map<LOG_TYPE, LOG_LEVEL> LogSwitchMap_t;
typedef LogSwitchMap_t::iterator LogSwitchMapIter_t;
static LogSwitchMap_t g_mapLogSwitch;

static bool DoesNeedToWrite(LOG_TYPE type, LOG_LEVEL level)
{
	LogSwitchMapIter_t iter = g_mapLogSwitch.find(type);
	if (iter != g_mapLogSwitch.end())
		return iter->second <= level;

	// 获取相应类型的开关
	bool bSwitch = false;
	HKEY regKey = 0;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_LogTypeData[type].lpszRegSubKey, 0, KEY_QUERY_VALUE, &regKey) == ERROR_SUCCESS)
	{
		DWORD dwType = 0;
		int iValue = 0;
		DWORD dwLen = sizeof(iValue);
		if (RegQueryValueEx(regKey, g_LogTypeData[type].lpszRegKey, NULL, &dwType, (LPBYTE)&iValue, &dwLen) == ERROR_SUCCESS)
		{
			bSwitch = (iValue <= level);
			g_mapLogSwitch.insert(std::make_pair(type, (LOG_LEVEL)iValue));
		}

		RegCloseKey(regKey);
	}

	return bSwitch;
}

void nslog::LogW(LOG_TYPE type, LOG_LEVEL level, LPCWSTR lpszFunction, LPCWSTR lpszFile, int iLine, LPCWSTR lpszFormat, ...)
{
	if (DoesNeedToWrite(type, level))
	{
		va_list args;
		va_start(args, lpszFormat);
		LogImplW(type, level, lpszFunction, lpszFile, iLine, lpszFormat, args);
		va_end(args);
	}
}
