/*
Copyright 2025 Taichi Murakami.
色ダイアログ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"

typedef INT_PTR(WINAPI FAR *DEFPROC)(HWND, UINT, WPARAM, LPARAM);

static
INT_PTR WINAPI OnClose(
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
INT_PTR CALLBACK ColorDialogProc(
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
	DestroyWindow(hDlg);
	return FALSE;
}

static
INT_PTR WINAPI OnInitDialog(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	SetWindowLongPtr(hDlg, DWLP_USER, lParam);
	return TRUE;
}
