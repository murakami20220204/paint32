/*
Copyright 2025 Taichi Murakami.
バージョン情報ダイアログ プロシージャを実装します。
*/

#include "stdafx.h"
#include "resource.h"
#include "shared.h"
#define LOADSTRING_MAX 32

static BOOL WINAPI Initialize(_In_ HWND hDlg);

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
		Initialize(hDlg);
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
BOOL WINAPI Initialize(
	_In_ HWND hDlg)
{
	HINSTANCE hInstance;
	BOOL bResult;
	TCHAR strText[LOADSTRING_MAX];
	hInstance = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);
	bResult = GetVersionFromResource(hInstance, strText, LOADSTRING_MAX);

	if (bResult)
	{
		SetDlgItemText(hDlg, IDC_VERSION, strText);
	}

	return bResult;
}
