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
#include "w3dafx.h"

class EmitterInstanceList;

class EmitterUserPropPageClass : public CPropertyPage
{
public:
    virtual ~EmitterUserPropPageClass() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnApply() override;

private:
    EmitterUserPropPageClass();
    DECLARE_DYNCREATE(EmitterUserPropPageClass)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnUserStringChanged();
    afx_msg void OnUser();

    void Initialize();

    CComboBox m_userCombo;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    unsigned int m_userType;
    CString m_userString;
};
