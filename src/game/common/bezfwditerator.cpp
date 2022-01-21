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
#include "bezfwditerator.h"

#include "matrix4.h"
#include "vector4.h"

BezFwdIterator::BezFwdIterator()
{
    m_index = 0;
    m_count = 0;
    m_point.Zero();
    m_add0.Zero();
    m_add1.Zero();
    m_add2.Zero();
}

BezFwdIterator::BezFwdIterator(int count, const BezierSegment *segment)
{
    m_point.Zero();
    m_add0.Zero();
    m_add1.Zero();
    m_add2.Zero();
    m_index = 0;
    m_count = count;
    m_segment = *segment;
}

void BezFwdIterator::Start()
{
    m_index = 0;

    if (m_count > 1) {
        const float distance = 1.0f / (m_count - 1);
        const float sqr_distance = distance * distance;
        const float cub_distance = distance * distance * distance;

        Vector4 xvec(m_segment.m_points[0].x, m_segment.m_points[1].x, m_segment.m_points[2].x, m_segment.m_points[3].x);
        Vector4 yvec(m_segment.m_points[0].y, m_segment.m_points[1].y, m_segment.m_points[2].y, m_segment.m_points[3].y);
        Vector4 zvec(m_segment.m_points[0].z, m_segment.m_points[1].z, m_segment.m_points[2].z, m_segment.m_points[3].z);

        Vector4 vectors[3];
        Matrix4::Transform_Vector(BezierSegment::s_bezBasisMatrix, xvec, &vectors[0]);
        Matrix4::Transform_Vector(BezierSegment::s_bezBasisMatrix, yvec, &vectors[1]);
        Matrix4::Transform_Vector(BezierSegment::s_bezBasisMatrix, zvec, &vectors[2]);

        m_point = m_segment.m_points[0];

        float *f1;
        float *f2;
        float *f3;
        int idx = 3;

        while (--idx >= 0) {
            const float x = vectors[idx].X;
            const float y = vectors[idx].Y;
            switch (idx) {
                case 0:
                    f1 = &m_add0.x;
                    f2 = &m_add1.x;
                    f3 = &m_add2.x;
                    break;
                case 1:
                    f1 = &m_add0.y;
                    f2 = &m_add1.y;
                    f3 = &m_add2.y;
                    break;
                case 2:
                    f1 = &m_add0.z;
                    f2 = &m_add1.z;
                    f3 = &m_add2.z;
                    break;
            }
            *f1 = x * cub_distance + y * sqr_distance + vectors[idx].Z * distance;
            *f2 = 6.0f * x * cub_distance + 2.0f * y * sqr_distance;
            *f3 = 6.0f * x * cub_distance;
        }
    }
}

bool BezFwdIterator::Done()
{
    return m_index >= m_count;
}

const Coord3D &BezFwdIterator::Get_Current()
{
    return m_point;
}

void BezFwdIterator::Next()
{
    m_point.Add(&m_add0);
    m_add0.Add(&m_add1);
    m_add1.Add(&m_add2);
    ++m_index;
}
