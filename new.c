/*
Copyright 2025 Taichi Murakami.
新規ダイアログ プロシージャを実装します。
*/

#include "stdafx.h"
#include "Paint32.h"

typedef INT_PTR(WINAPI FAR *DEFPROC)(HWND, UINT, WPARAM, LPARAM);

static
INT_PTR WINAPI OnClose(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
INT_PTR WINAPI OnCommand(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
INT_PTR WINAPI OnInitDialog(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
INT_PTR CALLBACK NewDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	DEFPROC lpProc;

	switch (uMsg)
	{
	case WM_CLOSE:
		lpProc = OnClose;
		break;
	case WM_COMMAND:
		lpProc = OnCommand;
		break;
	case WM_INITDIALOG:
		lpProc = OnInitDialog;
		break;
	default:
		lpProc = NULL;
		break;
	}

	return lpProc ? lpProc(hDlg, uMsg, wParam, lParam) : FALSE;
}

static
INT_PTR WINAPI OnClose(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	EndDialog(hDlg, 0);
	return FALSE;
}

static
INT_PTR WINAPI OnCommand(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDCANCEL:
		EndDialog(hDlg, IDCANCEL);
		break;
	case IDOK:
		EndDialog(hDlg, IDOK);
		break;
	}

	return FALSE;
}

static
INT_PTR WINAPI OnInitDialog(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	SetWindowPosOnCenter(hDlg);
	return TRUE;
}
