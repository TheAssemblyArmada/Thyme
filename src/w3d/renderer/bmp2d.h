/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Bitmap 2D
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
#include "dynamesh.h"
class Bitmap2D : public DynamicScreenMeshClass
{
public:
    Bitmap2D(TextureClass *texture, float x, float y, bool adjust_xy, bool additive, bool create_material, bool opaque);
    Bitmap2D(const char *texture,
        float x,
        float y,
        bool adjust_xy,
        bool additive,
        bool create_material,
        int tw,
        int th,
        bool opaque);
    Bitmap2D(const Bitmap2D &src);
    virtual RenderObjClass *Clone() const;
    virtual int Class_ID() const { return CLASSID_BITMAP2D; }
};
