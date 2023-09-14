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
#pragma once
#include "always.h"
#include "camera.h"
#include "multilist.h"
#include "vector3.h"
#include <new>

class CameraShakeSystemClass
{
public:
    class CameraShakerClass : public MultiListObjectClass, public AutoPoolClass<CameraShakerClass, 256>
    {
    public:
        CameraShakerClass(const Vector3 &position, float radius, float duration, float intensity);
        ~CameraShakerClass() {}

        void Compute_Rotations(const Vector3 &position, Vector3 *set_angles);

        void Timestep(float delta) { m_time += delta; }
        bool Is_Expired() const { return m_time >= m_duration; }

#ifdef GAME_DLL
        CameraShakerClass *Hook_Ctor(const Vector3 &position, float radius, float duration, float intensity)
        {
            return ::new (this) CameraShakerClass(position, radius, duration, intensity);
        }
        void Hook_Dtor() { CameraShakerClass::~CameraShakerClass(); }
#endif

    private:
        Vector3 m_position;
        float m_radius;
        float m_duration;
        float m_intensity;
        float m_time;
        Vector3 m_vector1;
        Vector3 m_vector2;
    };

public:
    CameraShakeSystemClass() {}
    ~CameraShakeSystemClass();

    bool Add_Camera_Shake(const Vector3 &position, float radius, float duration, float amplitude);

    bool Is_Camera_Shaking() const;

    void Timestep(float delta);

    void Update_Camera_Shaker(Vector3 position, Vector3 *angles);

#ifdef GAME_DLL
    CameraShakeSystemClass *Hook_Ctor() { return new (this) CameraShakeSystemClass(); }
    void Hook_Dtor() { CameraShakeSystemClass::~CameraShakeSystemClass(); }
#endif

private:
    MultiListClass<CameraShakerClass> m_shakers;
};

#ifdef GAME_DLL
extern CameraShakeSystemClass &g_theCameraShakerSystem;
#else
extern CameraShakeSystemClass g_theCameraShakerSystem;
#endif
