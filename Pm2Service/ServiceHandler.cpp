#include "ServiceHandler.h"
#include "SystemEventLog.h"
#include "ServiceManagement.h"

// Code in here heavily based on: https://docs.microsoft.com/en-us/windows/desktop/Services/svc-cpp

HANDLE hShutdownEvent = NULL;

HANDLE ghSvcStopEvent = NULL;

VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID SvcInit(DWORD, LPTSTR*);

HRESULT RunService() 
{ 
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONW)SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    if (!StartServiceCtrlDispatcherW(DispatchTable)) 
    { 
        return GetLastError();
    } 

    return S_OK;
} 

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler function for the service

    SERVICE_STATUS_HANDLE hStatus = RegisterServiceCtrlHandlerW( 
        SERVICE_NAME, 
        SvcCtrlHandler);

    if(!hStatus)
    { 
        LogError(L"Error in SvcMain", GetLastError());
        return; 
    } 
    SetStatusHandle(hStatus);

    LogInfo(L"In SvcMain");

    // Report initial status to the SCM
    SetStartPending(0, 10);

    // Perform service-specific initialization and work.
    SvcInit(dwArgc, lpszArgv);
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if (ghSvcStopEvent == NULL)
    {
        SetStopped(0);
        return;
    }

    // Start up the PM2 Process
    

    // Report running status when initialization is complete.
    bool ok = SetRunning();
    if (!ok)
    {
        LogError(L"Service Start Failed", -1);
        return;
    }
    LogSuccess(L"Service Started");


    // TO_DO: Perform work until service stops.
    while(1)
    {
        // Check on our process
        



        // Check whether to stop the service.
        if (WaitForSingleObject(ghSvcStopEvent, 500) != WAIT_TIMEOUT) {
            SetStopped(0);
            return;
        }
    }
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
    // Handle the requested control code. 
    switch(dwCtrl) 
    {  
    case SERVICE_CONTROL_STOP:
        SetStopPending(0, 500);
        // Signal the service to stop.
        SetEvent(ghSvcStopEvent);
        return;
    case SERVICE_CONTROL_INTERROGATE: 
        break; 
    default: 
        break;
   } 
   
}


