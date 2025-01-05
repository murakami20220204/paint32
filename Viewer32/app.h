/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を公開します。
*/

#pragma once
#include <windows.h>
#define APPLICATIONCLASSNAME    TEXT("Viewer32")
#define WM_USERAPP              0x1000

#ifdef _WIN64
#define APPLICATIONWINDOWEXTRA  24
#else
#define APPLICATIONWINDOWEXTRA  12
#endif

enum APPLICATION_WINDOW_MESSAGE
{
	APPLICATION_ABOUT = WM_USERAPP,
	APPLICATION_ISDIALOGMESSAGE,
	APPLICATION_OPEN,
	APPLICATION_STATUS,
	APPLICATION_TOOLBAR,
};

/* Window Procedures */
EXTERN_C LRESULT CALLBACK ApplicationWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

/* Dialog Procedures */
EXTERN_C INT_PTR CALLBACK AboutDialogProc(_In_ HWND hDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
