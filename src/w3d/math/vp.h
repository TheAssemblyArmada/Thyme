/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Convenience functions to perform operations on arrays of vectors.
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

class Vector2;
class Vector3;
class Vector4;
class Matrix3D;
class Matrix4;

class VectorProcessorClass
{
public:
    static void Prefetch(void *address);
    static void TransformNoW(Vector3 *dst, const Vector3 *src, const Matrix3D &mtx, int count);
    static void Transform(Vector3 *dst, const Vector3 *src, const Matrix3D &mtx, int count);
    static void Transform(Vector4 *dst, const Vector3 *src, const Matrix4 &mtx, int count);
    static void Copy(Vector2 *dst, const Vector2 *src, int count);
    static void Copy(unsigned *dst, const unsigned *src, int count);
    static void Copy(Vector3 *dst, const Vector3 *src, int count);
    static void Copy(Vector4 *dst, const Vector4 *src, int count);
    static void Copy(Vector4 *dst, const Vector3 *src, const float *srca, int count);
    static void Copy(Vector4 *dst, const Vector3 *src, float srca, int count);
    static void Copy(Vector4 *dst, const Vector3 &src, const float *srca, int count);
    static void CopyIndexed(unsigned *dst, const unsigned *src, const unsigned *index, int count);
    static void CopyIndexed(Vector2 *dst, const Vector2 *src, const unsigned *index, int count);
    static void CopyIndexed(Vector3 *dst, const Vector3 *src, const unsigned *index, int count);
    static void CopyIndexed(Vector4 *dst, const Vector4 *src, const unsigned *index, int count);
    static void CopyIndexed(unsigned char *dst, unsigned char *src, const unsigned *index, int count);
    static void CopyIndexed(float *dst, float *src, const unsigned *index, int count);
    static void Clamp(Vector4 *dst, const Vector4 *src, float min, float max, int count);
    static void Clear(Vector3 *dst, int count);
    static void Normalize(Vector3 *dst, int count);
    static void MinMax(Vector3 *src, Vector3 &min, Vector3 &max, int count);
    static void MulAdd(float *dest, float multiplier, float add, int count);
    static void DotProduct(float *dst, const Vector3 &a, const Vector3 *b, int count);
    static void ClampMin(float *dst, float *src, float min, int count);
    static void Power(float *dst, float *src, float pow, int count);
};
