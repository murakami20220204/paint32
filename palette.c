/*
Copyright 2025 Taichi Murakami.
パレット ウィンドウ プロシージャを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define DefProc DefWindowProc
#define ID_HWNDCOLOR    1
#define LOADSTRING_MAX  32
#define SWP_LAYOUT (SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW)

typedef LRESULT(WINAPI FAR *DEFPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagWINDOWEXTRA
{
	LONG_PTR hWndColor;
	WORD wLayout;
	WORD wIDColor;
	WORD wIDHistory;
	WORD wIDFavorites;
} WINDOWEXTRA;

static_assert(sizeof(WINDOWEXTRA) == PALETTEWINDOWEXTRA, "PALETTEWINDOWEXTRA");
#define GWLP_HWNDCOLOR          offsetof(WINDOWEXTRA, hWndColor)
#define GWW_IDCOLOR             offsetof(WINDOWEXTRA, wIDColor)
#define GWW_IDFAVORITES         offsetof(WINDOWEXTRA, wIDFavorites)
#define GWW_IDHISTORY           offsetof(WINDOWEXTRA, wIDHistory)
#define GWW_LAYOUT              offsetof(WINDOWEXTRA, wLayout)

static
HWND WINAPI CreateColorDialog(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance);

static
BOOL WINAPI Layout(
	_In_ HWND hWnd);

static
BOOL WINAPI LayoutControl(
	_In_opt_ HWND hWnd,
	_In_ UINT uLayout,
	_Inout_ LPRECT lpClient);

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
LRESULT WINAPI NotifyParent(
	_In_ HWND hWnd,
	_In_ UINT uMsg);

static
LRESULT WINAPI OnPaint(
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

static
BOOL WINAPI PaintCaption(
	_In_ HWND hWnd,
	_In_ HDC hDC,
	_In_ int nIndex);

EXTERN_C
LRESULT CALLBACK PaletteWindowProc(
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
	case WM_PARENTNOTIFY:
		lpProc = OnParentNotify;
		break;
	case WM_PAINT:
		lpProc = OnPaint;
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
HWND WINAPI CreateColorDialog(
	_In_ HWND hWnd,
	_In_opt_ HINSTANCE hInstance)
{
	HWND hWndColor;
	hWndColor = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCOLOR);

	if (!hWndColor)
	{
		hWndColor = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_COLOR), hWnd, ColorDialogProc, GetWindowWord(hWnd, GWW_IDCOLOR));

		if (hWndColor)
		{
			SetWindowLongPtr(hWnd, GWLP_HWNDCOLOR, (LONG_PTR)hWndColor);
			SetWindowLong(hWndColor, GWL_STYLE, ~WS_CAPTION & GetWindowLong(hWndColor, GWL_STYLE));
			ShowWindow(hWndColor, SW_SHOW);
		}
	}

	return hWndColor;
}

static
BOOL WINAPI Layout(
	_In_ HWND hWnd)
{
	HWND hWndChild;
	RECT rcClient;
	ZeroMemory(&rcClient, sizeof rcClient);
	hWndChild = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDCOLOR);

	if (hWndChild)
	{
		rcClient.bottom += GetSystemMetrics(SM_CYCAPTION);
		LayoutControl(hWndChild, 0, &rcClient);
	}

	return FALSE;
}

static
BOOL WINAPI LayoutControl(
	_In_opt_ HWND hWnd,
	_In_ UINT uLayout,
	_Inout_ LPRECT lpClient)
{
	RECT rcWindow;
	BOOL bResult = FALSE;

	if (hWnd)
	{
		if (uLayout)
		{
			lpClient->left = lpClient->right;
		}
		else
		{
			lpClient->top = lpClient->bottom;
		}

		rcWindow.left = lpClient->left;
		rcWindow.top = lpClient->top;

		if (SetWindowPos(hWnd, NULL, rcWindow.left, rcWindow.top, 0, 0, SWP_LAYOUT) && GetWindowRect(hWnd, &rcWindow))
		{
			lpClient->right = lpClient->left + rcWindow.right - rcWindow.left;
			lpClient->bottom = lpClient->top + rcWindow.bottom - rcWindow.top;
			bResult = TRUE;
		}
	}

	return bResult;
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
	HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
	SetWindowLongPtr(hWnd, GWLP_USERDATA, ((LPPALETTECREATESTRUCT)((LPCREATESTRUCT)lParam)->lpCreateParams)->wID);
	CreateColorDialog(hWnd, hInstance);
	Layout(hWnd);
	return DefProc(hWnd, uMsg, wParam, lParam);
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
LRESULT WINAPI OnPaint(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT paint;
	hDC = BeginPaint(hWnd, &paint);

	if (hDC)
	{
		PaintCaption(hWnd, hDC, GWLP_HWNDCOLOR);
		EndPaint(hWnd, &paint);
	}

	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnParentNotify(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	BOOL bResult = FALSE;

	switch (LOWORD(wParam))
	{
	case WM_DESTROY:
		switch (HIWORD(wParam))
		{
		case ID_HWNDCOLOR:
			SetWindowLongPtr(hWnd, GWLP_HWNDCOLOR, 0);
			bResult = TRUE;
			break;
		}
		break;
	}

	return bResult ? 0 : DefProc(hWnd, uMsg, wParam, lParam);
}

static
LRESULT WINAPI OnSize(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	return DefProc(hWnd, uMsg, wParam, lParam);
}

static
BOOL WINAPI PaintCaption(
	_In_ HWND hWnd,
	_In_ HDC hDC,
	_In_ int nIndex)
{
	HWND hWndChild;
	RECT rcWindow;
	BOOL bResult = FALSE;
	int cchCaption;
	TCHAR strCaption[LOADSTRING_MAX];
	hWndChild = (HWND)GetWindowLongPtr(hWnd, nIndex);

	if (hWndChild && GetWindowRect(hWndChild, &rcWindow))
	{
		rcWindow.right -= rcWindow.left;

		if (ScreenToClient(hWnd, (LPPOINT)&rcWindow))
		{
			rcWindow.right += rcWindow.left;
			rcWindow.bottom = rcWindow.top;
			rcWindow.top = rcWindow.bottom - GetSystemMetrics(SM_CYCAPTION);
			bResult = FillRect(hDC, &rcWindow, GetSysColorBrush(COLOR_ACTIVECAPTION));
			cchCaption = GetWindowTextLength(hWndChild);
			GetWindowText(hWndChild, strCaption, LOADSTRING_MAX);
			SelectObject(hDC, GetStockObject(SYSTEM_FONT));
			SetBkColor(hDC, GetSysColor(COLOR_ACTIVECAPTION));
			TextOut(hDC, rcWindow.left, rcWindow.top, strCaption, cchCaption);
		}
	}

	return bResult;
}
