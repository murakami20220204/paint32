/*
Copyright 2025 Taichi Murakami.
バージョン情報ダイアログ プロシージャを実装します。
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Paint32.h"

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
		EndDialog(hDlg, 0);
		bResult = FALSE;
	case WM_COMMAND:
		EndDialog(hDlg, LOWORD(wParam));
		bResult = FALSE;
		break;
	case WM_INITDIALOG:
		SetWindowPosOnCenter(hDlg);
		bResult = TRUE;
		break;
	default:
		bResult = FALSE;
		break;
	}

	return bResult;
}
