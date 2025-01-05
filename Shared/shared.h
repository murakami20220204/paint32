/*
Copyright 2025 Taichi Murakami.
外部リンケージを持つ関数を公開します。
*/

#pragma once
#include <windows.h>

/* External Functions */
EXTERN_C int WINAPI ErrorMessageBox(_In_opt_ HINSTANCE hInstance, _In_opt_ HWND hWnd, _In_ DWORD dwError);
EXTERN_C BOOL WINAPI GetVersionFromResource(_In_opt_ HINSTANCE hInstance, _Out_writes_z_(cchBuffer) LPTSTR lpBuffer, _In_ UINT cchBuffer);
EXTERN_C BOOL WINAPI SetWindowPosOnCenter(_In_ HWND hWnd);
