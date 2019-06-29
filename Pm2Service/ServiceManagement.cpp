#include "ServiceManagement.h"

SERVICE_STATUS_HANDLE hStatusHandle = NULL;
void SetStatusHandle(SERVICE_STATUS_HANDLE hStatus)
{
    hStatusHandle = hStatus;
}
HANDLE GetStatusHandle()
{
    return hStatusHandle;
}

HRESULT InstallService(std::wstring& path, std::wstring& username, std::wstring& password)
{
	HRESULT hr = S_OK;

	SC_HANDLE hSc = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSc) 
	{
		SC_HANDLE hSvc = CreateServiceW(
			hSc,
			SERVICE_NAME,
			SERVICE_DISPLAY_NAME,
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			path.c_str(),
			NULL,
			NULL,
			NULL,
			username.c_str(),
			password.c_str());
		if (hSvc)
		{
			CloseServiceHandle(hSvc);
		}
		else 
		{
			hr = GetLastError();
		}
		CloseServiceHandle(hSc);
	}
	else
	{
		hr = GetLastError();
	}

	// Install the eventlog error resource dll
	HMODULE hErrorLib = LoadLibraryW(L"PM2ServiceErrors.dll");
	if (hErrorLib != INVALID_HANDLE_VALUE)
	{
		wchar_t libPath[MAX_PATH];
		ZeroMemory(libPath, MAX_PATH * sizeof(wchar_t));
		DWORD len = GetModuleFileNameW(hErrorLib, libPath, MAX_PATH);

		HKEY key;
		LSTATUS stat = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application", 0, KEY_WRITE, &key);
		if (stat == ERROR_SUCCESS)
		{
			HKEY subkey;
			stat = RegCreateKeyExW(key, L"PM2Service", 0, NULL, 0, KEY_WRITE, NULL, &subkey, NULL);
			if (stat == ERROR_SUCCESS)
			{
				stat = RegSetKeyValueW(subkey, NULL, L"EventMessageFile", REG_SZ, libPath, len * sizeof(wchar_t));
				
				DWORD types = 0xFFFFFFFF;
				stat = RegSetKeyValueW(subkey, NULL, L"TypesSupported", REG_DWORD, &types, sizeof(DWORD));

				RegCloseKey(subkey);
			}
			RegCloseKey(key);
		}
	}
	else
	{
		hr = GetLastError();
	}

	return hr;
}
HRESULT UninstallService()
{
	HRESULT hr = S_OK;

	SC_HANDLE hSc = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSc) 
	{
		SC_HANDLE hSvc = OpenServiceW(hSc, SERVICE_NAME, SERVICE_ALL_ACCESS);
		if (hSvc)
		{
			BOOL ok = DeleteService(hSvc);
			if (!ok) 
			{
				hr = GetLastError();
			}

			CloseServiceHandle(hSvc);
		}
		else 
		{
			hr = GetLastError();
		}
		CloseServiceHandle(hSc);
	}
	else
	{
		hr = GetLastError();
	}

	HKEY key;
	LSTATUS stat = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application", 0, KEY_WRITE, &key);
	if (stat == ERROR_SUCCESS)
	{
		stat = RegDeleteKeyW(key, L"PM2Service");
		if (stat != ERROR_SUCCESS)
		{
			hr = stat;
		}
	}
	return hr;
}
bool SetState(DWORD state, DWORD checkpoint, DWORD exitCode, DWORD waitHint)
{
	if (!hStatusHandle)
	{
		return false;
	}

	SERVICE_STATUS stat = {};
	stat.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	stat.dwCurrentState = state;
	stat.dwControlsAccepted = SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	stat.dwWin32ExitCode = exitCode;
	stat.dwCheckPoint = checkpoint;
	stat.dwWaitHint = waitHint;
    stat.dwServiceSpecificExitCode = 0;

	BOOL ok = SetServiceStatus(hStatusHandle, &stat);
	if (!ok) 
	{
		return false;
	}

	return true;
}
bool SetContinuePending(DWORD checkpoint, DWORD waitHint)
{
	return SetState(SERVICE_CONTINUE_PENDING, checkpoint, 0, waitHint);
}
bool SetPausePending(DWORD checkpoint, DWORD waitHint)
{
	return SetState(SERVICE_PAUSE_PENDING, checkpoint, 0, waitHint);
}
bool SetPaused()
{
	return SetState(SERVICE_PAUSED, 0, 0, 0);
}
bool SetRunning()
{
	return SetState(SERVICE_RUNNING, 0, 0, 0);
}
bool SetStartPending(DWORD checkpoint, DWORD waitHint)
{
	return SetState(SERVICE_START_PENDING, checkpoint, 0, waitHint);
}
bool SetStopPending(DWORD checkpoint, DWORD waitHint)
{
	return SetState(SERVICE_STOP_PENDING, checkpoint, 0, waitHint);
}
bool SetStopped(DWORD exitCode)
{
	return SetState(SERVICE_STOPPED, 0, exitCode, 0);
}

#include <iostream>
HRESULT Start()
{
    HRESULT hr = S_OK;
	SC_HANDLE hSc = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    std::wcout << L"SC Manager: 0x" << std::hex << std::uppercase << hSc << std::endl;
    if (hSc)
    {
        SC_HANDLE hService = OpenServiceW(hSc, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
        std::wcout << L"Service: 0x" << std::hex << std::uppercase << hService << std::endl;
        if (hService) 
        {
            BOOL ok = StartServiceW(hService, 0, NULL);
            if (!ok)
            {
                hr = GetLastError();
            }
        }
        else
        {
            hr = GetLastError();
        }
    }
    else
    {
        hr = GetLastError();
    }
    return hr;
}
HRESULT Stop()
{
    HRESULT hr = S_OK;
	SC_HANDLE hSc = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSc)
    {
        SC_HANDLE hService = OpenServiceW(hSc, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
        if (hService) 
        {
            SERVICE_STATUS stat = { 0 };
            BOOL ok = ControlService(hService, SERVICE_CONTROL_STOP, &stat);
            if (!ok)
            {
                hr = GetLastError();
            }
        }
        else
        {
            hr = GetLastError();
        }
    }
    else
    {
        hr = GetLastError();
    }
    return hr;
}

