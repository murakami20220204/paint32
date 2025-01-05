/*
Copyright 2025 Taichi Murakami.
アプリケーション ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "app.h"
#include "resource.h"

/* Window Extra */
typedef struct tagWINDOWEXTRA
{
	LONG_PTR lpPictureBox;
	LONG_PTR lpStatusBar;
	LONG_PTR lpToolbar;
} WINDOWEXTRA;

#define DefProc DefWindowProc
#define GWLP_HWNDPICTURE        offsetof(WINDOWEXTRA, lpPictureBox)
#define GWLP_HWNDSTATUS         offsetof(WINDOWEXTRA, lpStatusBar)
#define GWLP_HWNDTOOLBAR        offsetof(WINDOWEXTRA, lpToolbar)
#define ID_HWNDPICTURE          1
#define ID_HWNDSTATUS           2
#define ID_HWNDTOOLBAR          3
#define WS_HWNDCLIENT           (WS_CHILD | WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDSTATUS           (WS_CHILD | SBARS_SIZEGRIP | CCS_NODIVIDER)
#define WS_HWNDTOOLBAR          (WS_CHILD | TBSTYLE_FLAT | CCS_NODIVIDER)

static_assert(sizeof(WINDOWEXTRA) == APPLICATIONWINDOWEXTRA, "APPLICATIONWINDOWEXTRA");
static LRESULT WINAPI OnAbout(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnCommand(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnCreate(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnDestroy(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static BOOL WINAPI ShowAboutDialog(_In_ HWND hWnd);

/*
アプリケーション ウィンドウ プロシージャ。
*/
EXTERN_C
LRESULT CALLBACK ApplicationWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult = 0;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_ABOUT:
			PostMessage(hWnd, APPLICATION_ABOUT, 0, 0);
			break;
		case IDM_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		default:
			nResult = DefProc(hWnd, uMsg, wParam, lParam);
			break;
		}

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case APPLICATION_ABOUT:
		ShowAboutDialog(hWnd);
		break;
	default:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return nResult;
}

static
BOOL WINAPI ShowAboutDialog(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
	return FALSE;
}
