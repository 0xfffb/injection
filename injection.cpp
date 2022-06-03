// injection.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "injection.h"
#include "shlobj_core.h"
#include <stdlib.h>


INT_PTR CALLBACK Injection(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL GetFileFullPath(HWND hDlg);
BOOL InjectionDll(DWORD ProcessId, CONST CHAR DllAddressStr[MAX_PATH]);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, Injection);
    return TRUE;
}

INT_PTR CALLBACK Injection(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    switch (message)
    {
    case WM_INITDIALOG:

        SetDlgItemTextA(hDlg, IDC_EDIT_FILEPATH, "path");
        SetDlgItemTextA(hDlg, IDC_EDIT_PID, "pid");

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        //  IDC_BUTTON_SELECT   打开文件_按钮
        if (LOWORD(wParam) == IDC_BUTTON_SELECT)
        {
            if (GetFileFullPath(hDlg))
            {
                return (INT_PTR)TRUE;
            }
        }
        //  IDC_BUTTON_INJECTION    注入_按钮
        if (LOWORD(wParam) == IDC_BUTTON_INJECTION) 
        {
            CHAR FilePath[MAX_PATH] = { NULL };
            //  DWORD 4字节
            CHAR ProcessIdStr[0x10] = { NULL };

            GetDlgItemTextA(hDlg, IDC_EDIT_FILEPATH, FilePath, MAX_PATH);
            GetDlgItemTextA(hDlg, IDC_EDIT_PID, (LPSTR)ProcessIdStr, sizeof(ProcessIdStr));

            //  字符串转DWORD
            DWORD ProcessId = atol(ProcessIdStr);


            //  注入
            if (!InjectionDll(ProcessId, FilePath))
            {
                MessageBoxA(hDlg, "注入失败!", "注入失败!", MB_OK);
                break;
            }
           
            MessageBoxA(hDlg, "注入成功!", "注入成功!", MB_OK);
            
            return (INT_PTR)TRUE;


        }
        break;
    }
    return (INT_PTR)FALSE;
}




BOOL InjectionDll(DWORD ProcessId, CONST CHAR DllAddressStr[MAX_PATH])
{
    //  打开句柄
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
    //  申请虚拟内存存放DLL地址
    /*CHAR DllAddressStr[MAX_PATH] = { "E://code//cpp//Visual Studio//EntryDll//x64//Debug//EntryDll.dll" };*/
    LPVOID dllAddress = VirtualAllocEx(hProcess, NULL, strlen(DllAddressStr), MEM_COMMIT, PAGE_READWRITE);
    if (!dllAddress)
    {
        return FALSE;
    }
    //  DLL写入目标进程内存
    if (!WriteProcessMemory(hProcess, dllAddress, DllAddressStr, strlen(DllAddressStr), NULL))
    {
        return FALSE;
    }

    //	创建远程线程执行dll
    //      每个程序都会包含Kernel32.dll，所以使用Kernel32.dll执行LoadLibraryA函数加载目标DLL
    HMODULE hKernel32 = GetModuleHandle(L"Kernel32.dll");
    if (!hKernel32)
    {
        return FALSE;
    }

    LPCVOID kernalAddress = GetProcAddress(hKernel32, "LoadLibraryA");
    if (!kernalAddress)
    {
        return FALSE;
    }
    //      创建远程线程加载DLL
    if (CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)kernalAddress, dllAddress, 0, NULL))
    {
        return TRUE;
    }
    return FALSE;

}




/*
    使用SHBrowseForFolderA查询文件
*/
BOOL GetFileFullPath(HWND hDlg)
{
    TCHAR szTitle[MAX_PATH] = { 0 };
    TCHAR szPath[MAX_PATH] = { 0 };
    TCHAR szDisplay[MAX_PATH] = { 0 };

    BROWSEINFOA lpbi;

    lpbi.hwndOwner = NULL;
    lpbi.pidlRoot = NULL;
    lpbi.pszDisplayName = (LPSTR)szDisplay;
    lpbi.lpszTitle = (LPSTR)szTitle;
    lpbi.ulFlags = BIF_BROWSEINCLUDEFILES;
    lpbi.iImage = IDR_MAINFRAME;
    lpbi.lpfn = NULL;
    lpbi.lParam = 0;

    LPITEMIDLIST Lpi = SHBrowseForFolderA(&lpbi);

    SHGetPathFromIDListA(Lpi, (LPSTR)szPath);

    SetDlgItemTextA(hDlg, IDC_EDIT_FILEPATH, (LPCSTR)szPath);;
    return TRUE;
}
