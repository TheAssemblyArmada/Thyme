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
#include "assetpropertysheet.h"

IMPLEMENT_DYNAMIC(CAssetPropertySheet, CPropertySheet)

// clang-format off
BEGIN_MESSAGE_MAP(CAssetPropertySheet, CPropertySheet)
END_MESSAGE_MAP()
// clang-format on

CAssetPropertySheet::CAssetPropertySheet(UINT uID, CPropertyPage *page, CWnd *pParentWnd) :
    CPropertySheet(uID, pParentWnd), m_propertyPage(page)
{
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    AddPage(page);
}
