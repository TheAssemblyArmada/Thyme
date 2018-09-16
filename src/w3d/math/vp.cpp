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

void VectorProcessorClass::Prefetch(void *address)
{
    // Nothing in checked binaries, if found fill it in
}

void VectorProcessorClass::TransformNoW(Vector3 *dst, Vector3 const *src, Matrix3D const &mtx, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = mtx.Rotate_Vector(src[i]);
    }
}

void VectorProcessorClass::Transform(Vector3 *dst, Vector3 const *src, Matrix3D const &mtx, int count)
{
    while (count--) {
        dst[count] = mtx * src[count];
    }
}

void VectorProcessorClass::Transform(Vector4 *dst, Vector3 const *src, Matrix4 const &mtx, int count)
{
    while (count--) {
        dst[count] = mtx * src[count];
    }
}

void VectorProcessorClass::Copy(Vector2 *dst, Vector2 const *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(Vector2) * count);
    }
}

void VectorProcessorClass::Copy(unsigned int *dst, unsigned int const *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(unsigned int) * count);
    }
}

void VectorProcessorClass::Copy(Vector3 *dst, Vector3 const *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(Vector3) * count);
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, Vector4 const *src, int count)
{
    if (count > 0) {
        memcpy(dst, src, sizeof(Vector4) * count);
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, Vector3 const *src, float const *srca, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = src[i].X;
        dst[i].Y = src[i].Y;
        dst[i].Z = src[i].Z;
        dst[i].W = srca[i];
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, Vector3 const *src, float srca, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = src[i].X;
        dst[i].Y = src[i].Y;
        dst[i].Z = src[i].Z;
        dst[i].W = srca;
    }
}

void VectorProcessorClass::Copy(Vector4 *dst, Vector3 const &src, float const *srca, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = src.X;
        dst[i].Y = src.Y;
        dst[i].Z = src.Z;
        dst[i].W = srca[i];
    }
}

void VectorProcessorClass::CopyIndexed(unsigned int *dst, unsigned int const *src, unsigned int const *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// i think this is right
void VectorProcessorClass::CopyIndexed(Vector2 *dst, Vector2 const *src, const unsigned int *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// i think this is right
void VectorProcessorClass::CopyIndexed(Vector3 *dst, Vector3 const *src, const unsigned int *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// i think this is right
void VectorProcessorClass::CopyIndexed(Vector4 *dst, Vector4 const *src, unsigned int const *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

void VectorProcessorClass::CopyIndexed(unsigned char *dst, unsigned char *src, unsigned int const *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

void VectorProcessorClass::CopyIndexed(float *dst, float *src, unsigned int const *index, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i] = src[index[i]];
    }
}

// this has a bugfix where it always set minf value only
// dunno what consequences fixing this could cause
void VectorProcessorClass::Clamp(Vector4 *dst, Vector4 const *src, float min, float max, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].X = GameMath::Max(min, src[i].X);
        dst[i].X = GameMath::Min(max, dst[i].X);
        dst[i].Y = GameMath::Max(min, src[i].Y);
        dst[i].Y = GameMath::Min(max, dst[i].Y);
        dst[i].Z = GameMath::Max(min, src[i].Z);
        dst[i].Z = GameMath::Min(max, dst[i].Z);
        dst[i].W = GameMath::Max(min, src[i].W);
        dst[i].W = GameMath::Min(max, dst[i].W);
    }
}

void VectorProcessorClass::Clear(Vector3 *dst, int count)
{
    if (count > 0) {
        memset(dst, 0, sizeof(Vector3) * count);
    }
}

// code confirmish
void VectorProcessorClass::Normalize(Vector3 *dst, int count)
{
    for (int i = 0; i < count; i++) {
        dst[i].Normalize();
    }
}

// this has a bugfix where it always set minf value only
// dunno what consequences fixing this could cause
void VectorProcessorClass::MinMax(Vector3 *src, Vector3 &min, Vector3 &max, int count)
{
    min = *src;
    max = *src;
    for (int i = 1; i < count; i++) {
        min.X = GameMath::Min(min.X, src[i].X);
        min.Y = GameMath::Min(min.Y, src[i].Y);
        min.Z = GameMath::Min(min.Z, src[i].Z);
        max.X = GameMath::Max(max.X, min.X);
        max.Y = GameMath::Max(max.Y, min.Y);
        max.Z = GameMath::Max(max.Z, min.Z);
    }
}

void VectorProcessorClass::MulAdd(float *dest, float multiplier, float add, int count)
{
    for (int i = 0; i < count; i++) {
        dest[i] = (dest[i] * multiplier) + add;
    }
}

void VectorProcessorClass::DotProduct(float *dst, Vector3 const &a, Vector3 const *b, int count)
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
        dst[i] = powf(src[i], pow);
    }
}
