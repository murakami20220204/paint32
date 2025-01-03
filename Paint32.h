/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を公開します。
*/

#pragma once
#include <windows.h>
#define APPLICATIONCLASSNAME    TEXT("Application")
#define DOCUMENTCLASSNAME       TEXT("Document")
#define MDICLIENTCLASSNAME      TEXT("MDIClient")
#define OUTLINECLASSNAME        TEXT("Outline")
#define PALETTECLASSNAME        TEXT("Palette")
#define CUSTOMCOLORSLENGTH      16
#define WM_USERAPP 0x1000

#ifdef _WIN64
#define APPLICATIONWINDOWEXTRA  40
#define DOCUMENTWINDOWEXTRA     0
#define FRAMEWINDOWEXTRA        24
#define OUTLINEWINDOWEXTRA      8
#define PALETTEWINDOWEXTRA      16
#else
#define APPLICATIONWINDOWEXTRA  20
#define DOCUMENTWINDOWEXTRA     0
#define FRAMEWINDOWEXTRA        12
#define OUTLINEWINDOWEXTRA      4
#define PALETTEWINDOWEXTRA      12
#endif

enum APPLICATION_WINDOW_MESSAGE
{
	APPLICATION_ABOUT = 0x1000,
	APPLICATION_ISDIALOGMESSAGE,
	APPLICATION_NEW,
	APPLICATION_OUTLINE,
	APPLICATION_PALETTE,
	APPLICATION_STATUS,
	APPLICATION_TOOLBAR,
	APPLICATION_PREFERENCES,
};

typedef struct tagPALETTECREATESTRUCT
{
	WORD wLayout;
	WORD wID;
	WORD wIDColor;
	WORD wIDHistory;
	WORD wIDFavorites;
} PALETTECREATESTRUCT, FAR *LPPALETTECREATESTRUCT;

typedef struct tagOUTLINECREATESTRUCT
{
	WORD wID;
} OUTLINECREATESTRUCT, FAR *LPOUTLINECREATESTRUCT;

EXTERN_C
INT_PTR CALLBACK AboutDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
INT_PTR CALLBACK AutosaveDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
INT_PTR CALLBACK ColorDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
INT_PTR CALLBACK DialDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
INT_PTR CALLBACK NewDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
INT_PTR CALLBACK StartupDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
LRESULT CALLBACK ApplicationWindowProc(
	_In_ HWND hWnd,
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
LRESULT CALLBACK OutlineWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
LRESULT CALLBACK PaletteWindowProc(
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
BOOL WINAPI SetWindowPosOnCenter(
	_In_ HWND hWnd);

EXTERN_C
BOOL WINAPI SetWindowPosOnSize(
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ const RECT *lpRect);
