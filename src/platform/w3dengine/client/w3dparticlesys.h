/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Particle System
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
#include "particlesysmanager.h"
#include "sharebuf.h"
#include "vector3.h"
#include "vector4.h"

class RenderInfoClass;
class PointGroupClass;
class StreakLineClass;

class W3DParticleSystemManager : public ParticleSystemManager
{
public:
    W3DParticleSystemManager();
    virtual ~W3DParticleSystemManager() override;
    virtual int Get_On_Screen_Particle_Count() override;
    virtual void Do_Particles(RenderInfoClass &rinfo) override;
    virtual void Queue_Particle_Render() override;

private:
    enum
    {
        MAX_POINTS_PER_GROUP = 512,
    };

    PointGroupClass *m_pointGroup;
    StreakLineClass *m_streakLine;
    ShareBufferClass<Vector3> *m_posBuffer;
    ShareBufferClass<Vector4> *m_RGBABuffer;
    ShareBufferClass<float> *m_sizeBuffer;
    ShareBufferClass<unsigned char> *m_angleBuffer;
    bool m_readyToRender;
};

void Do_Particles(RenderInfoClass &rinfo);
