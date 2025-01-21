/*
Copyright 2025 Taichi Murakami.
Paint プロジェクト内で外部リンケージを持つ関数を公開します。
*/

#pragma once
#include "resource.h"
#include <windows.h>

#define DOCUMENTCLASSNAME       TEXT("Document")
#define MDICLIENTCLASSNAME      TEXT("MDIClient")
#define FRAMECLASSNAME          TEXT("Paint")

#ifdef _WIN64
#define CANVASWINDOWEXTRA       0
#define DOCUMENTWINDOWEXTRA     80
#define FRAMEWINDOWEXTRA        56
#else
#define CANVASWINDOWEXTRA       0
#define DOCUMENTWINDOWEXTRA     36
#define FRAMEWINDOWEXTRA        40
#endif

typedef struct tagDOCUMENTCREATESTRUCT
{
	HWND hWndFrame;
} DOCUMENTCREATESTRUCT, FAR *LPDOCUMENTCREATESTRUCT;

EXTERN_C int WINAPI ErrorMessageBox(_In_opt_ HINSTANCE hInstance, _In_opt_ HWND hWnd, _In_ DWORD dwError);
EXTERN_C BOOL WINAPI MoveWindowForRect(_In_ HWND hWnd, _In_ CONST RECT FAR *lpRect, _In_ BOOL bRepaint);
EXTERN_C BOOL WINAPI SetMinMaxInfoForDpi(_Inout_ LPMINMAXINFO lpInfo, _In_ int nWidth, _In_ int nHeight, _In_ UINT uDpi);
EXTERN_C BOOL WINAPI SetWindowPosOnCenter(_In_ HWND hWnd);
EXTERN_C LRESULT CALLBACK DocumentWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
EXTERN_C LRESULT CALLBACK FrameWindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
EXTERN_C INT_PTR CALLBACK AboutDialogProc(_In_ HWND hDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
