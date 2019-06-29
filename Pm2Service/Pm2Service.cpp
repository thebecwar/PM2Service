// Pm2Service.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "ServiceManagement.h"
#include "SystemEventLog.h"
#include "ServiceHandler.h"

HANDLE waitHandle = NULL;
void TestEventHandler(PVOID, BOOLEAN)
{
	UnregisterWait(waitHandle);
	return;
}

int wmain(int argc, wchar_t** argv)
{
	if (argc >= 2)
	{
		if (_wcsicmp(argv[1], L"install") == 0)
		{
			// Created Local User:
			// fusion, 1qazXSW@
			wchar_t szPath[MAX_PATH];
			if(!GetModuleFileNameW(NULL, szPath, MAX_PATH))
			{
				printf("Cannot install service (%d)\n", GetLastError());
				return 1;
			}
			std::wstring path(szPath);
			path = path + L" run";
			
			if (argc < 3) 
			{
				std::wcout << L"Error Message TODO" << std::endl;
				return 1;
			}
			std::wstring username(argv[2]);
			std::wstring password;
			if (argc == 4)
			{
				password = argv[3];
			}
			else
			{
				std::wcout << L"Enter the password for user " << argv[2] << L":  ";
				std::wcin >> password;
				std::wcout << std::endl;
			}

			HRESULT hr = InstallService(path, username, password);
			if (hr == S_OK)
			{
				std::wcout << L"Service Installed Successfully" << std::endl;
				LogSuccess(L"PM2 Service Installed Successfully");
			}
			else
			{
				std::wcout << L"Service Installation Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
				LogError(L"Service Installation Error", hr);
			}
		}
		else if (_wcsicmp(argv[1], L"uninstall") == 0)
		{
			HRESULT hr = UninstallService();
			if (hr == S_OK)
			{
				std::wcout << L"Service Uninstalled Successfully" << std::endl;
				LogSuccess(L"PM2 Service Uninstalled Successfully");
			}
			else
			{
				std::wcout << L"Service Uninstalled Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
				LogError(L"Service Uninstallation Error", hr);
			}
		}
		else if (_wcsicmp(argv[1], L"run") == 0)
		{
			LogInfo(L"Starting PM2 Service");
			HRESULT hr = RunService();
			if (hr != S_OK)
			{
				std::wcout << L"Service Start Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
				LogError(L"Service Start Error", hr);
			}
		}
        else if (_wcsicmp(argv[1], L"start") == 0)
        {
            HRESULT hr = Start();
			if (hr != S_OK)
			{
				std::wcout << L"Service Start Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
			}
            else
            {
                std::wcout << L"Service Started Successfully" << std::endl;
            }
        }
        else if (_wcsicmp(argv[1], L"stop") == 0)
        {
            HRESULT hr = Stop();
			if (hr != S_OK)
			{
				std::wcout << L"Service Stop Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
			}
            else
            {
                std::wcout << L"Service Stopped Successfully" << std::endl;
            }
        }
        else if (_wcsicmp(argv[1], L"restart") == 0)
        {
            HRESULT hr = Stop();
            if (hr != ERROR_SERVICE_NOT_ACTIVE)
            {
				std::wcout << L"Service Stop Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
                return;
            }

            hr = Start();
			if (hr != S_OK)
			{
				std::wcout << L"Service Restart Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
			}
            else
            {
                std::wcout << L"Service Restarted Successfully" << std::endl;
            }
        }
        else if (_wcsicmp(argv[1], L"config") == 0)
        {

        }
	}
	return 0;
}
