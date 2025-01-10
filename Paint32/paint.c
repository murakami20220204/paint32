/*
Copyright 2025 Taichi Murakami.
お絵かきソフト ウィンドウ プロシージャを実装します。
*/

#include <windows.h>
#include <commctrl.h>
#include "Paint32.h"
#include "resource.h"

/* User Data */
typedef struct tagUSERDATA
{
	COLORREF rgbHistoryColors[CUSTOMCOLORSLENGTH];
	COLORREF rgbFavoriteColors[CUSTOMCOLORSLENGTH];
} USERDATA, FAR *LPUSERDATA;

/* Window Extra */
typedef struct tagWINDOWEXTRA
{
	LONG_PTR lpClientWindow;
	LONG_PTR lpOutlineWindow;
	LONG_PTR lpPaletteWindow;
	LONG_PTR lpStatusWindow;
	LONG_PTR lpToolbarWindow;
	LONG dwDpi;
	LONG dwFlags;
} WINDOWEXTRA;

#define DOCK_TOP                0x0001
#define DOCK_BOTTOM             0x0002
#define DOCK_HORZ               0x0000
#define DOCK_VERT               0x0004
#define DOCK_FLOAT              0x0008
#define DOCK_MASK               0x000F
#define DOCK_LEFT               (DOCK_VERT | DOCK_TOP)
#define DOCK_RIGHT              (DOCK_VERT | DOCK_BOTTOM)
#define GWL_DPI                 offsetof(WINDOWEXTRA, dwDpi)
#define GWL_FLAGS               offsetof(WINDOWEXTRA, dwFlags)
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
#define USERDATA_OFFPALETTE     0
#define USERDATA_OFFOUTLINE     4
#define USERDATA_OFFAUTOSAVE    8
#define USERDATA_SHOWSTATUS     0x80000000
#define USERDATA_SHOWTOOLBAR    0x40000000
#define USERDATA_SHOWOUTLINE    0x20000000
#define USERDATA_SHOWPALETTE    0x10000000
#define USERDATA_ENABLEAUTOSAVE 0x08000000
#define WS_HWNDCLIENT           (WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDOUTLINE          (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
#define WS_HWNDPALETTE          (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
#define WS_HWNDSTATUS           (WS_CHILD | SBARS_SIZEGRIP | CCS_NODIVIDER)
#define WS_HWNDTOOLBAR          (WS_CHILD | TBSTYLE_FLAT | CCS_NODIVIDER)

static_assert(sizeof(WINDOWEXTRA) == PAINTWINDOWEXTRA, "PAINTWINDOWEXTRA");
static LRESULT WINAPI DefProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static VOID WINAPI OnAbout(_In_ HWND hWnd);
static BOOL WINAPI OnClose(_In_ HWND hWnd);
static BOOL WINAPI OnCommand(_In_ HWND hWnd, _In_ UINT uCommand);
static BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ CONST CREATESTRUCT FAR *lpParam);
static VOID WINAPI OnDestroy(_In_ HWND hWnd);
static VOID WINAPI OnDpiChanged(_In_ HWND hWnd, _In_ UINT uDpi, _In_ CONST RECT FAR *lpWindow);
static VOID WINAPI OnGetMinMaxInfo(_In_ HWND hWnd, _Inout_ LPMINMAXINFO lpParam);
static BOOL WINAPI OnParentNotify(_In_ HWND hWnd, _In_ UINT uNotify, _In_ UINT uChild, _In_ LPARAM hWndChild);
static BOOL WINAPI OnSize(_In_ HWND hWnd, _In_ UINT uState);
static BOOL WINAPI OnTranslateAccelerator(_In_ HWND hWnd, _Inout_ LPMSG lpMsg);

/*
お絵かきソフト ウィンドウ プロシージャ。
*/
EXTERN_C
LRESULT CALLBACK PaintWindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult;

	switch (uMsg)
	{
	case WM_ABOUT:
		OnAbout(hWnd);
		nResult = 0;
		break;
	case WM_CLOSE:
		nResult = OnClose(hWnd) ? DefProc(hWnd, uMsg, wParam, lParam) : -1;
		break;
	case WM_COMMAND:
		nResult = OnCommand(hWnd, LOWORD(wParam)) ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_CREATE:
		nResult = OnCreate(hWnd, (LPCREATESTRUCT)lParam) ? DefProc(hWnd, uMsg, wParam, lParam) : -1;
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
	case WM_PARENTNOTIFY:
		nResult = OnParentNotify(hWnd, LOWORD(wParam), HIWORD(wParam), lParam) ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SIZE:
		OnSize(hWnd, LOWORD(wParam));
		nResult = 0;
		break;
	case WM_TRANSLATEACCELERATOR:
		nResult = OnTranslateAccelerator(hWnd, (LPMSG)lParam);
		break;
	default:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return nResult;
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
	HWND hWndClient;
	hWndClient = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT);
	return DefFrameProc(hWnd, hWndClient, uMsg, wParam, lParam);
}

static
VOID WINAPI OnAbout(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
}

static
BOOL WINAPI OnClose(
	_In_ HWND hWnd)
{
	return TRUE;
}

static
BOOL WINAPI OnCommand(
	_In_ HWND hWnd,
	_In_ UINT uCommand)
{
	BOOL bResult;

	switch (uCommand)
	{
	case IDM_ABOUT:
		bResult = PostMessage(hWnd, WM_ABOUT, 0, 0);
		break;
	case IDM_EXIT:
		bResult = PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	default:
		bResult = FALSE;
		break;
	}

	return bResult;
}

static
BOOL WINAPI OnCreate(
	_In_ HWND hWnd,
	_In_ CONST CREATESTRUCT FAR *lpParam)
{
	SetWindowLong(hWnd, GWL_DPI, GetDpiForWindow(hWnd));
	return TRUE;
}

static
VOID WINAPI OnDestroy(
	_In_ HWND hWnd)
{
	LPUSERDATA pSelf;
	pSelf = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pSelf)
	{
		LocalFree(pSelf);
	}

	PostQuitMessage(0);
}

static
VOID WINAPI OnDpiChanged(
	_In_ HWND hWnd,
	_In_ UINT uDpi,
	_In_ CONST RECT FAR *lpWindow)
{
	SetWindowLong(hWnd, GWL_DPI, uDpi);
	MoveWindow(hWnd, lpWindow->left, lpWindow->top, lpWindow->right - lpWindow->left, lpWindow->bottom - lpWindow->top, TRUE);
}

/*
ウィンドウの最小サイズを決定します。
*/
static
VOID WINAPI OnGetMinMaxInfo(
	_In_ HWND hWnd,
	_Inout_ LPMINMAXINFO lpParam)
{
	POINT ptSize;
	UINT uDpi;
	uDpi = GetWindowLong(hWnd, GWL_DPI);
	ptSize.x = MulDiv(MINTRACKSIZE_X, uDpi, USER_DEFAULT_SCREEN_DPI);
	ptSize.y = MulDiv(MINTRACKSIZE_Y, uDpi, USER_DEFAULT_SCREEN_DPI);
	lpParam->ptMinTrackSize.x = max(lpParam->ptMinTrackSize.x, ptSize.x);
	lpParam->ptMinTrackSize.y = max(lpParam->ptMinTrackSize.y, ptSize.y);
}

static
BOOL WINAPI OnParentNotify(
	_In_ HWND hWnd,
	_In_ UINT uNotify,
	_In_ UINT uChild,
	_In_ LPARAM hWndChild)
{
	return FALSE;
}

static
BOOL WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uState)
{
	const BOOL bResult = (uState == SIZE_RESTORED) || (uState == SIZE_MAXIMIZED);

	if (bResult)
	{

	}

	return bResult;
}

/*
子ウィンドウのアクセラレーターを実行します。
*/
static
BOOL WINAPI OnTranslateAccelerator(
	_In_ HWND hWnd,
	_Inout_ LPMSG lpMsg)
{
	HWND hWndClient;
	hWndClient = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT);
	return TranslateMDISysAccel(hWndClient, lpMsg);
}
