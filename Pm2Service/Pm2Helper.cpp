#include "Pm2Helper.h"
#include "Process.h"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <PathCch.h>

#include <locale>
#include <codecvt>
#include <string>
#include <sstream>

void ConvertNarrowToWide(std::string& narrow, std::wstring& wide)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    wide = converter.from_bytes(narrow);
}

void FindPm2Path(std::wstring& path)
{
    std::wstring command(L"node -e \"process.stdout.write(require.resolve('pm2/bin/pm2'))\"");
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    PathCchRemoveFileSpec(buf, MAX_PATH);
    std::wstring programPath(buf);

    Process proc(command);
    proc.SetWorkingDir(programPath);
    proc.StartProcess();
    std::string pm2path;
    proc.ReadStdOut(pm2path);

    ConvertNarrowToWide(pm2path, path);
}
void BuildPm2Command(std::wstring& args, std::wstring& target)
{
    std::wstring pm2path;
    FindPm2Path(pm2path);

    std::wstringstream ss;
    ss << L"node.exe \"" << pm2path << L"\" " << args;
    target = ss.str();
}
