// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE. 
// 
// Copyright 1993 - 1998 Microsoft Corporation.  All Rights Reserved. 
// 
//  MODULE:   service.c 
// 
//  PURPOSE:  Implements functions required by all services 
//            windows. 
// 
//  FUNCTIONS: 
//    main(int argc, char **argv); 
//    service_ctrl(DWORD dwCtrlCode); 
//    service_main(DWORD dwArgc, LPTSTR *lpszArgv); 
//    ControlHandler ( DWORD dwCtrlType ); 
//    GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize ); 
// 
//  COMMENTS: 
// 
//  AUTHOR: Craig Link - Microsoft Developer Support 
// 
#ifdef DEDICATED_ONLY
#include <windows.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <process.h> 
#include <tchar.h> 

#include "../qcommon/qcommon.h"
#include "service.h"
#include "winquake.h"

// internal variables 
SERVICE_STATUS          ssStatus;       // current status of the service 
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
TCHAR                   szErr[256];
static SERVICE_STATUS          MyServiceStatus;
static SERVICE_STATUS_HANDLE   MyServiceStatusHandle;

int                     bRunningAsService = FALSE;

// internal function prototypes 
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
BOOL WINAPI ControlHandler (DWORD dwCtrlType);
LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize);

extern int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

// 
//  FUNCTION: main 
// 
//  PURPOSE: entrypoint for service 
// 
//  PARAMETERS: 
//    argc - number of command line arguments 
//    argv - array of command line arguments 
// 
//  RETURN VALUE: 
//    none 
// 
//  COMMENTS: 
//    main() either performs the command line task, or 
//    call StartServiceCtrlDispatcher to register the 
//    main service thread.  When the this call returns, 
//    the service has stopped, so exit. 
// 

// 
//  FUNCTION: service_main 
// 
//  PURPOSE: To perform actual initialization of the service 
// 
//  PARAMETERS: 
//    dwArgc   - number of command line arguments 
//    lpszArgv - array of command line arguments 
// 
//  RETURN VALUE: 
//    none 
// 
//  COMMENTS: 
//    This routine performs the service initialization and then calls 
//    the user defined ServiceStart() routine to perform majority 
//    of the work. 
// 
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
    // register our service control handler: 
    // 
    sshStatusHandle = RegisterServiceCtrlHandler(TEXT(SZSERVICENAME), service_ctrl);

    if (!sshStatusHandle)
    {
        goto cleanup;
    }

    // SERVICE_STATUS members that don't change in example 
    // 
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;

    // report the status to the service control manager. 
    // 
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state 
        NO_ERROR,              // exit code 
        3000))                 // wait hint 
    {
        goto cleanup;
    }

    ServiceStart(dwArgc, lpszArgv);

cleanup:
    // try to report the stopped status to the service control manager. 
    // 
    if (sshStatusHandle)
    {
        (VOID)ReportStatusToSCMgr(
            SERVICE_STOPPED,
            dwErr,
            0);
    }

    return;
}

// 
//  FUNCTION: service_ctrl 
// 
//  PURPOSE: This function is called by the SCM whenever 
//           ControlService() is called on this service. 
// 
//  PARAMETERS: 
//    dwCtrlCode - type of control requested 
// 
//  RETURN VALUE: 
//    none 
// 
//  COMMENTS: 
// 
VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
    // Handle the requested control code. 
    // 
    switch (dwCtrlCode)
    {
        // Stop the service. 
        // 
        // SERVICE_STOP_PENDING should be reported before 
        // setting the Stop Event - hServerStopEvent - in 
        // ServiceStop().  This avoids a race condition 
        // which may result in a 1053 - The Service did not respond... 
        // error. 
        case SERVICE_CONTROL_STOP:
            ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            ServiceStop();
            return;
            // Update the service status. 
            // 
        case SERVICE_CONTROL_INTERROGATE:
            break;
            // invalid control code 
            // 
        default:
            break;
    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}

// 
//  FUNCTION: ReportStatusToSCMgr() 
// 
//  PURPOSE: Sets the current status of the service and 
//           reports it to the Service Control Manager 
// 
//  PARAMETERS: 
//    dwCurrentState - the state of the service 
//    dwWin32ExitCode - error code to report 
//    dwWaitHint - worst case estimate to next checkpoint 
// 
//  RETURN VALUE: 
//    TRUE  - success 
//    FALSE - failure 
// 
//  COMMENTS: 
// 
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if (dwCurrentState == SERVICE_START_PENDING)
        ssStatus.dwControlsAccepted = 0;
    else
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwWaitHint = dwWaitHint;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        ssStatus.dwCheckPoint = 0;
    else
        ssStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the service control manager. 
    // 
    fResult = SetServiceStatus(sshStatusHandle, &ssStatus);

    return fResult;
}

// 
//  FUNCTION: ControlHandler ( DWORD dwCtrlType ) 
// 
//  PURPOSE: Handled console control events 
// 
//  PARAMETERS: 
//    dwCtrlType - type of control event 
// 
//  RETURN VALUE: 
//    True - handled 
//    False - unhandled 
// 
//  COMMENTS: 
// 
BOOL WINAPI ControlHandler (DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate 
        case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode 
            _tprintf(TEXT("Stopping %s.\n"), TEXT(SZSERVICEDISPLAYNAME));
            ServiceStop();
            return TRUE;
            break;

    }
    return FALSE;
}

// 
//  FUNCTION: GetLastErrorText 
// 
//  PURPOSE: copies error message text to string 
// 
//  PARAMETERS: 
//    lpszBuf - destination buffer 
//    dwSize - size of buffer 
// 
//  RETURN VALUE: 
//    destination buffer 
// 
//  COMMENTS: 
// 
LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize)
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        GetLastError(),
        LANG_NEUTRAL,
        (LPTSTR)&lpszTemp,
        0,
        NULL);

    // supplied buffer is not long enough 
    if (!dwRet || ((long)dwSize < (long)dwRet + 14))
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp) - 2] = TEXT('\0');  //remove cr and newline character 
        _stprintf(lpszBuf, TEXT("%s (0x%x)"), lpszTemp, (unsigned int)GetLastError());
    }

    if (lpszTemp)
        LocalFree((HLOCAL)lpszTemp);

    return lpszBuf;
}

void ServiceCtrlHandler (DWORD Opcode)
{
    switch (Opcode)
    {
        case SERVICE_CONTROL_STOP:
            Com_Quit();

            MyServiceStatus.dwWin32ExitCode = 0;
            MyServiceStatus.dwCurrentState = SERVICE_STOPPED;
            MyServiceStatus.dwCheckPoint = 0;
            MyServiceStatus.dwWaitHint = 0;

            if (MyServiceStatusHandle)
            {
                SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus);
            }

            return;
    }
    // Send current status. 
    SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus);
}

void ServiceStart (DWORD argc, LPTSTR *argv)
{
    MyServiceStatus.dwServiceType = SERVICE_WIN32;
    MyServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    MyServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    MyServiceStatus.dwWin32ExitCode = 0;
    MyServiceStatus.dwServiceSpecificExitCode = 0;
    MyServiceStatus.dwCheckPoint = 0;
    MyServiceStatus.dwWaitHint = 0;

    MyServiceStatusHandle = RegisterServiceCtrlHandler(argv[0],
        (LPHANDLER_FUNCTION)ServiceCtrlHandler);

    // Initialization complete - report running status. 
    MyServiceStatus.dwCurrentState = SERVICE_RUNNING;
    MyServiceStatus.dwCheckPoint = 0;
    MyServiceStatus.dwWaitHint = 0;

    SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus);

    bRunningAsService = true;

    WinMain (0, NULL, cmdline, 0);

    return;
}

void ServiceStop(void)
{
    ServiceCtrlHandler (SERVICE_CONTROL_STOP);
}

int __cdecl main(void)
{
    SERVICE_TABLE_ENTRY   DispatchTable[] =
    {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
        { NULL,              NULL          }
    };

    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        return 1;
    }

    return 0;
}

#endif // WIN32
