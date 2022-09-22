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
#pragma once
#include "w3dafx.h"

class CMeshPropPage : public CPropertyPage
{
public:
    CMeshPropPage() {}
    CMeshPropPage(const CString &name);
    virtual ~CMeshPropPage() override {}
    virtual BOOL OnInitDialog() override;

private:
    DECLARE_DYNCREATE(CMeshPropPage)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnClose();

    CString m_name;
};
