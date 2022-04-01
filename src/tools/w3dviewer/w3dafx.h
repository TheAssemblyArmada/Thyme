/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Wrapper around always.h to include MFC correctly
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"

// need to include these in a certain order
// clang-format off
#define _d3d9TYPES_H_ // this is here to stop MFC including d3d9types.h (which will conflict with d3d8types.h that we are using). The MFC headers don't actually need anything from d3d9types.h so this is fine.
#include <afxwin.h> // MFC core and standard components
#include <afxext.h> // MFC extensions
#include <afxcmn.h>
#include <afxcview.h>
// clang-format on

#define ID_OBJECT_TOOLBAR 101
#define ID_ANIMATION_TOOLBAR 102
