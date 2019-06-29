#pragma once

#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <NTSecAPI.h>
#include <string>

#ifndef STATUS_SUCCESS
#include <ntstatus.h>
#endif

bool VerifyUserExists(std::wstring& username);
bool VerifyUser(std::wstring& username);
HRESULT ConfigureUser(std::wstring& username);
bool NormalizeUsername(std::wstring& username);


#endif
