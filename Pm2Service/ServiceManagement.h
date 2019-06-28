#pragma once

#ifndef SERVICEMANAGEMENT_H
#define SERVICEMANAGEMENT_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>


#define SERVICE_NAME L"PM2Service"
#define SERVICE_DISPLAY_NAME L"PM2 Service Runtime"

HRESULT InstallService(std::wstring& path, std::wstring& username, std::wstring& password);
HRESULT UninstallService();





#endif

