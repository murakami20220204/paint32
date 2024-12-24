/*
Copyright 2024 Taichi Murakami.
アプリケーションのメイン エントリ ポイントを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define ERRORMESSAGEBOXFLAGS (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM)
#define LOADSTRING_MAX 32

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
INT_PTR CALLBACK AboutDialogProc(
	_In_ HWND hDlg,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	INT_PTR bResult;

	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_COMMAND:
		EndDialog(hDlg, 0);
		bResult = FALSE;
		break;
	case WM_INITDIALOG:
		bResult = TRUE;
		break;
	default:
		bResult = FALSE;
		break;
	}

	return bResult;
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

	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	return !!RegisterClassEx(&wc);
}
