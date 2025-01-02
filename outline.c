/*
Copyright 2025 Taichi Murakami.
アウトライン ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "Paint32.h"
#include "resource.h"
#define DefProc DefWindowProc
#define ID_HWNDTREE 1
#define WS_HWNDTREE (WS_CHILD | WS_BORDER)

typedef struct tagWINDOWEXTRA
{
	LONG_PTR hWndTree;
} WINDOWEXTRA;

static_assert(sizeof(WINDOWEXTRA) == OUTLINEWINDOWEXTRA, "OUTLINEWINDOWEXTRA");
#define GWLP_HWNDTREE offsetof(WINDOWEXTRA, hWndTree)

typedef LRESULT(CALLBACK FAR *DEFPROC)(HWND, UINT, WPARAM, LPARAM);

static
HWND WINAPI CreateTreeView(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance);

static
BOOL WINAPI LayoutTreeView(
	_In_ HWND hWnd);

static
LRESULT WINAPI NotifyParent(
	_In_ HWND hWnd,
	_In_ UINT uMsg);

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
LRESULT WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam);

EXTERN_C
LRESULT CALLBACK OutlineWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	DEFPROC lpProc;

	switch (uMsg)
	{
	case WM_CREATE:
		lpProc = OnCreate;
		break;
	case WM_DESTROY:
		lpProc = OnDestroy;
		break;
	case WM_SIZE:
		lpProc = OnSize;
		break;
	default:
		lpProc = DefProc;
		break;
	}

	return lpProc(hWnd, uMsg, wParam, lParam);
}

static
HWND WINAPI CreateTreeView(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndTree;
	hWndTree = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTREE);

	if (!hWndTree)
	{
		hWndTree = CreateWindow(WC_TREEVIEW, NULL, WS_HWNDTREE, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDTREE, hInstance, NULL);

		if (hWndTree)
		{
			SetWindowLongPtr(hWnd, GWLP_HWNDTREE, (LONG_PTR)hWndTree);
		}
	}

	return hWndTree;
}

static
BOOL WINAPI LayoutTreeView(
	_In_ HWND hWnd)
{
	HWND hWndTree;
	RECT rcClient;
	hWndTree = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTREE);
	return hWndTree && GetClientRect(hWnd, &rcClient) && SetWindowPos(hWndTree, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
}

static
LRESULT WINAPI NotifyParent(
	_In_ HWND hWnd,
	_In_ UINT uMsg)
{
	HWND hWndParent;
	LRESULT nResult;
	hWndParent = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPARENT);

	if (hWndParent)
	{
		nResult = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		nResult = SendMessage(hWndParent, WM_PARENTNOTIFY, MAKEWPARAM(uMsg, nResult), (LPARAM)hWnd);
	}
	else
	{
		nResult = 0;
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
	HINSTANCE hInstance;
	LRESULT nResult = -1;
	hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
	SetWindowLongPtr(hWnd, GWLP_USERDATA, ((LPOUTLINECREATESTRUCT)((LPCREATESTRUCT)lParam)->lpCreateParams)->wID);

	if (CreateTreeView(hWnd, hInstance))
	{
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		NotifyParent(hWnd, WM_CREATE);
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
	NotifyParent(hWnd, WM_DESTROY);
	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LayoutTreeView(hWnd);
	return DefProc(hWnd, uMsg, wParam, lParam);
}
