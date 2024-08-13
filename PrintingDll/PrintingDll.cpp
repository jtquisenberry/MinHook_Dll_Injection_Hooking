// Defines the entry point for the DLL application.

#pragma once


#include "pch.h"
#include "PrintingDll.h"
#include "MinHook.h"
#include <iostream>
#include <windows.h>
#include <winspool.h>

#pragma comment(lib, "libMinHook.x64.lib")
#pragma comment(lib, "winspool.lib")

using namespace std;

// Function declarations
std::wstring current_time_to_stringW();


// Module-level variables
FILE* log_file;


/* 01 */
typedef int (WINAPI* MessageBoxWType)(HWND, LPCWSTR, LPCWSTR, UINT);
typedef BOOL(WINAPI* ExtTextOutWType)(HDC, int, int, UINT, const RECT*, LPCWSTR, UINT, const INT*);

/* 02 */
// Pointers to original API functions.
MessageBoxWType fpMessageBoxW = NULL;
ExtTextOutWType fpExtTextOutW = NULL;

/* 03 */
// Proxy functions, which intercept API calls.
int WINAPI ProxyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	return fpMessageBoxW(hWnd, L"Hooked!", lpCaption, uType);
	return 1;
}

BOOL WINAPI ProxyExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx)
{

	log_file = logger.GetFile();
	
	fputs("\n", log_file);
	fputs("---------------------------------------- START\n", log_file);
	fputs("ProxyExtTextOutW\n", log_file);
	fprintf(log_file, "hdc: %p\n", hdc);
	fwprintf(log_file, L"lpString: %s\n", lpString);
	fprintf(log_file, "x: %d\n", x);
	fprintf(log_file, "y: %d\n", y);
	fprintf(log_file, "lprect address: %p\n", lprect);
    if (lprect > (RECT*)0x0) {
		fprintf(log_file, "lprect->left: %d\n", lprect->left);
		fprintf(log_file, "lprect->right: %d\n", lprect->right);
	    fprintf(log_file, "lprect->top: %d\n", lprect->top);
	    fprintf(log_file, "lprect->bottom: %d\n", lprect->bottom);
	}
	fputs("\n", log_file);
	fflush(log_file);

	BOOL result = 0;
	result = fpExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	fputs("---------------------------------------- END\n", log_file);
	fflush(log_file);
	return result;
}


int AddHooks()
{
	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		return 1;
	}

	/* 04 */
	/* Install Hooks */
	// Create a hook for MessageBoxW, in disabled state.
	if (MH_CreateHook(&MessageBoxW, &ProxyMessageBoxW,
		reinterpret_cast<LPVOID*>(&fpMessageBoxW)) != MH_OK)
	{
		return 1;
	}

	if (MH_CreateHook(&ExtTextOutW, &ProxyExtTextOutW,
		reinterpret_cast<LPVOID*>(&fpExtTextOutW)) != MH_OK)
	{
		return 1;
	}


	/* 05 */
	/* Enable Hooks  */
	if (MH_EnableHook(&ExtTextOutW) != MH_OK)
	{
		return 1;
	}

	/* 06 */
	/* Disable Hooks */
	if (MH_DisableHook(&MessageBoxW) != MH_OK)
	{
		return 1;
	}

	// Uninitialize MinHook.
	if (MH_Uninitialize() != MH_OK)
	{
		return 1;
	}

	return 0;
}


/*===============================*/
/* The Abort Procudure */
/* ==============================*/
BOOL CALLBACK AbortProc(HDC hDC, int Error)
{
	MSG   msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}


/*===============================*/
/* Initialize DOCINFO structure */
/* ==============================*/
void InitPrintJobDoc(DOCINFO* di, wchar_t* docname)
{
	memset(di, 0, sizeof(DOCINFO));
	/* Fill in the required members. */
	di->cbSize = sizeof(DOCINFO);
	di->lpszDocName = docname;
}


std::wstring current_time_to_stringW()
{
	struct tm newtime;
	//char am_pm[] = "AM";
	__time64_t long_time;
	// char timebuf[26];
	errno_t err;

	// Get current time as 64-bit integer.
	_time64(&long_time);

	// Convert to local time.
	err = _localtime64_s(&newtime, &long_time);

	wchar_t output[30];
	wcsftime(output, 30, L"%Y %m-%d %H-%M-%S", &newtime);

	// Convert to an ASCII representation.
	// err = asctime_s(timebuf, 26, &newtime);

	return output;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		log_file = logger.GetFile();
		AddHooks();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		//fclose(log_file);
        break;
    }
    return TRUE;
}

