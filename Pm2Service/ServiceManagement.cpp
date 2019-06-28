#include "ServiceManagement.h"

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

	return hr;
}
