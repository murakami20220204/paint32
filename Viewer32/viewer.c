/*
Copyright 2025 Taichi Murakami.
ビューアー ウィンドウ プロシージャを実装します。
*/

#include <windows.h>
#include <commctrl.h>
#include <pathcch.h>
#include <strsafe.h>
#include "Viewer32.h"
#include "resource.h"

/* User Data */
typedef struct tagUSERDATA
{
	IWICBitmapDecoder *pDecoder;
	IWICImagingFactory *pFactory;
	TCHAR strFileName[PATHCCH_MAX_CCH];
} USERDATA, FAR *LPUSERDATA;

/* Window Extra */
typedef struct tagWINDOWEXTRA
{
	LONG_PTR lpBitmap;
	LONG_PTR lpImageList;
	LONG_PTR lpPictureWindow;
	LONG_PTR lpStatusWindow;
	LONG_PTR lpToolbarWindow;
	LONG dwDpi;
	LONG dwFlags;
	LONG dwFrameCount;
	LONG dwFrameIndex;
	LONG dwHeight;
	LONG dwWidth;
	LONG dwZoom;
} WINDOWEXTRA;

typedef struct tagZOOMSTRUCT
{
	UINT uNext;
	UINT uPrevious;
} ZOOMSTRUCT, FAR *LPZOOMSTRUCT;

#define ALPHATHRESHOLDPERCENT   1.0
#define CXTOOLBAR               32
#define DefProc                 DefWindowProc
#define MAXZOOM                 2000
#define MINZOOM                 10
#define NUMSTATUSPARTS          ARRAYSIZE(STATUSPARTS)
#define NUMTOOLBARBUTTONS       ARRAYSIZE(TOOLBARBUTTONS)
#define OPENFILEFILTER          TEXT("All Files (*.*)\0*.*\0")
#define ZOOMFACTOR              25
#define DEFAULT_ZOOM            100
#define DEFAULT_ZOOMINDEX       4
#define GWL_DPI                 offsetof(WINDOWEXTRA, dwDpi)
#define GWL_FLAGS               offsetof(WINDOWEXTRA, dwFlags)
#define GWL_FRAMECOUNT          offsetof(WINDOWEXTRA, dwFrameCount)
#define GWL_FRAMEINDEX          offsetof(WINDOWEXTRA, dwFrameIndex)
#define GWL_HEIGHT              offsetof(WINDOWEXTRA, dwHeight)
#define GWL_WIDTH               offsetof(WINDOWEXTRA, dwWidth)
#define GWL_ZOOM                offsetof(WINDOWEXTRA, dwZoom)
#define GWLP_HBITMAP            offsetof(WINDOWEXTRA, lpBitmap)
#define GWLP_HIMAGELIST         offsetof(WINDOWEXTRA, lpImageList)
#define GWLP_HWNDPICTURE        offsetof(WINDOWEXTRA, lpPictureWindow)
#define GWLP_HWNDSTATUS         offsetof(WINDOWEXTRA, lpStatusWindow)
#define GWLP_HWNDTOOLBAR        offsetof(WINDOWEXTRA, lpToolbarWindow)
#define ID_HWNDPICTURE          1
#define ID_HWNDSTATUS           2
#define ID_HWNDTOOLBAR          3
#define ID_STATUSZOOM           0
#define ID_STATUSFRAME          1
#define ID_STATUSSIZE           2
#define ID_TOOLBAROPEN          0
#define ID_TOOLBAROPENNEXT      1
#define ID_TOOLBAROPENPRIOR     2
#define ID_TOOLBARPAGENEXT      3
#define ID_TOOLBARPAGEPRIOR     4
#define ID_TOOLBARZOOMIN        5
#define ID_TOOLBARZOOMOUT       6
#define ID_TOOLBARZOOM          7
#define LOADSTRING_MAX          32
#define MINTRACKSIZE_X          200
#define MINTRACKSIZE_Y          200
#define OFN_ONOPEN              (OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER)
#define USERDATA_SHOWSTATUS     0x80000000
#define USERDATA_SHOWTOOLBAR    0x40000000
#define WS_HWNDPICTURE          (WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | SS_OWNERDRAW)
#define WS_HWNDSTATUS           (WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | CCS_NODIVIDER)
#define WS_HWNDTOOLBAR          (WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | TBSTYLE_FLAT | CCS_NODIVIDER)
#define WS_EX_HWNDPICTURE       WS_EX_CLIENTEDGE
#define WS_EX_HWNDSTATUS        0
#define WS_EX_HWNDTOOLBAR       TBSTYLE_EX_MIXEDBUTTONS

static_assert(sizeof(WINDOWEXTRA) == VIEWERWINDOWEXTRA, "VIEWERWINDOWEXTRA");
static HIMAGELIST WINAPI CreateImageList(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreatePictureBox(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreateStatusBar(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static HWND WINAPI CreateToolbar(_In_ HWND hWnd, _In_opt_ HINSTANCE hInstance);
static LPUSERDATA WINAPI CreateUserData(_In_ HWND hWnd);
static BOOL WINAPI GetBitmapSize(_In_ HBITMAP hBitmap, _Out_ LPINT lpWidth, _Out_ LPINT lpHeight);
static VOID WINAPI LayoutChildren(_In_ HWND hWnd);
static VOID WINAPI OnAbout(_In_ HWND hWnd);
static BOOL WINAPI OnCommand(_In_ HWND hWnd, _In_ UINT uCommand);
static BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ const CREATESTRUCT FAR *lpParam);
static VOID WINAPI OnDecode(_In_ HWND hWnd);
static VOID WINAPI OnDestroy(_In_ HWND hWnd);
static VOID WINAPI OnDpiChanged(_In_ HWND hWnd, _In_ UINT uDpi, _In_ CONST RECT FAR *lpWindow);
static BOOL WINAPI OnDrawItem(_In_ HWND hWnd, _In_ UINT idItem, _In_ CONST DRAWITEMSTRUCT FAR *lpParam);
static VOID WINAPI OnGetMinMaxInfo(_In_ HWND hWnd, _Inout_ LPMINMAXINFO lpInfo);
static VOID WINAPI OnInitMenuPopup(_In_ HWND hWnd, _In_ HMENU hMenu);
static VOID WINAPI OnLoad(_In_ HWND hWnd);
static VOID WINAPI OnOpen(_In_ HWND hWnd);
static BOOL WINAPI OnParentNotify(_In_ HWND hWnd, _In_ UINT uNotify, _In_ UINT idChild, _In_ LPARAM lParam);
static VOID WINAPI OnStatusWindow(_In_ HWND hWnd);
static VOID WINAPI OnToolbarWindow(_In_ HWND hWnd);
static VOID WINAPI OnZoom(_In_ HWND hWnd, _In_ BOOL bIncrement);
static int CDECL OnZoomProc(_In_opt_ LPVOID lpContext, _In_ LPCVOID lpFirst, _In_ LPCVOID lpSecond);
static BOOL WINAPI ResizeStatusParts(_In_ HWND hWnd);
static VOID WINAPI ResizeToolbarParts(_In_ HWND hWnd);
static BOOL WINAPI ResizeWindow(_In_ HWND hWnd, _In_ CONST RECT FAR *lpWindow);
static UINT WINAPI SetFrameIndex(_In_ HWND hWnd, _In_ UINT uIndex);
static UINT WINAPI SetZoom(_In_ HWND hWnd, _In_ UINT uZoom);
static BOOL WINAPI UpdateStatusFrameText(_In_ HWND hWnd);
static BOOL WINAPI UpdateStatusSizeText(_In_ HWND hWnd);
static BOOL WINAPI UpdateStatusZoomText(_In_ HWND hWnd);

/* Status Parts */
static const
int STATUSPARTS[] = { 100, 200, 300, -1 };

/* Toolbar Buttons */
static const
TBBUTTON TOOLBARBUTTONS[] =
{
	{ ID_TOOLBAROPEN,      IDM_OPEN,      TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ ID_TOOLBAROPENPRIOR, IDM_OPENPRIOR, TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ ID_TOOLBAROPENNEXT,  IDM_OPENNEXT,  TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ 0,                   0,             0,               BTNS_SEP,     },
	{ ID_TOOLBARPAGEPRIOR, IDM_PAGEPRIOR, TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ ID_TOOLBARPAGENEXT,  IDM_PAGENEXT,  TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ 0,                   0,             0,               BTNS_SEP      },
	{ ID_TOOLBARZOOMOUT,   IDM_ZOOMOUT,   TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ ID_TOOLBARZOOMIN,    IDM_ZOOMIN,    TBSTATE_ENABLED, BTNS_AUTOSIZE },
	{ ID_TOOLBARZOOM,      IDM_ZOOM,      TBSTATE_ENABLED, BTNS_AUTOSIZE },
};

static const
UINT ZOOMS[] =
{
	10,  25,  50,  75,
	100, 125, 150, 175,
	200, 250, 300, 400,
	600, 800, 1200,
};

/*
ビューアー ウィンドウ プロシージャ。
*/
EXTERN_C
LRESULT CALLBACK ViewerWindowProc(
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
	case WM_COMMAND:
		nResult = OnCommand(hWnd, LOWORD(wParam)) ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_CREATE:
		nResult = OnCreate(hWnd, (LPCREATESTRUCT)lParam) ? DefProc(hWnd, uMsg, wParam, lParam) : -1;
		break;
	case WM_DECODE:
		OnDecode(hWnd);
		nResult = 0;
		break;
	case WM_DESTROY:
		OnDestroy(hWnd);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DPICHANGED:
		OnDpiChanged(hWnd, LOWORD(wParam), (LPRECT)lParam);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DRAWITEM:
		nResult = OnDrawItem(hWnd, LOWORD(wParam), (LPDRAWITEMSTRUCT)lParam) ? TRUE : DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_GETMINMAXINFO:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		OnGetMinMaxInfo(hWnd, (LPMINMAXINFO)lParam);
		break;
	case WM_INITMENUPOPUP:
		OnInitMenuPopup(hWnd, (HMENU)wParam);
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_LOAD:
		OnLoad(hWnd);
		nResult = 0;
		break;
	case WM_OPEN:
		OnOpen(hWnd);
		nResult = 0;
		break;
	case WM_PARENTNOTIFY:
		nResult = OnParentNotify(hWnd, LOWORD(wParam), HIWORD(wParam), lParam) ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SETFRAMEINDEX:
		nResult = SetFrameIndex(hWnd, (UINT)wParam);
		break;
	case WM_SETZOOM:
		nResult = SetZoom(hWnd, (UINT)wParam);
		break;
	case WM_SIZE:
		LayoutChildren(hWnd);
		nResult = 0;
		break;
	case WM_STATUSWINDOW:
		OnStatusWindow(hWnd);
		nResult = 0;
		break;
	case WM_TOOLBARWINDOW:
		OnToolbarWindow(hWnd);
		nResult = 0;
		break;
	case WM_ZOOM:
		OnZoom(hWnd, LOWORD(wParam));
		nResult = 0;
		break;
	default:
		nResult = DefProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return nResult;
}

/*
ビットマップ リソースからイメージ リストを作成します。
*/
static
HIMAGELIST WINAPI CreateImageList(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HBITMAP hbmBuffer, hbmSource;
	HDC hDC, hdcBuffer, hdcSource;
	HIMAGELIST hImageList;
	UINT uDpi, uBufferHeight, uBufferWidth, uSourceHeight, uSourceWidth;
	uDpi = GetWindowLong(hWnd, GWL_DPI);

	if (uDpi == USER_DEFAULT_SCREEN_DPI)
	{
		hImageList = ImageList_LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TOOLBAR), CXTOOLBAR, 0, CLR_DEFAULT);
	}
	else
	{
		hImageList = NULL;
		hDC = GetDC(hWnd);

		if (hDC)
		{
			hbmSource = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TOOLBAR));

			if (hbmSource)
			{
				GetBitmapSize(hbmSource, &uSourceWidth, &uSourceHeight);
				uBufferWidth = MulDiv(uSourceWidth, uDpi, USER_DEFAULT_SCREEN_DPI);
				uBufferHeight = MulDiv(uSourceHeight, uDpi, USER_DEFAULT_SCREEN_DPI);
				hbmBuffer = CreateCompatibleBitmap(hDC, uBufferWidth, uBufferHeight);

				if (hbmBuffer)
				{
					hdcSource = CreateCompatibleDC(hDC);

					if (hdcSource)
					{
						SelectObject(hdcSource, hbmSource);
						hdcBuffer = CreateCompatibleDC(hDC);

						if (hdcBuffer)
						{
							SelectObject(hdcBuffer, hbmBuffer);

							if (StretchBlt(hdcBuffer, 0, 0, uBufferWidth, uBufferHeight, hdcSource, 0, 0, uSourceWidth, uSourceHeight, SRCCOPY))
							{
								hImageList = ImageList_Create(uBufferHeight, uBufferHeight, ILC_MASK, uBufferHeight / uBufferWidth, 0);

								if (hImageList)
								{
									ImageList_AddMasked(hImageList, hbmBuffer, CLR_DEFAULT);
								}
							}

							DeleteDC(hdcBuffer);
						}

						DeleteDC(hdcSource);
					}

					DeleteObject(hbmBuffer);
				}

				DeleteObject(hbmSource);
			}

			ReleaseDC(hWnd, hDC);
		}
	}
	
	return hImageList;
}

static
HWND WINAPI CreatePictureBox(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	return CreateWindowEx(WS_EX_HWNDPICTURE, WC_STATIC, NULL, WS_HWNDPICTURE, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDPICTURE, hInstance, NULL);
}

static
HWND WINAPI CreateStatusBar(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndStatus;
	hWndStatus = CreateWindowEx(WS_EX_HWNDSTATUS, STATUSCLASSNAME, NULL, WS_HWNDSTATUS, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDSTATUS, hInstance, NULL);

	if (hWndStatus)
	{
		ResizeStatusParts(hWnd);
		UpdateStatusFrameText(hWnd);
		UpdateStatusSizeText(hWnd);
		UpdateStatusZoomText(hWnd);
	}

	return hWndStatus;
}

static
HWND WINAPI CreateToolbar(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HIMAGELIST hImageList;
	HWND hWndToolbar;
	TBBUTTON Buttons[NUMTOOLBARBUTTONS];
	UINT uIndex;
	hWndToolbar = CreateWindowEx(WS_EX_HWNDTOOLBAR, TOOLBARCLASSNAME, NULL, WS_HWNDTOOLBAR, 0, 0, 0, 0, hWnd, (HMENU)ID_HWNDTOOLBAR, hInstance, NULL);

	if (hWndToolbar)
	{
		hImageList = (HIMAGELIST)GetWindowLongPtr(hWnd, GWLP_HIMAGELIST);
		CopyMemory(Buttons, TOOLBARBUTTONS, sizeof(TOOLBARBUTTONS));

		for (uIndex = 0; uIndex < NUMTOOLBARBUTTONS; uIndex++)
		{
			Buttons[uIndex].iString = Buttons[uIndex].iBitmap;
		}

		SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		SendMessage(hWndToolbar, TB_ADDSTRING, (WPARAM)hInstance, IDS_TOOLBAR);
		SendMessage(hWndToolbar, TB_ADDBUTTONS, NUMTOOLBARBUTTONS, (LPARAM)Buttons);

		if (hImageList)
		{
			SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
		}
	}

	return hWndToolbar;
}

static
LPUSERDATA WINAPI CreateUserData(
	_In_ HWND hWnd)
{
	LPUSERDATA lpUserData;
	lpUserData = LocalAlloc(LMEM_ZEROINIT, sizeof(USERDATA));

	if (lpUserData)
	{
		lpUserData->strFileName[0] = 0;
	}

	return lpUserData;
}

/*
指定したビットマップの大きさを返します。
*/
static
BOOL WINAPI GetBitmapSize(
	_In_ HBITMAP hBitmap,
	_Out_ LPINT lpWidth,
	_Out_ LPINT lpHeight)
{
	BITMAPINFOHEADER header;
	BOOL bResult;

	if (GetObject(hBitmap, sizeof header, &header))
	{
		*lpWidth = header.biWidth;
		*lpHeight = header.biHeight;
		bResult = TRUE;
	}
	else
	{
		*lpWidth = 0;
		*lpHeight = 0;
		bResult = FALSE;
	}

	return bResult;
}

static
VOID WINAPI LayoutChildren(
	_In_ HWND hWnd)
{
	HWND hWndChild;
	RECT rcClient, rcWindow;

	if (GetClientRect(hWnd, &rcClient))
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
	case IDM_OPEN:
		bResult = PostMessage(hWnd, WM_OPEN, 0, 0);
		break;
	case IDM_STATUSWINDOW:
		bResult = PostMessage(hWnd, WM_STATUSWINDOW, 0, 0);
		break;
	case IDM_TOOLBARWINDOW:
		bResult = PostMessage(hWnd, WM_TOOLBARWINDOW, 0, 0);
		break;
	case IDM_ZOOM:
		bResult = PostMessage(hWnd, WM_SETZOOM, DEFAULT_ZOOM, 0);
		break;
	case IDM_ZOOMIN:
		bResult = PostMessage(hWnd, WM_ZOOM, TRUE, 0);
		break;
	case IDM_ZOOMOUT:
		bResult = PostMessage(hWnd, WM_ZOOM, FALSE, 0);
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
	_In_ const CREATESTRUCT FAR *lpParam)
{
	LPUSERDATA lpUserData;
	HIMAGELIST hImageList;
	HWND hWndChild;
	UINT uDpi;
	UINT uFlags = USERDATA_SHOWSTATUS | USERDATA_SHOWTOOLBAR;
	BOOL bResult = FALSE;
	uDpi = GetDpiForWindow(hWnd);
	SetWindowLong(hWnd, GWL_DPI, uDpi);
	SetWindowLong(hWnd, GWL_FLAGS, uFlags);
	SetWindowLong(hWnd, GWL_ZOOM, DEFAULT_ZOOM);
	lpUserData = CreateUserData(hWnd);

	if (lpUserData)
	{
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpUserData);
		bResult = !!CreatePictureBox(hWnd, lpParam->hInstance);
	}
	if (bResult && (uFlags & USERDATA_SHOWTOOLBAR))
	{
		bResult = !!CreateToolbar(hWnd, lpParam->hInstance);
	}
	if (bResult && (uFlags & USERDATA_SHOWSTATUS))
	{
		hWndChild = CreateStatusBar(hWnd, lpParam->hInstance);

		if (hWndChild)
		{
			hImageList = CreateImageList(hWnd, lpParam->hInstance);

			if (hImageList)
			{
				SetWindowLongPtr(hWnd, GWLP_HIMAGELIST, (LONG_PTR)hImageList);
				SendMessage(hWndChild, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
			}
			else
			{
				bResult = FALSE;
			}
		}
		else
		{
			bResult = FALSE;
		}
	}
	if (bResult)
	{
		ResizeStatusParts(hWnd);
		ResizeToolbarParts(hWnd);
		UpdateStatusFrameText(hWnd);
		UpdateStatusZoomText(hWnd);
	}

	return bResult;
}

static
VOID WINAPI OnDecode(
	_In_ HWND hWnd)
{
	IWICBitmap *pBitmap;
	IWICBitmapFrameDecode *pFrame;
	IWICFormatConverter *pConverter;
	IWICBitmapLock *pLock;
	LPUSERDATA This;
	LPBYTE lpBuffer;
	HBITMAP hBitmap;
	HDC hDC;
	HWND hWndPicture;
	BITMAPINFOHEADER bih;
	HRESULT hResult;
	UINT cbBuffer, uFrameIndex, uHeight, uWidth;
	This = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	uFrameIndex = GetWindowLong(hWnd, GWL_FRAMEINDEX);

	if (This && This->pFactory && This->pDecoder)
	{
		hDC = GetDC(hWnd);

		if (hDC)
		{
			hResult = This->pDecoder->lpVtbl->GetFrame(This->pDecoder, uFrameIndex, &pFrame);

			if (SUCCEEDED(hResult))
			{
				hResult = This->pFactory->lpVtbl->CreateFormatConverter(This->pFactory, &pConverter);

				if (SUCCEEDED(hResult))
				{
					hResult = pConverter->lpVtbl->Initialize(pConverter, (IWICBitmapSource*)pFrame, &GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, ALPHATHRESHOLDPERCENT, WICBitmapPaletteTypeMedianCut);

					if (SUCCEEDED(hResult))
					{
						hResult = This->pFactory->lpVtbl->CreateBitmapFromSource(This->pFactory, (IWICBitmapSource*)pConverter, WICBitmapCacheOnLoad, &pBitmap);

						if (SUCCEEDED(hResult))
						{
							hResult = pBitmap->lpVtbl->Lock(pBitmap, NULL, WICBitmapLockRead, &pLock);

							if (SUCCEEDED(hResult))
							{
								hResult = pLock->lpVtbl->GetSize(pLock, &uWidth, &uHeight);

								if (SUCCEEDED(hResult))
								{
									hResult = pLock->lpVtbl->GetDataPointer(pLock, &cbBuffer, &lpBuffer);

									if (SUCCEEDED(hResult))
									{
										ZeroMemory(&bih, sizeof bih);
										bih.biSize = sizeof bih;
										bih.biWidth = uWidth;
										bih.biHeight = uHeight;
										bih.biPlanes = 1;
										bih.biBitCount = 32;
										bih.biSizeImage = cbBuffer;
										hBitmap = (HBITMAP)GetWindowLongPtr(hWnd, GWLP_HBITMAP);

										if (hBitmap)
										{
											DeleteObject(hBitmap);
										}

										hBitmap = CreateDIBitmap(hDC, &bih, CBM_INIT, lpBuffer, (LPBITMAPINFO)&bih, DIB_RGB_COLORS);

										if (!hBitmap)
										{
											hResult = HRESULT_FROM_WIN32(GetLastError());
										}

										SetWindowLongPtr(hWnd, GWLP_HBITMAP, (LONG_PTR)hBitmap);
										hWndPicture = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPICTURE);

										if (hWndPicture)
										{
											InvalidateRect(hWndPicture, NULL, TRUE);
										}
									}
								}

								pLock->lpVtbl->Release(pLock);
							}

							pBitmap->lpVtbl->Release(pBitmap);
						}
					}

					pConverter->lpVtbl->Release(pConverter);
				}

				pFrame->lpVtbl->Release(pFrame);
			}

			ReleaseDC(hWnd, hDC);
		}
		else
		{
			hResult = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		hResult = E_ILLEGAL_METHOD_CALL;
	}
	if (FAILED(hResult))
	{
		ErrorMessageBox(NULL, hWnd, hResult);
	}
}

/*
作成した資源を破棄します。
*/
static
VOID WINAPI OnDestroy(
	_In_ HWND hWnd)
{
	HANDLE handle;
	PostQuitMessage(0);

	if (handle = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR))
	{
		SendMessage((HWND)handle, TB_SETIMAGELIST, 0, 0);
	}
	if (handle = (HANDLE)SetWindowLongPtr(hWnd, GWLP_HBITMAP, 0))
	{
		DeleteObject((HGDIOBJ)handle);
	}
	if (handle = (HANDLE)SetWindowLongPtr(hWnd, GWLP_HIMAGELIST, 0))
	{
		ImageList_Destroy((HIMAGELIST)handle);
	}
	if (handle = (HANDLE)SetWindowLongPtr(hWnd, GWLP_USERDATA, 0))
	{
		if (((LPUSERDATA)handle)->pDecoder)
		{
			((LPUSERDATA)handle)->pDecoder->lpVtbl->Release(((LPUSERDATA)handle)->pDecoder);
		}
		if (((LPUSERDATA)handle)->pFactory)
		{
			((LPUSERDATA)handle)->pFactory->lpVtbl->Release(((LPUSERDATA)handle)->pFactory);
		}

		LocalFree((HLOCAL)handle);
	}
}

static
VOID WINAPI OnDpiChanged(
	_In_ HWND hWnd,
	_In_ UINT uDpi,
	_In_ CONST RECT FAR *lpWindow)
{
	SetWindowLong(hWnd, GWL_DPI, uDpi);
	ResizeWindow(hWnd, lpWindow);
	ResizeStatusParts(hWnd);
	ResizeToolbarParts(hWnd);
}

static
BOOL WINAPI OnDrawItem(
	_In_ HWND hWnd,
	_In_ UINT idItem,
	_In_ CONST DRAWITEMSTRUCT FAR *lpParam)
{
	const bResult = idItem == ID_HWNDPICTURE;
	LPUSERDATA This;
	HBITMAP hBitmap;
	HDC hdcBitmap;
	UINT uZoom;
	int nOffsetX, nOffsetY, nZoomWidth, nZoomHeight, nBitmapWidth, nBitmapHeight;

	if (bResult)
	{
		This = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		hBitmap = (HBITMAP)GetWindowLongPtr(hWnd, GWLP_HBITMAP);

		if (This)
		{
			if (hBitmap)
			{
				hdcBitmap = CreateCompatibleDC(lpParam->hDC);

				if (hdcBitmap)
				{
					if (GetBitmapSize(hBitmap, &nBitmapWidth, &nBitmapHeight))
					{
						nOffsetX = -GetScrollPos(lpParam->hwndItem, SB_HORZ);
						nOffsetY = -GetScrollPos(lpParam->hwndItem, SB_VERT);
						uZoom = GetWindowLong(hWnd, GWL_ZOOM);
						SelectObject(hdcBitmap, hBitmap);

						if (uZoom == DEFAULT_ZOOM)
						{
							BitBlt(lpParam->hDC, nOffsetX, nOffsetY, nBitmapWidth, nBitmapHeight, hdcBitmap, 0, 0, SRCCOPY);
						}
						else
						{
							nZoomWidth = MulDiv(nBitmapWidth, uZoom, DEFAULT_ZOOM);
							nZoomHeight = MulDiv(nBitmapHeight, uZoom, DEFAULT_ZOOM);
							StretchBlt(lpParam->hDC, nOffsetX, nOffsetY, nZoomWidth, nZoomHeight, hdcBitmap, 0, 0, nBitmapWidth, nBitmapHeight, SRCCOPY);
						}
					}

					DeleteDC(hdcBitmap);
				}
			}
		}
	}

	return bResult;
}

/*
ウィンドウの最小サイズを決定します。
*/
static
VOID WINAPI OnGetMinMaxInfo(
	_In_ HWND hWnd,
	_Inout_ LPMINMAXINFO lpInfo)
{
	POINT ptSize;
	UINT uDpi;
	uDpi = GetWindowLong(hWnd, GWL_DPI);
	ptSize.x = MulDiv(MINTRACKSIZE_X, uDpi, USER_DEFAULT_SCREEN_DPI);
	ptSize.y = MulDiv(MINTRACKSIZE_Y, uDpi, USER_DEFAULT_SCREEN_DPI);
	lpInfo->ptMinTrackSize.x = max(lpInfo->ptMinTrackSize.x, ptSize.x);
	lpInfo->ptMinTrackSize.y = max(lpInfo->ptMinTrackSize.y, ptSize.y);
}

static
VOID WINAPI OnInitMenuPopup(
	_In_ HWND hWnd,
	_In_ HMENU hMenu)
{
	MENUITEMINFO item;
	int nCount, nIndex;
	ZeroMemory(&item, sizeof item);
	item.cbSize = sizeof item;
	nCount = GetMenuItemCount(hMenu);

	for (nIndex = 0; nIndex < nCount; nIndex++)
	{
		item.fMask = MIIM_STATE | MIIM_ID;

		if (GetMenuItemInfo(hMenu, nIndex, TRUE, &item))
		{
			switch (item.wID)
			{
			case IDM_PRINT:
				item.fMask = MIIM_STATE;
				item.fState = GetWindowLongPtr(hWnd, GWLP_HBITMAP) ? (item.fState & ~MFS_DISABLED) : (item.fState | MFS_DISABLED);
				break;
			case IDM_STATUSWINDOW:
				item.fMask = MIIM_STATE;
				item.fState = GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS) ? (item.fState | MFS_CHECKED) : (item.fState & ~MFS_CHECKED);
				break;
			case IDM_TOOLBARWINDOW:
				item.fMask = MIIM_STATE;
				item.fState = GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR) ? (item.fState | MFS_CHECKED) : (item.fState & ~MFS_CHECKED);
				break;
			default:
				item.fMask = 0;
				break;
			}
			if (item.fMask)
			{
				SetMenuItemInfo(hMenu, nIndex, TRUE, &item);
			}
		}
	}
}

/*
現在のファイル名で画像ファイルを開きます。
*/
static
VOID WINAPI OnLoad(
	_In_ HWND hWnd)
{
	LPUSERDATA pSelf;
	HRESULT hResult;
	UINT uFrameCount;
	pSelf = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	PostMessage(hWnd, WM_DECODE, 0, 0);

	if (pSelf)
	{
		if (pSelf->pFactory)
		{
			hResult = S_OK;
		}
		else
		{
			hResult = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, &pSelf->pFactory);
		}
		if (SUCCEEDED(hResult))
		{
			if (pSelf->pDecoder)
			{
				pSelf->pDecoder->lpVtbl->Release(pSelf->pDecoder);
			}

			hResult = pSelf->pFactory->lpVtbl->CreateDecoderFromFilename(pSelf->pFactory, pSelf->strFileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pSelf->pDecoder);

			if (SUCCEEDED(hResult))
			{
				hResult = pSelf->pDecoder->lpVtbl->GetFrameCount(pSelf->pDecoder, &uFrameCount);
			}
			else
			{
				uFrameCount = 0;
			}

			SetWindowLong(hWnd, GWL_FRAMECOUNT, uFrameCount);
			SetWindowLong(hWnd, GWL_FRAMEINDEX, 0);
			SetWindowLong(hWnd, GWL_ZOOM, DEFAULT_ZOOM);
			UpdateStatusFrameText(hWnd);
			UpdateStatusZoomText(hWnd);
		}
	}
	else
	{
		hResult = E_ILLEGAL_METHOD_CALL;
	}
	if (FAILED(hResult))
	{
		ErrorMessageBox(NULL, hWnd, hResult);
	}
}

/*
開くダイアログ ボックスを表示します。
*/
static
VOID WINAPI OnOpen(
	_In_ HWND hWnd)
{
	LPUSERDATA pSelf;
	OPENFILENAME ofn;
	pSelf = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pSelf)
	{
		ZeroMemory(&ofn, sizeof ofn);
		ofn.lStructSize = sizeof ofn;
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = OPENFILEFILTER;
		ofn.lpstrFile = pSelf->strFileName;
		ofn.nMaxFile = PATHCCH_MAX_CCH;
		ofn.Flags = OFN_ONOPEN;

		if (GetOpenFileName(&ofn))
		{
			PostMessage(hWnd, WM_LOAD, 0, 0);
		}
	}
}

/*
作成したウィンドウ ハンドルを保持します。
*/
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
		case ID_HWNDPICTURE:
			SetWindowLongPtr(hWnd, GWLP_HWNDPICTURE, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDSTATUS:
			SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, lParam);
			bResult = TRUE;
			break;
		case ID_HWNDTOOLBAR:
			SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, lParam);
			bResult = TRUE;
			break;
		}

		break;
	case WM_DESTROY:
		switch (idChild)
		{
		case ID_HWNDPICTURE:
			SetWindowLongPtr(hWnd, GWLP_HWNDPICTURE, 0);
			bResult = TRUE;
			break;
		case ID_HWNDSTATUS:
			SetWindowLongPtr(hWnd, GWLP_HWNDSTATUS, 0);
			bResult = TRUE;
			break;
		case ID_HWNDTOOLBAR:
			SetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR, 0);
			bResult = TRUE;
			break;
		}

		break;
	}

	return bResult;
}

/*
ステータス バーを表示または削除します。
*/
static
VOID WINAPI OnStatusWindow(
	_In_ HWND hWnd)
{
	HANDLE Handle;
	Handle = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (Handle)
	{
		SendMessage((HWND)Handle, WM_CLOSE, 0, 0);
	}
	else
	{
		Handle = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		CreateStatusBar(hWnd, (HINSTANCE)Handle);
	}

	LayoutChildren(hWnd);
}

/*
ツールバーを表示または削除します。
*/
static
VOID WINAPI OnToolbarWindow(
	_In_ HWND hWnd)
{
	HANDLE Handle;
	Handle = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (Handle)
	{
		SendMessage((HWND)Handle, WM_CLOSE, 0, 0);
	}
	else
	{
		Handle = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		CreateToolbar(hWnd, (HINSTANCE)Handle);
	}

	LayoutChildren(hWnd);
}

static
VOID WINAPI OnZoom(
	_In_ HWND hWnd,
	_In_ BOOL bIncrement)
{
	ZOOMSTRUCT Found;
	LPUINT lpZoom;
	UINT uZoom;
	uZoom = GetWindowLong(hWnd, GWL_ZOOM);
	ZeroMemory(&Found, sizeof Found);
	Found.uNext = 0;
	Found.uPrevious = MAXUINT;
	lpZoom = bsearch_s(&uZoom, ZOOMS, ARRAYSIZE(ZOOMS), sizeof(int), OnZoomProc, &Found);

	if (lpZoom)
	{
		uZoom = (UINT)(lpZoom - ZOOMS);

		if (bIncrement)
		{
			uZoom++;
		}
		else
		{
			uZoom--;
		}
		if (uZoom < ARRAYSIZE(ZOOMS))
		{
			uZoom = ZOOMS[uZoom];
		}
		else
		{
			uZoom = 0;
		}
	}
	else if (bIncrement)
	{
		if (Found.uNext <= MAXZOOM)
		{
			uZoom = Found.uNext;
		}
		else
		{
			uZoom = 0;
		}
	}
	else
	{
		if (Found.uPrevious >= MINZOOM)
		{
			uZoom = Found.uPrevious;
		}
		else
		{
			uZoom = 0;
		}
	}
	if (uZoom)
	{
		SetZoom(hWnd, uZoom);
	}
}

static
int CDECL OnZoomProc(
	_In_opt_ LPVOID lpContext,
	_In_ LPCVOID lpKey,
	_In_ LPCVOID lpValue)
{
	const UINT uKey = *(LPUINT)lpKey;
	const UINT uValue = *(LPUINT)lpValue;
	int nResult;

	if (uKey < uValue)
	{
		nResult = -1;

		if (lpContext)
		{
			((LPZOOMSTRUCT)lpContext)->uNext = uValue;
		}
	}
	else if (uKey > uValue)
	{
		nResult = 1;

		if (lpContext)
		{
			((LPZOOMSTRUCT)lpContext)->uPrevious = uValue;
		}
	}
	else
	{
		nResult = 0;
	}

	return nResult;
}

/*
ステータス バー パネルの大きさを更新します。
*/
static
BOOL WINAPI ResizeStatusParts(
	_In_ HWND hWnd)
{
	HWND hWndStatus;
	int parts[NUMSTATUSPARTS];
	UINT uDpi, uIndex;
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndStatus)
	{
		uDpi = GetWindowLong(hWnd, GWL_DPI);
		CopyMemory(parts, STATUSPARTS, sizeof STATUSPARTS);

		for (uIndex = 0; uIndex < NUMSTATUSPARTS - 1; uIndex++)
		{
			parts[uIndex] = MulDiv(parts[uIndex], uDpi, USER_DEFAULT_SCREEN_DPI);
		}

		SendMessage(hWndStatus, SB_SETPARTS, NUMSTATUSPARTS, (LPARAM)parts);
	}

	return !!hWndStatus;
}

static
VOID WINAPI ResizeToolbarParts(
	_In_ HWND hWnd)
{
	HIMAGELIST hImageList;
	HINSTANCE hInstance;
	HWND hWndToolbar;
	int cx;
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	hImageList = (HIMAGELIST)GetWindowLongPtr(hWnd, GWLP_HIMAGELIST);
	hWndToolbar = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDTOOLBAR);

	if (hImageList)
	{
		ImageList_Destroy(hImageList);
	}

	cx = GetWindowLong(hWnd, GWL_DPI);
	cx = MulDiv(CXTOOLBAR, cx, USER_DEFAULT_SCREEN_DPI);
	hImageList = ImageList_LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TOOLBAR), cx, 0, CLR_DEFAULT);
	SetWindowLongPtr(hWnd, GWLP_HIMAGELIST, (LONG_PTR)hImageList);

	if (hWndToolbar)
	{
		SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
	}
}

/*
ウィンドウの大きさとして矩形を指定します。
*/
static
BOOL WINAPI ResizeWindow(
	_In_ HWND hWnd,
	_In_ CONST RECT FAR *lpWindow)
{
	const int X = lpWindow->left;
	const int Y = lpWindow->top;
	const int nWidth = lpWindow->right - X;
	const int nHeight = lpWindow->bottom - Y;
	return MoveWindow(hWnd, X, Y, nWidth, nHeight, TRUE);
}

static
UINT WINAPI SetFrameIndex(
	_In_ HWND hWnd,
	_In_ UINT uIndex)
{
	uIndex = SetWindowLong(hWnd, GWL_FRAMEINDEX, uIndex);
	UpdateStatusFrameText(hWnd);
	return uIndex;
}

static
UINT WINAPI SetZoom(
	_In_ HWND hWnd,
	_In_ UINT uZoom)
{
	uZoom = SetWindowLong(hWnd, GWL_ZOOM, uZoom);
	UpdateStatusZoomText(hWnd);
	return uZoom;
}

/*
ステータス バーにページ数を表示します。
*/
static
BOOL WINAPI UpdateStatusFrameText(
	_In_ HWND hWnd)
{
	HWND hWndStatus;
	UINT uCount, uIndex;
	BOOL bResult;
	TCHAR strText[LOADSTRING_MAX];
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndStatus)
	{
		uCount = GetWindowLong(hWnd, GWL_FRAMECOUNT);
		uIndex = GetWindowLong(hWnd, GWL_FRAMEINDEX) + 1;
		bResult = SUCCEEDED(StringCchPrintf(strText, LOADSTRING_MAX, TEXT("Page: %u/%u"), uIndex, uCount));
		SendMessage(hWndStatus, SB_SETTEXT, ID_STATUSFRAME, (LPARAM)strText);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

/*
ステータス バーに画像の大きさを表示します。
*/
static
BOOL WINAPI UpdateStatusSizeText(
	_In_ HWND hWnd)
{
	HWND hWndStatus;
	UINT uHeight, uWidth;
	BOOL bResult;
	TCHAR strText[LOADSTRING_MAX];
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndStatus)
	{
		uWidth = GetWindowLong(hWnd, GWL_WIDTH);
		uHeight = GetWindowLong(hWnd, GWL_HEIGHT);
		bResult = SUCCEEDED(StringCchPrintf(strText, LOADSTRING_MAX, TEXT("Size: (%u, %u)"), uWidth, uHeight));
		SendMessage(hWndStatus, SB_SETTEXT, ID_STATUSSIZE, (LPARAM)strText);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

/*
ステータス バーにズーム率を表示します。
*/
static
BOOL WINAPI UpdateStatusZoomText(
	_In_ HWND hWnd)
{
	HWND hWndStatus;
	UINT uZoom;
	BOOL bResult;
	TCHAR strText[LOADSTRING_MAX];
	hWndStatus = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDSTATUS);

	if (hWndStatus)
	{
		uZoom = GetWindowLong(hWnd, GWL_ZOOM);
		bResult = SUCCEEDED(StringCchPrintf(strText, LOADSTRING_MAX, TEXT("Zoom: %u%%"), uZoom));
		SendMessage(hWndStatus, SB_SETTEXT, ID_STATUSZOOM, (LPARAM)strText);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}
