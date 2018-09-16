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
    static void TransformNoW(Vector3 *dst, Vector3 const *src, Matrix3D const &mtx, int count);
    static void Transform(Vector3 *dst, Vector3 const *src, Matrix3D const &mtx, int count);
    static void Transform(Vector4 *dst, Vector3 const *src, Matrix4 const &mtx, int count);
    static void Copy(Vector2 *dst, Vector2 const *src, int count);
    static void Copy(unsigned int *dst, unsigned int const *src, int count);
    static void Copy(Vector3 *dst, Vector3 const *src, int count);
    static void Copy(Vector4 *dst, Vector4 const *src, int count);
    static void Copy(Vector4 *dst, Vector3 const *src, float const *srca, int count);
    static void Copy(Vector4 *dst, Vector3 const *src, float srca, int count);
    static void Copy(Vector4 *dst, Vector3 const &src, float const *srca, int count);
    static void CopyIndexed(unsigned int *dst, unsigned int const *src, unsigned int const *index, int count);
    static void CopyIndexed(Vector2 *dst, Vector2 const *src, unsigned int const *index, int count);
    static void CopyIndexed(Vector3 *dst, Vector3 const *src, unsigned int const *index, int count);
    static void CopyIndexed(Vector4 *dst, Vector4 const *src, unsigned int const *index, int count);
    static void CopyIndexed(unsigned char *dst, unsigned char *src, unsigned int const *index, int count);
    static void CopyIndexed(float *dst, float *src, unsigned int const *index, int count);
    static void Clamp(Vector4 *dst, Vector4 const *src, float min, float max, int count);
    static void Clear(Vector3 *dst, int count);
    static void Normalize(Vector3 *dst, int count);
    static void MinMax(Vector3 *src, Vector3 &min, Vector3 &max, int count);
    static void MulAdd(float *dest, float multiplier, float add, int count);
    static void DotProduct(float *dst, Vector3 const &a, Vector3 const *b, int count);
    static void ClampMin(float *dst, float *src, float min, int count);
    static void Power(float *dst, float *src, float pow, int count);
};
