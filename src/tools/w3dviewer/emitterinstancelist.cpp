/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter instance list
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterinstancelist.h"

#define KEYFRAME_EPSILON 0.000001f

EmitterInstanceList::~EmitterInstanceList()
{
    Clear_Emitters();
}

void EmitterInstanceList::Set_Velocity(const Vector3 &value)
{
    ParticleEmitterDefClass::Set_Velocity(value);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Set_Base_Velocity(value);
    }
}

void EmitterInstanceList::Set_Acceleration(const Vector3 &value)
{
    ParticleEmitterDefClass::Set_Acceleration(value);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Set_Acceleration(value);
    }
}

void EmitterInstanceList::Set_Burst_Size(unsigned int size)
{
    ParticleEmitterDefClass::Set_Burst_Size(size);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Set_Burst_Size(size);
    }
}

void EmitterInstanceList::Set_Outward_Vel(float value)
{
    ParticleEmitterDefClass::Set_Outward_Vel(value);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Set_Outwards_Velocity(value);
    }
}

void EmitterInstanceList::Set_Vel_Inherit(float value)
{
    ParticleEmitterDefClass::Set_Vel_Inherit(value);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Set_Velocity_Inheritance_Factor(value);
    }
}

void EmitterInstanceList::Set_Velocity_Random(Vector3Randomizer *randomizer)
{
    ParticleEmitterDefClass::Set_Velocity_Random(randomizer);

    if (randomizer != nullptr) {
        for (int i = 0; i < m_emitters.Count(); i++) {
            m_emitters[i]->Set_Velocity_Randomizer(randomizer->Clone());
        }
    }
}

void EmitterInstanceList::Get_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes) const
{
    ParticleEmitterDefClass::Get_Color_Keyframes(keyframes);

    for (unsigned int i = 0; i < keyframes.NumKeyFrames; i++) {
        if (keyframes.Values[i].X < KEYFRAME_EPSILON) {
            keyframes.Values[i].X = 0.0f;
        }

        if (keyframes.Values[i].Y < KEYFRAME_EPSILON) {
            keyframes.Values[i].Y = 0.0f;
        }

        if (keyframes.Values[i].Z < KEYFRAME_EPSILON) {
            keyframes.Values[i].Z = 0.0f;
        }
    }
}

void EmitterInstanceList::Get_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes) const
{
    ParticleEmitterDefClass::Get_Opacity_Keyframes(keyframes);

    for (unsigned int i = 0; i < keyframes.NumKeyFrames; i++) {
        if (keyframes.Values[i] < KEYFRAME_EPSILON) {
            keyframes.Values[i] = 0.0f;
        }
    }
}

void EmitterInstanceList::Get_Size_Keyframes(ParticlePropertyStruct<float> &keyframes) const
{
    ParticleEmitterDefClass::Get_Size_Keyframes(keyframes);

    for (unsigned int i = 0; i < keyframes.NumKeyFrames; i++) {
        if (keyframes.Values[i] < KEYFRAME_EPSILON) {
            keyframes.Values[i] = 0.0f;
        }
    }
}

void EmitterInstanceList::Set_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes)
{
    if (keyframes.Rand.X != 0.0f || keyframes.Rand.Y != 0.0f || keyframes.Rand.Z != 0.0f) {
        for (unsigned int i = 0; i < keyframes.NumKeyFrames; i++) {
            if (keyframes.Values[i].X < KEYFRAME_EPSILON && keyframes.Values[i].Y < KEYFRAME_EPSILON
                && keyframes.Values[i].Z < KEYFRAME_EPSILON) {
                keyframes.Values[i].X = -keyframes.Rand.X;
                keyframes.Values[i].Y = -keyframes.Rand.Y;
                keyframes.Values[i].Z = -keyframes.Rand.Z;
            }
        }
    }

    ParticleEmitterDefClass::Set_Color_Keyframes(keyframes);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Reset_Colors(keyframes);
    }
}

void EmitterInstanceList::Set_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    if (keyframes.Rand != 0.0f) {
        for (unsigned int i = 0; i < keyframes.NumKeyFrames; i++) {
            if (keyframes.Values[i] < KEYFRAME_EPSILON) {
                keyframes.Values[i] = -keyframes.Rand;
            }
        }
    }

    ParticleEmitterDefClass::Set_Opacity_Keyframes(keyframes);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Reset_Opacity(keyframes);
    }
}

void EmitterInstanceList::Set_Size_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    if (keyframes.Rand != 0.0f) {
        for (unsigned int i = 0; i < keyframes.NumKeyFrames; i++) {
            if (keyframes.Values[i] < KEYFRAME_EPSILON) {
                keyframes.Values[i] = -keyframes.Rand;
            }
        }
    }

    ParticleEmitterDefClass::Set_Size_Keyframes(keyframes);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Reset_Size(keyframes);
    }
}

void EmitterInstanceList::Set_Rotation_Keyframes(ParticlePropertyStruct<float> &keyframes, float orient_rnd)
{
    ParticleEmitterDefClass::Set_Rotation_Keyframes(keyframes, orient_rnd);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Reset_Rotations(keyframes, orient_rnd);
    }
}

void EmitterInstanceList::Set_Frame_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    ParticleEmitterDefClass::Set_Frame_Keyframes(keyframes);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Reset_Frames(keyframes);
    }
}

void EmitterInstanceList::Set_Blur_Time_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    ParticleEmitterDefClass::Set_Blur_Time_Keyframes(keyframes);

    for (int i = 0; i < m_emitters.Count(); i++) {
        m_emitters[i]->Reset_Blur_Times(keyframes);
    }
}

void EmitterInstanceList::Add_Emitter(ParticleEmitterClass *emitter)
{
    if (emitter != nullptr) {
        if (m_emitters.Count() == 0) {
            ParticleEmitterDefClass *def = emitter->Build_Definition();

            if (def != nullptr) {
                *static_cast<ParticleEmitterDefClass *>(this) = *def;
                delete def;
            }
        }

        emitter->Add_Ref();
        m_emitters.Add(emitter);
    }
}

void EmitterInstanceList::Clear_Emitters()
{
    for (int i = 0; i < m_emitters.Count(); i++) {
        Ref_Ptr_Release(m_emitters[i]);
    }

    m_emitters.Delete_All();
}
