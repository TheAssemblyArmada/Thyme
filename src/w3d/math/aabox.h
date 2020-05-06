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
    AABoxClass(const Vector3 &center, const Vector3 &extent) : Center(center), Extent(extent) {}
    AABoxClass(const MinMaxAABoxClass &minmaxbox) { Init(minmaxbox); }
    AABoxClass(Vector3 *points, int num) { Init(points, num); }
    bool operator==(const AABoxClass &src) { return (Center == src.Center) && (Extent == src.Extent); }
    bool operator!=(const AABoxClass &src) { return (Center != src.Center) || (Extent != src.Extent); }

    void Init(const Vector3 &center, const Vector3 &extent)
    {
        Center = center;
        Extent = extent;
    }

    void Init(Vector3 *points, int num)
    {
        Vector3 Min = points[0];
        Vector3 Max = points[0];

        for (int i = 1; i < num; i++) {
            if (Min.X > points[i].X)
                Min.X = points[i].X;

            if (Min.Y > points[i].Y)
                Min.Y = points[i].Y;

            if (Min.Z > points[i].Z)
                Min.Z = points[i].Z;

            if (Max.X < points[i].X)
                Max.X = points[i].X;

            if (Max.Y < points[i].Y)
                Max.Y = points[i].Y;

            if (Max.Z < points[i].Z)
                Max.Z = points[i].Z;
        }

        Center = (Max + Min) * 0.5f;
        Extent = (Max - Min) * 0.5f;
    }

    void Init(const MinMaxAABoxClass &minmaxbox);

    void Init(const LineSegClass &line)
    {
        Vector3 min_corner = line.Get_P0();
        Vector3 max_corner = line.Get_P0();

        if (min_corner.X > line.Get_P1().X)
            min_corner.X = line.Get_P1().X;

        if (min_corner.Y > line.Get_P1().Y)
            min_corner.Y = line.Get_P1().Y;

        if (min_corner.Z > line.Get_P1().Z)
            min_corner.Z = line.Get_P1().Z;

        if (max_corner.X < line.Get_P1().X)
            max_corner.X = line.Get_P1().X;

        if (max_corner.Y < line.Get_P1().Y)
            max_corner.Y = line.Get_P1().Y;

        if (max_corner.Z < line.Get_P1().Z)
            max_corner.Z = line.Get_P1().Z;

        Center = (max_corner + min_corner) * 0.5f;
        Extent = (max_corner - min_corner) * 0.5f;
    }

    void Init_Min_Max(const Vector3 &min, const Vector3 &max)
    {
        Center = (max + min) * 0.5f;
        Extent = (max - min) * 0.5f;
    }

    void Init_Random(float min_center = -1.0f, float max_center = 1.0f, float min_extent = 0.5f, float max_extent = 1.0f);

    void Add_Point(const Vector3 &point)
    {
        Vector3 Min = Center - Extent;
        Vector3 Max = Center + Extent;

        if (Min.X > point.X)
            Min.X = point.X;

        if (Min.Y > point.Y)
            Min.Y = point.Y;

        if (Min.Z > point.Z)
            Min.Z = point.Z;

        if (Max.X < point.X)
            Max.X = point.X;

        if (Max.Y < point.Y)
            Max.Y = point.Y;

        if (Max.Z < point.Z)
            Max.Z = point.Z;

        Center = (Max + Min) / 2.0f;
        Extent = (Max - Min) / 2.0f;
    }

    void Add_Box(const AABoxClass &b)
    {
        Vector3 newmin = Center - Extent;
        Vector3 newmax = Center + Extent;
        newmin.Update_Min(b.Center - b.Extent);
        newmax.Update_Max(b.Center + b.Extent);
        Center = (newmax + newmin) * 0.5f;
        Extent = (newmax - newmin) * 0.5f;
    }

    void Add_Box(const MinMaxAABoxClass &b);

    float Project_To_Axis(const Vector3 &axis) const
    {
        float x = Extent[0] * axis[0];
        float y = Extent[1] * axis[1];
        float z = Extent[2] * axis[2];
        return (GameMath::Fabs(x) + GameMath::Fabs(y) + GameMath::Fabs(z));
    }

    void Transform(const Matrix3D &tm)
    {
        Vector3 oldcenter = Center;
        Vector3 oldextent = Extent;
        tm.Transform_Center_Extent_AABox(oldcenter, oldextent, &Center, &Extent);
    }

    void Translate(const Vector3 &trans) { Center += trans; }
    float Volume() const { return 2.0 * Extent.X * 2.0 * Extent.Y * 2.0 * Extent.Z; }
    bool Contains(const Vector3 &point) const;
    bool Contains(const AABoxClass &other_box) const;
    bool Contains(const MinMaxAABoxClass &other_box) const;
    static void Transform(const Matrix3D &tm, const AABoxClass &in, AABoxClass *out);
    Vector3 Center;
    Vector3 Extent;
};

class MinMaxAABoxClass
{
public:
    MinMaxAABoxClass() {}
    MinMaxAABoxClass(const Vector3 &min_corner, const Vector3 &max_corner) : MinCorner(min_corner), MaxCorner(max_corner) {}
    MinMaxAABoxClass(Vector3 *points, int num) { Init(points, num); }
    MinMaxAABoxClass(const AABoxClass &that) { Init(that); }

    void Init(Vector3 *points, int num)
    {
        MinCorner = points[0];
        MaxCorner = points[0];

        for (int i = 0; i < num; i++) {
            MinCorner.Update_Min(points[i]);
            MaxCorner.Update_Max(points[i]);
        }
    }

    void Init(const AABoxClass &box)
    {
        MinCorner = box.Center - box.Extent;
        MaxCorner = box.Center + box.Extent;
    }

    void Init_Empty();

    void Add_Point(const Vector3 &point)
    {
        MinCorner.Update_Min(point);
        MaxCorner.Update_Max(point);
    }

    void Add_Box(const MinMaxAABoxClass &box)
    {
        if (box.MinCorner == box.MaxCorner)
            return;

        MinCorner.Update_Min(box.MinCorner);
        MaxCorner.Update_Max(box.MaxCorner);
    }

    void Add_Box(const AABoxClass &box)
    {
        if (box.Extent == Vector3(0.0f, 0.0f, 0.0f))
            return;

        MinCorner.Update_Min(box.Center - box.Extent);
        MaxCorner.Update_Max(box.Center + box.Extent);
    }

    void Add_Box(const Vector3 &min_corner, const Vector3 &max_corner)
    {
        if (min_corner == max_corner)
            return;

        MinCorner.Update_Min(min_corner);
        MaxCorner.Update_Max(max_corner);
    }

    void Transform(const Matrix3D &tm)
    {
        Vector3 oldmin = MinCorner;
        Vector3 oldmax = MaxCorner;
        tm.Transform_Min_Max_AABox(oldmin, oldmax, &MinCorner, &MaxCorner);
    }

    void Translate(const Vector3 &pos)
    {
        MinCorner += pos;
        MaxCorner += pos;
    }

    float Volume() const
    {
        Vector3 size = MaxCorner - MinCorner;
        return size.X * size.Y * size.Z;
    }

    Vector3 MinCorner;
    Vector3 MaxCorner;
};

void AABoxClass::Init(const MinMaxAABoxClass &mmbox)
{
    Center = (mmbox.MaxCorner + mmbox.MinCorner) * 0.5f;
    Extent = (mmbox.MaxCorner - mmbox.MinCorner) * 0.5f;
}

void AABoxClass::Add_Box(const MinMaxAABoxClass &b)
{
    Vector3 newmin = Center - Extent;
    Vector3 newmax = Center + Extent;
    newmin.Update_Min(b.MinCorner);
    newmax.Update_Max(b.MaxCorner);
    Center = (newmax + newmin) * 0.5f;
    Extent = (newmax - newmin) * 0.5f;
}
