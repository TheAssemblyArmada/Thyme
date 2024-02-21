/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View color utils
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

void DrawSunkenRect(BYTE *bits, RECT *rect, int stride);
void DrawRaisedRect(BYTE *bits, RECT *rect, int stride);
void DrawOutlinedRect(BYTE *bits, RECT *rect, int color, int stride);
