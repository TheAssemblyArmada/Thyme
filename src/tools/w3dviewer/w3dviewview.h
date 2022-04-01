/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View view
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

class CW3DViewView : public CView
{
public:
    virtual ~CW3DViewView() override {}
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
    virtual void OnDraw(CDC *pDC) override {}

protected:
    CW3DViewView() {}
    DECLARE_DYNCREATE(CW3DViewView)
    DECLARE_MESSAGE_MAP()
};
