/*
Copyright 2025 Taichi Murakami.
バージョン情報ダイアログ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define VERSIONINFO_FORMAT      TEXT("\\StringFileInfo\\%04X%04X\\ProductVersion")
#define VERSIONINFO_TRANSLATION TEXT("\\VarFileInfo\\Translation")
static BOOL WINAPI LoadVersion(_In_ HWND hDlg);

/*
バージョン情報ダイアログ プロシージャ。
*/
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

/*
現在のインスタンスに埋め込まれたリソースからバージョン情報を取得します。
バージョン情報は子コントロールのテキストとして設定されます。
*/
static
BOOL WINAPI LoadVersion(
	_In_ HWND hDlg)
{
	HANDLE hHeap;
	HINSTANCE hInstance;
	LPWORD lpBlock;
	LPTSTR lpFileName;
	LPVOID lpVersionInfo;
	DWORD cbVersionInfo;
	UINT cbBlock;
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
				VerQueryValue(lpVersionInfo, VERSIONINFO_TRANSLATION, &lpBlock, &cbBlock) &&
				(cbBlock >= sizeof(DWORD)) &&
				SUCCEEDED(StringCchPrintf(lpFileName, PATHCCH_MAX_CCH, VERSIONINFO_FORMAT, lpBlock[0], lpBlock[1])) &&
				VerQueryValue(lpVersionInfo, lpFileName, &lpBlock, &cbBlock) &&
				cbBlock)
			{
				bResult = SetDlgItemText(hDlg, IDC_VERSION, (LPCTSTR)lpBlock);
			}

			HeapFree(hHeap, 0, lpVersionInfo);
		}

		HeapFree(hHeap, 0, lpFileName);
	}

	return bResult;
}
