﻿/*
Copyright 2025 Taichi Murakami.
スタートアップ ダイアログ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"

EXTERN_C
INT_PTR CALLBACK StartupDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	INT_PTR nResult;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		nResult = TRUE;
		break;
	default:
		nResult = FALSE;
		break;
	}

	return nResult;
}

