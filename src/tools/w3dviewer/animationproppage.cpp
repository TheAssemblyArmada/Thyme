/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View animation property page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "animationproppage.h"
#include "hanim.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

IMPLEMENT_DYNCREATE(CAnimationPropPage, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(CAnimationPropPage, CPropertyPage)
END_MESSAGE_MAP()
// clang-format on

CAnimationPropPage::CAnimationPropPage() : CPropertyPage(IDD_ANIMATION, 0) {}

BOOL CAnimationPropPage::OnInitDialog()
{
    CDialog::OnInitDialog();
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        HAnimClass *anim = doc->m_animation;

        if (anim != nullptr) {
            CString str;
            str.Format(IDS_ANIMATIONPROPERTYSTRING, anim->Get_Name());
            SetDlgItemText(IDC_PROPERTIES, str);
            SetDlgItemInt(IDC_FRAMES, anim->Get_Num_Frames());
            str.Format("%.2f fps", anim->Get_Frame_Rate());
            SetDlgItemText(IDC_FRAMERATE, str);
            str.Format("%.3f seconds", anim->Get_Total_Time());
            SetDlgItemText(IDC_TOTALTIME, str);
            SetDlgItemText(IDC_HIERARCHY, anim->Get_HName());
        }
    }

    return TRUE;
}
