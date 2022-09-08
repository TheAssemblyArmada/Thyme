/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Background Object Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "backgroundobjectdialog.h"
#include "assetmgr.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(CBackgroundObjectDialog, CDialog)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_HIERARCHIES, OnItemChanged)
    ON_COMMAND(IDABORT, OnClear)
END_MESSAGE_MAP()
// clang-format on

CBackgroundObjectDialog::CBackgroundObjectDialog(CWnd *pParentWnd) : CDialog(IDD_BACKGROUNDOBJECT, pParentWnd) {}

void CBackgroundObjectDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_HIERARCHIES, m_objectList);
}

BOOL CBackgroundObjectDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    PositionWindow(this->m_hWnd);
    m_objectList.InsertColumn(0, "Name");

    RenderObjIterator *iter = W3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator();

    if (iter != nullptr) {
        for (iter->First(); !iter->Is_Done(); iter->Next()) {
            const char *name = iter->Current_Item_Name();

            if (W3DAssetManager::Get_Instance()->Render_Obj_Exists(name)
                && iter->Current_Item_Class_ID() == RenderObjClass::CLASSID_HLOD) {
                m_objectList.InsertItem(1, 0, name, 0, 0, 0, 0);
            }
        }

        delete iter;
    }

    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        CString str(doc->m_backgroundObjectName);
        LVFINDINFO info = { 0 };
        info.flags = LVFI_STRING;
        info.psz = str;
        int index = m_objectList.FindItem(&info);
        CString name;

        if (index == -1) {
            m_objectList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
            name = m_objectList.GetItemText(0, 0);
        } else {
            m_objectList.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
            name = m_objectList.GetItemText(index, 0);
        }

        SetDlgItemText(IDC_CURRENT, name);
    }

    m_objectList.SetColumnWidth(0, -1);
    return TRUE;
}

void CBackgroundObjectDialog::OnOK()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        int index = m_objectList.GetNextItem(-1, LVNI_SELECTED);

        if (index == -1) {
            doc->SetBackgroundObject(nullptr);
        } else {
            doc->SetBackgroundObject(m_objectList.GetItemText(index, 0));
        }
    }

    CDialog::OnOK();
}

void CBackgroundObjectDialog::OnItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (pNMHDR != nullptr) {
        NMLISTVIEW *nm = reinterpret_cast<NMLISTVIEW *>(pNMHDR);

        if ((nm->uChanged & LVIF_STATE) == LVIF_STATE) {
            if ((nm->uNewState & LVIS_SELECTED) == LVIS_SELECTED) {
                SetDlgItemText(IDC_CURRENT, m_objectList.GetItemText(nm->iItem, 0));
            } else if (m_objectList.GetNextItem(-1, LVNI_SELECTED) == -1) {
                SetDlgItemText(IDC_CURRENT, "");
            }
        }
    }

    *pResult = 0;
}

void CBackgroundObjectDialog::OnClear()
{
    int index = m_objectList.GetNextItem(-1, LVNI_SELECTED);

    if (index == -1) {
        m_objectList.SetItemState(index, 0, LVIS_SELECTED);
    }
}
