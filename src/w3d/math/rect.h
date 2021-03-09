/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief Rectangle object class.
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
#include "vector2.h"
#include <algorithm>

class RectClass
{
public:
    RectClass(float new_left, float new_top, float new_right, float new_bottom);
    RectClass() {}
    RectClass(const RectClass &r);
    RectClass(const Vector2 &top_left, const Vector2 &bottom_right);

    RectClass &operator=(const RectClass &r);
    RectClass &operator*=(float k) { return Scale(k); }
    RectClass &operator/=(float k) { return Scale(1 / k); }

    RectClass &operator+=(const Vector2 &o)
    {
        left += o.X;
        top += o.Y;
        right += o.X;
        bottom += o.Y;
        return *this;
    }

    RectClass &operator-=(const Vector2 &o)
    {
        left -= o.X;
        top -= o.Y;
        right -= o.X;
        bottom -= o.Y;
        return *this;
    }

    RectClass &operator+=(const RectClass &r)
    {
        left = std::min(left, r.left);
        top = std::min(top, r.top);
        right = std::max(right, r.right);
        bottom = std::max(bottom, r.bottom);
        return *this;
    }

    bool operator==(const RectClass &rval) const
    {
        return (rval.left == left) && (rval.right == right) && (rval.top == top) && (rval.bottom == bottom);
    }

    bool operator!=(const RectClass &rval) const
    {
        return (rval.left != left) || (rval.right != right) || (rval.top != top) || (rval.bottom != bottom);
    }

    void Set(float _left, float _top, float _right, float _bottom)
    {
        left = _left;
        top = _top;
        right = _right;
        bottom = _bottom;
    }

    void Set(const Vector2 &top_left, const Vector2 &bottom_right)
    {
        left = top_left.X;
        top = top_left.Y;
        right = bottom_right.X;
        bottom = bottom_right.Y;
    }

    void Set(const RectClass &r)
    {
        left = r.left;
        top = r.top;
        right = r.right;
        bottom = r.bottom;
    }

    Vector2 Upper_Right() const { return Vector2(right, top); }
    Vector2 Lower_Left() const { return Vector2(left, bottom); }
    float Width() const { return right - left; }
    float Height() const { return bottom - top; }
    Vector2 Center() const { return Vector2((left + right) / 2, (top + bottom) / 2); }
    Vector2 Extent() const { return Vector2((right - left) / 2, (bottom - top) / 2); }

    RectClass &Scale_Relative_Center(float k)
    {
        Vector2 center = Center();
        *this -= center;
        left *= k;
        top *= k;
        right *= k;
        bottom *= k;
        *this += center;
        return *this;
    }

    RectClass &Scale(float k)
    {
        left *= k;
        top *= k;
        right *= k;
        bottom *= k;
        return *this;
    }

    RectClass &Scale(const Vector2 &k)
    {
        left *= k.X;
        top *= k.Y;
        right *= k.X;
        bottom *= k.Y;
        return *this;
    }

    RectClass &Inverse_Scale(const Vector2 &k)
    {
        left /= k.X;
        top /= k.Y;
        right /= k.X;
        bottom /= k.Y;
        return *this;
    }

    void Inflate(const Vector2 &o)
    {
        left -= o.X;
        top -= o.Y;
        right += o.X;
        bottom += o.Y;
    }

    Vector2 Upper_Left() const { return Vector2(left, top); }
    Vector2 Lower_Right() const { return Vector2(right, bottom); }

    bool Contains(const Vector2 &pos) const
    {
        return (pos.X >= left) && (pos.X <= right) && (pos.Y >= top) && (pos.Y <= bottom);
    }

    void Snap_To_Units(const Vector2 &u)
    {
        left = (int)(left / u.X + 0.5f) * u.X;
        right = (int)(right / u.X + 0.5f) * u.X;
        top = (int)(top / u.Y + 0.5f) * u.Y;
        bottom = (int)(bottom / u.Y + 0.5f) * u.Y;
    }

    RectClass Intersect(const RectClass &other) const
    {
        return RectClass(std::max(left, other.left),
            std::max(top, other.top),
            std::min(right, other.right),
            std::min(bottom, other.bottom));
    }

public:
    float left;
    float top;
    float right;
    float bottom;
};

inline RectClass::RectClass(float new_left, float new_top, float new_right, float new_bottom)
{
    left = new_left;
    top = new_top;
    right = new_right;
    bottom = new_bottom;
}

inline RectClass::RectClass(const RectClass &r)
{
    left = r.left;
    top = r.top;
    right = r.right;
    bottom = r.bottom;
}

inline RectClass::RectClass(const Vector2 &top_left, const Vector2 &bottom_right)
{
    left = top_left.X;
    top = top_left.Y;
    right = bottom_right.X;
    bottom = bottom_right.Y;
}

inline RectClass &RectClass::operator=(const RectClass &r)
{
    left = r.left;
    top = r.top;
    right = r.right;
    bottom = r.bottom;
    return *this;
}
