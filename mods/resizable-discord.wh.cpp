// ==WindhawkMod==
// @id              resizable-discord
// @name            Discord Transparency Resize Fix
// @description     Forces discord to be resizable with vencord window transparency
// @version         1.0.2
// @author          gameknight963
// @include         discord.exe
// @github          https://github.com/Gameknight963
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod forces resizing on Discord when using window transparency + native titlebar.

How it works: It hooks ShowWindow to enable WS_THICKFRAME, which Discord 
disables because it uses a custom titlebar. When it's disabled, Electron handles
the resize logic through software. When Vencord turns on custom titlebar, it doesn't 
set WS_THICKFRAME, so the window is not resizable. This mod fixes that.

![Image showing resizable cursor on border](https://i.imgur.com/GNRdQ5A.png)
*/
// ==/WindhawkModReadme==

#include <windows.h>

typedef BOOL(WINAPI* ShowWindow_t)(HWND, int);
ShowWindow_t ShowWindow_Original;

typedef LRESULT(WINAPI* DispatchMessageW_t)(const MSG*);
DispatchMessageW_t DispatchMessageW_Original;

void HookWindow(HWND hWnd)
{
    DWORD pid;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid != GetCurrentProcessId()) return;

    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
    if (style & WS_POPUP)
        return;
    SetWindowLongPtr(hWnd, GWL_STYLE, style | WS_THICKFRAME);
}

BOOL WINAPI ShowWindow_Hook(HWND hWnd, int nCmdShow) {
    HookWindow(hWnd);
    return ShowWindow_Original(hWnd, nCmdShow);
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM) { HookWindow(hWnd); return TRUE; }

LRESULT WINAPI DispatchMessageW_Hook(const MSG* lpMsg)
{
    if (lpMsg->message == WM_NCLBUTTONDOWN && lpMsg->wParam == HTMAXBUTTON)
    {
        ShowWindow_Original(lpMsg->hwnd, IsZoomed(lpMsg->hwnd) ? SW_RESTORE : SW_MAXIMIZE);
        return 0;
    }
    return DispatchMessageW_Original(lpMsg);
}

BOOL Wh_ModInit()
{
    Wh_SetFunctionHook((void*)ShowWindow, (void*)ShowWindow_Hook, (void**)&ShowWindow_Original);
    Wh_SetFunctionHook((void*)DispatchMessageW, (void*)DispatchMessageW_Hook, (void**)&DispatchMessageW_Original);
    EnumWindows(EnumWindowsProc, 0);
    return TRUE;
}