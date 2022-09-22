/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View mesh property page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshproppage.h"
#include "assetmgr.h"
#include "mesh.h"
#include "meshmdl.h"
#include "resource.h"
#include "utils.h"
#include "w3d_file.h"
#include "w3dviewdoc.h"

IMPLEMENT_DYNCREATE(CMeshPropPage, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(CMeshPropPage, CPropertyPage)
    ON_WM_CLOSE()
END_MESSAGE_MAP()
// clang-format on

CMeshPropPage::CMeshPropPage(const CString &name) : CPropertyPage(IDD_MESH, 0), m_name(name) {}

BOOL CMeshPropPage::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_name.GetLength() > 0) {
        MeshClass *mesh = static_cast<MeshClass *>(W3DAssetManager::Get_Instance()->Create_Render_Obj(m_name));

        if (mesh != nullptr) {
            CString str;
            str.Format(IDS_MESHPROPERTYSTRING, m_name);
            SetDlgItemText(IDC_PROPERTIES, str);
            SetDlgItemInt(IDC_POLYGONS, mesh->Get_Num_Polys());
            MeshModelClass *model = mesh->Get_Model();

            if (model != nullptr) {
                SetDlgItemInt(IDC_VERTICIES, model->Get_Vertex_Count());
            }

            SetDlgItemText(IDC_USERTEXT, mesh->Get_User_Text());
            uint32_t flags = mesh->Get_W3D_Flags();
            int id;

            if ((flags & W3D_MESH_FLAG_COLLISION_BOX) != 0) {
                id = IDC_COLBOX;
            } else if ((flags & W3D_MESH_FLAG_SKIN) != 0) {
                id = IDC_SKIN;
            } else if ((flags & W3D_MESH_FLAG_SHADOW) != 0) {
                id = IDC_SHADOW;
            } else {
                id = IDC_NORMAL;
            }

            SendDlgItemMessage(id, BM_SETCHECK, 1, 0);

            if ((flags & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) != 0) {
                SendDlgItemMessage(IDC_PHYSICAL, BM_SETCHECK, 1, 0);
            }

            if ((flags & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) != 0) {
                SendDlgItemMessage(IDC_PROJECTILE, BM_SETCHECK, 1, 0);
            }

            if ((flags & W3D_MESH_FLAG_HIDDEN) != 0) {
                SendDlgItemMessage(IDC_HIDDEN, BM_SETCHECK, 1, 0);
            }

            mesh->Release_Ref();
        }
    }

    GetParent()->GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
    GetParent()->GetDlgItem(IDCANCEL)->SetWindowText("Close");
    return TRUE;
}

void CMeshPropPage::OnClose()
{
    CPropertyPage::OnClose();
}
