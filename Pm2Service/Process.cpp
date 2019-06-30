#include "Process.h"

#include <sstream>

Process::Process(std::wstring& commandLine) : 
    m_commandLine(NULL), m_envBlock(NULL), m_hProcess(NULL), 
    m_hMainThread(NULL), m_hStdOutRead(NULL), m_hStdOutWrite(NULL),
    m_hStdInRead(NULL), m_hStdInWrite(NULL), m_workingDir(NULL)
{
    this->m_commandLine = (LPWSTR)calloc(commandLine.size() + 1, sizeof(wchar_t));
    memcpy_s(this->m_commandLine, commandLine.size() * sizeof(wchar_t), commandLine.c_str(), commandLine.size() * sizeof(wchar_t));

    this->m_envBlock = GetEnvironmentStringsW();
}
Process::Process(std::wstring& commandLine, std::vector<std::pair<std::wstring, std::wstring>>& envBlock) : 
    m_commandLine(NULL), m_envBlock(NULL), m_hProcess(NULL), 
    m_hMainThread(NULL), m_hStdOutRead(NULL), m_hStdOutWrite(NULL),
    m_hStdInRead(NULL), m_hStdInWrite(NULL), m_workingDir(NULL)
{
    this->m_commandLine = (LPWSTR)calloc(commandLine.size() + 1, sizeof(wchar_t));
    memcpy_s(this->m_commandLine, commandLine.size() * sizeof(wchar_t), commandLine.c_str(), commandLine.size() * sizeof(wchar_t));

    LPWCH tempBlock = GetEnvironmentStringsW();
    BOOL ok;
    for (auto iter = envBlock.begin(); iter != envBlock.end(); iter++)
    {
        ok = SetEnvironmentVariableW(iter->first.c_str(), iter->second.c_str());
    }
    this->m_envBlock = GetEnvironmentStringsW();
    SetEnvironmentStringsW(tempBlock);
    FreeEnvironmentStringsW(tempBlock);
}
Process::~Process()
{
    if (this->m_envBlock)
    {
        FreeEnvironmentStringsW(this->m_envBlock);
        this->m_envBlock = NULL;
    }
    if (this->m_commandLine)
    {
        free(this->m_commandLine);
        this->m_commandLine = NULL;
    }
    if (this->m_hProcess)
    {
        CloseHandle(this->m_hProcess);
        this->m_hProcess = NULL;
    }
    if (this->m_hMainThread)
    {
        CloseHandle(this->m_hMainThread);
        this->m_hMainThread = NULL;
    }
    if (this->m_hStdOutRead)
    {
        CloseHandle(this->m_hStdOutRead);
        this->m_hStdOutRead = NULL;
    }
    if (this->m_hStdOutWrite)
    {
        CloseHandle(this->m_hStdOutWrite);
        this->m_hStdOutWrite = NULL;
    }
    if (this->m_hStdInRead)
    {
        CloseHandle(this->m_hStdInRead);
        this->m_hStdInRead = NULL;
    }
    if (this->m_hStdInWrite)
    {
        CloseHandle(this->m_hStdInWrite);
        this->m_hStdInWrite = NULL;
    }
    if (this->m_workingDir)
    {
        free(this->m_workingDir);
        this->m_workingDir = NULL;
    }
}
void Process::SetWorkingDir(std::wstring& path)
{
    if (this->m_workingDir)
    {
        free(this->m_workingDir);
        this->m_workingDir = NULL;
    }
    if (path.size() > 0)
    {
        this->m_workingDir = (LPWSTR)calloc(path.size() + 1, sizeof(wchar_t));
        memcpy_s(this->m_workingDir, path.size() * sizeof(wchar_t), path.c_str(), path.size() * sizeof(wchar_t));
    }
}
void Process::StartProcess()
{
    SECURITY_ATTRIBUTES sAttr = { 0 };
    sAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sAttr.bInheritHandle = TRUE;
    sAttr.lpSecurityDescriptor = NULL;

    BOOL ok = CreatePipe(&this->m_hStdOutRead, &this->m_hStdOutWrite, &sAttr, 0);
    ok = SetHandleInformation(this->m_hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    ok = CreatePipe(&this->m_hStdInRead, &this->m_hStdInWrite, &sAttr, 0);
    ok = SetHandleInformation(this->m_hStdInWrite, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION pi = { 0 };
    STARTUPINFOW si = { 0 };

    si.cb = sizeof(STARTUPINFOW);
    si.hStdError = this->m_hStdOutWrite;
    si.hStdOutput = this->m_hStdOutWrite;
    si.hStdInput = this->m_hStdInRead;
    si.dwFlags = STARTF_USESTDHANDLES;

    ok = CreateProcessW(
        NULL,
        this->m_commandLine,
        NULL,
        NULL,
        TRUE,
        CREATE_UNICODE_ENVIRONMENT,
        this->m_envBlock,
        this->m_workingDir,
        &si,
        &pi);

    // Close our copies of the handles we don't need.
    if (this->m_hStdInWrite)
    {
        CloseHandle(this->m_hStdInWrite);
        this->m_hStdInWrite = NULL;
    }
    if (this->m_hStdOutWrite)
    {
        CloseHandle(this->m_hStdOutWrite);
        this->m_hStdOutWrite = NULL;
    }

    if (!ok)
    {
        CloseHandle(this->m_hStdOutRead);
        this->m_hStdOutRead = NULL;

        CloseHandle(this->m_hStdInRead);
        this->m_hStdInRead = NULL;

        return;
    }

    this->m_hProcess = pi.hProcess;
    this->m_hMainThread = pi.hThread;

}
void Process::ReadStdOut(std::string& output)
{
    BOOL readOk = TRUE;
    DWORD read;
    char buf[1024];
    std::stringstream ss;

    for (;;)
    {
        ZeroMemory(buf, 1024);
        readOk = ReadFile(this->m_hStdOutRead, buf, 1023, &read, NULL);
        if (!readOk || read == 0)
            break;

        ss << buf;
    }

    output = ss.str();
}
void Process::StreamStdOut()
{
    BOOL readOk = TRUE;
    DWORD read;
    BOOL writeOk = TRUE;
    DWORD written;
    char buf[1024];

    HANDLE writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    for (;;)
    {
        ZeroMemory(buf, 1024);
        readOk = ReadFile(this->m_hStdOutRead, buf, 1023, &read, NULL);
        if (!readOk || read == 0)
            break;

        writeOk = WriteFile(writeHandle, buf, read, &written, NULL);
        if (!writeOk)
            break;
    }

    CloseHandle(writeHandle);
}
bool Process::Wait(int timeout)
{
    bool result = true;
    if (this->m_hProcess)
    {
        DWORD waitResult = WaitForSingleObject(this->m_hProcess, (DWORD)timeout);
        result = waitResult == WAIT_OBJECT_0;
    }
    return result;
}
