#include "SystemEventLog.h"
#include "ServiceManagement.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <sstream>
#include <iomanip>

// Generated Code Header. Doesn't exist until Pre-Build Step
#include "Errors.h"


HANDLE hEventLog = NULL;
bool OpenLog()
{
	if (!hEventLog)
	{
		hEventLog = RegisterEventSourceW(NULL, SERVICE_NAME);
	}
	return hEventLog != NULL;
}
void Log(const std::wstring& message, WORD type, int code = 0)
{
	if (!hEventLog)
	{
		hEventLog = RegisterEventSourceW(NULL, SERVICE_NAME);
		if (!hEventLog)
		{
			return;
		}
	}

	const wchar_t* messages[1] = {
		message.c_str(),
	};
	ReportEventW(hEventLog, type, 0, code, NULL, 1, 0, messages, NULL);
}

void LogError(const std::wstring& message, int code) 
{
	if (!OpenLog()) return;
	std::wstringstream ss;
	ss << L"0x" << std::uppercase << std::setw(8) << std::setfill(L'0') << std::hex << code;
	std::wstring errCode = ss.str();
	
	const wchar_t* messages[] = {
		errCode.c_str(),
		message.c_str(),
	};
	ReportEventW(hEventLog, EVENTLOG_ERROR_TYPE, 0, SVC_ERROR, NULL, 2, 0, messages, NULL);
}
void LogInfo(const std::wstring& message)
{
	if (!OpenLog()) return;
	const wchar_t* messages[] = {
		message.c_str()
	};
	ReportEventW(hEventLog, EVENTLOG_INFORMATION_TYPE, 0, SVC_INFO, NULL, 1, 0, messages, NULL);
}
void LogWarning(const std::wstring& message)
{
	if (!OpenLog()) return;
	const wchar_t* messages[] = {
		message.c_str()
	};
	ReportEventW(hEventLog, EVENTLOG_WARNING_TYPE, 0, SVC_WARN, NULL, 1, 0, messages, NULL);
}
void LogSuccess(const std::wstring& message)
{
	if (!OpenLog()) return;
	const wchar_t* messages[] = {
		message.c_str()
	};
	ReportEventW(hEventLog, EVENTLOG_SUCCESS, 0, SVC_SUCCESS, NULL, 1, 0, messages, NULL);
}
