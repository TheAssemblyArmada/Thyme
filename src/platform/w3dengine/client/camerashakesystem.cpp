/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Class for managing camera shaking.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "camerashakesystem.h"
#include <algorithm>

#ifndef GAME_DLL
CameraShakeSystemClass g_theCameraShakerSystem;
#endif

Vector3 s_axis_rotation(DEG_TO_RADF(7.5f), DEG_TO_RADF(15.0f), DEG_TO_RADF(5.0f));

CameraShakeSystemClass::CameraShakerClass::CameraShakerClass(
    const Vector3 &position, float radius, float duration, float intensity) :
    m_position(position), m_radius(radius), m_duration(duration), m_intensity(intensity), m_time(0)
{
    m_vector1.X = GameMath::Random_Float(78.539818f, 94.24778f);
    m_vector1.Y = GameMath::Random_Float(78.539818f, 94.24778f);
    m_vector1.Z = GameMath::Random_Float(78.539818f, 94.24778f);

    m_vector2.X = GameMath::Random_Float(0.0f, GAMEMATH_PI2);
    m_vector2.Y = GameMath::Random_Float(0.0f, GAMEMATH_PI2);
    m_vector2.Z = GameMath::Random_Float(0.0f, GAMEMATH_PI2);
}

void CameraShakeSystemClass::CameraShakerClass::Compute_Rotations(const Vector3 &position, Vector3 *set_angles)
{
    captainslog_assert(set_angles != nullptr);

    Vector3 pos_delta = position - m_position;

    float len2 = pos_delta.Length2();

    if (m_radius * m_radius >= len2) {

        float angle = (1.0f - GameMath::Sqrt(len2) / m_radius) * m_intensity * (1.0f - m_time / m_duration);

        for (int i = 0; i < 3; ++i) {

            float f2 = (GAMEMATH_PI2 - m_vector1[i]) * m_time + m_vector1[i];
            float f3 = angle * s_axis_rotation[i];
            float f4 = (f2 * m_time) + m_vector2[i];

            (*set_angles)[i] += GameMath::Sin(f4) * f3;

            Vector3 angles;

            float angle_range = angle / 2.0f;

            angles.X = GameMath::Random_Float(-angle_range, angle_range);
            angles.Y = GameMath::Random_Float(-angle_range, angle_range);
            angles.Z = GameMath::Random_Float(-angle_range, angle_range);

            *set_angles += angles;
        }
    }
}

CameraShakeSystemClass::~CameraShakeSystemClass()
{
    while (!m_shakers.Is_Empty()) {
        CameraShakerClass *shaker = m_shakers.Remove_Head();
        m_shakers.Remove(shaker);
        delete shaker;
    }
}

bool CameraShakeSystemClass::Add_Camera_Shake(const Vector3 &position, float radius, float duration, float amplitude)
{
    float intensity = DEG_TO_RADF(amplitude);

    return m_shakers.Add(new CameraShakerClass(position, radius, duration, intensity));
}

bool CameraShakeSystemClass::Is_Camera_Shaking() const
{
    MultiListIterator<CameraShakerClass> it(&m_shakers);

    for (; !it.Is_Done(); it.Next()) {
        if (it.Peek_Obj() != nullptr) {
            return true;
        }
    }

    return false;
}

void CameraShakeSystemClass::Timestep(float delta)
{
    MultiListClass<CameraShakerClass> expired;
    MultiListIterator<CameraShakerClass> it(&m_shakers);

    for (; !it.Is_Done(); it.Next()) {
        CameraShakerClass *shaker = it.Get_Obj();

        shaker->Timestep(delta);

        if (shaker->Is_Expired()) {
            expired.Add(shaker);
        }
    }

    while (!expired.Is_Empty()) {
        CameraShakerClass *shaker = expired.Remove_Head();
        m_shakers.Remove(shaker);
        delete shaker;
    }
}

void CameraShakeSystemClass::Update_Camera_Shaker(Vector3 position, Vector3 *angles)
{
    MultiListIterator<CameraShakerClass> it(&m_shakers);
    Vector3 set_angles(0, 0, 0);

    for (; !it.Is_Done(); it.Next()) {
        CameraShakerClass *shaker = it.Get_Obj();
        shaker->Compute_Rotations(position, &set_angles);
    }

    for (int i = 0; i < 3; ++i) {
        // TODO BUG!!
        // this does nothing, investigate
        // perhaps was meant to be set_angles[i] = std::clamp
        std::clamp(set_angles[i], -s_axis_rotation[i], s_axis_rotation[i]);
    }

    *angles = set_angles;
}
