#pragma once

#ifndef SERVICEMANAGEMENT_H
#define SERVICEMANAGEMENT_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>


#define SERVICE_NAME L"PM2Service"
#define SERVICE_DISPLAY_NAME L"PM2 Service Runtime"

void SetStatusHandle(SERVICE_STATUS_HANDLE hStatus);
HANDLE GetStatusHandle();

HRESULT InstallService(std::wstring& path, std::wstring& username, std::wstring& password);
HRESULT UninstallService();
bool SetContinuePending(DWORD checkpoint, DWORD waitHint);
bool SetPausePending(DWORD checkpoint, DWORD waitHint);
bool SetPaused();
bool SetRunning();
bool SetStartPending(DWORD checkpoint, DWORD waitHint);
bool SetStopPending(DWORD checkpoint, DWORD waitHint);
bool SetStopped(DWORD exitCode);


HRESULT Start();
HRESULT Stop();

#endif

