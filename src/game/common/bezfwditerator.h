/**
 * @file
 *
 * @author xezon
 *
 * @brief Bezier Forward Iterator class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "beziersegment.h"

class BezFwdIterator
{
public:
    BezFwdIterator();

    BezFwdIterator(int32_t count, const BezierSegment *segment);

    void Start();

    bool Done();

    const Coord3D &Get_Current();

    void Next();

private:
    int32_t m_index;
    int32_t m_count;
    BezierSegment m_segment;
    Coord3D m_point;
    Coord3D m_add0;
    Coord3D m_add1;
    Coord3D m_add2;
};
