/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Line 2D
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
#include "coord.h"

bool Clip_Line_2D(ICoord2D *p1, ICoord2D *p2, ICoord2D *c1, ICoord2D *c2, IRegion2D *clip_region);
bool Coord_3D_Inside_Rect_2D(const Coord3D *input_point, const Coord2D *tl, const Coord2D *br);
void Scale_Rect_2D(Coord2D *tl, Coord2D *br, float scale_factor);
