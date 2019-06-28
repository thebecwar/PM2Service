// Pm2Service.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "ServiceManagement.h"
#include "SystemEventLog.h"

int wmain(int argc, wchar_t** argv)
{
	if (argc >= 2)
	{
		if (_wcsicmp(argv[1], L"install") == 0)
		{
			// Created Local User:
			// fusion, 1qazXSW@
			std::wstring path(argv[0]);
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
		else if (_wcsicmp(argv[1], L"configure") == 0)
		{
		}
		else if (_wcsicmp(argv[1], L"run") == 0)
		{
			LogInfo(L"Starting PM2 Service");
		}
	}
	return 0;
}
