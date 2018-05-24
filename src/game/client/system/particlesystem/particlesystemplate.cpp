/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Template object that particle systems are instantiated from.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particlesystemplate.h"
#include "ini.h"
#include "particlesysmanager.h"

/**
 * @brief Parses random keyframe data.
 *
 * 0x004D1540;
 */
void ParticleSystemTemplate::Parse_Random_Keyframe(INI *ini, void *formal, void *store, const void *user_data)
{
    RandomKeyframe *rkf = static_cast<RandomKeyframe *>(store);
    const char *token = ini->Get_Next_Token();
    float low = INI::Scan_Real(token);
    token = ini->Get_Next_Token();
    float high = INI::Scan_Real(token);
    token = ini->Get_Next_Token();
    rkf->frame = INI::Scan_Int(token);
    rkf->var.Set_Range(low, high, GameClientRandomVariable::UNIFORM);
}

/**
 * @brief Parses RGB key frame data.
 *
 * 0x004D15A0;
 */
void ParticleSystemTemplate::Parse_RGB_Color_Keyframe(INI *ini, void *formal, void *store, const void *user_data)
{
    RGBColorKeyframe *rkf = static_cast<RGBColorKeyframe *>(store);
    INI::Parse_RGB_Color(ini, formal, &rkf->color, user_data);
    INI::Parse_Int(ini, formal, &rkf->frame, user_data);
}

ParticleSystem *ParticleSystemTemplate::Create_Slave_System(bool create_slaves)
{
    if (m_slaveTemplate == nullptr && m_slaveSystemName.Is_Not_Empty()) {
        m_slaveTemplate = g_theParticleSystemManager->Find_Template(m_slaveSystemName);
    }

    if (m_slaveTemplate == nullptr) {
        return nullptr;
    }

    return g_theParticleSystemManager->Create_Particle_System(m_slaveTemplate, create_slaves);
}
