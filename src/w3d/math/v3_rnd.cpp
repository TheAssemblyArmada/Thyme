/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "v3_rnd.h"
#include "vector2.h"

const float Vector3Randomizer::s_ooIntMax = 4.6566129e-10f;
const float Vector3Randomizer::s_ooUIntMax = 2.3283064e-10f;

Vector3SolidBoxRandomizer::Vector3SolidBoxRandomizer(const Vector3 &extents)
{
    m_extents = Vector3(0, 0, 0);
    m_extents.Update_Max(extents);
}

Vector3SolidBoxRandomizer::Vector3SolidBoxRandomizer(const Vector3SolidBoxRandomizer &that)
{
    m_extents = that.m_extents;
}

Vector3Randomizer *Vector3SolidBoxRandomizer::Clone() const
{
    return new Vector3SolidBoxRandomizer(*this);
}

float Vector3SolidBoxRandomizer::Get_Maximum_Extent()
{
    float f;

    if (m_extents.Y >= m_extents.X) {
        f = m_extents.Y;
    } else {
        f = m_extents.X;
    }

    if (f <= m_extents.Z) {
        f = m_extents.Z;
    }

    return f;
}

void Vector3SolidBoxRandomizer::Scale(float scale)
{
    if (scale <= 0) {
        scale = 0;
    }

    m_extents *= scale;
}

void Vector3SolidBoxRandomizer::Get_Vector(Vector3 &vector)
{
    vector.X = randomizer() * s_ooIntMax * m_extents.X;
    vector.Y = randomizer() * s_ooIntMax * m_extents.Y;
    vector.Z = randomizer() * s_ooIntMax * m_extents.Z;
}

Random3Class Vector3Randomizer::randomizer;

Vector3SolidSphereRandomizer::Vector3SolidSphereRandomizer(float radius)
{
    if (radius <= 0) {
        radius = 0;
    }

    m_radius = radius;
}

Vector3SolidSphereRandomizer::Vector3SolidSphereRandomizer(const Vector3SolidSphereRandomizer &that)
{
    m_radius = that.m_radius;
}

void Vector3SolidSphereRandomizer::Get_Vector(Vector3 &vector)
{
    float rad_squared = m_radius * m_radius;

    do {
        vector.X = randomizer() * s_ooIntMax * m_radius;
        vector.Y = randomizer() * s_ooIntMax * m_radius;
        vector.Z = randomizer() * s_ooIntMax * m_radius;
    } while (rad_squared < vector.Length2());
}

float Vector3SolidSphereRandomizer::Get_Maximum_Extent()
{
    return m_radius;
}

void Vector3SolidSphereRandomizer::Scale(float scale)
{
    if (scale <= 0) {
        scale = 0;
    }

    m_radius *= scale;
}

Vector3Randomizer *Vector3SolidSphereRandomizer::Clone() const
{
    return new Vector3SolidSphereRandomizer(*this);
}

Vector3HollowSphereRandomizer::Vector3HollowSphereRandomizer(float radius)
{
    if (radius <= 0) {
        radius = 0;
    }

    m_radius = radius;
}

Vector3HollowSphereRandomizer::Vector3HollowSphereRandomizer(const Vector3HollowSphereRandomizer &that)
{
    m_radius = that.m_radius;
}

void Vector3HollowSphereRandomizer::Get_Vector(Vector3 &vector)
{
    float v_l2;

    do {
        vector.X = randomizer() * s_ooIntMax;
        vector.Y = randomizer() * s_ooIntMax;
        vector.Z = randomizer() * s_ooIntMax;
        v_l2 = vector.Length2();
    } while (v_l2 > 1.0f || v_l2 <= 0.0f);

    vector *= 1.0f / GameMath::Inv_Sqrt(v_l2) * m_radius;
}

float Vector3HollowSphereRandomizer::Get_Maximum_Extent()
{
    return m_radius;
}

void Vector3HollowSphereRandomizer::Scale(float scale)
{
    if (scale <= 0) {
        scale = 0;
    }

    m_radius *= scale;
}

Vector3Randomizer *Vector3HollowSphereRandomizer::Clone() const
{
    return new Vector3HollowSphereRandomizer(*this);
}

Vector3SolidCylinderRandomizer::Vector3SolidCylinderRandomizer(float extent, float radius)
{
    if (radius <= 0) {
        radius = 0;
    }

    m_radius = radius;

    if (extent <= 0) {
        extent = 0;
    }

    m_extent = extent;
}

Vector3SolidCylinderRandomizer::Vector3SolidCylinderRandomizer(const Vector3SolidCylinderRandomizer &that)
{
    m_radius = that.m_radius;
    m_extent = that.m_extent;
}

void Vector3SolidCylinderRandomizer::Get_Vector(Vector3 &vector)
{
    vector.X = randomizer() * s_ooIntMax * m_extent;
    Vector2 vec2;
    float rad_squared = m_radius * m_radius;

    do {
        vec2.X = randomizer() * s_ooIntMax * m_radius;
        vec2.Y = randomizer() * s_ooIntMax * m_radius;
    } while (rad_squared < vec2.Length2());

    vector.Y = vec2.X;
    vector.Z = vec2.Y;
}

float Vector3SolidCylinderRandomizer::Get_Maximum_Extent()
{
    return m_radius;
}

void Vector3SolidCylinderRandomizer::Scale(float scale)
{
    if (scale <= 0) {
        scale = 0;
    }

    m_radius *= scale;
    m_extent *= scale;
}

Vector3Randomizer *Vector3SolidCylinderRandomizer::Clone() const
{
    return new Vector3SolidCylinderRandomizer(*this);
}
