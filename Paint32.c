/*
Copyright 2025 Taichi Murakami.
アプリケーションのメイン エントリ ポイントを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define ERRORMESSAGEBOXFLAGS (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM)
#define LOADSTRING_MAX 32

static const
INITCOMMONCONTROLSEX ICCEX = { sizeof(INITCOMMONCONTROLSEX), ICC_TREEVIEW_CLASSES };

static
HWND WINAPI CreateFrameWindow(
	_In_opt_ HINSTANCE hInstance);

static
BOOL WINAPI RegisterPrivateClasses(
	_In_opt_ HINSTANCE hInstance);

EXTERN_C
int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInst,
	_In_ LPTSTR lpCmdLine,
	_In_ int nCmdShow)
{
	HACCEL hAccel;
	HWND hWndClient, hWndDialog, hWndFrame = NULL;
	MSG msg;
	HRESULT hResult;
	int nExitCode = -1;
	hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (SUCCEEDED(hResult))
	{
		if (RegisterPrivateClasses(hInstance) &&
			InitCommonControlsEx(&ICCEX) &&
			(hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAIN))) &&
			(hWndFrame = CreateFrameWindow(hInstance)))
		{
			hWndClient = (HWND)SendMessage(hWndFrame, FRAME_GETMDICLIENT, 0, 0);
			ShowWindow(hWndFrame, nCmdShow);
			UpdateWindow(hWndFrame);

			while ((nExitCode = GetMessage(&msg, NULL, 0, 0)) > 0)
			{
				hWndDialog = (HWND)SendMessage(hWndFrame, FRAME_GETDIALOG, 0, 0);

				if (!(hWndDialog &&
					IsDialogMessage(hWndDialog, &msg) ||
					TranslateMDISysAccel(hWndClient, &msg) ||
					TranslateAccelerator(hWndFrame, hAccel, &msg)))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			if (nExitCode)
			{
				hResult = HRESULT_FROM_WIN32(GetLastError());
			}
		}

		CoUninitialize();
	}
	if (FAILED(hResult))
	{
		ErrorMessageBox(hInstance, hWndFrame, hResult);
	}

	return nExitCode;
}

EXTERN_C
int WINAPI ErrorMessageBox(
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ HWND hWnd,
	_In_ DWORD dwError)
{
	LPTSTR lpText = NULL;
	int nResult = 0;
	TCHAR strCaption[LOADSTRING_MAX];
	LoadString(hInstance, IDS_APPTITLE, strCaption, LOADSTRING_MAX);

	if (FormatMessage(ERRORMESSAGEBOXFLAGS, NULL, dwError, 0, (LPTSTR)&lpText, 0, NULL))
	{
		nResult = MessageBox(hWnd, lpText, strCaption, MB_ICONERROR);
	}

	LocalFree(lpText);
	return nResult;
}

EXTERN_C
BOOL WINAPI SetWindowPosOnCenter(
	_In_ HWND hWnd)
{
	HWND hWndParent;
	RECT rcWnd, rcWndParent;
	BOOL bResult = FALSE;
	hWndParent = (HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPARENT);

	if (hWndParent &&
		GetWindowRect(hWnd, &rcWnd) &&
		GetWindowRect(hWndParent, &rcWndParent))
	{
		rcWnd.right -= rcWnd.left;
		rcWnd.bottom -= rcWnd.top;
		rcWndParent.right -= rcWndParent.left;
		rcWndParent.bottom -= rcWndParent.top;
		rcWnd.left = rcWndParent.left + (rcWndParent.right - rcWnd.right) / 2;
		rcWnd.top = rcWndParent.top + (rcWndParent.bottom - rcWnd.bottom) / 2;
		bResult = SetWindowPos(hWnd, NULL, rcWnd.left, rcWnd.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	return bResult;
}

EXTERN_C
BOOL WINAPI SetWindowPosOnSize(
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ const RECT *lpRect)
{
	const int X = lpRect->left;
	const int Y = lpRect->top;
	const int nWidth = lpRect->right - X;
	const int nHeight = lpRect->bottom - Y;
	return SetWindowPos(hWnd, hWndInsertAfter, X, Y, nWidth, nHeight, SWP_SHOWWINDOW);
}

static
HWND WINAPI CreateFrameWindow(
	_In_opt_ HINSTANCE hInstance)
{
	TCHAR strCaption[LOADSTRING_MAX];
	LoadString(hInstance, IDS_APPTITLE, strCaption, LOADSTRING_MAX);
	return CreateWindow(FRAMECLASSNAME, strCaption, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
}

static
BOOL WINAPI RegisterPrivateClasses(
	_In_opt_ HINSTANCE hInstance)
{
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),
		CS_VREDRAW | CS_HREDRAW,
		FrameWindowProc,
		0,
		FRAMEWINDOWEXTRA,
		hInstance,
		NULL,
		NULL,
		(HBRUSH)(COLOR_3DFACE + 1),
		MAKEINTRESOURCE(IDR_MAIN),
		FRAMECLASSNAME,
	};

	UINT atom;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	atom = RegisterClassEx(&wc);

	if (!atom) goto EXIT;
	wc.lpfnWndProc = OutlineWindowProc;
	wc.cbWndExtra = OUTLINEWINDOWEXTRA;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = OUTLINECLASSNAME;
	atom = RegisterClassEx(&wc);

	if (!atom) goto EXIT;
	wc.lpfnWndProc = PaletteWindowProc;
	wc.cbWndExtra = PALETTEWINDOWEXTRA;
	wc.lpszClassName = PALETTECLASSNAME;
	atom = RegisterClassEx(&wc);

EXIT:
	return !!atom;
}
