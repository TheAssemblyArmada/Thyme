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
#pragma once
#include "w3dafx.h"

class CHierarchyPropPage : public CPropertyPage
{
public:
    CHierarchyPropPage() {}
    CHierarchyPropPage(const CString &name);
    virtual ~CHierarchyPropPage() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;

private:
    DECLARE_DYNCREATE(CHierarchyPropPage)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnDblClk(NMHDR *pNMHDR, LRESULT *pResult);

    CListCtrl m_list;
    CString m_name;
};
