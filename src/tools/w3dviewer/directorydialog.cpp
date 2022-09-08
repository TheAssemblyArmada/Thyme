/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Directory Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "directorydialog.h"
#include "resource.h"
#include "utils.h"
#include <dlgs.h>

UINT_PTR CALLBACK ChooseDirectoryHook(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND && LOWORD(wParam) == 1 && HIWORD(wParam) == 0) {
        SetDlgItemText(hwnd, edt1, "xxx.xxx");
    }

    return 0;
}

bool ChooseDirectory(HWND parent, const char *initialdir, CString &directory)
{
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    char name[MAX_PATH] = { 0 };
    ofn.hwndOwner = parent;
    ofn.lpstrInitialDir = initialdir;
    ofn.hInstance = AfxGetModuleState()->m_hCurrentResourceHandle;
    ofn.lpstrFilter = "";
    ofn.lpstrFile = name;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Choose Directory";
    ofn.lpfnHook = ChooseDirectoryHook;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SELECTDIR);
    ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_LONGNAMES;

    if (!GetOpenFileName(&ofn)) {
        return false;
    }

    directory = GetFilePath(name);
    return true;
}
