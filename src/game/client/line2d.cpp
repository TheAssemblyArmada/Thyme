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
#include "line2d.h"

bool Clip_Line_2D(ICoord2D *p1, ICoord2D *p2, ICoord2D *c1, ICoord2D *c2, IRegion2D *clip_region)
{
    enum OutCodeEnum
    {
        CODE_INSIDE = 0, // 0000
        CODE_LEFT = 1, // 0001
        CODE_RIGHT = 2, // 0010
        CODE_BOTTOM = 4, // 0100
        CODE_TOP = 8, // 1000
    };

    int x_min = clip_region->lo.x;
    int x_max = clip_region->hi.x;
    int y_min = clip_region->lo.y;
    int y_max = clip_region->hi.y;
    int x0 = p1->x;
    int y0 = p1->y;
    int x1 = p2->x;
    int y1 = p2->y;
    int out_code0 = CODE_INSIDE;

    if (x0 < x_min) {
        out_code0 = CODE_LEFT;
    } else if (x0 > x_max) {
        out_code0 = CODE_RIGHT;
    }

    if (y0 < y_min) {
        out_code0 |= CODE_TOP;
    } else if (y0 > y_max) {
        out_code0 |= CODE_BOTTOM;
    }

    int out_code1 = CODE_INSIDE;

    if (x1 < x_min) {
        out_code1 = CODE_LEFT;
    } else if (x1 > x_max) {
        out_code1 = CODE_RIGHT;
    }

    if (y1 < y_min) {
        out_code1 |= CODE_TOP;
    } else if (y1 > y_max) {
        out_code1 |= CODE_BOTTOM;
    }

    // Both points inside of region, no clipping required
    if (out_code1 == CODE_INSIDE && out_code0 == CODE_INSIDE) {
        *c1 = *p1;
        *c2 = *p2;
        return true;
    }

    // If both points are outside region on same sides then clipping does not bring them inside of region
    if ((out_code1 & out_code0) != 0) {
        return false;
    }

    if (out_code0 != CODE_INSIDE) {
        if ((out_code0 & CODE_TOP) != 0) {
            if (y1 == y0) {
                return false;
            }

            x0 += (y_min - y0) * (x1 - x0) / (y1 - y0);
            y0 = y_min;
        } else if ((out_code0 & CODE_BOTTOM) != 0) {
            if (y1 == y0) {
                return false;
            }

            x0 += (y_max - y0) * (x1 - x0) / (y1 - y0);
            y0 = y_max;
        }
        if (x0 > x_max) {
            if (x1 == x0) {
                return false;
            }

            y0 += (x_max - x0) * (y1 - y0) / (x1 - x0);
            x0 = x_max;
        } else {
            if (x0 < x_min) {
                if (x1 == x0) {
                    return false;
                }

                y0 += (x_min - x0) * (y1 - y0) / (x1 - x0);
                x0 = x_min;
            }
        }
    }

    if (out_code1 != CODE_INSIDE) {
        if ((out_code1 & CODE_TOP) != 0) {
            if (y1 == y0) {
                return false;
            }

            x1 += (y_min - y1) * (x1 - x0) / (y1 - y0);
            y1 = y_min;
        } else if ((out_code1 & CODE_BOTTOM) != 0) {
            if (y1 == y0) {
                return false;
            }

            x1 += (y_max - y1) * (x1 - x0) / (y1 - y0);
            y1 = y_max;
        }
        if (x1 > x_max) {
            if (x1 == x0) {
                return false;
            }

            y1 += (x_max - x1) * (y1 - y0) / (x1 - x0);
            x1 = x_max;
        } else {
            if (x1 < x_min) {
                if (x1 == x0) {
                    return false;
                }

                y1 += (x_min - x1) * (y1 - y0) / (x1 - x0);
                x1 = x_min;
            }
        }
    }

    c1->x = x0;
    c1->y = y0;
    c2->x = x1;
    c2->y = y1;

    return x0 >= x_min && x0 <= x_max && y0 >= y_min && y0 <= y_max && x1 >= x_min && x1 <= x_max && y1 >= y_min
        && y1 <= y_max;
}

bool Coord_3D_Inside_Rect_2D(const Coord3D *input_point, const Coord2D *tl, const Coord2D *br)
{
    return input_point->x >= tl->x && input_point->x <= br->x && input_point->y >= tl->y && input_point->y <= br->y;
}

void Scale_Rect_2D(Coord2D *tl, Coord2D *br, float scale_factor)
{
    float delta = scale_factor - 1.0f;
    float x = (br->x - tl->x) * delta * 0.5f;
    float y = (br->y - tl->y) * delta * 0.5f;
    tl->x = tl->x - x;
    tl->y = tl->y - y;
    br->x = x + br->x;
    br->y = y + br->y;
}
