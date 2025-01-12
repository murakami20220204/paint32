/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を公開します。
*/

#pragma once
#include <windows.h>
#include <wincodec.h>

/* Window Classes */
#define PICTURECLASSNAME        TEXT("Picture")
#define VIEWERCLASSNAME         TEXT("Viewer32")

/* Window Extra */
#ifdef _WIN64
#define PICTUREWINDOWEXTRA      0
#define VIEWERWINDOWEXTRA       72
#else
#define PICTUREWINDOWEXTRA      0
#define VIEWERWINDOWEXTRA       48
#endif

/* Application Window Messages */
#define WM_ABOUT                0x0FFC
#define WM_DECODE               0x0FFD
#define WM_LOAD                 0x0FFE
#define WM_OPEN                 0x0FFF
#define WM_SETFRAMEINDEX        0x0FF7
#define WM_SETZOOM              0x0FFB
#define WM_STATUSWINDOW         0x0FF9
#define WM_TOOLBARWINDOW        0x0FF8
#define WM_ZOOM                 0x0FFA

/* External Functions */
EXTERN_C int WINAPI ErrorMessageBox(_In_opt_ HINSTANCE hInstance, _In_opt_ HWND hWnd, _In_ DWORD dwError);
EXTERN_C BOOL WINAPI SetWindowPosOnCenter(_In_ HWND hWnd);

/* Window Procedures */
EXTERN_C LRESULT CALLBACK PictureWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
EXTERN_C LRESULT CALLBACK ViewerWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

/* Dialog Procedures */
EXTERN_C INT_PTR CALLBACK AboutDialogProc(_In_ HWND hDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
