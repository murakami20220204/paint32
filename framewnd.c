/*
Copyright 2025 Taichi Murakami.
Application ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define ID_FIRSTCHILD   1
#define ID_HWNDCLIENT   1
#define ID_HWNDOUTLINE  2
#define ID_HWNDPALETTE  3
#define ID_WINDOWMENU   4
#define LOADSTRING_MAX  32
#define MINTRACKSIZE_X  300
#define MINTRACKSIZE_Y  200
#define WS_HWNDCLIENT   (WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDOUTLINE  (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
#define WS_HWNDPALETTE  (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)

typedef struct tagFRAMEWINDOWEXTRA
{
	LONG_PTR hWndClient;
	LONG_PTR hWndOutline;
	LONG_PTR hWndPalette;
} WINDOWEXTRA;

static_assert(sizeof(WINDOWEXTRA) == FRAMEWINDOWEXTRA, "FRAMEWINDOWEXTRA");
#define GWLP_HWNDCLIENT         offsetof(WINDOWEXTRA, hWndClient)
#define GWLP_HWNDOUTLINE        offsetof(WINDOWEXTRA, hWndOutline)
#define GWLP_HWNDPALETTE        offsetof(WINDOWEXTRA, hWndPalette)

static
HWND WINAPI CreateClientWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance);

static
HWND WINAPI CreateOutlineWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance);

static
HWND WINAPI CreatePaletteWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance);

static
LRESULT WINAPI DefProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnAbout(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnCommand(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnCreate(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnDestroy(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnGetMinMaxInfo(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnNew(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnOutline(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnPalette(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnParentNotify(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

static
LRESULT WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
LRESULT CALLBACK FrameWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	FARPROC lpProc;

	switch (uMsg)
	{
	case WM_COMMAND:
		lpProc = OnCommand;
		break;
	case WM_CREATE:
		lpProc = OnCreate;
		break;
	case WM_DESTROY:
		lpProc = OnDestroy;
		break;
	case WM_GETMINMAXINFO:
		lpProc = OnGetMinMaxInfo;
		break;
	case WM_PARENTNOTIFY:
		lpProc = OnParentNotify;
		break;
	case WM_SIZE:
		lpProc = OnSize;
		break;
	case FRAME_ABOUT:
		lpProc = OnAbout;
		break;
	case FRAME_NEW:
		lpProc = OnNew;
		break;
	case FRAME_OUTLINE:
		lpProc = OnOutline;
		break;
	case FRAME_PALETTE:
		lpProc = OnPalette;
		break;
	default:
		lpProc = DefProc;
		break;
	}

	return lpProc(hWnd, uMsg, wParam, lParam);
}

static
HWND WINAPI CreateClientWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndClient;
	CLIENTCREATESTRUCT param = { NULL, ID_FIRSTCHILD };
	param.hWindowMenu = GetMenu(hWnd);
	param.hWindowMenu = param.hWindowMenu ? GetSubMenu(param.hWindowMenu, ID_WINDOWMENU) : NULL;
	hWndClient = CreateWindowEx(WS_EX_CLIENTEDGE, MDICLIENTCLASSNAME, NULL, WS_HWNDCLIENT, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDCLIENT, hInstance, &param);

	if (hWndClient)
	{
		assert(!SetWindowLongPtr(hWnd, GWLP_HWNDCLIENT, (LONG_PTR)hWndClient));
	}

	return hWndClient;
}

static
HWND WINAPI CreateOutlineWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndOutline;
	OUTLINECREATESTRUCT param;
	TCHAR strCaption[LOADSTRING_MAX];
	hWndOutline = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDOUTLINE);

	if (!hWndOutline)
	{
		param.wID = ID_HWNDOUTLINE;
		LoadString(hInstance, IDS_LAYER, strCaption, LOADSTRING_MAX);
		hWndOutline = CreateWindow(OUTLINECLASSNAME, strCaption, WS_HWNDOUTLINE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWnd, NULL, hInstance, &param);

		if (hWndOutline)
		{
			SetWindowLongPtr(hWnd, GWLP_HWNDOUTLINE, (LONG_PTR)hWndOutline);
			ShowWindow(hWndOutline, SW_SHOW);
		}
	}

	return hWndOutline;
}

static
HWND WINAPI CreatePaletteWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndPalette;
	PALETTECREATESTRUCT param;
	TCHAR strCaption[LOADSTRING_MAX];
	hWndPalette = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPALETTE);

	if (!hWndPalette)
	{
		ZeroMemory(&param, sizeof param);
		param.wID = ID_HWNDPALETTE;
		LoadString(hInstance, IDS_PALETTE, strCaption, LOADSTRING_MAX);
		hWndPalette = CreateWindow(PALETTECLASSNAME, strCaption, WS_HWNDPALETTE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWnd, NULL, hInstance, &param);

		if (hWndPalette)
		{
			SetWindowLongPtr(hWnd, GWLP_HWNDPALETTE, (LONG_PTR)hWndPalette);
			ShowWindow(hWndPalette, SW_SHOW);
		}
	}

	return hWndPalette;
}

static
LRESULT WINAPI DefProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HWND hWndClient = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT);
	return DefFrameProc(hWnd, hWndClient, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnAbout(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
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
	FARPROC lpProc;

	switch (LOWORD(wParam))
	{
	case IDM_ABOUT:
		lpProc = SendMessage;
		uMsg = FRAME_ABOUT;
		wParam = 0;
		lParam = 0;
		break;
	case IDM_EXIT:
		lpProc = SendMessage;
		uMsg = WM_CLOSE;
		wParam = 0;
		lParam = 0;
		break;
	case IDM_LAYER:
		lpProc = SendMessage;
		uMsg = FRAME_OUTLINE;
		wParam = 0;
		lParam = 0;
		break;
	case IDM_NEW:
		lpProc = SendMessage;
		uMsg = FRAME_NEW;
		wParam = 0;
		lParam = 0;
		break;
	case IDM_PALETTE:
		lpProc = SendMessage;
		uMsg = FRAME_PALETTE;
		wParam = 0;
		lParam = 0;
		break;
	default:
		lpProc = DefProc;
		break;
	}

	return lpProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnCreate(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
	LRESULT nResult;

	if (CreateClientWindow(hWnd, hInstance))
	{
		nResult = DefProc(hWnd, uMsg, wParam, lParam);;
	}
	else
	{
		nResult = -1;
	}

	return nResult;
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

static
LRESULT WINAPI OnGetMinMaxInfo(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	const POINT ptSize = { MINTRACKSIZE_X, MINTRACKSIZE_Y };
	CopyMemory(&((LPMINMAXINFO)lParam)->ptMinTrackSize, &ptSize, sizeof ptSize);
	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnNew(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEW), hWnd, NewDialogProc);
	return 0;
}

static
LRESULT WINAPI OnOutline(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HWND hWndOutline;
	hWndOutline = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDOUTLINE);

	if (hWndOutline)
	{
		SendMessage(hWndOutline, WM_CLOSE, 0, 0);
	}
	else
	{
		hWndOutline = CreateOutlineWindow(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	}

	return (LRESULT)hWndOutline;
}

static
LRESULT WINAPI OnPalette(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HWND hWndPalette;
	hWndPalette = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPALETTE);

	if (hWndPalette)
	{
		SendMessage(hWndPalette, WM_CLOSE, 0, 0);
	}
	else
	{
		hWndPalette = CreatePaletteWindow(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	}

	return (LRESULT)hWndPalette;
}

static
LRESULT WINAPI OnParentNotify(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult = FALSE;

	switch (LOWORD(wParam))
	{
	case WM_DESTROY:
		switch (HIWORD(wParam))
		{
		case ID_HWNDOUTLINE:
			SetWindowLongPtr(hWnd, GWLP_HWNDOUTLINE, 0);
			nResult = TRUE;
			break;
		case ID_HWNDPALETTE:
			SetWindowLongPtr(hWnd, GWLP_HWNDPALETTE, 0);
			nResult = TRUE;
			break;
		}

		break;
	}

	return nResult ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HWND hWndAfter = HWND_TOP, hWndChild;
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	if (hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT))
	{
		SetWindowPosOnSize(hWndChild, hWndAfter, &rcClient);
	}

	return 0;
}
