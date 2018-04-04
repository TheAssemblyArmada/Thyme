////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: QUATERNION.H
//
//        Author:: Tiberian Technologies
//
//  Contributors:: OmniBlade
//
//   Description:: Quaternion class
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "vector3.h"

class Vector2;
class Vector3;
class Matrix3;
class Matrix3D;
class Matrix4;

// Struct used for cached linear interpolation.
struct SlerpInfoStruct
{
    float sin_theta;
    float theta;
    bool flip;
    bool linear;
};

class Quaternion
{
public:
    Quaternion() {}
    __forceinline explicit Quaternion(bool init) { if ( init ) { X = 0.0f; Y = 0.0f; Z = 0.0f; W = 1.0f; } }
    __forceinline explicit Quaternion(float a, float b, float c, float d) { X = a; Y = b; Z = c; W = d; }
    Quaternion(float angle, const Vector3 &axis);

    void Normalize();
    
    float &operator[](int i) { return (&X)[i]; }
    const float &operator[](int i) const { return (&X)[i]; }

    Quaternion conjugate() { return Quaternion(t, -i, -j, -k); }

    Matrix3 Build_Matrix3();
    Matrix3D Build_Matrix3D();
    Matrix4 Build_Matrix4();

    static Quaternion Trackball(float x0, float y0, float x1, float y1, float sphsize);
    void Scale(float s) { X = (float)(s * X); Y = (float)(s * Y); Z = (float)(s * Z); W = (float)(s * W); }

    friend Quaternion operator*(const Vector3 &a, const Quaternion &b);
    friend Quaternion operator*(const Quaternion &a, const Quaternion &b);

    bool Is_Valid() const;
    __forceinline void Make_Identity() { Set(); }
    __forceinline void Set(float a = 0.0f, float b = 0.0f, float c = 0.0f, float d = 1.0f) { X = a; Y = b; Z = c; W = d; }
    Vector3 Rotate_Vector(const Vector3 &v) const;

public:
    static const Quaternion IDENTITY;

    union
    {
        struct
        {
            float X;
            float Y;
            float Z;
            float W;
        };

        struct
        {
            float i;
            float j;
            float k;
            float t;
        };
    };
};

__forceinline Quaternion operator*(const Vector3 &a, const Quaternion &b)
{
    return Quaternion(a.X, a.Y, a.Z, 0) * b;
}

__forceinline Quaternion operator*(const Quaternion &a, const Quaternion &b)
{
    // Rv = Ar*Bv + Br*Av + Av cross Bv
    // Rr = Ar*Br + Av dot Bv

    return Quaternion(
        a.W * b.X + b.W * a.X + (a.Y * b.Z - a.Z * b.Y),
        a.W * b.Y + b.W * a.Y + (a.Z * b.X - a.X * b.Z),
        a.W * b.Z + b.W * a.Z + (a.X * b.Y - a.Y * b.X),
        a.W * b.W - (a.X * b.X + a.Y * b.Y + a.Z * b.Z)
    );
}

__forceinline bool Quaternion::Is_Valid() const
{
    return (GameMath::Is_Valid_Float(X) &&
        GameMath::Is_Valid_Float(Y) &&
        GameMath::Is_Valid_Float(Z) &&
        GameMath::Is_Valid_Float(W));
}

__forceinline Vector3 Quaternion::Rotate_Vector(const Vector3 &v) const
{
    float x = W * v.X + (Y * v.Z - v.Y * Z);
    float y = W * v.Y - (X * v.Z - v.X * Z);
    float z = W * v.Z + (X * v.Y - v.X * Y);
    float w = -(X * v.X + Y * v.Y + Z * v.Z);

    return Vector3(
        w * (-X) + W * x + (y * (-Z) - (-Y) * z),
        w * (-Y) + W * y - (x * (-Z) - (-X) * z),
        w * (-Z) + W * z + (x * (-Y) - (-X) * y)
    );
}

__forceinline Quaternion Inverse(const Quaternion &a)
{
    return Quaternion(-a[0], -a[1], -a[2], a[3]);
}

__forceinline Quaternion operator/(const Quaternion &a, const Quaternion &b)
{
    return a * Inverse(b);
}

__forceinline Quaternion operator*(float scl, const Quaternion &a)
{
    return Quaternion(scl * a[0], scl * a[1], scl * a[2], scl * a[3]);
}

__forceinline Quaternion operator*(const Quaternion &a, float scl)
{
    return scl * a;
}

Quaternion Build_Quaternion(const Matrix3D &mat);
void Slerp_Setup(const Quaternion &p, const Quaternion &q, SlerpInfoStruct *slerpinfo);
void Cached_Slerp(const Quaternion &p, const Quaternion &q, float alpha, SlerpInfoStruct *slerpinfo, Quaternion * set_q);
Quaternion Cached_Slerp(const Quaternion &p, const Quaternion &q, float alpha, SlerpInfoStruct *slerpinfo);
void Fast_Slerp(Quaternion &result, const Quaternion &a, const Quaternion &b, float t);
Matrix3 Build_Matrix3(const Quaternion &quat);