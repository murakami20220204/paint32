/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を公開します。
*/

#pragma once
#include <windows.h>

/* Window Classes */
#define DOCUMENTCLASSNAME       TEXT("Document")
#define OUTLINECLASSNAME        TEXT("Outline")
#define PAINTCLASSNAME          TEXT("Paint32")
#define PALETTECLASSNAME        TEXT("Palette")
#define CUSTOMCOLORSLENGTH      16

/* Window Extra */
#ifdef _WIN64
#define DOCUMENTWINDOWEXTRA     0
#define OUTLINEWINDOWEXTRA      0
#define PAINTWINDOWEXTRA        48
#define PALETTEWINDOWEXTRA      0
#else
#define DOCUMENTWINDOWEXTRA     0
#define OUTLINEWINDOWEXTRA      0
#define PAINTWINDOWEXTRA        28
#define PALETTEWINDOWEXTRA      0
#endif

/* Application Window Messages */
#define WM_ABOUT                0x0FFF
#define WM_NEW                  0x0FFE
#define WM_OUTLINEWINDOW        0x0FFC
#define WM_PALETTEWINDOW        0x0FFB
#define WM_PREFERENCES          0x0FF8
#define WM_STATUSWINDOW         0x0FFA
#define WM_TOOLBARWINDOW        0x0FF9
#define WM_TRANSLATEACCELERATOR 0x0FFD

/* External Functions */
EXTERN_C int WINAPI ErrorMessageBox(_In_opt_ HINSTANCE hInstance, _In_opt_ HWND hWnd, _In_ DWORD dwError);
EXTERN_C BOOL WINAPI SetWindowPosOnCenter(_In_ HWND hWnd);

/* Window Procedures */
EXTERN_C LRESULT CALLBACK DocumentWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
EXTERN_C LRESULT CALLBACK OutlineWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
EXTERN_C LRESULT CALLBACK PaintWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
EXTERN_C LRESULT CALLBACK PaletteWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

/* Dialog Procedures */
EXTERN_C INT_PTR CALLBACK AboutDialogProc(_In_ HWND hDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
