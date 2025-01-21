/*
Copyright 2025 Taichi Murakami.
アプリケーションのメイン エントリ ポイントを実装します。
*/

#define MAXLOADSTRING 32

#include "Paint.h"
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

static HWND WINAPI CreateFrameWindow(_In_opt_ HINSTANCE hInstance);
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
			hWnd = CreateFrameWindow(hInstance);

			if (hWnd)
			{
				ShowWindow(hWnd, nCmdShow);
				UpdateWindow(hWnd);
				hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_FRAME));

				while ((bResult = GetMessage(&msg, NULL, 0, 0)) > 0)
				{
					if (!TranslateAccelerator(hWnd, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
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
HWND WINAPI CreateFrameWindow(
	_In_opt_ HINSTANCE hInstance)
{
	TCHAR strCaption[MAXLOADSTRING];
	LoadString(hInstance, IDS_PAINT, strCaption, MAXLOADSTRING);
	return CreateWindow(FRAMECLASSNAME, strCaption, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
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
	/* フレーム ウィンドウ クラスを登録します。 */
	WNDCLASSEX wc;
	ATOM atom;
	ZeroMemory(&wc, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = FrameWindowProc;
	wc.cbWndExtra = FRAMEWINDOWEXTRA;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FRAME));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_FRAME);
	wc.lpszClassName = FRAMECLASSNAME;
	atom = RegisterClassEx(&wc);

	/* ドキュメント ウィンドウ クラスを登録します。 */
	if (!atom) goto EXIT;
	wc.lpfnWndProc = DocumentWindowProc;
	wc.cbWndExtra = DOCUMENTWINDOWEXTRA;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DOCUMENT));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = DOCUMENTCLASSNAME;
	atom = RegisterClassEx(&wc);

EXIT:
	return atom;
}
