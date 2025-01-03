/*
Copyright 2025 Taichi Murakami.
アプリケーション ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"

typedef struct tagUSERDATA
{
	COLORREF rgbHistoryColors[CUSTOMCOLORSLENGTH];
	COLORREF rgbFavoriteColors[CUSTOMCOLORSLENGTH];
	DWORD dwFlags;
	WORD wOutlineDock;
	WORD wPaletteDock;
} USERDATA, FAR *LPUSERDATA;

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
static BOOL WINAPI AppendChild(_In_ HWND hWnd, _In_ int nExtra, _In_ DWORD dwFlags, _In_ LONG_PTR hWndChild);
static BOOL WINAPI ClearChild(_In_ HWND hWnd, _In_ int nExtra, _In_ DWORD dwFlags);
static HWND WINAPI CreateClientWindow(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreateOutlineWindow(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreatePaletteWindow(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreateStatusBar(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreateToolbar(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static LPUSERDATA WINAPI CreateUserData( _In_ HWND hWnd);
static LRESULT WINAPI DefProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static BOOL WINAPI DestroyUserData(_In_ HWND hWnd);
static BOOL WINAPI LayoutControls(_In_ HWND hWnd);
static LRESULT WINAPI OnAbout(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnCommand(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnCreate(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnDestroy(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnDpiChanged(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnGetMinMaxInfo(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnInitMenuPopup(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnIsDialogMessage(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnNew(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnOutline(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnPalette(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnParentNotify(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnPreferences(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnSize(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnStatus(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI OnToolbar(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static BOOL WINAPI ResizeWindow(_In_ HWND hWnd, _In_ const RECT FAR *lpRect);
static HWND WINAPI ToggleWindow(_In_ HWND hWnd, _In_ int nExtra, _In_ CREATECHILDPROC lpfnCreate);

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
	case WM_COMMAND:
		lpProc = OnCommand;
		break;
	case WM_CREATE:
		lpProc = OnCreate;
		break;
	case WM_DESTROY:
		lpProc = OnDestroy;
		break;
	case WM_DPICHANGED:
		lpProc = OnDpiChanged;
		break;
	case WM_GETMINMAXINFO:
		lpProc = OnGetMinMaxInfo;
		break;
	case WM_INITMENUPOPUP:
		lpProc = OnInitMenuPopup;
		break;
	case WM_PARENTNOTIFY:
		lpProc = OnParentNotify;
		break;
	case WM_SIZE:
		lpProc = OnSize;
		break;
	case APPLICATION_ABOUT:
		lpProc = OnAbout;
		break;
	case APPLICATION_ISDIALOGMESSAGE:
		lpProc = OnIsDialogMessage;
		break;
	case APPLICATION_NEW:
		lpProc = OnNew;
		break;
	case APPLICATION_OUTLINE:
		lpProc = OnOutline;
		break;
	case APPLICATION_PALETTE:
		lpProc = OnPalette;
		break;
	case APPLICATION_STATUS:
		lpProc = OnStatus;
		break;
	case APPLICATION_TOOLBAR:
		lpProc = OnToolbar;
		break;
	case APPLICATION_PREFERENCES:
		lpProc = OnPreferences;
		break;
	default:
		lpProc = DefProc;
		break;
	}

	return lpProc(hWnd, uMsg, wParam, lParam);
}

static
BOOL WINAPI AppendChild(
	_In_ HWND hWnd,
	_In_ int nExtra,
	_In_ DWORD dwFlags,
	_In_ LONG_PTR hWndChild)
{
	LPUSERDATA lpUserData;
	lpUserData = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	SetWindowLongPtr(hWnd, nExtra, hWndChild);

	if (lpUserData)
	{
		lpUserData->dwFlags |= dwFlags;
	}

	return !!lpUserData;
}

static
BOOL WINAPI ClearChild(
	_In_ HWND hWnd,
	_In_ int nExtra,
	_In_ DWORD dwFlags)
{
	LPUSERDATA lpUserData;
	lpUserData = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	SetWindowLongPtr(hWnd, nExtra, 0);

	if (lpUserData)
	{
		lpUserData->dwFlags &= ~dwFlags;
	}

	return !!lpUserData;
}

static
HWND WINAPI CreateClientWindow(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndClient;
	CLIENTCREATESTRUCT param;
	hWndClient = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT);

	if (!hWndClient)
	{
		ZeroMemory(&param, sizeof param);
		param.hWindowMenu = GetMenu(hWnd);
		param.hWindowMenu = param.hWindowMenu ? GetSubMenu(param.hWindowMenu, ID_WINDOWMENU) : NULL;
		param.idFirstChild = ID_FIRSTCHILD;
		hWndClient = CreateWindowEx(WS_EX_CLIENTEDGE, MDICLIENTCLASSNAME, NULL, WS_HWNDCLIENT, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDCLIENT, hInstance, &param);

		if (hWndClient)
		{
			SetWindowLongPtr(hWnd, GWLP_HWNDCLIENT, (LONG_PTR)hWndClient);
		}
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
		ZeroMemory(&param, sizeof param);
		LoadString(hInstance, IDS_LAYER, strCaption, LOADSTRING_MAX);
		param.wID = ID_HWNDOUTLINE;
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
	ZeroMemory(&param, sizeof param);
	LoadString(hInstance, IDS_PALETTE, strCaption, LOADSTRING_MAX);
	param.wID = ID_HWNDPALETTE;
	hWndPalette = CreateWindow(PALETTECLASSNAME, strCaption, WS_HWNDPALETTE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWnd, NULL, hInstance, &param);

	if (hWndPalette)
	{
		ShowWindow(hWndPalette, SW_SHOW);
	}

	return hWndPalette;
}

static
HWND WINAPI CreateStatusBar(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndStatus;
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (!hWndStatus)
	{
		hWndStatus = CreateWindow(STATUSCLASSNAME, NULL, WS_HWNDSTATUS, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDSTATUS, hInstance, NULL);

		if (hWndStatus)
		{
			//SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, (LONG_PTR)hWndStatus);
		}
	}

	return hWndStatus;
}

static
HWND WINAPI CreateToolbar(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndToolbar;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (!hWndToolbar)
	{
		hWndToolbar = CreateWindow(TOOLBARCLASSNAME, NULL, WS_HWNDTOOLBAR, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDTOOLBAR, hInstance, NULL);

		if (hWndToolbar)
		{
			//SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, (LONG_PTR)hWndToolbar);
		}
	}

	return hWndToolbar;
}

static
LPUSERDATA WINAPI CreateUserData(
	_In_ HWND hWnd)
{
	LPUSERDATA lpUserData;
	lpUserData = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (!lpUserData)
	{
		lpUserData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(USERDATA));

		if (lpUserData)
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpUserData);
			lpUserData->dwFlags = USERDATA_STATUS | USERDATA_TOOLBAR;
		}
	}

	return lpUserData;
}

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
BOOL WINAPI DestroyUserData(
	_In_ HWND hWnd)
{
	LPUSERDATA lpUserData;
	lpUserData = (LPUSERDATA)SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
	return lpUserData && HeapFree(GetProcessHeap(), 0, lpUserData);
}

static
BOOL WINAPI LayoutControls(
	_In_ HWND hWnd)
{
	HWND hWndChild;
	RECT rcClient, rcWindow;
	BOOL bResult;
	bResult = GetClientRect(hWnd, &rcClient);

	if (bResult)
	{
		if (hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR))
		{
			SendMessage(hWndChild, WM_SIZE, 0, 0);
			ShowWindow(hWndChild, SW_SHOW);
			if (GetWindowRect(hWndChild, &rcWindow)) rcClient.top += rcWindow.bottom - rcWindow.top;
		}
		if (hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS))
		{
			SendMessage(hWndChild, WM_SIZE, 0, 0);
			ShowWindow(hWndChild, SW_SHOW);
			if (GetWindowRect(hWndChild, &rcWindow)) rcClient.bottom -= rcWindow.bottom - rcWindow.top;
		}
		if (hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT))
		{
			rcClient.right -= rcClient.left;
			rcClient.bottom -= rcClient.top;
			SetWindowPos(hWndChild, NULL, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
		}
	}

	return bResult;
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
	case IDM_NEW:
		PostMessage(hWnd, APPLICATION_NEW, 0, 0);
		break;
	case IDM_OUTLINE:
		PostMessage(hWnd, APPLICATION_OUTLINE, 0, 0);
		break;
	case IDM_PALETTE:
		PostMessage(hWnd, APPLICATION_PALETTE, 0, 0);
		break;
	case IDM_PREFERENCES:
		PostMessage(hWnd, APPLICATION_PREFERENCES, 0, 0);
		break;
	case IDM_STATUS:
		PostMessage(hWnd, APPLICATION_STATUS, 0, 0);
		break;
	case IDM_TOOLBAR:
		PostMessage(hWnd, APPLICATION_TOOLBAR, 0, 0);
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
	HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
	LPUSERDATA lpUserData;
	lpUserData = CreateUserData(hWnd);
	return lpUserData && CreateClientWindow(hWnd, hInstance) &&
		(!(lpUserData->dwFlags & USERDATA_TOOLBAR) || CreateToolbar(hWnd, hInstance)) &&
		(!(lpUserData->dwFlags & USERDATA_STATUS) || CreateStatusBar(hWnd, hInstance)) &&
		(!(lpUserData->dwFlags & USERDATA_OUTLINE) || CreateOutlineWindow(hWnd, hInstance)) &&
		(!(lpUserData->dwFlags & USERDATA_PALETTE) || CreatePaletteWindow(hWnd, hInstance)) ?
		DefProc(hWnd, uMsg, wParam, lParam) : -1;
}

static
LRESULT WINAPI OnDestroy(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	DestroyUserData(hWnd);
	PostQuitMessage(0);
	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnDpiChanged(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	ResizeWindow(hWnd, (LPRECT)lParam);
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
LRESULT WINAPI OnInitMenuPopup(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	MENUITEMINFO mii;
	int nCount, nExtra, nIndex;
	ZeroMemory(&mii, sizeof mii);
	mii.cbSize = sizeof mii;
	nCount = GetMenuItemCount((HMENU)wParam);

	for (nIndex = 0; nIndex < nCount; nIndex++)
	{
		mii.fMask = MIIM_STATE | MIIM_ID;

		if (GetMenuItemInfo((HMENU)wParam, nIndex, TRUE, &mii))
		{
			switch (mii.wID)
			{
			case IDM_OUTLINE:
				nExtra = GWLP_HWNDOUTLINE;
				break;
			case IDM_PALETTE:
				nExtra = GWLP_HWNDPALETTE;
				break;
			case IDM_STATUS:
				nExtra = GWLP_HWNDSTATUS;
				break;
			case IDM_TOOLBAR:
				nExtra = GWLP_HWNDTOOLBAR;
				break;
			default:
				continue;
			}

			mii.fMask = MIIM_STATE;
			mii.fState = GetWindowLongPtr(hWnd, nExtra) ? (mii.fState | MFS_CHECKED) : (mii.fState & ~MFS_CHECKED);
			SetMenuItemInfo((HMENU)wParam, nIndex, TRUE, &mii);
		}
	}

	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnIsDialogMessage(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	const int nExtras[] = { GWLP_HWNDOUTLINE, GWLP_HWNDPALETTE };
	HWND hWndChild;
	int nIndex;
	BOOL bResult = FALSE;

	for (nIndex = 0; nIndex < ARRAYSIZE(nExtras); nIndex++)
	{
		hWndChild = (HWND)GetWindowLongPtr(hWnd, nExtras[nIndex]);

		if (hWndChild)
		{
			bResult = IsDialogMessage(hWndChild, (LPMSG)lParam);
			if (bResult) goto EXIT;
		}
	}
	if (!bResult)
	{
		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCLIENT);
		bResult = TranslateMDISysAccel(hWndChild, (LPMSG)lParam);
	}

EXIT:
	return bResult;
}

static
LRESULT WINAPI OnNew(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
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
	return (LRESULT)ToggleWindow(hWnd, GWLP_HWNDOUTLINE, CreateOutlineWindow);
}

static
LRESULT WINAPI OnPalette(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	return (LRESULT)ToggleWindow(hWnd, GWLP_HWNDPALETTE, CreatePaletteWindow);
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
	case WM_CREATE:
		switch (HIWORD(wParam))
		{
		case ID_HWNDOUTLINE:
			nResult = AppendChild(hWnd, GWLP_HWNDOUTLINE, USERDATA_OUTLINE, lParam);
			break;
		case ID_HWNDPALETTE:
			nResult = AppendChild(hWnd, GWLP_HWNDPALETTE, USERDATA_PALETTE, lParam);
			break;
		case ID_HWNDSTATUS:
			nResult = AppendChild(hWnd, GWLP_HWNDSTATUS, USERDATA_STATUS, lParam);
			break;
		case ID_HWNDTOOLBAR:
			nResult = AppendChild(hWnd, GWLP_HWNDTOOLBAR, USERDATA_TOOLBAR, lParam);
			break;
		}

		break;
	case WM_DESTROY:
		switch (HIWORD(wParam))
		{
		case ID_HWNDOUTLINE:
			nResult = ClearChild(hWnd, GWLP_HWNDOUTLINE, USERDATA_OUTLINE);
			break;
		case ID_HWNDPALETTE:
			nResult = ClearChild(hWnd, GWLP_HWNDPALETTE, USERDATA_PALETTE);
			break;
		case ID_HWNDSTATUS:
			nResult = ClearChild(hWnd, GWLP_HWNDSTATUS, USERDATA_STATUS);
			break;
		case ID_HWNDTOOLBAR:
			nResult = ClearChild(hWnd, GWLP_HWNDTOOLBAR, USERDATA_TOOLBAR);
			break;
		}

		break;
	}

	return nResult ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnPreferences(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HPROPSHEETPAGE phpage[2];
	PROPSHEETHEADER psh;
	PROPSHEETPAGE psp;
	TCHAR strCaption[LOADSTRING_MAX];
	ZeroMemory(phpage, sizeof phpage);
	ZeroMemory(&psh, sizeof psh);
	ZeroMemory(&psp, sizeof psp);
	psh.dwSize = sizeof psh;
	psh.dwFlags = PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	psh.hwndParent = hWnd;
	psh.hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	psh.hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
	psh.pszCaption = strCaption;
	psh.nPages = ARRAYSIZE(phpage);
	psh.phpage = phpage;
	psp.dwSize = sizeof psp;
	psp.hInstance = psh.hInstance;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_AUTOSAVE);
	psp.pfnDlgProc = AutosaveDialogProc;
	phpage[0] = CreatePropertySheetPage(&psp);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_STARTUP);
	psp.pfnDlgProc = StartupDialogProc;
	phpage[1] = CreatePropertySheetPage(&psp);
	LoadString(psh.hInstance, IDS_PREFERENCES, strCaption, LOADSTRING_MAX);
	PropertySheet(&psh);
	return 0;
}

static LRESULT WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LayoutControls(hWnd);
	return 0;
}

static
LRESULT WINAPI OnStatus(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	ToggleWindow(hWnd, GWLP_HWNDSTATUS, CreateStatusBar);
	LayoutControls(hWnd);
	return 0;
}

static
LRESULT WINAPI OnToolbar(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	ToggleWindow(hWnd, GWLP_HWNDTOOLBAR, CreateToolbar);
	LayoutControls(hWnd);
	return 0;
}

static
BOOL WINAPI ResizeWindow(
	_In_ HWND hWnd,
	_In_ const RECT FAR *lpRect)
{
	const int X = lpRect->left;
	const int Y = lpRect->top;
	const int nWidth = lpRect->right - X;
	const int nHeight = lpRect->bottom - Y;
	return MoveWindow(hWnd, X, Y, nWidth, nHeight, TRUE);
}

static
HWND WINAPI ToggleWindow(
	_In_ HWND hWnd,
	_In_ int nExtra,
	_In_ CREATECHILDPROC lpfnCreate)
{
	HWND hWndChild;
	hWndChild = (HWND)GetWindowLongPtr(hWnd, nExtra);

	if (hWndChild)
	{
		SendMessage(hWndChild, WM_CLOSE, 0, 0);
		hWndChild = NULL;
	}
	else
	{
		hWndChild = lpfnCreate(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	}

	return hWndChild;
}
