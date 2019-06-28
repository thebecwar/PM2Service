#pragma once

#ifndef SYSTEMEVENTLOG_H
#define SYSTEMEVENTLOG_H

#include <string>

void LogError(const std::wstring& message, int code);
void LogInfo(const std::wstring& message);
void LogWarning(const std::wstring& message);
void LogSuccess(const std::wstring& message);

#endif
