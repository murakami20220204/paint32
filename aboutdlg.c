/*
Copyright 2024 Taichi Murakami.
About ダイアログ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define FORMAT TEXT("%s\r\nVersion %s\r\n%s")
#define SUBBLOCK(Text) (TEXT("\\StringFileInfo\\041104B0\\") TEXT(Text))
#define SUBBLOCK_COPYRIGHT SUBBLOCK("LegalCopyright")
#define SUBBLOCK_PRODUCTNAME SUBBLOCK("ProductName")
#define SUBBLOCK_PRODUCTVERSION SUBBLOCK("ProductVersion")

static
BOOL WINAPI LoadVersion(
	_In_ HWND hDlg);

EXTERN_C
INT_PTR CALLBACK AboutDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	INT_PTR bResult;

	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_COMMAND:
		EndDialog(hDlg, 0);
		bResult = FALSE;
		break;
	case WM_INITDIALOG:
		LoadVersion(hDlg);
		SetWindowPosOnCenter(hDlg);
		bResult = TRUE;
		break;
	default:
		bResult = FALSE;
		break;
	}

	return bResult;
}

static
BOOL WINAPI LoadVersion(
	_In_ HWND hDlg)
{
	HANDLE hHeap;
	HINSTANCE hInstance;
	LPTSTR lpCopyright, lpFileName, lpProductName, lpProductVersion;
	LPVOID lpVersionInfo;
	DWORD cbVersionInfo;
	UINT uLength;
	BOOL bResult = FALSE;

	if ((hInstance = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE)) &&
		(hHeap = GetProcessHeap()) &&
		(lpFileName = HeapAlloc(hHeap, 0, PATHCCH_MAX_CCH * sizeof(TCHAR))))
	{
		if (GetModuleFileName(hInstance, lpFileName, PATHCCH_MAX_CCH) &&
			(cbVersionInfo = GetFileVersionInfoSize(lpFileName, NULL)) &&
			(lpVersionInfo = HeapAlloc(hHeap, 0, cbVersionInfo)))
		{
			if (GetFileVersionInfo(lpFileName, 0, cbVersionInfo, lpVersionInfo) &&
				VerQueryValue(lpVersionInfo, SUBBLOCK_PRODUCTNAME, &lpProductName, &uLength) &&
				VerQueryValue(lpVersionInfo, SUBBLOCK_PRODUCTVERSION, &lpProductVersion, &uLength) &&
				VerQueryValue(lpVersionInfo, SUBBLOCK_COPYRIGHT, &lpCopyright, &uLength) &&
				SUCCEEDED(StringCchPrintf(lpFileName, PATHCCH_MAX_CCH, FORMAT, lpProductName, lpProductVersion, lpCopyright)))
			{
				bResult = SetDlgItemText(hDlg, IDC_CONTENT, lpFileName);
			}

			HeapFree(hHeap, 0, lpVersionInfo);
		}

		HeapFree(hHeap, 0, lpFileName);
	}

	return bResult;
}
