/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View hierarchy property page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "hierarchyproppage.h"
#include "assetmgr.h"
#include "assetpropertysheet.h"
#include "meshproppage.h"
#include "rendobj.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CHierarchyPropPage, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(CHierarchyPropPage, CPropertyPage)
    ON_NOTIFY(NM_DBLCLK, IDC_SUBOBJECTLIST, &OnDblClk)
END_MESSAGE_MAP()
// clang-format on

CHierarchyPropPage::CHierarchyPropPage(const CString &name) : CPropertyPage(IDD_HIERARCHY), m_name(name) {}

void CHierarchyPropPage::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_SUBOBJECTLIST, m_list);
}

BOOL CHierarchyPropPage::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_name.GetLength() > 0) {
        RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(m_name);

        if (robj != nullptr) {
            CString str;
            str.Format(IDS_HIERARCHYPROPERTYSTRING, m_name);
            SetDlgItemText(IDC_PROPERTIES, str);
            SetDlgItemInt(IDC_POLYS, robj->Get_Num_Polys());
            int count = robj->Get_Num_Sub_Objects();
            SetDlgItemInt(IDC_SUBOBJECTS, count);
            m_list.InsertColumn(0, "Name");

            for (int i = 0; i < count; i++) {
                RenderObjClass *o = robj->Get_Sub_Object(i);

                if (o != nullptr) {
                    m_list.InsertItem(1, 0, o->Get_Name(), 0, 0, 0, 0);
                    o->Release_Ref();
                }
            }

            m_list.SetColumnWidth(0, 0xFFFF);
            robj->Release_Ref();
        }
    }

    GetParent()->GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
    GetParent()->GetDlgItem(IDCANCEL)->SetWindowText("Close");
    return TRUE;
}

void CHierarchyPropPage::OnDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
    int index = m_list.GetNextItem(-1, LVNI_SELECTED);

    if (index != -1) {
        CString str = m_list.GetItemText(index, 0);
        CMeshPropPage page(str);
        CAssetPropertySheet sheet(IDS_MESHPROPERTIES, &page, this);
        sheet.DoModal();
    }

    *pResult = 0;
}
