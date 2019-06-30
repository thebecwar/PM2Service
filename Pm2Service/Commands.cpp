#include "Commands.h"

#include <string>
#include <iostream>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <PathCch.h>
#include <ShlObj.h>
#include <AclAPI.h>

#include "ServiceManagement.h"
#include "SystemEventLog.h"
#include "ServiceHandler.h"
#include "UserManagement.h"
#include "Process.h"

const wchar_t* InstallShortHelp = L"Install the service";
const wchar_t* InstallLongHelp = LR"(
Installs the service

    usage: PM2Service install username [password]

    - username: The name of the service username. Use verifyuser to check the account setup.
    - password: If provided, this is the password for the user account. If not provided, you will be prompted.
)";
int Install(int argc, const wchar_t** argv)
{
    // Created Local User:
    // fusion, 1qazXSW@
    wchar_t szPath[MAX_PATH];
    if (!GetModuleFileNameW(NULL, szPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return -1;
    }
    std::wstring path(szPath);
    path = path + L" run";

    if (argc < 3)
    {
        std::wcout << L"Error Message TODO" << std::endl;
        return -1;
    }
    std::wstring username(argv[2]);

    if (!VerifyUserExists(username))
    {
        std::wcout << L"Specified user " << username << L"does not exist" << std::endl;
        return -1;
    }
    if (!VerifyUser(username))
    {
        std::wcout << L"User doesn't have the SeServiceLogonRight." << std::endl;
        std::wcout << L"execute `configaccount " << username << L"` to enable the right." << std::endl;
        return -1;
    }
    if (!NormalizeUsername(username))
    {
        std::wcout << L"Error attempting to normalize username" << std::endl;
        return -1;
    }


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

    // Create Shared path in ProgramData
    CoInitialize(NULL);
    wchar_t* buf;
    SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &buf);
    std::wstring sharePath(buf);
    CoTaskMemFree(buf);

    sharePath += L"\\PM2Service";

    BOOL ok = CreateDirectoryW(sharePath.c_str(), NULL);
    int err = GetLastError();
    if (ok)
    {
        std::wcout << L"Created: " << sharePath << std::endl;
    }
    else if (err != ERROR_ALREADY_EXISTS)
    {
        std::wcout << L"Failed to create: " << sharePath << std::endl;
        std::wcout << L"Error Code: " << GetLastError() << std::endl;
    }
    
    PSECURITY_DESCRIPTOR des;
    PACL acl;
    DWORD res = GetNamedSecurityInfoW(
        sharePath.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL,
        NULL,
        &acl,
        NULL,
        &des);
    if (res == ERROR_SUCCESS)
    {
        EXPLICIT_ACCESSW ea = { 0 };
        ea.grfAccessPermissions = FILE_ALL_ACCESS;
        ea.grfAccessMode = GRANT_ACCESS;
        ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
        ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea.Trustee.ptstrName = (LPWCH)username.c_str();

        PACL newAcl;

        res = SetEntriesInAclW(1, &ea, acl, &newAcl);
        if (res == ERROR_SUCCESS)
        {
            res = SetNamedSecurityInfoW(
                (LPWSTR)sharePath.c_str(),
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION,
                NULL,
                NULL,
                newAcl,
                NULL);
            if (res != ERROR_SUCCESS)
            {
                std::wcout << L"Unable to set permissions on " << sharePath << std::endl;
                LogError(L"Installation Error - Unable to set permissions", res);
            }
            LocalFree((HLOCAL)newAcl);
        }
        else
        {
            std::wcout << L"Unable to set permissions on " << sharePath << std::endl;
            LogError(L"Installation Error - Unable to set permissions", res);
        }

        LocalFree((HLOCAL)des);
        LocalFree((HLOCAL)acl);
    }
    else
    {
        std::wcout << L"Unable to set permissions on " << sharePath << std::endl;
        LogError(L"Installation Error - Unable to set permissions", res);
    }

    return 0;
}
const wchar_t* UninstallShortHelp = L"Uninstalls the service";
const wchar_t* UninstallLongHelp = LR"(
Uninstalls the service

    usage: PM2Service uninstall
)";
int Uninstall(int argc, const wchar_t** argv)
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
    return 0;
}
const wchar_t* RunShortHelp = L"Internal command used by the service control manager.";
int Run(int argc, const wchar_t** argv)
{
    LogInfo(L"Starting PM2 Service");
    HRESULT hr = RunService();
    if (hr != S_OK)
    {
        std::wcout << L"Service Start Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
        LogError(L"Service Start Error", hr);
        return -1;
    }
    return 0;
}
const wchar_t* StartShortHelp = L"Starts the service";
const wchar_t* StartLongHelp = LR"(
Starts the service

    usage: PM2Service start
)";
int Start(int argc, const wchar_t** argv)
{
    HRESULT hr = Start();
    if (hr != S_OK)
    {
        std::wcout << L"Service Start Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
        return -1;
    }
    else
    {
        std::wcout << L"Service Started Successfully" << std::endl;
        return 0;
    }
}
const wchar_t* StopShortHelp = L"Stops the service";
const wchar_t* StopLongHelp = LR"(
Stops the service

    usage: PM2Service stop
)";
int Stop(int argc, const wchar_t** argv)
{
    HRESULT hr = Stop();
    if (hr != S_OK)
    {
        std::wcout << L"Service Stop Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
        return -1;
    }
    else
    {
        std::wcout << L"Service Stopped Successfully" << std::endl;
        return 0;
    }
}
const wchar_t* RestartShortHelp = L"Restarts the service";
const wchar_t* RestartLongHelp = LR"(
Restarts the service

    usage: PM2Service restart
)";
int Restart(int argc, const wchar_t** argv)
{
    HRESULT hr = Stop();
    if (hr != ERROR_SERVICE_NOT_ACTIVE)
    {
        std::wcout << L"Service Stop Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
        return -1;
    }

    hr = Start();
    if (hr != S_OK)
    {
        std::wcout << L"Service Restart Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
        return -1;
    }
    else
    {
        std::wcout << L"Service Restarted Successfully" << std::endl;
        return 0;
    }
}
const wchar_t* ConfigAccountShortHelp = L"Configures the service user account logon.";
const wchar_t* ConfigAccountLongHelp = LR"(
Configures the service user account. Enables SeLoginServiceRight for the specified user.

    usage: PM2Service configaccount username

    - username: the name of the user to enable SeLoginServiceRight to.
)";
int ConfigAccount(int argc, const wchar_t** argv)
{
    std::wstring username(argv[2]);
    if (VerifyUser(username))
    {
        std::wcout << L"User account already configured. Nothing to do." << std::endl;
        return 0;
    }
    else
    {
        HRESULT hr = ConfigureUser(username);
        if (SUCCEEDED(hr))
        {
            std::wcout << L"User configuration successful" << std::endl;
            return 0;
        }
        else
        {
            std::wcout << L"User Configuration Error: " << hr << L" (0x" << std::hex << hr << L")" << std::endl;
            return -1;
        }
    }
}
const wchar_t* ConfigShortHelp = L"";
const wchar_t* ConfigLongHelp = LR"(
)";
int Config(int argc, const wchar_t** argv)
{
    return 0;
}
const wchar_t* SetPm2VersionShortHelp = L"Sets the PM2 Version. Must be run as administrator.";
const wchar_t* SetPm2VersionLongHelp = LR"(
Downloads and installs the specified version of PM2. If no version is provided, installs the latest.

    usage: PM2Service setpm2version [version]

    - version: If provided, installs the specific PM2 version. If absent, installs the latest.

)";
int SetPM2Version(int argc, const wchar_t** argv)
{
    std::wstring command(L"npm.cmd install pm2");
    if (argc > 2)
    {
        command += L"@";
        command += argv[2];
    }

    // Build path
    wchar_t buffer[MAX_PATH];
    ZeroMemory(buffer, MAX_PATH);
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    PathCchRemoveFileSpec(buffer, MAX_PATH);

    std::wstring path(buffer);

    Process proc(command);
    proc.SetWorkingDir(path);
    proc.StartProcess();
    proc.StreamStdOut();

    return 0;
}
const wchar_t* ListPm2VersionsShortHelp = L"Lists the available PM2 versions.";
const wchar_t* ListPm2VersionsLongHelp = LR"(
Lists the available PM2 versions.

    usage: PM2Service listpm2versions
)";
int ListPm2Versions(int argc, const wchar_t** argv)
{
    std::wstring cmdline(L"npm.cmd show pm2 versions --json");
    Process proc(cmdline);
    proc.StartProcess();

    std::string output;
    proc.ReadStdOut(output);

    bool inQuote = false;
    for (int i = 0; i < output.size(); i++)
    {
        if (inQuote)
        {
            if (output.at(i) == '"')
            {
                inQuote = false;
                std::cout << std::endl;
            }
            else
            {
                std::cout << output.at(i);
            }
        }
        else
        {
            if (output.at(i) == '"')
            {
                inQuote = true;
            }
        }
    }

    return 0;
}

const wchar_t* CheckUserShortHelp = L"Checks that a user has the appropriate permissions";
const wchar_t* CheckUserLongHelp = LR"(
Checks that a user has the appropriate permissions

    usage: PM2Service checkuser username

    - username: the name of the user to check
)";
int CheckUser(int argc, const wchar_t** argv)
{
    std::wstring username(argv[2]);
    bool exists = VerifyUserExists(username);
    if (!exists)
    {
        std::wcout << L"Specified user " << username << " does not exist." << std::endl;
        return -1;
    }
    bool valid = VerifyUser(username);
    if (!valid)
    {
        std::wcout << L"User " << username << " does not have permission to log in as a service." << std::endl;
        return -1;
    }
    else
    {
        std::wcout << L"User " << username << " is valid." << std::endl;
        return 0;
    }
}

int Help(int argc, const wchar_t** argv);

#include <vector>

int TestEnv(int argc, const wchar_t** argv)
{

    std::vector<std::pair<std::wstring, std::wstring>> env;
    env.emplace_back(std::pair<std::wstring, std::wstring>(L"TESTENVVAR", L"this is a test"));

    std::wstring command(L"cmd.exe /c echo %TESTENVVAR%");
    Process proc(command, env);
    proc.StartProcess();
    proc.StreamStdOut();

    return 0;
}

CommandInfo AllCommands[] = {
    { L"install", 1, InstallShortHelp, InstallLongHelp, Install },
    { L"uninstall", 0, UninstallShortHelp, UninstallLongHelp, Uninstall },
    { L"run", 0, RunShortHelp, NULL, Run },
    { L"start", 0, StartShortHelp, StartLongHelp, Start },
    { L"stop", 0, StopShortHelp, StopLongHelp, Stop },
    { L"restart", 0, RestartShortHelp, RestartLongHelp, Restart },
    { L"configaccount", 1, ConfigAccountShortHelp, ConfigAccountLongHelp, ConfigAccount },
    { L"config", 1, ConfigShortHelp, ConfigLongHelp, Config },
    { L"checkuser", 1, CheckUserShortHelp, CheckUserLongHelp, CheckUser },
    { L"setpm2version", 0, SetPm2VersionShortHelp, SetPm2VersionLongHelp, SetPM2Version },
    { L"listpm2versions", 0, ListPm2VersionsShortHelp, ListPm2VersionsLongHelp, ListPm2Versions },
{ L"testenv", 0, NULL, NULL, TestEnv },
    { L"help", 0, L"Get information about a specific command with `help COMMAND`", nullptr, Help },
};

int RunCommand(int argc, const wchar_t** argv)
{
    // Testing code. Hopefully I'm smart enough to delete it _before_ I commit.


    if (argc >= 2)
    {
        for (int i = 0; i < _countof(AllCommands); i++)
        {
            if (_wcsicmp(AllCommands[i].command, argv[1]) == 0)
            {
                if ((argc - 2) < AllCommands[i].minArgs)
                {
                    if (AllCommands[i].longHelp)
                    {
                        std::wcout << AllCommands[i].longHelp << std::endl;
                    }
                    else
                    {
                        std::wcout << AllCommands[i].shortHelp << std::endl;
                    }
                    return -1;
                }
                else
                {
                    return AllCommands[i].cmd(argc, argv);
                }
            }
        }
    }
    return Help(argc, argv);
}
int Help(int argc, const wchar_t** argv)
{
    if (argc >= 3 && _wcsicmp(argv[1], L"help") == 0)
    {
        // long help for a specific command
        for (int i = 0; i < _countof(AllCommands); i++)
        {
            if (_wcsicmp(AllCommands[i].command, argv[2]) == 0)
            {
                if (AllCommands[i].longHelp)
                {
                    std::wcout << AllCommands[i].longHelp << std::endl;
                }
                else
                {
                    std::wcout << AllCommands[i].shortHelp << std::endl;
                }
                return 0;
            }
        }
    }

    std::wcout << L"PM2Service - Windows service for running PM2" << std::endl << std::endl;
    for (int i = 0; i < _countof(AllCommands); i++)
    {
        std::wcout 
            << std::setw(20) 
            << std::left 
            << AllCommands[i].command 
            << std::resetiosflags(0)
            << L"  " 
            << AllCommands[i].shortHelp 
            << std::endl;
    }

    return 0;
}
