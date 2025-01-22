/*
Copyright 2025 Taichi Murakami.
フレーム ウィンドウ プロシージャを実装します。
*/

#define CXTOOLBARSMALL          16
#define CXTOOLBARLARGE          32
#define IMAGELISTMASKCOLOR      0xFF00FF
#define IMAGELISTBKCOLOR        0xFFFFFF
#define MAXLOADSTRING           32
#define ID_FIRSTCHILD           0x8000
#define ID_HWNDMDICLIENT        1
#define ID_HWNDTOOLBAR          2
#define ID_HWNDSTATUS           3
#define ID_HWNDTABCTRL          4
#define ID_WINDOWMENU           2
#define MINTRACKSIZE_X          300
#define MINTRACKSIZE_Y          200
#define TOOLBAR_NONE            0
#define TOOLBAR_SMALL           1
#define TOOLBAR_LARGE           2
#define TOOLBAR_NEW             0
#define TOOLBAR_OPEN            1
#define TOOLBAR_SAVE            2
#define TOOLBAR_PRINT           3
#define USERDATA_SHOWTOOLBAR            0x00000001
#define USERDATA_SHOWSTATUS             0x00000002
#define USERDATA_LARGETOOLBAR           0x00000004
#define USERDATA_DOCUMENTCHANGED        0x80000000

#include "paint.h"
#include <windows.h>
#include <commctrl.h> /* HIMAGELIST */

#define WM_ABOUT                0x0410
#define WM_CLOSEDOCUMENT        0x0418
#define WM_LAYOUT               0x0417
#define WM_NEW                  0x0416
#define WM_PAGESETUP            0x0414
#define WM_PREFERENCES          0x0411
#define WM_PRINTOUT             0x0413
#define WM_STATUS               0x0415
#define WM_TABCTRL              0x041B
#define WM_TOOLBAR              0x0412
#define WM_UPDATEDOCUMENT       0x041A
#define WM_UPDATETABCTRL        0x0419
#define WS_HWNDDOCUMENT         (WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDMDICLIENT        (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL)
#define WS_HWNDSTATUS           (WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | CCS_NODIVIDER)
#define WS_HWNDTABCTRL          (WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NODIVIDER | CCS_NORESIZE)
#define WS_HWNDTOOLBAR          (WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER)
#define WS_EX_HWNDMDICLIENT     WS_EX_CLIENTEDGE
#define WS_EX_HWNDSTATUS        0
#define WS_EX_HWNDTABCTRL       TBSTYLE_EX_MIXEDBUTTONS
#define WS_EX_HWNDTOOLBAR       0

typedef struct tagFRAMEWINDOWEXTRA
{
	LONG_PTR lpLayer;
	LONG_PTR lpMDIClient;
	LONG_PTR lpPalette;
	LONG_PTR lpStatus;
	LONG_PTR lpTabControl;
	LONG_PTR lpToolbar;
	LONG dwDpi;
	LONG dwFlags;
	WORD wLayerDock;
	WORD wPaletteDock;
} WINDOWEXTRA;

#define GWLP_HWNDLAYER          offsetof(WINDOWEXTRA, lpLayer)
#define GWLP_HWNDMDICLIENT      offsetof(WINDOWEXTRA, lpMDIClient)
#define GWLP_HWNDPALETTE        offsetof(WINDOWEXTRA, lpPalette)
#define GWLP_HWNDSTATUS         offsetof(WINDOWEXTRA, lpStatus)
#define GWLP_HWNDTABCTRL        offsetof(WINDOWEXTRA, lpTabControl)
#define GWLP_HWNDTOOLBAR        offsetof(WINDOWEXTRA, lpToolbar)
#define GWL_DPI                 offsetof(WINDOWEXTRA, dwDpi)
#define GWL_FLAGS               offsetof(WINDOWEXTRA, dwFlags)
#define GWW_LAYERDOCK           offsetof(WINDOWEXTRA, wLayerDock)
#define GWW_PALETTEDOCK         offsetof(WINDOWEXTRA, wPaletteDock)

static_assert(sizeof(WINDOWEXTRA) == FRAMEWINDOWEXTRA, "FRAMEWINDOWEXTRA");
static VOID WINAPI ClearDocumentChanged(_In_ HWND hWnd);
static HWND WINAPI CreateDocument(_In_ HWND hWnd);
static HWND WINAPI CreateMDIClient(_In_ HWND hWnd);
static HWND WINAPI CreateStatus(_In_ HWND hWnd);
static BOOL WINAPI CreateStatusParts(_In_ HWND hWnd);
static HWND WINAPI CreateTabControl(_In_ HWND hWnd);
static HWND WINAPI CreateToolbar(_In_ HWND hWnd);
static HIMAGELIST WINAPI CreateToolbarImageList(_In_ HWND hWnd);
static BOOL WINAPI DestroyToolbarImageList(_In_ HWND hWnd);
static BOOL WINAPI EnumDocumentWindows(_In_ HWND hWnd, _In_ WNDENUMPROC lpEnumFunc, _In_ LPARAM lParam);
static HWND WINAPI GetActiveDocument(_In_ HWND hWnd);
static BOOL WINAPI GetCommandInfo(_In_ HWND hWnd, _Inout_ LPNMTTDISPINFO lpInfo);
static BOOL WINAPI HasDocument(_In_ HWND hWnd);
static VOID WINAPI OnAbout(_In_ HWND hWnd);
static BOOL WINAPI OnClose(_In_ HWND hWnd);
static BOOL WINAPI OnCloseDocument(_In_ HWND hWnd, _In_ BOOL bForAll);
static BOOL CALLBACK OnCloseDocumentProc(_In_ HWND hWnd, _In_ LPARAM lParam);
static BOOL WINAPI OnCommand(_In_ HWND hWnd, _In_ UINT uCmd);
static BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ CONST CREATESTRUCT FAR *lpParam);
static VOID WINAPI OnDestroy(_In_ HWND hWnd);
static VOID WINAPI OnDocumentChanged(_In_ HWND hWnd);
static VOID WINAPI OnDpiChanged(_In_ HWND hWnd, _In_ UINT uDpi, _In_ CONST RECT FAR *lpWindow);
static VOID WINAPI OnGetMinMaxInfo(_In_ HWND hWnd, _Inout_ LPMINMAXINFO lpInfo);
static VOID WINAPI OnLayout(_In_ HWND hWnd);
static VOID WINAPI OnNew(_In_ HWND hWnd);
static BOOL WINAPI OnNotify(_In_ HWND hWnd, _Inout_ LPNMHDR lpNotify);
static VOID WINAPI OnPageSetup(_In_ HWND hWnd);
static BOOL WINAPI OnParentNotify(_In_ HWND hWnd, _In_ UINT uNotify, _In_ UINT idChild, _In_ LPARAM lParam);
static VOID WINAPI OnPreferences(_In_ HWND hWnd);
static VOID WINAPI OnPrintOut(_In_ HWND hWnd);
static VOID WINAPI OnSize(_In_ HWND hWnd, _In_ UINT uReason);
static VOID WINAPI OnStatus(_In_ HWND hWnd);
static VOID WINAPI OnTabControl(_In_ HWND hWnd);
static VOID WINAPI OnToolbar(_In_ HWND hWnd);
static VOID WINAPI OnUpdateDocument(_In_ HWND hWnd);
static LRESULT WINAPI SendMDIActiveMessage(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static LRESULT WINAPI SendExtraMessage(_In_ int nExtra, _In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
static BOOL WINAPI SetMenuItemChecked(_In_ HWND hWnd, _In_ UINT uItem, _In_ BOOL bCheck);
static BOOL WINAPI UpdateMenuItems(_In_ HWND hWnd);
static BOOL WINAPI UpdateTabControl(_In_ HWND hWnd);
static BOOL WINAPI UpdateToolbar(_In_ HWND hWnd);

#define DefProc(hWnd, uMsg, wParam, lParam) DefFrameProc((hWnd), (HWND)GetWindowLongPtr((hWnd), GWLP_HWNDMDICLIENT), (uMsg), (wParam), (lParam))
#define SendMDIClientMessage(hWnd, uMsg, wParam, lParam) SendExtraMessage(GWLP_HWNDMDICLIENT, (hWnd), (uMsg), (wParam), (lParam))

/* ドキュメントを操作するコマンドの配列。 */
static const
UINT DOCUMENTCOMMANDS[] =
{
	IDM_CLOSE,
	IDM_CLOSEALL,
	IDM_CASCADE,
	IDM_ICONARRANGE,
	IDM_TILEHORZ,
	IDM_TILEVERT,
	IDM_PRINT,
	IDM_SAVE,
	IDM_SAVEALL,
	IDM_SAVEAS,
};

/* ステータス バー。 */
static const
int STATUSPARTS[] =
{
	100,
	200,
	-1,
};

/* タブ コントロール。 */
static const
TBBUTTON TABCTRLBUTTONS[] =
{
	{ I_IMAGENONE, ID_FIRSTCHILD + 0, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 1, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 2, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 3, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 4, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 5, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 6, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 7, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, ID_FIRSTCHILD + 8, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
	{ I_IMAGENONE, 0, 0, BTNS_SEP },
	{ I_IMAGENONE, ID_FIRSTCHILD + 9, TBSTATE_HIDDEN, BTNS_AUTOSIZE | BTNS_SHOWTEXT },
};

/* ツールバー。 */
static const
TBBUTTON TOOLBARBUTTONS[] =
{
	{ TOOLBAR_NEW, IDM_NEW, TBSTATE_ENABLED },
	{ TOOLBAR_OPEN, IDM_OPEN, TBSTATE_ENABLED },
	{ I_IMAGENONE, 0, 0, BTNS_SEP },
	{ TOOLBAR_SAVE, IDM_SAVE, 0 },
	{ TOOLBAR_PRINT, IDM_PRINT, 0 },
};

/* ドキュメントを操作するコマンドの配列。 */
static const
UINT TOOLBARCOMMANDS[] =
{
	IDM_PRINT,
	IDM_SAVE,
};

#define MAXDOCUMENTCOMMANDS     ARRAYSIZE(DOCUMENTCOMMANDS)
#define MAXSTATUSPARTS          ARRAYSIZE(STATUSPARTS)
#define MAXTABCTRLBUTTONS       ARRAYSIZE(TABCTRLBUTTONS)
#define MAXTOOLBARBUTTONS       ARRAYSIZE(TOOLBARBUTTONS)
#define MAXTOOLBARCOMMANDS      ARRAYSIZE(TOOLBARCOMMANDS)

/*******************************************************************************
フレーム ウィンドウ プロシージャ。
*******************************************************************************/
EXTERN_C
LRESULT CALLBACK FrameWindowProc(
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
		if (OnClose(hWnd)) nResult = DefProc(hWnd, uMsg, wParam, lParam);
		else nResult = -1;
		break;
	case WM_CLOSEDOCUMENT:
		nResult = OnCloseDocument(hWnd, (BOOL)wParam);
		break;
	case WM_COMMAND:
		if (OnCommand(hWnd, LOWORD(wParam))) nResult = 0;
		else nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_CREATE:
		if (OnCreate(hWnd, (LPCREATESTRUCT)lParam)) nResult = DefProc(hWnd, uMsg, wParam, lParam);
		else nResult = -1;
		break;
	case WM_DESTROY:
		OnDestroy(hWnd);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DOCUMENTCHANGED:
		OnDocumentChanged(hWnd);
		nResult = 0;
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
	case WM_MDICASCADE:
	case WM_MDIICONARRANGE:
	case WM_MDITILE:
		nResult = SendMDIClientMessage(hWnd, uMsg, wParam, lParam);
		break;
	case WM_NEW:
		OnNew(hWnd);
		nResult = 0;
		break;
	case WM_NOTIFY:
		if (OnNotify(hWnd, (LPNMHDR)lParam)) nResult = 0;
		else nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_PAGESETUP:
		OnPageSetup(hWnd);
		nResult = 0;
		break;
	case WM_PARENTNOTIFY:
		if (OnParentNotify(hWnd, LOWORD(wParam), HIWORD(wParam), lParam)) nResult = 0;
		else nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_PREFERENCES:
		OnPreferences(hWnd);
		nResult = 0;
		break;
	case WM_PRINTOUT:
		OnPrintOut(hWnd);
		nResult = 0;
		break;
	case WM_SIZE:
		OnSize(hWnd, LOWORD(wParam));
		nResult = 0;
		break;
	case WM_STATUS:
		OnStatus(hWnd);
		nResult = 0;
		break;
	case WM_TABCTRL:
		OnTabControl(hWnd);
		nResult = 0;
		break;
	case WM_TOOLBAR:
		OnToolbar(hWnd);
		nResult = 0;
		break;
	case WM_UPDATEDOCUMENT:
		OnUpdateDocument(hWnd);
		nResult = 0;
		break;
	default:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return nResult;
}

/*******************************************************************************
ドキュメント保留状態を解除します。
*******************************************************************************/
static
VOID WINAPI ClearDocumentChanged(
	_In_ HWND hWnd)
{
	DWORD dwFlags;
	dwFlags = GetWindowLong(hWnd, GWL_FLAGS);
	dwFlags &= ~USERDATA_DOCUMENTCHANGED;
	SetWindowLong(hWnd, GWL_FLAGS, dwFlags);
}

/*******************************************************************************
MDI クライアント上に新しいドキュメント ウィンドウを作成します。
*******************************************************************************/
static
HWND WINAPI CreateDocument(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	HWND hWndDocument;
	DOCUMENTCREATESTRUCT param;
	TCHAR strCaption[MAXLOADSTRING];
	hWndDocument = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT);

	if (hWndDocument)
	{
		hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		LoadString(hInstance, IDS_UNTITLED, strCaption, MAXLOADSTRING);
		ZeroMemory(&param, sizeof param);
		param.hWndFrame = hWnd;
		hWndDocument = CreateMDIWindow(DOCUMENTCLASSNAME, strCaption, WS_HWNDDOCUMENT, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWndDocument, hInstance, (LPARAM)&param);
	}
	else
	{
		hWndDocument = NULL;
	}

	return hWndDocument;
}

/*******************************************************************************
新しい MDI クライアントを作成します。
*******************************************************************************/
static
HWND WINAPI CreateMDIClient(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	CLIENTCREATESTRUCT param;
	ZeroMemory(&param, sizeof param);
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	param.hWindowMenu = GetMenu(hWnd);
	param.idFirstChild = ID_FIRSTCHILD;

	if (param.hWindowMenu)
	{
		param.hWindowMenu = GetSubMenu(param.hWindowMenu, ID_WINDOWMENU);
	}
	else
	{
		param.hWindowMenu = NULL;
	}

	return CreateWindowEx(WS_EX_HWNDMDICLIENT, MDICLIENTCLASSNAME, NULL, WS_HWNDMDICLIENT, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDMDICLIENT, hInstance, &param);
}

/*******************************************************************************
新しいステータス ウィンドウを作成します。
*******************************************************************************/
static
HWND WINAPI CreateStatus(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	return CreateWindowEx(WS_EX_HWNDSTATUS, STATUSCLASSNAME, NULL, WS_HWNDSTATUS, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDSTATUS, hInstance, NULL);
}

/*******************************************************************************
現在のステータス パネルを再配置します。
*******************************************************************************/
static
BOOL WINAPI CreateStatusParts(
	_In_ HWND hWnd)
{
	HWND hWndStatus;
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndStatus)
	{
		SendMessage(hWndStatus, SB_SETPARTS, MAXSTATUSPARTS, (LPARAM)STATUSPARTS);
		SendMessage(hWndStatus, SB_SETTEXT, 0, (LPARAM)TEXT("Test"));
	}

	return hWndStatus != NULL;
}

/*******************************************************************************
新しいタブ コントロールを作成します。
*******************************************************************************/
static
HWND WINAPI CreateTabControl(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	HWND hWndToolbar;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	hWndToolbar = CreateWindowEx(WS_EX_HWNDTABCTRL, TOOLBARCLASSNAME, NULL, WS_HWNDTABCTRL, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDTABCTRL, hInstance, NULL);

	if (hWndToolbar)
	{
		SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		SendMessage(hWndToolbar, TB_ADDBUTTONS, MAXTABCTRLBUTTONS, (LPARAM)TABCTRLBUTTONS);
	}

	return hWndToolbar;
}

/*******************************************************************************
新しいツールバー ウィンドウを作成します。
*******************************************************************************/
static
HWND WINAPI CreateToolbar(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	HWND hWndToolbar;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	hWndToolbar = CreateWindowEx(WS_EX_HWNDTOOLBAR, TOOLBARCLASSNAME, NULL, WS_HWNDTOOLBAR, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDTOOLBAR, hInstance, NULL);

	if (hWndToolbar)
	{
		SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		SendMessage(hWndToolbar, TB_ADDBUTTONS, MAXTOOLBARBUTTONS, (LPARAM)TOOLBARBUTTONS);
	}

	return hWndToolbar;
}

/*******************************************************************************
現在のツールバーにイメージ リストを関連付けます。
*******************************************************************************/
static
HIMAGELIST WINAPI CreateToolbarImageList(
	_In_ HWND hWnd)
{
	HIMAGELIST hHotImageList, hImageList;
	HINSTANCE hInstance;
	HWND hWndToolbar;
	LPCTSTR lpToolbar, lpToolbarHot;
	DWORD dwFlags;
	UINT cxToolbar;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hWndToolbar)
	{
		hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		dwFlags = GetWindowLong(hWnd, GWL_FLAGS);

		if (dwFlags & USERDATA_LARGETOOLBAR)
		{
			cxToolbar = CXTOOLBARLARGE;
			lpToolbar = MAKEINTRESOURCE(IDB_TOOLBARLARGE);
			lpToolbarHot = MAKEINTRESOURCE(IDB_TOOLBARLARGEHOT);
		}
		else
		{
			cxToolbar = CXTOOLBARSMALL;
			lpToolbar = MAKEINTRESOURCE(IDB_TOOLBARSMALL);
			lpToolbarHot = MAKEINTRESOURCE(IDB_TOOLBARSMALLHOT);
		}

		hImageList = ImageList_LoadBitmap(hInstance, lpToolbar, cxToolbar, 0, IMAGELISTMASKCOLOR);
		hHotImageList = (HIMAGELIST)SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

		if (hHotImageList)
		{
			ImageList_Destroy(hHotImageList);
		}

		hHotImageList = ImageList_LoadBitmap(hInstance, lpToolbarHot, cxToolbar, 0, IMAGELISTMASKCOLOR);
		hHotImageList = (HIMAGELIST)SendMessage(hWndToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)hHotImageList);

		if (hHotImageList)
		{
			ImageList_Destroy(hHotImageList);
		}
	}
	else
	{
		hImageList = NULL;
	}

	return hImageList;
}

/*******************************************************************************
ツールバーに関連付けられたイメージ リストを破棄します。
*******************************************************************************/
static
BOOL WINAPI DestroyToolbarImageList(
	_In_ HWND hWnd)
{
	HIMAGELIST hImageList;
	hWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hWnd)
	{
		hImageList = (HIMAGELIST)SendMessage(hWnd, TB_SETIMAGELIST, 0, 0);

		if (hImageList)
		{
			ImageList_Destroy(hImageList);
		}

		hImageList = (HIMAGELIST)SendMessage(hWnd, TB_SETHOTIMAGELIST, 0, 0);

		if (hImageList)
		{
			ImageList_Destroy(hImageList);
		}
	}

	return hWnd != NULL;
}

/*******************************************************************************
MDI クライアント上のドキュメント ウィンドウを列挙します。
*******************************************************************************/
static
BOOL WINAPI EnumDocumentWindows(
	_In_ HWND hWnd,
	_In_ WNDENUMPROC lpEnumFunc,
	_In_ LPARAM lParam)
{
	HWND hWndNext;
	BOOL bResult;
	hWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT);

	if (hWnd)
	{
		hWnd = GetWindow(hWnd, GW_CHILD);

		if (hWnd == NULL)
		{
			bResult = TRUE;
		}
		else do
		{
			hWndNext = GetWindow(hWnd, GW_HWNDNEXT);
			bResult = lpEnumFunc(hWnd, lParam);

			if (bResult)
			{
				hWnd = hWndNext;
			}
			else
			{
				hWnd = NULL;
			}
		} while (hWnd);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

/*******************************************************************************
アクティブ ドキュメント ウィンドウへのハンドルを返します。
*******************************************************************************/
static
HWND WINAPI GetActiveDocument(
	_In_ HWND hWnd)
{
	hWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT);

	if (hWnd)
	{
		hWnd = (HWND)SendMessage(hWnd, WM_MDIGETACTIVE, 0, 0);
	}
	else
	{
		hWnd = NULL;
	}

	return hWnd;
}

/*******************************************************************************
指定したコマンド ID に関連付けられたメニュー項目から文字列を取得します。
*******************************************************************************/
static
BOOL WINAPI GetCommandInfo(
	_In_ HWND hWnd,
	_Inout_ LPNMTTDISPINFO lpInfo)
{
	HMENU hMenu;
	MENUITEMINFO item;
	BOOL bResult = FALSE;
	hMenu = GetMenu(hWnd);

	if (hMenu)
	{
		ZeroMemory(&item, sizeof item);
		item.cbSize = sizeof item;
		item.fMask = MIIM_STRING;
		item.dwTypeData = lpInfo->szText;
		item.cch = ARRAYSIZE(lpInfo->szText);
		GetMenuItemInfo(hMenu, (UINT)lpInfo->hdr.idFrom, FALSE, &item);
	}

	return bResult;
}

/*******************************************************************************
MDI ウィンドウが存在する場合は TRUE を返します。
*******************************************************************************/
static
BOOL WINAPI HasDocument(
	_In_ HWND hWnd)
{
	hWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT);

	if (hWnd)
	{
		hWnd = GetWindow(hWnd, GW_CHILD);
	}
	else
	{
		hWnd = NULL;
	}

	return hWnd != NULL;
}

/*******************************************************************************
バージョン情報ダイアログ ボックスを表示します。
*******************************************************************************/
static
VOID WINAPI OnAbout(
	_In_ HWND hWnd)
{
	HINSTANCE hInstance;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
}

/*******************************************************************************
WM_CLOSE:
ドキュメント ウィンドウを閉じた場合は TRUE を返します。
*******************************************************************************/
static
BOOL WINAPI OnClose(
	_In_ HWND hWnd)
{
	return (BOOL)SendMessage(hWnd, WM_CLOSEDOCUMENT, TRUE, 0);
}

/*******************************************************************************
各ドキュメント ウィンドウを閉じます。
@param bForAll: すべてのドキュメント ウィンドウを閉じます。
@return ドキュメント ウィンドウを閉じた場合は TRUE を返します。
*******************************************************************************/
static
BOOL WINAPI OnCloseDocument(
	_In_ HWND hWnd,
	_In_ BOOL bForAll)
{
	BOOL bResult;

	if (bForAll)
	{
		bResult = EnumDocumentWindows(hWnd, OnCloseDocumentProc, 0);
	}
	else
	{
		bResult = !SendMDIActiveMessage(hWnd, WM_CLOSE, 0, 0);
	}

	return bResult;
}

/*******************************************************************************
指定したウィンドウを閉じます。
@return ウィンドウを閉じた場合は TRUE を返します。
*******************************************************************************/
static
BOOL CALLBACK OnCloseDocumentProc(
	_In_ HWND hWnd,
	_In_ LPARAM lParam)
{
	return !SendMessage(hWnd, WM_CLOSE, 0, 0);
}

/*******************************************************************************
WM_COMMAND:
@return コマンドを処理した場合は TRUE を返します。
*******************************************************************************/
static
BOOL WINAPI OnCommand(
	_In_ HWND hWnd,
	_In_ UINT uCmd)
{
	WPARAM wParam = 0;

	switch (uCmd)
	{
	case IDM_ABOUT:
		uCmd = WM_ABOUT;
		break;
	case IDM_CLOSE:
		uCmd = WM_CLOSEDOCUMENT;
		break;
	case IDM_CLOSEALL:
		uCmd = WM_CLOSEDOCUMENT;
		wParam = TRUE;
		break;
	case IDM_EXIT:
		uCmd = WM_CLOSE;
		break;
	case IDM_CASCADE:
		uCmd = WM_MDICASCADE;
		break;
	case IDM_ICONARRANGE:
		uCmd = WM_MDIICONARRANGE;
		break;
	case IDM_TILEHORZ:
		uCmd = WM_MDITILE;
		wParam = MDITILE_HORIZONTAL;
		break;
	case IDM_TILEVERT:
		uCmd = WM_MDITILE;
		wParam = MDITILE_VERTICAL;
		break;
	case IDM_NEW:
		uCmd = WM_NEW;
		break;
	case IDM_PAGESETUP:
		uCmd = WM_PAGESETUP;
		break;
	case IDM_PREFERENCES:
		uCmd = WM_PREFERENCES;
		break;
	case IDM_PRINT:
		uCmd = WM_PRINTOUT;
		break;
	case IDM_STATUS:
		uCmd = WM_STATUS;
		break;
	case IDM_TABCTRL:
		uCmd = WM_TABCTRL;
		break;
	case IDM_TOOLBAR:
		uCmd = WM_TOOLBAR;
		break;
	default:
		uCmd = 0;
		break;
	}
	if (uCmd)
	{
		PostMessage(hWnd, uCmd, wParam, 0);
	}

	return uCmd;
}

/*******************************************************************************
WM_CREATE:
*******************************************************************************/
static
BOOL WINAPI OnCreate(
	_In_ HWND hWnd,
	_In_ CONST CREATESTRUCT FAR *lpParam)
{
	SetWindowLong(hWnd, GWL_DPI, GetDpiForWindow(hWnd));
	SetWindowLong(hWnd, GWL_FLAGS, USERDATA_SHOWTOOLBAR);
	return CreateMDIClient(hWnd) &&
		CreateStatus(hWnd) &&
		CreateStatusParts(hWnd) &&
		CreateToolbar(hWnd) &&
		CreateToolbarImageList(hWnd) &&
		CreateTabControl(hWnd);
}

/*******************************************************************************
WM_DESTROY:
現在のウィンドウが保持している資源を解放します。
*******************************************************************************/
static
VOID WINAPI OnDestroy(
	_In_ HWND hWnd)
{
	DestroyToolbarImageList(hWnd);
	PostQuitMessage(0);
}

/*******************************************************************************
ドキュメントに関する更新を保留状態にします。
*******************************************************************************/
static
VOID WINAPI OnDocumentChanged(
	_In_ HWND hWnd)
{
	DWORD dwFlags;
	dwFlags = GetWindowLong(hWnd, GWL_FLAGS);

	if (!(dwFlags & USERDATA_DOCUMENTCHANGED))
	{
		dwFlags |= USERDATA_DOCUMENTCHANGED;
		SetWindowLong(hWnd, GWL_FLAGS, dwFlags);
		PostMessage(hWnd, WM_UPDATEDOCUMENT, 0, 0);
	}
}

/*******************************************************************************
WM_DPICHANGED:
新しい DPI を保存します。
*******************************************************************************/
static
VOID WINAPI OnDpiChanged(
	_In_ HWND hWnd,
	_In_ UINT uDpi,
	_In_ CONST RECT FAR *lpWindow)
{
	SetWindowLong(hWnd, GWL_DPI, uDpi);
	MoveWindowForRect(hWnd, lpWindow, TRUE);
}

/*******************************************************************************
WM_GETMINMAXINFO:
現在のウィンドウの最小サイズを指定します。
*******************************************************************************/
static
VOID WINAPI OnGetMinMaxInfo(
	_In_ HWND hWnd,
	_Inout_ LPMINMAXINFO lpInfo)
{
	UINT uDpi;
	uDpi = GetWindowLong(hWnd, GWL_DPI);
	SetMinMaxInfoForDpi(lpInfo, MINTRACKSIZE_X, MINTRACKSIZE_Y, uDpi);
}

/*******************************************************************************
各子ウィンドウを再配置します。
*******************************************************************************/
static
VOID WINAPI OnLayout(
	_In_ HWND hWnd)
{
	HWND hWndChild;
	RECT rcClient, rcWindow;

	if (GetClientRect(hWnd, &rcClient))
	{
		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

		/* ステータス バー */
		if (hWndChild)
		{
			SendMessage(hWndChild, WM_SIZE, 0, 0);

			if (GetWindowRect(hWndChild, &rcWindow))
			{
				rcClient.bottom -= rcWindow.bottom - rcWindow.top;
			}
		}

		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTABCTRL);

		/* タブ コントロール */
		if (hWndChild)
		{
			rcWindow.left = MAXLONG;
			rcWindow.top = rcClient.bottom;
			SendMessage(hWndChild, TB_GETIDEALSIZE, TRUE, (LPARAM)&rcWindow);
			rcClient.bottom -= rcWindow.top;
			MoveWindow(hWndChild, 0, rcClient.bottom, rcClient.right, rcWindow.top, TRUE);
		}

		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

		/* ツールバー */
		if (hWndChild)
		{
			SendMessage(hWndChild, TB_AUTOSIZE, 0, 0);

			if (GetWindowRect(hWndChild, &rcWindow))
			{
				rcClient.top += rcWindow.bottom - rcWindow.top;
			}
		}

		hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT);

		/* MDI クライアント */
		if (hWndChild)
		{
			rcClient.right -= rcClient.left;
			rcClient.bottom -= rcClient.top;
			MoveWindow(hWndChild, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, TRUE);
		}
	}
}

/*******************************************************************************
新しいドキュメントを作成します。
*******************************************************************************/
static
VOID WINAPI OnNew(
	_In_ HWND hWnd)
{
	CreateDocument(hWnd);
}

/*******************************************************************************
WM_NOTIFY:
*******************************************************************************/
static
BOOL WINAPI OnNotify(
	_In_ HWND hWnd,
	_Inout_ LPNMHDR lpNotify)
{
	BOOL bResult;

	if (lpNotify->code == TTN_NEEDTEXT)
	{
		bResult = GetCommandInfo(hWnd, (LPNMTTDISPINFO)lpNotify);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

/*******************************************************************************
ページ設定ダイアログ ボックスを表示します。
*******************************************************************************/
static
VOID WINAPI OnPageSetup(
	_In_ HWND hWnd)
{
	PAGESETUPDLG param;
	ZeroMemory(&param, sizeof param);
	param.lStructSize = sizeof param;
	param.hwndOwner = hWnd;
	PageSetupDlg(&param);
}

/*******************************************************************************
WM_PARENTNOTIFY:
作成したウィンドウ ハンドルを保持します。
メニュー項目にチェック記号を付けます。
*******************************************************************************/
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
		case ID_HWNDMDICLIENT:
			SetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDSTATUS:
			SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, lParam);
			SetMenuItemChecked(hWnd, IDM_STATUS, TRUE);
			bResult = TRUE;
			break;
		case ID_HWNDTABCTRL:
			SetWindowLongPtr(hWnd, GWLP_HWNDTABCTRL, lParam);
			SetMenuItemChecked(hWnd, IDM_TABCTRL, TRUE);
			bResult = TRUE;
			break;
		case ID_HWNDTOOLBAR:
			SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, lParam);
			SetMenuItemChecked(hWnd, IDM_TOOLBAR, TRUE);
			bResult = TRUE;
			break;
		}

		break;
	case WM_DESTROY:
		switch (idChild)
		{
		case ID_HWNDMDICLIENT:
			SetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT, 0);
			bResult = TRUE;
			break;
		case ID_HWNDSTATUS:
			SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, 0);
			SetMenuItemChecked(hWnd, IDM_STATUS, FALSE);
			bResult = TRUE;
			break;
		case ID_HWNDTABCTRL:
			SetWindowLongPtr(hWnd, GWLP_HWNDTABCTRL, 0);
			SetMenuItemChecked(hWnd, IDM_TABCTRL, FALSE);
			bResult = TRUE;
			break;
		case ID_HWNDTOOLBAR:
			DestroyToolbarImageList(hWnd);
			SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, 0);
			SetMenuItemChecked(hWnd, IDM_TOOLBAR, FALSE);
			bResult = TRUE;
			break;
		}

		break;
	}

	return bResult;
}

/*******************************************************************************
環境設定ダイアログ ボックスを表示します。
*******************************************************************************/
static
VOID WINAPI OnPreferences(
	_In_ HWND hWnd)
{
	//PROPSHEETHEADER header;
	//PROPSHEETPAGE page;
	//HPROPSHEETPAGE pages[MAXPREFERENCEPAGES];
	//UINT uIndex;
	//ZeroMemory(&header, sizeof header);
	//ZeroMemory(&page, sizeof page);
	//header.dwSize = sizeof header;
	//header.dwFlags = PSH_PROPTITLE | PSH_USEHICON | PSH_NOCONTEXTHELP;
	//header.hwndParent = hWnd;
	//header.hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	//header.hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
	//header.nPages = MAXPREFERENCEPAGES;
	//header.phpage = pages;
	//page.dwSize = sizeof page;
	//page.hInstance = header.hInstance;

	//for (uIndex = 0; uIndex < MAXPREFERENCEPAGES; uIndex++)
	//{
	//	page.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_LARGE);
	//	page.pfnDlgProc = AboutDialogProc;
	//	pages[uIndex] = CreatePropertySheetPage(&page);
	//}

	//PropertySheet(&header);
}

/*******************************************************************************
印刷ダイアログ ボックスを表示します。
*******************************************************************************/
static
VOID WINAPI OnPrintOut(
	_In_ HWND hWnd)
{
	PRINTDLG param;
	ZeroMemory(&param, sizeof param);
	param.lStructSize = sizeof param;
	param.hwndOwner = hWnd;
	PrintDlg(&param);
}

/*******************************************************************************
WM_SIZE:
*******************************************************************************/
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

/*******************************************************************************
ステータス バーを表示または削除します。
*******************************************************************************/
static
VOID WINAPI OnStatus(
	_In_ HWND hWnd)
{
	HWND hWndStatus;
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndStatus)
	{
		SendMessage(hWndStatus, WM_CLOSE, 0, 0);
	}
	else if (CreateStatus(hWnd))
	{
		CreateStatusParts(hWnd);
	}

	SendMessage(hWnd, WM_LAYOUT, 0, 0);
}

/*******************************************************************************
タブ コントロールを表示または削除します。
*******************************************************************************/
static
VOID WINAPI OnTabControl(
	_In_ HWND hWnd)
{
	HWND hWndToolbar;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTABCTRL);

	if (hWndToolbar)
	{
		SendMessage(hWndToolbar, WM_CLOSE, 0, 0);
	}
	else if (CreateTabControl(hWnd))
	{
		UpdateTabControl(hWnd);
	}

	SendMessage(hWnd, WM_LAYOUT, 0, 0);
}

/*******************************************************************************
ツールバーを表示または削除します。
*******************************************************************************/
static
VOID WINAPI OnToolbar(
	_In_ HWND hWnd)
{
	HWND hWndToolbar;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hWndToolbar)
	{
		SendMessage(hWndToolbar, WM_CLOSE, 0, 0);
	}
	else if (CreateToolbar(hWnd))
	{
		CreateToolbarImageList(hWnd);
	}

	SendMessage(hWnd, WM_LAYOUT, 0, 0);
}

/*******************************************************************************
ドキュメントに関する表示を更新します。
*******************************************************************************/
static VOID WINAPI OnUpdateDocument(
	_In_ HWND hWnd)
{
	ClearDocumentChanged(hWnd);
	UpdateMenuItems(hWnd);
	UpdateToolbar(hWnd);
	UpdateTabControl(hWnd);
}

/*******************************************************************************
現在のドキュメント ウィンドウにメッセージを投稿します。
*******************************************************************************/
static
LRESULT WINAPI SendMDIActiveMessage(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult = 0;
	hWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDMDICLIENT);

	if (hWnd)
	{
		hWnd = (HWND)SendMessage(hWnd, WM_MDIGETACTIVE, 0, 0);

		if (hWnd)
		{
			nResult = SendMessage(hWnd, uMsg, wParam, lParam);
		}
	}

	return nResult;
}

/*******************************************************************************
指定した子ウィンドウにメッセージを投稿します。
*******************************************************************************/
static
LRESULT WINAPI SendExtraMessage(
	_In_ int nExtra,
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	LRESULT nResult;
	hWnd = (HWND)GetWindowLongPtr(hWnd, nExtra);

	if (hWnd)
	{
		nResult = SendMessage(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		nResult = 0;
	}

	return nResult;
}

/*******************************************************************************
指定したメニュー項目にチェック記号を付けます。
*******************************************************************************/
static
BOOL WINAPI SetMenuItemChecked(
	_In_ HWND hWnd,
	_In_ UINT uItem,
	_In_ BOOL bCheck)
{
	HMENU hMenu;
	MENUITEMINFO item;
	BOOL bResult = FALSE;
	hMenu = GetMenu(hWnd);

	if (hMenu)
	{
		ZeroMemory(&item, sizeof item);
		item.cbSize = sizeof item;
		item.fMask = MIIM_STATE;

		if (GetMenuItemInfo(hMenu, uItem, FALSE, &item))
		{
			if (bCheck)
			{
				item.fState |= MF_CHECKED;
			}
			else
			{
				item.fState &= ~MF_CHECKED;
			}

			bResult = SetMenuItemInfo(hMenu, uItem, FALSE, &item);
		}
	}

	return bResult;
}

/*******************************************************************************
ドキュメント ウィンドウが存在する場合はドキュメント コマンドを有効化します。
*******************************************************************************/
static
BOOL WINAPI UpdateMenuItems(
	_In_ HWND hWnd)
{
	HMENU hMenu;
	MENUITEMINFO item;
	UINT uIndex;
	hMenu = GetMenu(hWnd);

	if (hMenu)
	{
		ZeroMemory(&item, sizeof item);
		item.cbSize = sizeof item;
		item.fMask = MIIM_STATE;

		if (!HasDocument(hWnd))
		{
			item.fState = MF_DISABLED;
		}
		for (uIndex = 0; uIndex < MAXDOCUMENTCOMMANDS; uIndex++)
		{
			SetMenuItemInfo(hMenu, DOCUMENTCOMMANDS[uIndex], FALSE, &item);
		}
	}

	return hMenu != NULL;
}

/*******************************************************************************
タブ コントロール上のボタン テキストを更新します。
現在のメニュー項目から各ボタンに適したテキストを取得します。
*******************************************************************************/
static
BOOL WINAPI UpdateTabControl(
	_In_ HWND hWnd)
{
	HMENU hMenu;
	HWND hWndToolbar;
	TBBUTTONINFO button;
	MENUITEMINFO item;
	UINT uCommand, uIndex;
	BOOL bResult = FALSE;
	TCHAR strText[MAXLOADSTRING];
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTABCTRL);

	if (hWndToolbar)
	{
		hMenu = GetMenu(hWnd);

		if (hMenu)
		{
			ZeroMemory(&button, sizeof button);
			ZeroMemory(&item, sizeof item);
			button.cbSize = sizeof button;
			button.pszText = strText;
			item.cbSize = sizeof item;
			item.fMask = MIIM_STATE | MIIM_STRING;
			item.dwTypeData = strText;
			bResult = TRUE;

			for (uIndex = 0; uIndex < MAXTABCTRLBUTTONS; uIndex++)
			{
				uCommand = TABCTRLBUTTONS[uIndex].idCommand;

				if (uCommand)
				{
					item.cch = MAXLOADSTRING;

					if (GetMenuItemInfo(hMenu, uCommand, FALSE, &item))
					{
						button.dwMask = TBIF_TEXT | TBIF_STATE;
						button.fsState = TBSTATE_ENABLED;

						if (item.fState & MF_CHECKED)
						{
							button.fsState |= TBSTATE_CHECKED;
						}
					}
					else
					{
						button.dwMask = TBIF_STATE;
						button.fsState = TBSTATE_HIDDEN;
					}

					SendMessage(hWndToolbar, TB_SETBUTTONINFO, uCommand, (LPARAM)&button);
				}
			}
		}
	}

	return bResult;
}

/*******************************************************************************
ドキュメント ウィンドウが存在する場合はドキュメント コマンドを有効化します。
*******************************************************************************/
static
BOOL WINAPI UpdateToolbar(
	_In_ HWND hWnd)
{
	HWND hWndToolbar;
	TBBUTTONINFO button;
	UINT uIndex;
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hWndToolbar)
	{
		ZeroMemory(&button, sizeof button);
		button.cbSize = sizeof button;
		button.dwMask = TBIF_STATE;

		if (HasDocument(hWnd))
		{
			button.fsState = TBSTATE_ENABLED;
		}
		else
		{
			button.fsState = 0;
		}
		for (uIndex = 0; uIndex < MAXTOOLBARCOMMANDS; uIndex++)
		{
			SendMessage(hWndToolbar, TB_SETBUTTONINFO, TOOLBARCOMMANDS[uIndex], (LPARAM)&button);
		}
	}

	return hWndToolbar != NULL;
}
