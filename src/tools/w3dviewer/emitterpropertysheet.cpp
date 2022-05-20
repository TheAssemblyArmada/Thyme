/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter property sheet
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterpropertysheet.h"
#include "utils.h"

void EmitterPropertySheetClass::UpdateRenderingMode(int mode)
{
    DisableWindows(m_linePage.m_hWnd, mode == W3D_EMITTER_RENDER_MODE_LINE);
}
