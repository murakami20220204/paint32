/*
Copyright 2025 Taichi Murakami.
アプリケーション ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "app.h"
#include "resource.h"

/* User Data */
typedef struct tagUSERDATA
{
	COLORREF rgbHistoryColors[CUSTOMCOLORSLENGTH];
	COLORREF rgbFavoriteColors[CUSTOMCOLORSLENGTH];
	DWORD dwFlags;
	WORD wOutlineDock;
	WORD wPaletteDock;
} USERDATA, FAR *LPUSERDATA;

/* Window Extra */
typedef struct tagWINDOWEXTRA
{
	LONG_PTR lpClientWindow;
	LONG_PTR lpOutlineWindow;
	LONG_PTR lpPaletteWindow;
	LONG_PTR lpStatusBar;
	LONG_PTR lpToolbar;
} WINDOWEXTRA;

#define GWLP_HWNDCLIENT         offsetof(WINDOWEXTRA, lpClientWindow)
#define GWLP_HWNDOUTLINE        offsetof(WINDOWEXTRA, lpOutlineWindow)
#define GWLP_HWNDPALETTE        offsetof(WINDOWEXTRA, lpPaletteWindow)
#define GWLP_HWNDSTATUS         offsetof(WINDOWEXTRA, lpStatusBar)
#define GWLP_HWNDTOOLBAR        offsetof(WINDOWEXTRA, lpToolbar)
#define ID_FIRSTCHILD           1
#define ID_HWNDCLIENT           1
#define ID_HWNDOUTLINE          4
#define ID_HWNDPALETTE          5
#define ID_HWNDSTATUS           2
#define ID_HWNDTOOLBAR          3
#define ID_WINDOWMENU           4
#define LOADSTRING_MAX          32
#define MINTRACKSIZE_X          300
#define MINTRACKSIZE_Y          200
#define USERDATA_STATUS         0x0001
#define USERDATA_TOOLBAR        0x0002
#define USERDATA_OUTLINE        0x0004
#define USERDATA_PALETTE        0x0008
#define WS_HWNDCLIENT           (WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDOUTLINE          (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
#define WS_HWNDPALETTE          (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
#define WS_HWNDSTATUS           (WS_CHILD | SBARS_SIZEGRIP | CCS_NODIVIDER)
#define WS_HWNDTOOLBAR          (WS_CHILD | TBSTYLE_FLAT | CCS_NODIVIDER)

static_assert(sizeof(WINDOWEXTRA) == APPLICATIONWINDOWEXTRA, "APPLICATIONWINDOWEXTRA");
typedef HWND(WINAPI FAR *CREATECHILDPROC)(HWND, HINSTANCE);
typedef LRESULT(WINAPI FAR *DEFPROC)(HWND, UINT, WPARAM, LPARAM);
static LRESULT WINAPI DefProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnAbout(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnCommand(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnCreate(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnDestroy(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

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
	DEFPROC lpProc;

	switch (uMsg)
	{
	case APPLICATION_ABOUT:
		lpProc = OnAbout;
		break;
	case WM_COMMAND:
		lpProc = OnCommand;
		break;
	case WM_CREATE:
		lpProc = OnCreate;
		break;
	case WM_DESTROY:
		lpProc = OnDestroy;
		break;
	default:
		lpProc = DefProc;
		break;
	}

	return lpProc(hWnd, uMsg, wParam, lParam);
}

/*
現在の MDI クライアントを用いて MDI ウィンドウ プロシージャを呼び出します。
*/
static
LRESULT WINAPI DefProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	return DefFrameProc(hWnd, (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT), uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnAbout(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
	return 0;
}

static
LRESULT WINAPI OnCommand(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult = 0;

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

	return nResult;
}

static
LRESULT WINAPI OnCreate(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnDestroy(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	PostQuitMessage(0);
	return DefProc(hWnd, uMsg, wParam, lParam);
}
