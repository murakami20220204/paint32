/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を実装します。
*/

#include "stdafx.h"
#include "resource.h"
#define ERRORMESSAGEBOXFLAGS (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM)
#define LOADSTRING_MAX 32
#define VERSIONINFO_FORMAT      TEXT("\\StringFileInfo\\%04X%04X\\ProductVersion")
#define VERSIONINFO_TRANSLATION TEXT("\\VarFileInfo\\Translation")

/*
指定したエラーを説明するメッセージ ボックスを表示します。
*/
EXTERN_C
int WINAPI ErrorMessageBox(
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ HWND hWnd,
	_In_ DWORD dwError)
{
	LPTSTR lpCaption, lpText = NULL;
	int nResult = 0;
	TCHAR strCaption[LOADSTRING_MAX];

	if (LoadString(hInstance, IDS_APPTITLE, strCaption, LOADSTRING_MAX))
	{
		lpCaption = strCaption;
	}
	else
	{
		lpCaption = NULL;
	}
	if (FormatMessage(ERRORMESSAGEBOXFLAGS, NULL, dwError, 0, (LPTSTR)&lpText, 0, NULL))
	{
		nResult = MessageBox(hWnd, lpText, lpCaption, MB_ICONERROR);
	}

	LocalFree(lpText);
	return nResult;
}

#if 1
EXTERN_C
BOOL WINAPI GetVersionFromResource(
	_In_opt_ HINSTANCE hInstance,
	_Out_writes_z_(cchBuffer) LPTSTR lpBuffer,
	_In_ UINT cchBuffer)
{
	HANDLE hHeap;
	LPVOID lpBlock;
	LPTSTR lpFileName;
	LPVOID lpVersionInfo;
	DWORD cbVersionInfo;
	UINT cbBlock;
	BOOL bResult = FALSE;

	if (cchBuffer)
	{
		*lpBuffer = 0;
		hHeap = GetProcessHeap();
		lpFileName = HeapAlloc(hHeap, 0, PATHCCH_MAX_CCH * sizeof(TCHAR));

		if (lpFileName)
		{
			if (GetModuleFileName(hInstance, lpFileName, PATHCCH_MAX_CCH) &&
				(cbVersionInfo = GetFileVersionInfoSize(lpFileName, NULL)) &&
				(lpVersionInfo = HeapAlloc(hHeap, 0, cbVersionInfo)))
			{
				bResult =
					GetFileVersionInfo(lpFileName, 0, cbVersionInfo, lpVersionInfo) &&
					VerQueryValue(lpVersionInfo, VERSIONINFO_TRANSLATION, &lpBlock, &cbBlock) &&
					(cbBlock >= sizeof(DWORD)) &&
					SUCCEEDED(StringCchPrintf(lpFileName, PATHCCH_MAX_CCH, VERSIONINFO_FORMAT, ((LPWORD)lpBlock)[0], ((LPWORD)lpBlock)[1])) &&
					VerQueryValue(lpVersionInfo, lpFileName, &lpBlock, &cbBlock) &&
					(cbBlock >= sizeof(LPCTSTR)) &&
					SUCCEEDED(StringCchCopy(lpBuffer, cchBuffer, (LPCTSTR)lpBlock));
				HeapFree(hHeap, 0, lpVersionInfo);
			}

			HeapFree(hHeap, 0, lpFileName);
		}
	}

	return bResult;
}
#endif

/*
指定したウィンドウを中央揃えにします。
*/
EXTERN_C
BOOL WINAPI SetWindowPosOnCenter(
	_In_ HWND hWnd)
{
	HWND hWndParent;
	RECT rcWnd, rcWndParent;
	BOOL bResult;
	hWndParent = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPARENT);

	if (hWndParent && GetWindowRect(hWnd, &rcWnd) && GetWindowRect(hWndParent, &rcWndParent))
	{
		rcWnd.right -= rcWnd.left;
		rcWnd.bottom -= rcWnd.top;
		rcWndParent.right -= rcWndParent.left;
		rcWndParent.bottom -= rcWndParent.top;
		rcWnd.left = rcWndParent.left + (rcWndParent.right - rcWnd.right) / 2;
		rcWnd.top = rcWndParent.top + (rcWndParent.bottom - rcWnd.bottom) / 2;
		bResult = SetWindowPos(hWnd, NULL, rcWnd.left, rcWnd.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}
