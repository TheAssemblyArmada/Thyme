/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View utillity functions
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "utils.h"
#include "assetmgr.h"
#include "mainfrm.h"
#include "w3dviewdoc.h"

CString GetFilePath(const char *name)
{
    char path[MAX_PATH];
    strcpy(path, name);
    char *c = strrchr(path, '\\');

    if (c != nullptr) {
        *c = 0;
    }

    return path;
}

CString GetFilenameFromPath(const char *path)
{
    const char *c = strrchr(path, '\\');

    if (c != nullptr) {
        return c + 1;
    } else {
        return path;
    }
}

CGraphicView *GetCurrentGraphicView()
{
    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        CW3DViewDoc *document = static_cast<CW3DViewDoc *>(frame->GetActiveDocument());

        if (document != nullptr) {
            return document->GetGraphicView();
        }
    }

    return nullptr;
}

bool HasBaseModelName(const char *name)
{
    bool has = false;

    RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(name);

    if (robj != nullptr) {
        if (robj->Get_Base_Model_Name() != nullptr) {
            has = true;
        }

        robj->Release_Ref();
    }

    return has;
}

bool HasMultipleLODs(const char *name)
{
    bool has = false;

    RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(name);

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD && robj->Get_LOD_Count() > 1) {
            has = true;
        }

        robj->Release_Ref();
    }

    return has;
}

CW3DViewDoc *GetCurrentDocument()
{
    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        CW3DViewDoc *document = static_cast<CW3DViewDoc *>(frame->GetActiveDocument());

        if (document != nullptr) {
            return document;
        }
    }

    return nullptr;
}

void SetDlgItemFloat(HWND hDlg, int nIDDlgItem, float f)
{
    CString str;
    str.Format("%.2f", f);
    SetDlgItemText(hDlg, nIDDlgItem, str);
}

float GetDlgItemFloat(HWND hDlg, int nIDDlgItem)
{
    char str[20];
    GetDlgItemText(hDlg, nIDDlgItem, str, sizeof(str));
    return atof(str);
}

void InitializeSpinButton(CSpinButtonCtrl *spin, float value, float min, float max)
{
    spin->SetRange32(min * 100.0f, max * 100.0f);
    spin->SetPos(value * 100.0f);
    CWnd *wnd = spin->GetBuddy();

    if (wnd != nullptr) {
        SetWindowFloat(wnd->m_hWnd, value);
    }
}

void UpdateEditCtrl(HWND hwnd, int delta)
{
    if ((GetWindowLong(hwnd, GWL_STYLE) & UDS_SETBUDDYINT) == 0) {
        HWND edit = reinterpret_cast<HWND>(SendMessage(hwnd, UDM_GETBUDDY, 0, 0));

        if (IsWindow(edit)) {
            char str[20];
            GetWindowText(edit, str, sizeof(str));
            int min = 0;
            int max = 0;
            float f = atof(str) + delta * 0.01f;
            SendMessage(hwnd, UDM_GETRANGE32, reinterpret_cast<WPARAM>(&min), reinterpret_cast<LPARAM>(&max));

            f = std::clamp(f, min * 0.01f, max * 0.01f);
            SetWindowFloat(edit, f);
        }
    }
}

void DisableWindows(HWND window, bool disable)
{
    char name[64];

    for (HWND h = GetWindow(window, GW_CHILD); h != nullptr; h = GetWindow(h, GW_HWNDNEXT)) {
        GetClassName(h, name, sizeof(name));

        if (strcmpi(name, "STATIC") != 0) {
            EnableWindow(h, disable);
        }
    }
}

void SetWindowFloat(HWND hWnd, float f)
{
    CString str;
    str.Format("%.3f", f);
    SetWindowText(hWnd, str);
}
