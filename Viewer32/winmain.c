/*
Copyright 2025 Taichi Murakami.
アプリケーションのメイン エントリ ポイントを実装します。
*/

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Viewer32.h"
#include "resource.h"

#define LOADSTRING_MAX 32

static HWND WINAPI CreateViewerWindow(_In_opt_ HINSTANCE hInstance);
static BOOL WINAPI RegisterCommonClasses(void);
static BOOL WINAPI RegisterPrivateClasses(_In_opt_ HINSTANCE hInstance);

/*
アプリケーションのメイン エントリ ポイントです。
*/
EXTERN_C
int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInst,
	_In_ LPTSTR lpCmdLine,
	_In_ int nCmdShow)
{
	HACCEL hAccel;
	HWND hWnd = NULL;
	MSG msg;
	HRESULT hResult;
	BOOL bResult = -1;
	hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (SUCCEEDED(hResult))
	{
		if (RegisterCommonClasses() && RegisterPrivateClasses(hInstance))
		{
			hWnd = CreateViewerWindow(hInstance);

			if (hWnd)
			{
				ShowWindow(hWnd, nCmdShow);
				UpdateWindow(hWnd);
				hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_VIEWER));
				bResult = GetMessage(&msg, NULL, 0, 0);

				while (bResult > 0)
				{
					if (!TranslateAccelerator(hWnd, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}

					bResult = GetMessage(&msg, NULL, 0, 0);
				}
				if (!bResult)
				{
					bResult = (int)msg.wParam;
				}
			}
		}
		if (bResult)
		{
			hResult = HRESULT_FROM_WIN32(GetLastError());
		}

		CoUninitialize();
	}
	if (FAILED(hResult))
	{
		ErrorMessageBox(hInstance, hWnd, hResult);
	}

	return bResult;
}

/*
新しいアプリケーション ウィンドウを作成します。
*/
static
HWND WINAPI CreateViewerWindow(
	_In_opt_ HINSTANCE hInstance)
{
	TCHAR strCaption[LOADSTRING_MAX];
	LoadString(hInstance, IDS_APPTITLE, strCaption, LOADSTRING_MAX);
	return CreateWindow(VIEWERCLASSNAME, strCaption, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
}

/*
コモン コントロールを利用可能にします。
*/
static
BOOL WINAPI RegisterCommonClasses(
	void)
{
	INITCOMMONCONTROLSEX icc;
	ZeroMemory(&icc, sizeof icc);
	icc.dwSize = sizeof icc;
	icc.dwICC = ICC_BAR_CLASSES;
	return InitCommonControlsEx(&icc);
}

/*
プライベート ウィンドウを利用可能にします。
*/
static
BOOL WINAPI RegisterPrivateClasses(
	_In_opt_ HINSTANCE hInstance)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = ViewerWindowProc;
	wc.cbWndExtra = VIEWERWINDOWEXTRA;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIEWER));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_VIEWER);
	wc.lpszClassName = VIEWERCLASSNAME;
	return RegisterClassEx(&wc);
}
