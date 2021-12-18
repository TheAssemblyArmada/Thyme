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
#include "vp.h"
#include "gamemath.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include <algorithm>
#include <cstring>

using std::memcpy;
using std::memset;

void VectorProcessorClass::Prefetch(void *address)
{
    // Nothing in checked binaries, if found fill it in
}

void VectorProcessorClass::TransformNoW(Vector3 *dst, const Vector3 *src, const Matrix3D &mtx, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = mtx.Rotate_Vector(src[i]);
    }
}

void VectorProcessorClass::Transform(Vector3 *dst, const Vector3 *src, const Matrix3D &mtx, int count)
{
    while (count--) {
        dst[count] = mtx * src[count];
    }
}

void VectorProcessorClass::Transform(Vector4 *dst, const Vector3 *src, const Matrix4 &mtx, int count)
{
    while (count--) {
        dst[count] = mtx * src[count];
    }
}

void VectorProcessorClass::Copy(Vector2 *dst, const Vector2 *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(Vector2) * count);
    }
}

void VectorProcessorClass::Copy(unsigned *dst, const unsigned *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(unsigned) * count);
    }
}

void VectorProcessorClass::Copy(Vector3 *dst, const Vector3 *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(Vector3) * count);
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, const Vector4 *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(Vector4) * count);
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, const Vector3 *src, const float *srca, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = src[i].X;
        dst[i].Y = src[i].Y;
        dst[i].Z = src[i].Z;
        dst[i].W = srca[i];
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, const Vector3 *src, float srca, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = src[i].X;
        dst[i].Y = src[i].Y;
        dst[i].Z = src[i].Z;
        dst[i].W = srca;
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, const Vector3 &src, const float *srca, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = src.X;
        dst[i].Y = src.Y;
        dst[i].Z = src.Z;
        dst[i].W = srca[i];
    }
}

void VectorProcessorClass::CopyIndexed(unsigned *dst, const unsigned *src, const unsigned *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// i think this is right
void VectorProcessorClass::CopyIndexed(Vector2 *dst, const Vector2 *src, const unsigned *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// i think this is right
void VectorProcessorClass::CopyIndexed(Vector3 *dst, const Vector3 *src, const unsigned *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// i think this is right
void VectorProcessorClass::CopyIndexed(Vector4 *dst, const Vector4 *src, const unsigned *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

void VectorProcessorClass::CopyIndexed(unsigned char *dst, unsigned char *src, const unsigned *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

void VectorProcessorClass::CopyIndexed(float *dst, float *src, const unsigned *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

void VectorProcessorClass::Clamp(Vector4 *dst, const Vector4 *src, float min, float max, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = std::clamp(src[i].X, min, max);
        dst[i].Y = std::clamp(src[i].Y, min, max);
        dst[i].Z = std::clamp(src[i].Z, min, max);
        dst[i].W = std::clamp(src[i].W, min, max);
    }
}

void VectorProcessorClass::Clear(Vector3 *dst, int count)
{
    if (count > 0) {
        memset(dst, 0, sizeof(Vector3) * count);
    }
}

void VectorProcessorClass::Normalize(Vector3 *dst, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].Normalize();
    }
}

// This has a bugfix where it always set minf value only dunno what consequences fixing this could cause
void VectorProcessorClass::MinMax(Vector3 *src, Vector3 &min, Vector3 &max, int count)
{
    if (count > 0) {
        min = src[0];
        max = src[0];

        for (int i = 1; i < count; ++i) {
            min.Update_Min(src[i]);
            max.Update_Max(src[i]);
        }
    }
}

void VectorProcessorClass::MulAdd(float *dest, float multiplier, float add, int count)
{
    for (int i = 0; i < count; i++) {
        dest[i] = (dest[i] * multiplier) + add;
    }
}

void VectorProcessorClass::DotProduct(float *dst, const Vector3 &a, const Vector3 *b, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = a * b[i];
    }
}

void VectorProcessorClass::ClampMin(float *dst, float *src, float min, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = GameMath::Max(src[i], min);
    }
}

void VectorProcessorClass::Power(float *dst, float *src, float pow, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = GameMath::Pow(src[i], pow);
    }
}
