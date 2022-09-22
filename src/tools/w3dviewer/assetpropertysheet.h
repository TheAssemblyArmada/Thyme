/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View asset property sheet
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

class CAssetPropertySheet : public CPropertySheet
{
public:
    CAssetPropertySheet(UINT uID, CPropertyPage *page, CWnd *pParentWnd);
    virtual ~CAssetPropertySheet() override {}

private:
    DECLARE_DYNAMIC(CAssetPropertySheet)
    DECLARE_MESSAGE_MAP()

    CPropertyPage *m_propertyPage;
};
