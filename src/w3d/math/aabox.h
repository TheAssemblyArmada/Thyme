/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Axis-Aligned bounding box
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
#include "lineseg.h"

class MinMaxAABoxClass;

class AABoxClass
{
public:
    AABoxClass() {}
    AABoxClass(const Vector3 &center, const Vector3 &extent) : m_center(center), m_extent(extent) {}
    AABoxClass(const MinMaxAABoxClass &minmaxbox) { Init(minmaxbox); }
    AABoxClass(Vector3 *points, int num) { Init(points, num); }
    bool operator==(const AABoxClass &src) { return (m_center == src.m_center) && (m_extent == src.m_extent); }
    bool operator!=(const AABoxClass &src) { return (m_center != src.m_center) || (m_extent != src.m_extent); }

    void Init(const Vector3 &center, const Vector3 &extent)
    {
        m_center = center;
        m_extent = extent;
    }

    void Init(Vector3 *points, int num)
    {
        Vector3 Min = points[0];
        Vector3 Max = points[0];

        for (int i = 1; i < num; i++) {
            if (Min.X > points[i].X) {
                Min.X = points[i].X;
            }

            if (Min.Y > points[i].Y) {
                Min.Y = points[i].Y;
            }

            if (Min.Z > points[i].Z) {
                Min.Z = points[i].Z;
            }

            if (Max.X < points[i].X) {
                Max.X = points[i].X;
            }

            if (Max.Y < points[i].Y) {
                Max.Y = points[i].Y;
            }

            if (Max.Z < points[i].Z) {
                Max.Z = points[i].Z;
            }
        }

        m_center = (Max + Min) * 0.5f;
        m_extent = (Max - Min) * 0.5f;
    }

    void Init(const MinMaxAABoxClass &minmaxbox);

    void Init(const LineSegClass &line)
    {
        Vector3 min_corner = line.Get_P0();
        Vector3 max_corner = line.Get_P0();

        if (min_corner.X > line.Get_P1().X) {
            min_corner.X = line.Get_P1().X;
        }

        if (min_corner.Y > line.Get_P1().Y) {
            min_corner.Y = line.Get_P1().Y;
        }

        if (min_corner.Z > line.Get_P1().Z) {
            min_corner.Z = line.Get_P1().Z;
        }

        if (max_corner.X < line.Get_P1().X) {
            max_corner.X = line.Get_P1().X;
        }

        if (max_corner.Y < line.Get_P1().Y) {
            max_corner.Y = line.Get_P1().Y;
        }

        if (max_corner.Z < line.Get_P1().Z) {
            max_corner.Z = line.Get_P1().Z;
        }

        m_center = (max_corner + min_corner) * 0.5f;
        m_extent = (max_corner - min_corner) * 0.5f;
    }

    void Init_Min_Max(const Vector3 &min, const Vector3 &max)
    {
        m_center = (max + min) * 0.5f;
        m_extent = (max - min) * 0.5f;
    }

    void Init_Random(float min_center = -1.0f, float max_center = 1.0f, float min_extent = 0.5f, float max_extent = 1.0f);

    void Add_Point(const Vector3 &point)
    {
        Vector3 Min = m_center - m_extent;
        Vector3 Max = m_center + m_extent;

        if (Min.X > point.X) {
            Min.X = point.X;
        }

        if (Min.Y > point.Y) {
            Min.Y = point.Y;
        }

        if (Min.Z > point.Z) {
            Min.Z = point.Z;
        }

        if (Max.X < point.X) {
            Max.X = point.X;
        }

        if (Max.Y < point.Y) {
            Max.Y = point.Y;
        }

        if (Max.Z < point.Z) {
            Max.Z = point.Z;
        }

        m_center = (Max + Min) / 2.0f;
        m_extent = (Max - Min) / 2.0f;
    }

    void Add_Box(const AABoxClass &b)
    {
        Vector3 newmin = m_center - m_extent;
        Vector3 newmax = m_center + m_extent;
        newmin.Update_Min(b.m_center - b.m_extent);
        newmax.Update_Max(b.m_center + b.m_extent);
        m_center = (newmax + newmin) * 0.5f;
        m_extent = (newmax - newmin) * 0.5f;
    }

    void Add_Box(const MinMaxAABoxClass &b);

    float Project_To_Axis(const Vector3 &axis) const
    {
        float x = m_extent[0] * axis[0];
        float y = m_extent[1] * axis[1];
        float z = m_extent[2] * axis[2];
        return (GameMath::Fabs(x) + GameMath::Fabs(y) + GameMath::Fabs(z));
    }

    void Transform(const Matrix3D &tm)
    {
        Vector3 oldcenter = m_center;
        Vector3 oldextent = m_extent;
        tm.Transform_Center_Extent_AABox(oldcenter, oldextent, &m_center, &m_extent);
    }

    void Translate(const Vector3 &trans) { m_center += trans; }
    float Volume() const { return 2.0 * m_extent.X * 2.0 * m_extent.Y * 2.0 * m_extent.Z; }
    bool Contains(const Vector3 &point) const; // needs CollisionMath
    bool Contains(const AABoxClass &other_box) const; // needs CollisionMath
    bool Contains(const MinMaxAABoxClass &other_box) const; // needs CollisionMath
    static void Transform(const Matrix3D &tm, const AABoxClass &in, AABoxClass *out);
    Vector3 m_center;
    Vector3 m_extent;
};

class MinMaxAABoxClass
{
public:
    MinMaxAABoxClass() {}
    MinMaxAABoxClass(const Vector3 &min_corner, const Vector3 &max_corner) : m_minCorner(min_corner), m_maxCorner(max_corner)
    {
    }
    MinMaxAABoxClass(Vector3 *points, int num) { Init(points, num); }
    MinMaxAABoxClass(const AABoxClass &that) { Init(that); }

    void Init(Vector3 *points, int num)
    {
        m_minCorner = points[0];
        m_maxCorner = points[0];

        for (int i = 0; i < num; i++) {
            m_minCorner.Update_Min(points[i]);
            m_maxCorner.Update_Max(points[i]);
        }
    }

    void Init(const AABoxClass &box)
    {
        m_minCorner = box.m_center - box.m_extent;
        m_maxCorner = box.m_center + box.m_extent;
    }

    void Init_Empty();

    void Add_Point(const Vector3 &point)
    {
        m_minCorner.Update_Min(point);
        m_maxCorner.Update_Max(point);
    }

    void Add_Box(const MinMaxAABoxClass &box)
    {
        if (box.m_minCorner == box.m_maxCorner) {
            return;
        }

        m_minCorner.Update_Min(box.m_minCorner);
        m_maxCorner.Update_Max(box.m_maxCorner);
    }

    void Add_Box(const AABoxClass &box)
    {
        if (box.m_extent == Vector3(0.0f, 0.0f, 0.0f)) {
            return;
        }

        m_minCorner.Update_Min(box.m_center - box.m_extent);
        m_maxCorner.Update_Max(box.m_center + box.m_extent);
    }

    void Add_Box(const Vector3 &min_corner, const Vector3 &max_corner)
    {
        if (min_corner == max_corner) {
            return;
        }

        m_minCorner.Update_Min(min_corner);
        m_maxCorner.Update_Max(max_corner);
    }

    void Transform(const Matrix3D &tm)
    {
        Vector3 oldmin = m_minCorner;
        Vector3 oldmax = m_maxCorner;
        tm.Transform_Min_Max_AABox(oldmin, oldmax, &m_minCorner, &m_maxCorner);
    }

    void Translate(const Vector3 &pos)
    {
        m_minCorner += pos;
        m_maxCorner += pos;
    }

    float Volume() const
    {
        Vector3 size = m_maxCorner - m_minCorner;
        return size.X * size.Y * size.Z;
    }

    Vector3 m_minCorner;
    Vector3 m_maxCorner;
};

inline void AABoxClass::Init(const MinMaxAABoxClass &mmbox)
{
    m_center = (mmbox.m_maxCorner + mmbox.m_minCorner) * 0.5f;
    m_extent = (mmbox.m_maxCorner - mmbox.m_minCorner) * 0.5f;
}

inline void AABoxClass::Add_Box(const MinMaxAABoxClass &b)
{
    Vector3 newmin = m_center - m_extent;
    Vector3 newmax = m_center + m_extent;
    newmin.Update_Min(b.m_minCorner);
    newmax.Update_Max(b.m_maxCorner);
    m_center = (newmax + newmin) * 0.5f;
    m_extent = (newmax - newmin) * 0.5f;
}
