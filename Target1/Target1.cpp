// Target1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Targert1.h"
#include <Lmcons.h>
#include <iostream>
#include <windows.h>
#include <winspool.h>
#include <string.h>
#include <cstring>
#include <conio.h>
#include <ctype.h>


int PrintUsername()
{
	wchar_t username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);
	std::wcout << L"Username: " << username << std::endl;
	std::wcout << std::endl;
	std::wcout << std::endl;
	return 0;
}


int PrintPrinters()
{
	DWORD dwSizeNeeded = 0;
	DWORD dwPrinterCount = 0;
	BOOL bEnumResult;
	HANDLE hProcessHeap = GetProcessHeap();
	LPPRINTER_INFO_2 lpPrinterData;

	// Call `EnumPrinters` the first time to get the size of the buffer that
	// will store an array of `LPPRINTER_INFO_2`
	EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &dwSizeNeeded, &dwPrinterCount);

	// Allocates memory on the heap to the buffer.
	if ((lpPrinterData = (LPPRINTER_INFO_2)HeapAlloc(hProcessHeap,
		HEAP_ZERO_MEMORY, dwSizeNeeded)) == NULL)
	{
		// An error has occurred.
		return 1;
	}
	// Call `EnumPrinters` a second time, specifyng the size of the buffer retrieved
	// from the first call. This call fills the buffer.
	bEnumResult = EnumPrinters(PRINTER_ENUM_LOCAL,
		NULL,
		2,
		(LPBYTE)lpPrinterData,
		dwSizeNeeded,
		&dwSizeNeeded,
		&dwPrinterCount);

	if (bEnumResult)
	{
		std::wcout << L"BEGIN LIST PRINTERS" << std::endl;
		if (dwPrinterCount > 0)
		{
			for (UINT i = 0; i < dwPrinterCount; i++)
			{
				// Print the name of each printer.
				std::wcout << lpPrinterData[i].pPrinterName << std::endl;
			}
		}
		std::wcout << L"END LIST PRINTERS" << std::endl;
		std::wcout << std::endl;
		std::wcout << std::endl;

	}
	// Free the memory allocated to the buffer.
	HeapFree(hProcessHeap, 0, lpPrinterData);

	return 0;
}


int main()
{
	fwprintf_s(logger.GetFile(), L"%s", L"[Target] Top of main().\n");
	
	// Print username using GetUserName	
	PrintUsername();
	
	// Print printers using EnumPrinters
	PrintPrinters();

	int ch;
	std::wcout << L"Type 'U' for username, 'P' for printers, or 'Q' to quit.";
	do
	{
		ch = _getch();
		ch = toupper(ch);
		if (ch == 'U')
		{
			PrintUsername();
		}
		else if (ch == 'P')
		{
			PrintPrinters();
		}
	} while (ch != 'Q');
	
	return 0;
}