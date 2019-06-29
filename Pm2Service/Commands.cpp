#include "Commands.h"

#include <string>
#include <iostream>
#include <iomanip>

#include "ServiceManagement.h"
#include "SystemEventLog.h"
#include "ServiceHandler.h"
#include "UserManagement.h"

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
    { L"help", 0, L"Get information about a specific command with `help COMMAND`", nullptr, Help },
};

int RunCommand(int argc, const wchar_t** argv)
{
    if (argc >= 2)
    {
        for (int i = 0; i < _countof(AllCommands); i++)
        {
            if (_wcsicmp(AllCommands[i].command, argv[1]) == 0)
            {
                if ((argc - 2) < AllCommands[i].minArgs)
                {
                    std::wcout << AllCommands[i].longHelp << std::endl;
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
                std::wcout << AllCommands[i].longHelp << std::endl;
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
