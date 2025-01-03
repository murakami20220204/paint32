/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define ERRORMESSAGEBOXFLAGS (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM)
#define LOADSTRING_MAX 32

/*
指定したエラーを説明するメッセージ ボックスを表示します。
*/
EXTERN_C
int WINAPI ErrorMessageBox(
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ HWND hWnd,
	_In_ DWORD dwError)
{
	LPTSTR lpText = NULL;
	int nResult = 0;
	TCHAR strCaption[LOADSTRING_MAX];
	LoadString(hInstance, IDS_APPTITLE, strCaption, LOADSTRING_MAX);

	if (FormatMessage(ERRORMESSAGEBOXFLAGS, NULL, dwError, 0, (LPTSTR)&lpText, 0, NULL))
	{
		nResult = MessageBox(hWnd, lpText, strCaption, MB_ICONERROR);
	}

	LocalFree(lpText);
	return nResult;
}

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
