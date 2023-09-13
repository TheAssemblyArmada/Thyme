/**
 * @file
 *
 * @author xezon
 *
 * @brief Bezier Segment class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "beziersegment.h"

#include "bezfwditerator.h"
#include "vector4.h"

// clang-format off
// Symmetrical matrix. Identical when transposed, thus equally compatible for D3DMATRIX and Matrix4.
const Matrix4 BezierSegment::s_bezBasisMatrix = Matrix4(
    -1.0f,  3.0f, -3.0f, 1.0f,
     3.0f, -6.0f,  3.0f, 0.0f,
    -3.0f,  3.0f,  0.0f, 0.0f,
     1.0f,  0.0f,  0.0f, 0.0f
);
// clang-format on

BezierSegment::BezierSegment()
{
    for (Coord3D &point : m_points) {
        point.Zero();
    }
}

BezierSegment::BezierSegment(float p0x,
    float p0y,
    float p0z,
    float p1x,
    float p1y,
    float p1z,
    float p2x,
    float p2y,
    float p2z,
    float p3x,
    float p3y,
    float p3z)
{
    m_points[0].x = p0x;
    m_points[0].y = p0y;
    m_points[0].z = p0z;

    m_points[1].x = p1x;
    m_points[1].y = p1y;
    m_points[1].z = p1z;

    m_points[2].x = p2x;
    m_points[2].y = p2y;
    m_points[2].z = p2z;

    m_points[3].x = p3x;
    m_points[3].y = p3y;
    m_points[3].z = p3z;
}

BezierSegment::BezierSegment(float xyz_points[12])
{
    m_points[0].x = xyz_points[0];
    m_points[0].y = xyz_points[1];
    m_points[0].z = xyz_points[2];

    m_points[1].x = xyz_points[3];
    m_points[1].y = xyz_points[4];
    m_points[1].z = xyz_points[5];

    m_points[2].x = xyz_points[6];
    m_points[2].y = xyz_points[7];
    m_points[2].z = xyz_points[8];

    m_points[3].x = xyz_points[9];
    m_points[3].y = xyz_points[10];
    m_points[3].z = xyz_points[11];
}

BezierSegment::BezierSegment(const Coord3D &p0, const Coord3D &p1, const Coord3D &p2, const Coord3D &p3)
{
    m_points[0] = p0;
    m_points[1] = p1;
    m_points[2] = p2;
    m_points[3] = p3;
}

BezierSegment::BezierSegment(const Coord3D points[4])
{
    m_points[0] = points[0];
    m_points[1] = points[1];
    m_points[2] = points[2];
    m_points[3] = points[3];
}

void BezierSegment::Evaluate_Bez_Segment_At_T(float t, Coord3D *point)
{
    if (point != nullptr) {
        float sqr = t * t;
        float cub = t * t * t;

        Vector4 vecW(cub, sqr, t, 1.0f);
        Vector4 vecX(m_points[0].x, m_points[1].x, m_points[2].x, m_points[3].x);
        Vector4 vecY(m_points[0].y, m_points[1].y, m_points[2].y, m_points[3].y);
        Vector4 vecZ(m_points[0].z, m_points[1].z, m_points[2].z, m_points[3].z);

        Vector4 vecF;
        Matrix4::Transform_Vector(s_bezBasisMatrix, vecW, &vecF);

        point->x = Vector4::Dot_Product(vecX, vecF);
        point->y = Vector4::Dot_Product(vecY, vecF);
        point->z = Vector4::Dot_Product(vecZ, vecF);
    }
}

void BezierSegment::Get_Segment_Points(int count, std::vector<Coord3D> *points)
{
    if (points != nullptr) {
        points->clear();
        points->resize(count);
        BezFwdIterator it(count, this);
        int k = 0;
        for (it.Start(); !it.Done(); it.Next()) {
            (*points)[k++] = it.Get_Current();
        }
    }
}

float BezierSegment::Get_Approximate_Length(float t)
{
    Coord3D p0p1 = m_points[1] - m_points[0];
    Coord3D p1p2 = m_points[2] - m_points[1];
    Coord3D p2p3 = m_points[3] - m_points[2];
    Coord3D p0p3 = m_points[3] - m_points[0];

    float p0p3_len = p0p3.Length();
    float bez_len = p0p1.Length() + p1p2.Length() + p2p3.Length();

    if (bez_len - p0p3_len > t) {
        BezierSegment half0;
        BezierSegment half1;
        BezierSegment::Split_Segment_At_T(0.5f, half0, half1);
        float half0_len = half0.Get_Approximate_Length(t);
        float half1_len = half1.Get_Approximate_Length(t);
        return half0_len + half1_len;
    }

    return (p0p3_len + bez_len) / 2.0f;
}

void BezierSegment::Split_Segment_At_T(float t, BezierSegment &segment1, BezierSegment &segment2)
{
    Coord3D p0p1 = m_points[1] - m_points[0];
    Coord3D p1p2 = m_points[2] - m_points[1];
    Coord3D p2p3 = m_points[3] - m_points[2];

    p0p1.Scale(t);
    p1p2.Scale(t);
    p2p3.Scale(t);

    p0p1.Add(&m_points[0]);
    p1p2.Add(&m_points[1]);
    p2p3.Add(&m_points[2]);

    Coord3D p01p12 = p1p2 - p0p1;
    Coord3D p12p23 = p2p3 - p1p2;

    p01p12.Scale(t);
    p12p23.Scale(t);
    p01p12.Add(&p0p1);
    p12p23.Add(&p1p2);

    segment1.m_points[0] = m_points[0];
    segment1.m_points[1] = p0p1;
    segment1.m_points[2] = p01p12;
    BezierSegment::Evaluate_Bez_Segment_At_T(t, &segment1.m_points[3]);

    segment2.m_points[0] = segment1.m_points[3];
    segment2.m_points[1] = p12p23;
    segment2.m_points[2] = p2p3;
    segment2.m_points[3] = m_points[3];
}
