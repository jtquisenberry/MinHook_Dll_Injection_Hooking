// Injector.cpp : Defines the entry point for the console application.
//

#include "Injector.h"
#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <tchar.h>
#include <wchar.h>
#include <vector>
#include <string>
#include <codecvt>
#include <locale>
#pragma comment(lib, "ntdll")

#include "stdafx.h"

using namespace std;

void PrintError(LPTSTR lpszFunction);
PROCESS_INFORMATION StartTargetExe(__in LPCWSTR targetPath);
HANDLE GetTargetExe(__in int pid);
bool is_numeric(char* str);
BOOL WINAPI InjectDll(__in HANDLE hProcess, __in LPCWSTR lpcwszDll);
BOOL WINAPI InjectDll(__in PROCESS_INFORMATION pi, __in LPCWSTR lpcwszDll);
BOOL WINAPI InjectDll_Running(__in HANDLE hProcess, __in LPCWSTR lpcwszDll);
BOOL WINAPI InjectDll_Suspended(__in PROCESS_INFORMATION processInformation, __in LPCWSTR lpcwszDll);
BOOL WINAPI ResumeRemoteMainThread(HANDLE hProcess, HANDLE hThreadMain);
BOOL WINAPI Cleanup_Suspended(HANDLE hProcess, HANDLE hThreadMain);
BOOL ListProcessThreads(DWORD dwOwnerPID);
bool is_numeric(string str);
void PrintHelp(void);


void PrintHelp()
{
    std::cout << "Usage: \n";
    // std::cout << "Injector.exe" << "--pid <PID> | --exe <Path> --dll <Path> [--log <Path>]\n";
    std::cout << "Injector.exe" << "--pid <PID> | --exe <Path> --dll <Path>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Shows this help message\n";
    std::cout << "  -v, --version    Shows the application version\n";
    std::cout << "                                                \n";
    std::cout << "Specify either a PID or an .EXE. If both a PID and an executable are  \n";
    std::cout << "specified, the PID is used and the executable is ignored. \n";
    std::cout << "      --pid <PID>  The process ID of a running process into which a .DLL will be injected.\n";
    std::cout << "      --exe <Path> The fully-qualified filename of a non-running executable that will be.\n";
    std::cout << "                   started and into which a .DLL will be injected.\n";
    std::cout << "      --dll <Path> The fully-qualified filename of the .DLL to inject.\n";
    // std::cout << "      --log <Path> The fully-qualified filename of a log file. Optional.\n";
}


int main(int argc, char* argv[])
{
    fwprintf_s(logger.GetFile(), L"%s\n", L"[Injector] Top of main()");
    
    std::vector<std::string> arguments;

    string strLogFile = "";
    wstring wstrLogFile = L"";
    string strPid = "";
    string strExeFile = "";
    wstring wstrExeFile = L"";
    string strDllFile = "";
    wstring wstrDllFile = L"";
    int intPid = -1;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            PrintHelp();
        }
        else if (arg == "-v" || arg == "--version") {
            std::cout << "Injector version 1.0\n";
        }
        else if (arg == "--pid") {
            strPid = argv[i + 1];
            if (is_numeric(strPid)) {
                intPid = stoi(strPid);
            }
        }
        else if (arg == "--exe") {
            strExeFile = argv[i + 1];
            wstrExeFile = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(strExeFile);
        }
        else if (arg == "--log") {
            strLogFile = argv[i + 1];
            wstrLogFile = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(strExeFile);
        }
        else if (arg == "--dll") {
            strDllFile = argv[i + 1];
            wstrDllFile = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(strDllFile);
        }
        
        arguments.push_back(arg);

    }

    // Process custom arguments
    std::cout << "ARGUMENTS" << std::endl;
    for (const std::string& arg : arguments) {
        // Add your custom argument processing logic here
        std::cout << "Argument: " << arg << "\n";
    }
    
    HANDLE hProcess;
    PROCESS_INFORMATION pi;

    wchar_t selfdir[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, selfdir, MAX_PATH);
    PathRemoveFileSpec(selfdir);

    if ((( intPid <= 0) && (wstrExeFile == L"")) || (wstrDllFile == L""))
    {
        PrintHelp();
    }
    
    
    if (wstrDllFile == L"")
    {
        wstrDllFile = std::wstring(selfdir) + TEXT("\\hooks2.dll");
    }

    if (intPid > 0) {
        hProcess = GetTargetExe(intPid);

        // if (InjectDll(hProcess, wstrDllFile.c_str()))
        if (InjectDll_Running(hProcess, wstrDllFile.c_str())) 
        {
            printf("\n");
            printf("*** Dll was successfully injected. ***\n");
        }
        else {
            printf("\n");
            printf("*** Terminating the Injector application. ***\n");
        }
    }
    else { 
        if (wstrExeFile == L"") {
            wstrExeFile = wstring(selfdir) + L"\\target.exe";
        }
        
        pi = StartTargetExe(wstrExeFile.c_str());

        // if (InjectDll(pi, wstrDllFile.c_str()))
        if (InjectDll_Suspended(pi, wstrDllFile.c_str())) 
        {
            printf("\n");
            printf("*** Dll was successfully injected. ***\n");
        }
        else {
            printf("\n");
            printf("*** Terminating the Injector application. ***\n");
        }
    }

    printf("\n");
    printf("*** End of injector application. ***\n");
    printf("\n");

    return 0;
}

bool is_numeric(char* str) {
    printf(str);
    printf("\n");

    int total = strlen(str);
    int numeric_count = 0;

    for (int i = 0; i < strlen(str); i++) {
        if (isdigit(str[i]))
            numeric_count++;
    }
    
    if (total - numeric_count == 0)
        return true;
    
    return false;
}

bool is_numeric(string str) {
    int total = str.length();
    int numeric_count = 0;

    for (int i = 0; i < str.length(); i++) {
        if (isdigit(str[i]))
            numeric_count++;
    }

    if (total - numeric_count == 0)
        return true;

    return false;
}


HANDLE GetTargetExe(__in int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return hProcess;
}

PROCESS_INFORMATION StartTargetExe(__in LPCWSTR targetPath)
{
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInformation;

    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(STARTUPINFO);

    if (!CreateProcess(targetPath, NULL, NULL, NULL, FALSE,
        CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInformation))
    {
        PrintError(TEXT("CreateProcess"));
        return processInformation;
    }

    HANDLE hProcess = processInformation.hProcess;

    ListProcessThreads(processInformation.dwProcessId);
    return processInformation;
}


BOOL WINAPI InjectDll(__in HANDLE hProcess, __in LPCWSTR lpcwszDll)
{
    
    SIZE_T nLength;
    LPVOID lpLoadLibraryW = NULL;
    LPVOID lpRemoteString;

    lpLoadLibraryW = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");

    if (!lpLoadLibraryW)
    {
        PrintError(TEXT("GetProcAddress"));
        // close process handle
        CloseHandle( hProcess);
        return FALSE;
    }

    nLength = wcslen(lpcwszDll) * sizeof(WCHAR);

    // allocate mem for dll name
    lpRemoteString = VirtualAllocEx(hProcess, NULL, nLength + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!lpRemoteString)
    {
        PrintError(TEXT("VirtualAllocEx"));

        // close process handle
        CloseHandle(hProcess);

        return FALSE;
    }

    // write dll name
    if (!WriteProcessMemory(hProcess, lpRemoteString, lpcwszDll, nLength, NULL)) {

        PrintError(TEXT("WriteProcessMemory"));
        // free allocated memory
        VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);

        // close process handle
        CloseHandle(hProcess);

        return FALSE;
    }

    // call loadlibraryw
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpRemoteString, NULL, NULL);

    if (!hThread) {
        PrintError(TEXT("CreateRemoteThread"));
    }
    else {
        WaitForSingleObject(hThread, 4000);
    }

    //  free allocated memory
    VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);
    CloseHandle(hThread);

    return TRUE;
}


BOOL WINAPI InjectDll_Running(__in HANDLE hProcess, __in LPCWSTR lpcwszDll)
{
    SIZE_T nLength;
    LPVOID lpLoadLibraryW = NULL;
    
    InjectDll(hProcess, lpcwszDll);

    return TRUE;
}

BOOL WINAPI InjectDll_Suspended(__in PROCESS_INFORMATION processInformation, __in LPCWSTR lpcwszDll)
{
    SIZE_T nLength;
    LPVOID lpLoadLibraryW = NULL;
    HANDLE hProcess = processInformation.hProcess;
    HANDLE hThreadMain = processInformation.hThread;

    InjectDll(hProcess, lpcwszDll);
    ResumeRemoteMainThread(hProcess, hThreadMain);
    Cleanup_Suspended(hProcess, hThreadMain);

    return TRUE;
}


BOOL WINAPI ResumeRemoteMainThread(HANDLE hProcess, HANDLE hThreadMain)
{
    // Resume suspended process by resuming the main thread in the process.
    ResumeThread(hThreadMain);
    WaitForSingleObject(hProcess, 60);

    return TRUE;
}


BOOL WINAPI Cleanup_Running(HANDLE hProcess, LPVOID lpRemoteString)
{
    // Close the process handle
    CloseHandle(hProcess);

    return TRUE;
}


BOOL WINAPI Cleanup_Suspended(HANDLE hProcess, HANDLE hThreadMain)
{
    // Close the handle of main thread in the remote process.
    CloseHandle(hThreadMain);
    // Close the handle of the remote process
    CloseHandle(hProcess);

    return TRUE;
}




/*
BOOL WINAPI InjectDll(__in PROCESS_INFORMATION processInformation, __in LPCWSTR lpcwszDll)
{

    SIZE_T nLength;
    LPVOID lpLoadLibraryW = NULL;
    LPVOID lpRemoteString;
    HANDLE hProcess = processInformation.hProcess;

    lpLoadLibraryW = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");

    if (!lpLoadLibraryW)
    {
        PrintError(TEXT("GetProcAddress"));
        // close process handle
        CloseHandle(hProcess);
        return FALSE;
    }

    nLength = wcslen(lpcwszDll) * sizeof(WCHAR);

    // Allocate memory for DLL name
    lpRemoteString = VirtualAllocEx(hProcess, NULL, nLength + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!lpRemoteString)
    {
        PrintError(TEXT("VirtualAllocEx"));

        // close process handle
        CloseHandle(hProcess);

        return FALSE;
    }

    // Write DLL name
    if (!WriteProcessMemory(hProcess, lpRemoteString, lpcwszDll, nLength, NULL)) {

        PrintError(TEXT("WriteProcessMemory"));
        // free allocated memory
        VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);

        // close process handle
        CloseHandle(hProcess);

        return FALSE;
    }

    // Create a new thread in the remote process
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpRemoteString, NULL, NULL);

    if (!hThread) {
        PrintError(TEXT("CreateRemoteThread"));
    }
    else {
    }

    return TRUE;
}
*/


BOOL ListProcessThreads(DWORD dwOwnerPID)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Take a snapshot of all running threads  
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return(FALSE);

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(hThreadSnap, &te32))
    {
        PrintError(TEXT("Thread32First")); // show cause of failure
        CloseHandle(hThreadSnap);          // clean the snapshot object
        return(FALSE);
    }
    
    // Now walk the thread list of the system,
    // and display information about each thread
    // associated with the specified process
    do
    {
        int cx = 0;
        
        if (te32.th32OwnerProcessID == dwOwnerPID)
        {
            _tprintf(TEXT("\n\nTHREAD ID      = 0x%08X"), te32.th32ThreadID);
            _tprintf(TEXT("\nBase priority  = %d"), te32.tpBasePri);
            _tprintf(TEXT("\nDelta priority = %d"), te32.tpDeltaPri);
            _tprintf(TEXT("\n"));
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return(TRUE);
}


void PrintError(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);

    wprintf(L"%s", (LPCWSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

int StringToWString(std::wstring& ws, const std::string& s)
{
    std::wstring wsTmp(s.begin(), s.end());
    ws = wsTmp;
    return 0;
}