#pragma once

#ifndef PROCESS_H
#define PROCESS_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>
#include <string>

class Process
{
public:
    Process(std::wstring& commandLine);
    Process(std::wstring& commandLine, std::vector<std::pair<std::wstring, std::wstring>>& envBlock);
    ~Process();

    void SetWorkingDir(std::wstring& path);

    void StartProcess();
    void ReadStdOut(std::string& output);
    void StreamStdOut();
    bool Wait(int timeout);

private:
    HANDLE m_hStdOutRead;
    HANDLE m_hStdOutWrite;
    HANDLE m_hStdInRead;
    HANDLE m_hStdInWrite;
    HANDLE m_hProcess;
    HANDLE m_hMainThread;

    LPWCH m_envBlock;
    LPWSTR m_commandLine;
    LPWSTR m_workingDir;
};

#endif

