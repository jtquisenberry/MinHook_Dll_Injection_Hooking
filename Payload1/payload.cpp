#include <windows.h>
#include <iostream>
#include "MinHook.h"
#include "payload.h"
#include "logger.h"

#pragma comment(lib, "winspool.lib")
//#pragma comment(lib, "libMinHook-x86mt.lib")
#pragma comment(lib, "libMinHook.x64.lib")


wchar_t* convertCharArrayToLPCWSTR(const char*);



// Hooks that will be installed (see monitor.h)
//============================================


HOOK_INFO hooks[]= {
    {
        L"kernel32",
        "LoadLibraryW",
        &ProxyLoadLibraryW,
        &fpLoadLibraryW
    },
    {
        L"Advapi32",
        "GetUserNameW",
        &ProxyGetUserNameW,
        &fpGetUserNameW
    },
    {
        L"winspool.drv",
        "EnumPrintersW",
        &ProxyEnumPrintersW,
        &fpEnumPrintersW
    }
};


// Hook installation functions
//============================================

__forceinline BOOL install_hook(HOOK_INFO *pHookInfo)
{
    int cx = 0;
    
    int result = MH_CreateHookApi(pHookInfo->lib, pHookInfo->target, pHookInfo->proxy, (LPVOID*)(pHookInfo->fp));
    if (result != MH_OK)
    {
        cx = fwprintf_s(logger.GetFile(), L"%s: %s %d\n", L"[DLL] install_hook ERROR", convertCharArrayToLPCWSTR(pHookInfo->target), result);
        return FALSE;
    }

    return TRUE;
}

VOID install_all()
{
    int numElts= sizeof(hooks)/sizeof(hooks[0]);
    int cx = 0;

    for (int i= 0; i < numElts; i++)
    {
        cx = fwprintf_s(logger.GetFile(), L"%s %d\n", L"[DLL] Hook #: ", i);
        
        if (install_hook(&hooks[i]))
        {
            LPCWSTR wstrTarget = convertCharArrayToLPCWSTR(hooks[i].target);
            cx = fwprintf_s(logger.GetFile(), L"%s %s\n", L"[DLL] Installed hook in: ", wstrTarget);
        }
    }
}


// DLL entry 
//============================================

BOOL WINAPI DllMain(HINSTANCE const instance, DWORD const reason, LPVOID const reserved)  
{
    
    /*
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"Message",
        (LPCWSTR)L"Title",
        MB_DEFBUTTON2
    );
    */
    
    
    int cx = 0;
    int xxx = 0;
    
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            cx = fwprintf_s(logger.GetFile(), L"%s", L"[DLL] DLL_PROCESS_ATTACH\n");
            cx = fwprintf_s(logger.GetFile(), L"%s", L"[DLL] Installing hooks.\n");

            MH_Initialize();
            install_all();

            MH_EnableHook(MH_ALL_HOOKS);

            cx = fwprintf_s(logger.GetFile(), L"%s", L"[DLL] Hooks installed, Resuming main thread.\n");
            break;
        case DLL_THREAD_ATTACH:
            cx = fwprintf_s(logger.GetFile(), L"%s", L"[DLL] DLL_THREAD_ATTACH\n");
            break;
        case DLL_THREAD_DETACH:
            cx = fwprintf_s(logger.GetFile(), L"%s", L"[DLL] DLL_THREAD_DETACH\n");
            break;
        case DLL_PROCESS_DETACH:
            cx = fwprintf_s(logger.GetFile(), L"%s", L"[DLL] DLL_PROCESS_DETACH\n");
            break;
    }

    return TRUE;  
}

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}