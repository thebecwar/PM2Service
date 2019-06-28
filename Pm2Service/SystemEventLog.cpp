#include "SystemEventLog.h"
#include "ServiceManagement.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


HANDLE hEventLog = NULL;

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
	Log(message, EVENTLOG_ERROR_TYPE, code);
}
void LogInfo(const std::wstring& message)
{
	Log(message, EVENTLOG_INFORMATION_TYPE);
}
void LogWarning(const std::wstring& message)
{
	Log(message, EVENTLOG_WARNING_TYPE);
}
void LogSuccess(const std::wstring& message)
{
	Log(message, EVENTLOG_SUCCESS);
}
