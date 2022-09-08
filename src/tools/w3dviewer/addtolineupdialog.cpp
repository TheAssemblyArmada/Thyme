/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Add to lineup dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "addtolineupdialog.h"
#include "assetmgr.h"
#include "resource.h"
#include "viewerscene.h"

// clang-format off
BEGIN_MESSAGE_MAP(CAddToLineupDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

CAddToLineupDialog::CAddToLineupDialog(ViewerSceneClass *scene, CWnd *pParentWnd) :
    CDialog(IDD_ADDOBJECT, pParentWnd), m_scene(scene)
{
    m_objectName = "";
}

void CAddToLineupDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_CBString(pDX, IDC_OBJECT, m_objectName);
    DDV_MaxChars(pDX, m_objectName, 64);
}

BOOL CAddToLineupDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_scene == nullptr) {
        return true;
    }

    CWnd *item = GetDlgItem(IDC_OBJECT);

    RenderObjIterator *iter = W3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator();

    if (iter != nullptr) {
        for (iter->First(); !iter->Is_Done(); iter->Next()) {
            if (m_scene->Is_LOD(iter->Current_Item_Class_ID())) {
                ::SendMessage(item->m_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(iter->Current_Item_Name()));
            }
        }

        delete iter;
    }

    return true;
}

void CAddToLineupDialog::OnOK()
{
    CWnd *item = GetDlgItem(IDC_OBJECT);
    CString str;
    item->GetWindowText(str);

    if (str.GetLength() != 0) {
        CDialog::OnOK();
    } else {
        AfxMessageBox("Please select an object, or type in an object name.", 0, 0);
    }
}
