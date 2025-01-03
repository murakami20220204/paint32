/*
Copyright 2025 Taichi Murakami.
アプリケーションのメイン エントリ ポイントを実装します。
*/

#include "stdafx.h"
#include "paint32.h"
#include "resource.h"
#define LOADSTRING_MAX 32

static HWND WINAPI CreateApplicationWindow(_In_opt_ HINSTANCE hInstance);
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
	int nExitCode = -1;
	hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (SUCCEEDED(hResult))
	{
		if (RegisterCommonClasses() &&
			RegisterPrivateClasses(hInstance) &&
			(hWnd = CreateApplicationWindow(hInstance)))
		{
			ShowWindow(hWnd, nCmdShow);
			UpdateWindow(hWnd);
			hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_APPLICATION));

			while ((nExitCode = GetMessage(&msg, NULL, 0, 0)) > 0)
			{
				if (!(SendMessage(hWnd, APPLICATION_ISDIALOGMESSAGE, 0, (LPARAM)&msg) || TranslateAccelerator(hWnd, hAccel, &msg)))
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
		ErrorMessageBox(hInstance, hWnd, hResult);
	}

	return nExitCode;
}

/*
新しいアプリケーション ウィンドウを作成します。
*/
static
HWND WINAPI CreateApplicationWindow(
	_In_opt_ HINSTANCE hInstance)
{
	TCHAR strCaption[LOADSTRING_MAX];
	LoadString(hInstance, IDS_APPTITLE, strCaption, LOADSTRING_MAX);
	return CreateWindow(APPLICATIONCLASSNAME, strCaption, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
}

/*
コモン コントロールを利用可能にします。
*/
static
BOOL WINAPI RegisterCommonClasses(
	void)
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof icc;
	icc.dwICC = ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES;
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
	UINT atom;

	/* Application Class */
	ZeroMemory(&wc, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = ApplicationWindowProc;
	wc.cbWndExtra = APPLICATIONWINDOWEXTRA;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_APPLICATION);
	wc.lpszClassName = APPLICATIONCLASSNAME;
	atom = RegisterClassEx(&wc);

	/* Outline Class */
	if (!atom) goto EXIT;
	wc.lpfnWndProc = OutlineWindowProc;
	wc.cbWndExtra = OUTLINEWINDOWEXTRA;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = OUTLINECLASSNAME;
	atom = RegisterClassEx(&wc);

	/* Palette Class */
	if (!atom) goto EXIT;
	wc.lpfnWndProc = PaletteWindowProc;
	wc.cbWndExtra = PALETTEWINDOWEXTRA;
	wc.lpszClassName = PALETTECLASSNAME;
	atom = RegisterClassEx(&wc);

EXIT:
	return !!atom;
}
