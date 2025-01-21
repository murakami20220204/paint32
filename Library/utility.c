/*
Copyright 2025 Taichi Murakami.
各プロジェクトに共通する処理を実装します。
*/

#define WIN32_LEAN_AND_MEAN
#define MAXLOADSTRING 32
#define IDS_APPTITLE 101

#include <windows.h>

#define FORMAT_MESSAGE_ERROR (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM)
#define SWP_ONCENTER (SWP_NOSIZE | SWP_NOZORDER)

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
	TCHAR strCaption[MAXLOADSTRING];

	if (LoadString(hInstance, IDS_APPTITLE, strCaption, MAXLOADSTRING))
	{
		lpCaption = strCaption;
	}
	else
	{
		lpCaption = NULL;
	}
	if (FormatMessage(FORMAT_MESSAGE_ERROR, NULL, dwError, 0, (LPTSTR)&lpText, 0, NULL))
	{
		nResult = MessageBox(hWnd, lpText, lpCaption, MB_ICONERROR);
	}

	LocalFree(lpText);
	return nResult;
}

/*
指定した位置へウィンドウを移動します。
*/
EXTERN_C
BOOL WINAPI MoveWindowForRect(
	_In_ HWND hWnd,
	_In_ CONST RECT FAR *lpRect,
	_In_ BOOL bRepaint)
{
	const int x = lpRect->left;
	const int y = lpRect->top;
	const int cx = lpRect->right - x;
	const int cy = lpRect->bottom - y;
	return MoveWindow(hWnd, x, y, cx, cy, bRepaint);
}

EXTERN_C
BOOL WINAPI SetMinMaxInfoForDpi(
	_Inout_ LPMINMAXINFO lpInfo,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_ UINT uDpi)
{
	nWidth = MulDiv(nWidth, uDpi, USER_DEFAULT_SCREEN_DPI);
	nHeight = MulDiv(nHeight, uDpi, USER_DEFAULT_SCREEN_DPI);
	lpInfo->ptMinTrackSize.x = max(lpInfo->ptMinTrackSize.x, nWidth);
	lpInfo->ptMinTrackSize.y = max(lpInfo->ptMinTrackSize.y, nHeight);
	return uDpi;
}

/*
親ウィンドウの中央へ指定したウィンドウを配置します。
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
		bResult = SetWindowPos(hWnd, NULL, rcWnd.left, rcWnd.top, 0, 0, SWP_ONCENTER);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}
