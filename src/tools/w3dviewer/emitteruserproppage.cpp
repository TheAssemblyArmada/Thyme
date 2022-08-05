/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter user prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitteruserproppage.h"
#include "emitterinstancelist.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(EmitterUserPropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterUserPropPageClass, CPropertyPage)
    ON_EN_CHANGE(IDC_USERSTRING, OnUserStringChanged)
    ON_CBN_SELCHANGE(IDC_USERCOMBO, OnUser)
END_MESSAGE_MAP()
// clang-format on

EmitterUserPropPageClass::EmitterUserPropPageClass() :
    CPropertyPage(IDD_EMITTERUSER), m_instanceList(nullptr), m_isValid(true), m_userType(0)
{
    Initialize();
}

void EmitterUserPropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_USERCOMBO, m_userCombo);
}

BOOL EmitterUserPropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_userCombo.AddString("Default");
    m_userCombo.SetCurSel(m_userType);
    SetDlgItemText(IDC_USERSTRING, m_userString);
    return TRUE;
}

BOOL EmitterUserPropPageClass::OnApply()
{
    m_userType = m_userCombo.GetCurSel();
    GetDlgItemText(IDC_USERSTRING, m_userString);
    m_instanceList->Set_User_Type(m_userType);
    m_instanceList->Set_User_String(m_userString);
    return CPropertyPage::OnApply();
}

void EmitterUserPropPageClass::OnUserStringChanged()
{
    SetModified();
}

void EmitterUserPropPageClass::OnUser()
{
    SetModified();
}

void EmitterUserPropPageClass::Initialize()
{
    if (m_instanceList != nullptr) {
        m_userType = m_instanceList->Get_User_Type();
        m_userString = m_instanceList->Get_User_String();
    }
}
