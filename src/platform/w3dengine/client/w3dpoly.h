/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
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
#include "simplevec.h"

class PlaneClass;
class Vector3;

class ClipPolyClass
{
public:
    ClipPolyClass() {}
    ~ClipPolyClass() {}

    void Reset();
    void Add_Vertex(const Vector3 &point);
    void Clip(const PlaneClass &plane, ClipPolyClass &dest);

private:
    SimpleDynVecClass<Vector3> m_verts;
    friend class WaterRenderObjClass;
};
