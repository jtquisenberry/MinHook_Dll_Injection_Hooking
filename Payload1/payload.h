#include "logger.h"
#include <winspool.h>

#pragma once


// Standard information for every hook
struct HOOK_INFO {
    LPCWSTR lib;
    LPCSTR target;
    LPVOID proxy;
    LPVOID fp;
};

// Logger object - exposes a stream to report to the IPC channel
Logger logger;


// Original-function type declarations
//============================================
// typedef <return type> (WINAPI *<user-defined type>)(<argument types>)
// Do not include [in] or [out] attributes.
typedef int (WINAPI* LoadLibraryWType)(LPCWSTR);
typedef BOOL (WINAPI* EnumPrintersWType)(DWORD, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL(WINAPI* GetUserNameWType)(LPWSTR, LPDWORD);


// Original-function pointer declarations
//============================================

// <user-defined type> <user-defined name> = NULL
LoadLibraryWType fpLoadLibraryW = NULL;
EnumPrintersWType fpEnumPrintersW = NULL;
GetUserNameWType fpGetUserNameW = NULL;


// Proxy-function definitions
//============================================

int WINAPI ProxyLoadLibraryW(LPCWSTR lpLibFileName)
{
    int cx = 0;
    cx = fwprintf_s(logger.GetFile(), L"%s%s", L"[HOOK] Intercepted call to LoadLibraryW:\n", lpLibFileName);
    return fpLoadLibraryW(lpLibFileName);
}

int WINAPI ProxyEnumPrintersW(DWORD Flags, LPTSTR Name, DWORD Level, LPBYTE pPrinterEnum, DWORD cbBuf, LPDWORD pcbNeeded, LPDWORD pcReturned)
{
    int cx = 0;
    cx = fwprintf_s(logger.GetFile(), L"%s\n", L"[HOOK] Intercepted call to EnumPrinters");
    cx = fwprintf_s(logger.GetFile(), L"%s: %s\n", L"[HOOK] Name", Name);
    fflush(logger.GetFile());

    /*
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"Body",
        (LPCWSTR)L"Title",
        MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
    );
    */

    return fpEnumPrintersW(Flags, Name, Level, pPrinterEnum, cbBuf, pcbNeeded, pcReturned);
}

int WINAPI ProxyGetUserNameW(LPWSTR lpBuffer, LPDWORD pcbBuffer)
{
    int cx = 0;
    cx = fwprintf_s(logger.GetFile(), L"%s\n", L"[HOOK] Intercepted call to GetUserNameW");
    fflush(logger.GetFile());

    int result = fpGetUserNameW(lpBuffer, pcbBuffer);

    cx = fwprintf_s(logger.GetFile(), L"%s: %s\n", L"[HOOK] GetUserNameW", lpBuffer);
    fflush(logger.GetFile());

    return result;
}