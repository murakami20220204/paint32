/*
Copyright 2025 Taichi Murakami.
アプリケーション ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "app.h"
#include "resource.h"
#include "shared.h"

static const
TBBUTTON TOOLBARBUTTONS[] =
{
	{ 0, IDM_OPEN, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
	{ 0, 0,        0,               BTNS_SEP,    { 0 }, 0, 0 },
};

/* Window Extra */
typedef struct tagWINDOWEXTRA
{
	LONG_PTR lpImageList;
	LONG_PTR lpPictureBox;
	LONG_PTR lpStatusBar;
	LONG_PTR lpToolbar;
} WINDOWEXTRA;

#define CXTOOLBAR               32
#define DefProc                 DefWindowProc
#define GWLP_HIMAGELIST         offsetof(WINDOWEXTRA, lpImageList)
#define GWLP_HWNDPICTURE        offsetof(WINDOWEXTRA, lpPictureBox)
#define GWLP_HWNDSTATUS         offsetof(WINDOWEXTRA, lpStatusBar)
#define GWLP_HWNDTOOLBAR        offsetof(WINDOWEXTRA, lpToolbar)
#define ID_HWNDPICTURE          1
#define ID_HWNDSTATUS           2
#define ID_HWNDTOOLBAR          3
#define WS_HWNDPICTURE          (WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDSTATUS           (WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | CCS_NODIVIDER)
#define WS_HWNDTOOLBAR          (WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER)

static_assert(sizeof(WINDOWEXTRA) == APPLICATIONWINDOWEXTRA, "APPLICATIONWINDOWEXTRA");
static BOOL WINAPI Create(_In_ HWND hWnd, _In_ const CREATESTRUCT FAR *lpParam);
static HWND WINAPI CreatePictureBox(_In_ HWND hWnd, _In_ HINSTANCE hInstance);
static HWND WINAPI CreateStatusBar(_In_ HWND hWnd, _In_ HINSTANCE hInstance);
static HWND WINAPI CreateToolbar(_In_ HWND hWnd, _In_ HINSTANCE hInstance);
static BOOL WINAPI DestroyImageList(_In_ HWND hWnd);
static BOOL WINAPI GetToolbarText(_In_ HWND hWnd, _Inout_ LPTOOLTIPTEXT lpTooltip);
static BOOL WINAPI LayoutControls(_In_ HWND hWnd);
static BOOL WINAPI ShowAboutDialog(_In_ HWND hWnd);
static BOOL WINAPI UpdateDpi(_In_ HWND hWnd, _In_ UINT uDpi);
static BOOL WINAPI UpdateStatusParts(_In_ HWND hWndStatus, _In_ UINT uDpi);
static BOOL WINAPI UpdateToolbarButtons(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);

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
	case WM_CREATE:
		if (Create(hWnd, (LPCREATESTRUCT)lParam))
		{
			nResult = DefProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			ErrorMessageBox(NULL, hWnd, GetLastError());
			nResult = -1;
		}

		break;
	case WM_DESTROY:
		DestroyImageList(hWnd);
		PostQuitMessage(0);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DRAWITEM:
		ImageList_Draw(((HIMAGELIST)GetWindowLongPtr(hWnd, GWLP_HIMAGELIST)), 0, ((LPDRAWITEMSTRUCT)lParam)->hDC, ((LPDRAWITEMSTRUCT)lParam)->rcItem.left, ((LPDRAWITEMSTRUCT)lParam)->rcItem.top, ILD_NORMAL);
		return TRUE;
		break;
	case WM_DPICHANGED:
		UpdateDpi(hWnd, LOWORD(wParam));
		LayoutControls(hWnd);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_MEASUREITEM:
		((LPMEASUREITEMSTRUCT)lParam)->itemWidth = 16;
		((LPMEASUREITEMSTRUCT)lParam)->itemHeight = 16;
		nResult = TRUE;
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case TTN_NEEDTEXT:
			GetToolbarText(hWnd, (LPTOOLTIPTEXT)lParam);
			break;
		default:
			nResult = DefProc(hWnd, uMsg, wParam, lParam);
			break;
		}

		break;
	case WM_PARENTNOTIFY:
		switch (LOWORD(wParam))
		{
		case WM_CREATE:
			switch (HIWORD(wParam))
			{
			case ID_HWNDPICTURE:
				SetWindowLongPtr(hWnd, GWLP_HWNDPICTURE, lParam);
				break;
			case ID_HWNDSTATUS:
				SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, lParam);
				break;
			case ID_HWNDTOOLBAR:
				SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, lParam);
				break;
			}

			break;
		case WM_DESTROY:
			switch (HIWORD(wParam))
			{
			case ID_HWNDPICTURE:
				SetWindowLongPtr(hWnd, GWLP_HWNDPICTURE, 0);
				break;
			case ID_HWNDSTATUS:
				SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, 0);
				break;
			case ID_HWNDTOOLBAR:
				SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, 0);
				break;
			}

			break;
		}

		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SIZE:
		LayoutControls(hWnd);
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
BOOL WINAPI Create(
	_In_ HWND hWnd,
	_In_ const CREATESTRUCT FAR *lpParam)
{
	HWND hWndChild;
	UINT uDpi;
	BOOL bResult = FALSE;
	uDpi = GetDpiForWindow(hWnd);

	if (!CreatePictureBox(hWnd, lpParam->hInstance)) return FALSE;
	if (!CreateToolbar(hWnd, lpParam->hInstance)) return FALSE;

	hWndChild = CreateStatusBar(hWnd, lpParam->hInstance);

	if (hWndChild)
	{
		UpdateStatusParts(hWndChild, uDpi);
	}

	UpdateToolbarButtons(hWnd, lpParam->hInstance);
	return TRUE;
}

static
HWND WINAPI CreatePictureBox(
	_In_ HWND hWnd,
	_In_ HINSTANCE hInstance)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, WC_STATIC, NULL, WS_HWNDPICTURE, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDPICTURE, hInstance, NULL);
}

static
HWND WINAPI CreateStatusBar(
	_In_ HWND hWnd,
	_In_ HINSTANCE hInstance)
{
	HWND hWndStatus;
	hWndStatus = CreateWindow(STATUSCLASSNAME, NULL, WS_HWNDSTATUS, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDSTATUS, hInstance, NULL);

	if (hWndStatus)
	{
		SendMessage(hWndStatus, SB_SETPARTS, 0, 0);
	}

	return hWndStatus;
}

static
HWND WINAPI CreateToolbar(
	_In_ HWND hWnd,
	_In_ HINSTANCE hInstance)
{
	HWND hWndToolbar;
	hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_HWNDTOOLBAR, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDTOOLBAR, hInstance, NULL);

	if (hWndToolbar)
	{
		SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		//SendMessage(hWndToolbar, TB_ADDSTRING, (WPARAM)hInstance, IDS_TOOLBAR);
		SendMessage(hWndToolbar, TB_ADDBUTTONS, ARRAYSIZE(TOOLBARBUTTONS), (LPARAM)TOOLBARBUTTONS);
	}

	return hWndToolbar;
}

static
BOOL WINAPI DestroyImageList(
	_In_ HWND hWnd)
{
	HIMAGELIST hImageList;
	hImageList = (HIMAGELIST)SetWindowLongPtr(hWnd, GWLP_HIMAGELIST, 0);
	return hImageList && ImageList_Destroy(hImageList);
}

static
BOOL WINAPI GetToolbarText(
	_In_ HWND hWnd,
	_Inout_ LPTOOLTIPTEXT lpTooltip)
{
#if 0
	HWND hWndToolbar;
	BOOL bResult;
	TBBUTTONINFO info;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hWndToolbar)
	{
		info.cbSize = sizeof info;
		info.dwMask = TBIF_TEXT;
		info.pszText = lpTooltip->szText;
		info.cchText = ARRAYSIZE(lpTooltip->szText);
		bResult = SendMessage(hWndToolbar, TB_GETBUTTONINFO, lpTooltip->hdr.idFrom, (LPARAM)&info) >= 0;
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
#else
	HMENU hMenu;
	MENUITEMINFO info;
	BOOL bResult;
	hMenu = GetMenu(hWnd);

	if (hMenu)
	{
		ZeroMemory(&info, sizeof info);
		info.cbSize = sizeof info;
		info.fMask = MIIM_STRING;
		info.dwTypeData = lpTooltip->szText;
		info.cch = ARRAYSIZE(lpTooltip->szText);
		bResult = GetMenuItemInfo(hMenu, (UINT)lpTooltip->hdr.idFrom, FALSE, &info);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
#endif
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
			GetWindowRect(hWndChild, &rcWindow);
			rcClient.top += rcWindow.bottom - rcWindow.top;
		}
		if (hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS))
		{
			SendMessage(hWndChild, WM_SIZE, 0, 0);
			GetWindowRect(hWndChild, &rcWindow);
			rcClient.bottom -= rcWindow.bottom - rcWindow.top;
		}
		if (hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPICTURE))
		{
			rcClient.right -= rcClient.left;
			rcClient.bottom -= rcClient.top;
			MoveWindow(hWndChild, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, TRUE);
		}
	}

	return bResult;
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

static
BOOL WINAPI UpdateDpi(
	_In_ HWND hWnd,
	_In_ UINT uDpi)
{
	HWND hWndChild;
	hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndChild)
	{
		UpdateStatusParts(hWndChild, uDpi);
	}

	return FALSE;
}

static
BOOL WINAPI UpdateStatusParts(
	_In_ HWND hWndStatus,
	_In_ UINT uDpi)
{
	int nParts[] = { 100, 200, -1 };
	int nIndex;

	for (nIndex = 0; nIndex < ARRAYSIZE(nParts) - 1; nIndex++)
	{
		nParts[nIndex] = MulDiv(nParts[nIndex], uDpi, USER_DEFAULT_SCREEN_DPI);
	}

	return (BOOL)SendMessage(hWndStatus, SB_SETPARTS, ARRAYSIZE(nParts), (LPARAM)nParts);
}

static
BOOL WINAPI UpdateToolbarButtons(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HIMAGELIST hImageList;
	HWND hWndToolbar;
	BOOL bResult = FALSE;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hWndToolbar)
	{
		hImageList = (HIMAGELIST)SetWindowLongPtr(hWnd, GWLP_HIMAGELIST, 0);

		if (hImageList)
		{
			ImageList_Destroy(hImageList);
		}

		hImageList = ImageList_LoadImage(hInstance, MAKEINTRESOURCE(IDB_VIEWER16), 16, 0, CLR_DEFAULT, IMAGE_BITMAP, 0);
		SetWindowLongPtr(hWnd, GWLP_HIMAGELIST, (LONG_PTR)hImageList);
		SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

		if (hImageList)
		{
			//ImageList_AddMasked(hImageList, LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TOOLBAR)), CLR_DEFAULT);
			bResult = TRUE;

			HMENU hMenu;
			MENUITEMINFO info;
			hMenu = GetMenu(hWnd);

			if (hMenu)
			{
				ZeroMemory(&info, sizeof info);
				info.cbSize = sizeof info;
				info.fMask = MIIM_BITMAP;
				info.hbmpItem = HBMMENU_CALLBACK;
				SetMenuItemInfo(hMenu, IDM_OPEN, FALSE, &info);
				info.hbmpItem = HBMMENU_POPUP_CLOSE;
				SetMenuItemInfo(hMenu, IDM_EXIT, FALSE, &info);
			}
		}
	}

	return bResult;
}
