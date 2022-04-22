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
#include "w3dviewview.h"

IMPLEMENT_DYNCREATE(CW3DViewView, CView)

// clang-format off
BEGIN_MESSAGE_MAP(CW3DViewView, CView)
END_MESSAGE_MAP()
// clang-format on

BOOL CW3DViewView::PreCreateWindow(CREATESTRUCT &cs)
{
    return CView::PreCreateWindow(cs);
}
