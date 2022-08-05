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
#pragma once
#include "w3dafx.h"
#include "part_ldr.h"

class EmitterInstanceList : public ParticleEmitterDefClass
{
public:
    virtual ~EmitterInstanceList() override;
    virtual void Set_Velocity(const Vector3 &value) override;
    virtual void Set_Acceleration(const Vector3 &value) override;
    virtual void Set_Burst_Size(unsigned int size) override;
    virtual void Set_Outward_Vel(float value) override;
    virtual void Set_Vel_Inherit(float value) override;
    virtual void Set_Velocity_Random(Vector3Randomizer *randomizer) override;
    virtual void Get_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes) const override;
    virtual void Get_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes) const override;
    virtual void Get_Size_Keyframes(ParticlePropertyStruct<float> &keyframes) const override;
    virtual void Set_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes) override;
    virtual void Set_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes) override;
    virtual void Set_Size_Keyframes(ParticlePropertyStruct<float> &keyframes) override;
    virtual void Set_Rotation_Keyframes(ParticlePropertyStruct<float> &keyframes, float orient_rnd) override;
    virtual void Set_Frame_Keyframes(ParticlePropertyStruct<float> &keyframes) override;
    virtual void Set_Blur_Time_Keyframes(ParticlePropertyStruct<float> &keyframes) override;
    virtual void Add_Emitter(ParticleEmitterClass *emitter);
    virtual void Clear_Emitters();

private:
    DynamicVectorClass<ParticleEmitterClass *> m_emitters;
};
