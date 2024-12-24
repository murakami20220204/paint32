/*
Copyright 2024 Taichi Murakami.
外部リンケージを持つ関数を公開します。
*/

#pragma once
#include <windows.h>
#define DOCUMENTCLASSNAME       TEXT("Document")
#define FRAMECLASSNAME          TEXT("Frame")
#define MDICLIENTCLASSNAME      TEXT("MDIClient")
#define WM_USERAPP 0x1000

#ifdef _WIN64
#define DOCUMENTWINDOWEXTRA     0
#define FRAMEWINDOWEXTRA        8
#else
#define DOCUMENTWINDOWEXTRA     0
#define FRAMEWINDOWEXTRA        4
#endif

enum FRAME_WINDOW_MESSAGE
{
	FRAME_ABOUT = WM_USERAPP,
	FRAME_CLOSEDOCUMENT,
	FRAME_GETDIALOG,
	FRAME_GETMDICLIENT,
	FRAME_OPEN,
};

EXTERN_C
INT_PTR CALLBACK AboutDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
LRESULT CALLBACK DocumentWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
int WINAPI ErrorMessageBox(
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ HWND hWnd,
	_In_ DWORD dwError);

EXTERN_C
LRESULT CALLBACK FrameWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
BOOL WINAPI SetWindowPosOnCenter(
	_In_ HWND hWnd);

EXTERN_C
BOOL WINAPI SetWindowPosOnSize(
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ const RECT *lpRect);
