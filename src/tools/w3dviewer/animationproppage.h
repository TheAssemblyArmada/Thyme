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
#pragma once
#include "w3dafx.h"

class CAnimationPropPage : public CPropertyPage
{
public:
    CAnimationPropPage();
    virtual ~CAnimationPropPage() override {}
    virtual BOOL OnInitDialog() override;

private:
    DECLARE_DYNCREATE(CAnimationPropPage)
    DECLARE_MESSAGE_MAP()
};
