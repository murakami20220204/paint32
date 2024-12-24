/*
Copyright 2024 Taichi Murakami.
Application ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define ID_FIRSTCHILD 1
#define ID_HWNDCLIENT 1
#define ID_WINDOWMENU 4
#define MINTRACKSIZE_X 300
#define MINTRACKSIZE_Y 200
#define WS_HWNDCLIENT (WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL)

typedef struct tagFRAMEWINDOWEXTRA
{
	LONG_PTR hWndClient;
} WINDOWEXTRA;

static_assert(sizeof(WINDOWEXTRA) == FRAMEWINDOWEXTRA, "FRAMEWINDOWEXTRA");
#define GWLP_HWNDCLIENT offsetof(WINDOWEXTRA, hWndClient)

static
HWND WINAPI CreateClientWindow(
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
	case WM_SIZE:
		lpProc = OnSize;
		break;
	case FRAME_ABOUT:
		lpProc = OnAbout;
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
