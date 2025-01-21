/*
Copyright 2025 Taichi Murakami.
ドキュメント ウィンドウ プロシージャを実装します。
*/

#define CXBUTTON                5
#define ID_HWNDCANVAS           1
#define ID_HWNDHSCROLL          2
#define ID_HWNDLAYER            7
#define ID_HWNDROTATION         5
#define ID_HWNDSIZEBOX          6
#define ID_HWNDVSCROLL          3
#define ID_HWNDZOOM             4
#define MINTRACKSIZE_X          150
#define MINTRACKSIZE_Y          100

#include "paint.h"
#include <windows.h>
#include <commctrl.h>

#define DefProc                 DefMDIChildProc
#define WM_LAYOUT               0x0410
#define WS_HWNDCANVAS           (WS_CHILD | WS_VISIBLE)
#define WS_HWNDHSCROLL          (WS_CHILD | WS_VISIBLE | SBS_HORZ)
#define WS_HWNDLAYER            (WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST)
#define WS_HWNDROTATION         (WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON)
#define WS_HWNDSIZEBOX          (WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP)
#define WS_HWNDVSCROLL          (WS_CHILD | WS_VISIBLE | SBS_VERT)
#define WS_HWNDZOOM             (WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON)
#define WS_EX_HWNDCANVAS        0
#define WS_EX_HWNDHSCROLL       0
#define WS_EX_HWNDLAYER         0
#define WS_EX_HWNDROTATION      0
#define WS_EX_HWNDSIZEBOX       0
#define WS_EX_HWNDVSCROLL       0
#define WS_EX_HWNDZOOM          0

typedef struct tagDOCUMENTWINDOWEXTRA
{
	LONG_PTR lpCanvas;
	LONG_PTR lpFrame;
	LONG_PTR lpHorizontalScrollBar;
	LONG_PTR lpLayer;
	LONG_PTR lpRotation;
	LONG_PTR lpSizeBox;
	LONG_PTR lpScene;
	LONG_PTR lpVerticalScrollBar;
	LONG_PTR lpZoom;
	LONG dwDpi;
} WINDOWEXTRA;

#define GWLP_HWNDCANVAS         offsetof(WINDOWEXTRA, lpCanvas)
#define GWLP_HWNDFRAME          offsetof(WINDOWEXTRA, lpFrame)
#define GWLP_HWNDHSCROLL        offsetof(WINDOWEXTRA, lpHorizontalScrollBar)
#define GWLP_HWNDLAYER          offsetof(WINDOWEXTRA, lpLayer)
#define GWLP_HWNDROTATION       offsetof(WINDOWEXTRA, lpRotation)
#define GWLP_HWNDSIZEBOX        offsetof(WINDOWEXTRA, lpSizeBox)
#define GWLP_HWNDVSCROLL        offsetof(WINDOWEXTRA, lpVerticalScrollBar)
#define GWLP_HWNDZOOM           offsetof(WINDOWEXTRA, lpZoom)
#define GWL_DPI                 offsetof(WINDOWEXTRA, dwDpi)

static_assert(sizeof(WINDOWEXTRA) == DOCUMENTWINDOWEXTRA, "DOCUMENTWINDOWEXTRA");
static HWND WINAPI CreateChild(_In_ DWORD dwExStyle, _In_ LPCTSTR lpClassName, _In_ DWORD dwStyle, _In_ HWND hWnd, _In_ UINT uID);
static BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ CONST CREATESTRUCT FAR *lpParam);
static VOID WINAPI OnDestroy(_In_ HWND hWnd);
static VOID WINAPI OnDpiChanged(_In_ HWND hWnd, _In_ UINT uDpi, _In_ CONST RECT FAR *lpWindow);
static VOID WINAPI OnGetMinMaxInfo(_In_ HWND hWnd, _Inout_ LPMINMAXINFO lpInfo);
static VOID WINAPI OnLayout(_In_ HWND hWnd);
static BOOL WINAPI OnParentNotify(_In_ HWND hWnd, _In_ UINT uNotify, _In_ UINT idChild, _In_ LPARAM lParam);
static VOID WINAPI OnSize(_In_ HWND hWnd, _In_ UINT uReason);

#define CreateCanvas(hWnd) CreateChild(WS_EX_HWNDCANVAS, CANVASCLASSNAME, WS_HWNDCANVAS, (hWnd), ID_HWNDCANVAS)
#define CreateLayerComboBox(hWnd) CreateChild(WS_EX_HWNDLAYER, WC_COMBOBOX, WS_HWNDLAYER, (hWnd), ID_HWNDLAYER)
#define CreateVerticalScrollBar(hWnd) CreateChild(WS_EX_HWNDVSCROLL, WC_SCROLLBAR, WS_HWNDVSCROLL, (hWnd), ID_HWNDVSCROLL)

/*
ドキュメント ウィンドウ プロシージャ。
*/
EXTERN_C
LRESULT CALLBACK DocumentWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult;

	switch (uMsg)
	{
	case WM_CREATE:
		if (OnCreate(hWnd, (LPCREATESTRUCT)lParam)) nResult = DefProc(hWnd, uMsg, wParam, lParam);
		else nResult = -1;
		break;
	case WM_DESTROY:
		OnDestroy(hWnd);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DPICHANGED:
		OnDpiChanged(hWnd, LOWORD(wParam), (LPRECT)lParam);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_GETMINMAXINFO:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		OnGetMinMaxInfo(hWnd, (LPMINMAXINFO)lParam);
		break;
	case WM_LAYOUT:
		OnLayout(hWnd);
		nResult = 0;
		break;
	case WM_PARENTNOTIFY:
		if (OnParentNotify(hWnd, LOWORD(wParam), HIWORD(wParam), lParam)) nResult = 0;
		else nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SIZE:
		OnSize(hWnd, LOWORD(wParam));
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	default:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return nResult;
}

static
HWND WINAPI CreateChild(
	_In_ DWORD dwExStyle,
	_In_ LPCTSTR lpClassName,
	_In_ DWORD dwStyle,
	_In_ HWND hWnd,
	_In_ UINT uID)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	return CreateWindowEx(dwExStyle, lpClassName, NULL, dwStyle, 0, 0, 0, 0, hWnd, (HMENU)(LONG_PTR)uID, hInstance, NULL);
}

/*
WM_CREATE:
*/
static
BOOL WINAPI OnCreate(
	_In_ HWND hWnd,
	_In_ CONST CREATESTRUCT FAR *lpParam)
{
	SetWindowLong(hWnd, GWL_DPI, GetDpiForWindow(hWnd));
	return TRUE;
}

/*
WM_DESTROY:
現在のウィンドウが保持している資源を解放します。
*/
static
VOID WINAPI OnDestroy(
	_In_ HWND hWnd)
{
}

/*
WM_DPICHANGED:
*/
static
VOID WINAPI OnDpiChanged(
	_In_ HWND hWnd,
	_In_ UINT uDpi,
	_In_ CONST RECT FAR *lpWindow)
{
	SetWindowLong(hWnd, GWL_DPI, uDpi);
	MoveWindowForRect(hWnd, lpWindow, TRUE);
}

/*
WM_GETMINMAXINFO:
現在のウィンドウの最小サイズを指定します。
*/
static
VOID WINAPI OnGetMinMaxInfo(
	_In_ HWND hWnd,
	_Inout_ LPMINMAXINFO lpInfo)
{
	UINT uDpi;
	uDpi = GetWindowLong(hWnd, GWL_DPI);
	SetMinMaxInfoForDpi(lpInfo, MINTRACKSIZE_X, MINTRACKSIZE_Y, uDpi);
}

static
VOID WINAPI OnLayout(
	_In_ HWND hWnd)
{
	HWND hWndChild;
	RECT rcClient, rcWindow;
	UINT uDpi;
	int x, y, cxScroll, cyScroll;

	if (GetClientRect(hWnd, &rcClient))
	{
		uDpi = GetWindowLong(hWnd, GWL_DPI);
		cxScroll = GetSystemMetricsForDpi(SM_CXVSCROLL, uDpi);
		cyScroll = GetSystemMetricsForDpi(SM_CYHSCROLL, uDpi);
		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDLAYER);

		if (hWndChild)
		{
			MoveWindow(hWndChild, 0, 0, rcClient.right, cyScroll, TRUE);
			GetWindowRect(hWndChild, &rcWindow);
			rcClient.top += rcWindow.bottom - rcWindow.top;
		}

		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCANVAS);

		if (hWndChild)
		{
			rcWindow.right = rcClient.right - cxScroll;
			rcWindow.bottom = rcClient.bottom - rcClient.top - cyScroll;
			MoveWindow(hWndChild, 0, rcClient.top, rcWindow.right, rcWindow.bottom, TRUE);
		}

		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDVSCROLL);

		if (hWndChild)
		{
			x = rcClient.right - cxScroll;
			y = rcClient.bottom - rcClient.top - cyScroll;
			MoveWindow(hWndChild, x, rcClient.top, cxScroll, y, TRUE);
		}
	}
}

static
BOOL WINAPI OnParentNotify(
	_In_ HWND hWnd,
	_In_ UINT uNotify,
	_In_ UINT idChild,
	_In_ LPARAM lParam)
{
	BOOL bResult = FALSE;

	switch (uNotify)
	{
	case WM_CREATE:
		switch (idChild)
		{
		case ID_HWNDCANVAS:
			SetWindowLongPtr(hWnd, GWLP_HWNDCANVAS, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDHSCROLL:
			SetWindowLongPtr(hWnd, GWLP_HWNDHSCROLL, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDLAYER:
			SetWindowLongPtr(hWnd, GWLP_HWNDLAYER, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDROTATION:
			SetWindowLongPtr(hWnd, GWLP_HWNDROTATION, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDSIZEBOX:
			SetWindowLongPtr(hWnd, GWLP_HWNDSIZEBOX, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDVSCROLL:
			SetWindowLongPtr(hWnd, GWLP_HWNDVSCROLL, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDZOOM:
			SetWindowLongPtr(hWnd, GWLP_HWNDZOOM, lParam);
			bResult = TRUE;
			break;
		}

		break;
	case WM_DESTROY:
		switch (idChild)
		{
		case ID_HWNDCANVAS:
			SetWindowLongPtr(hWnd, GWLP_HWNDCANVAS, 0);
			bResult = TRUE;
			break;
		case ID_HWNDHSCROLL:
			SetWindowLongPtr(hWnd, GWLP_HWNDHSCROLL, 0);
			bResult = TRUE;
			break;
		case ID_HWNDLAYER:
			SetWindowLongPtr(hWnd, GWLP_HWNDLAYER, 0);
			bResult = TRUE;
			break;
		case ID_HWNDROTATION:
			SetWindowLongPtr(hWnd, GWLP_HWNDROTATION, 0);
			bResult = TRUE;
			break;
		case ID_HWNDSIZEBOX:
			SetWindowLongPtr(hWnd, GWLP_HWNDSIZEBOX, 0);
			bResult = TRUE;
			break;
		case ID_HWNDVSCROLL:
			SetWindowLongPtr(hWnd, GWLP_HWNDVSCROLL, 0);
			bResult = TRUE;
			break;
		case ID_HWNDZOOM:
			SetWindowLongPtr(hWnd, GWLP_HWNDZOOM, 0);
			bResult = TRUE;
			break;
		}

		break;
	}

	return bResult;
}

static
VOID WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uReason)
{
	switch (uReason)
	{
	case SIZE_RESTORED:
	case SIZE_MAXIMIZED:
		SendMessage(hWnd, WM_LAYOUT, 0, 0);
		break;
	}
}
