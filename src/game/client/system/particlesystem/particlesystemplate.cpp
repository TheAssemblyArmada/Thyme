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

constexpr const char *const ParticlePriorityNames[PARTICLE_PRIORITY_COUNT + 1] = {
    "NONE",
    "WEAPON_EXPLOSION",
    "SCORCHMARK",
    "DUST_TRAIL",
    "BUILDUP",
    "DEBRIS_TRAIL",
    "UNIT_DAMAGE_FX",
    "DEATH_EXPLOSION",
    "SEMI_CONSTANT",
    "CONSTANT",
    "WEAPON_TRAIL",
    "AREA_EFFECT",
    "CRITICAL",
    "ALWAYS_RENDER",
    nullptr,
};

constexpr const char *const ParticleShaderTypeNames[] = {
    "NONE",
    "ADDITIVE",
    "ALPHA",
    "ALPHA_TEST",
    "MULTIPLY",
    nullptr,
};

constexpr const char *const ParticleTypeNames[] = {
    "NONE",
    "PARTICLE",
    "DRAWABLE",
    "STREAK",
    "VOLUME_PARTICLE",
    "SMUDGE",
    nullptr,
};

constexpr const char *const EmissionVelocityTypeNames[] = {
    "NONE",
    "ORTHO",
    "SPHERICAL",
    "HEMISPHERICAL",
    "CYLINDRICAL",
    "OUTWARD",
    nullptr,
};

constexpr const char *const EmssionVolumeTypeNames[] = {
    "NONE",
    "POINT",
    "LINE",
    "BOX",
    "SPHERE",
    "CYLINDER",
    nullptr,
};

constexpr const char *const WindMotionTypeNames[] = {
    "NONE",
    "Unused",
    "PingPong",
    "Circular",
    nullptr,
};

// clang-format off
const FieldParse ParticleSystemTemplate::s_fieldParseTable[] = {
     FIELD_PARSE_INDEX_LIST("Priority", ParticlePriorityNames, ParticleSystemTemplate, m_priority),
     FIELD_PARSE_BOOL("IsOneShot", ParticleSystemTemplate, m_isOneShot),
     FIELD_PARSE_INDEX_LIST("Shader", ParticleShaderTypeNames, ParticleSystemTemplate, m_shaderType),
     FIELD_PARSE_INDEX_LIST("Type", ParticleTypeNames, ParticleSystemTemplate, m_particleType),
     FIELD_PARSE_ASCIISTRING("ParticleName", ParticleSystemTemplate, m_particleTypeName),
#ifndef GAME_DLL
     FIELD_PARSE_RANDOM_VARIABLE("AngleX", ParticleSystemTemplate, m_angleX),
     FIELD_PARSE_RANDOM_VARIABLE("AngleY", ParticleSystemTemplate, m_angleY),
#endif
     FIELD_PARSE_RANDOM_VARIABLE("AngleZ", ParticleSystemTemplate, m_angleZ),
#ifndef GAME_DLL
     FIELD_PARSE_RANDOM_VARIABLE("AngularRateX", ParticleSystemTemplate, m_angularRateX),
     FIELD_PARSE_RANDOM_VARIABLE("AngularRateY", ParticleSystemTemplate, m_angularRateY),
#endif
     FIELD_PARSE_RANDOM_VARIABLE("AngularRateZ", ParticleSystemTemplate, m_angularRateZ),
     FIELD_PARSE_RANDOM_VARIABLE("AngularDamping", ParticleSystemTemplate, m_angularDamping),
     FIELD_PARSE_RANDOM_VARIABLE("VelocityDamping", ParticleSystemTemplate, m_velDamping),
     FIELD_PARSE_REAL("Gravity", ParticleSystemTemplate, m_gravity),
     FIELD_PARSE_ASCIISTRING("SlaveSystem", ParticleSystemTemplate, m_slaveSystemName),
     FIELD_PARSE_COORD3D("SlavePosOffset", ParticleSystemTemplate, m_slavePosOffset),
     FIELD_PARSE_ASCIISTRING("PerParticleAttachedSystem", ParticleSystemTemplate, m_attachedSystemName),
     FIELD_PARSE_RANDOM_VARIABLE("Lifetime", ParticleSystemTemplate, m_lifetime),
     FIELD_PARSE_UNSIGNED_INT("SystemLifetime", ParticleSystemTemplate, m_systemLifetime),
     FIELD_PARSE_RANDOM_VARIABLE("Size", ParticleSystemTemplate, m_startSize),
     FIELD_PARSE_RANDOM_VARIABLE("StartSizeRate", ParticleSystemTemplate, m_startSizeRate),
     FIELD_PARSE_RANDOM_VARIABLE("SizeRate", ParticleSystemTemplate, m_sizeRate),
     FIELD_PARSE_RANDOM_VARIABLE("SizeRateDamping", ParticleSystemTemplate, m_sizeRateDamping),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha1", ParticleSystemTemplate, m_alphaKey[0]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha2", ParticleSystemTemplate, m_alphaKey[1]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha3", ParticleSystemTemplate, m_alphaKey[2]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha4", ParticleSystemTemplate, m_alphaKey[3]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha5", ParticleSystemTemplate, m_alphaKey[4]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha6", ParticleSystemTemplate, m_alphaKey[5]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha7", ParticleSystemTemplate, m_alphaKey[6]),
     FIELD_PARSE_RANDOM_KEYFRAME("Alpha8", ParticleSystemTemplate, m_alphaKey[7]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color1", ParticleSystemTemplate, m_colorKey[0]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color2", ParticleSystemTemplate, m_colorKey[1]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color3", ParticleSystemTemplate, m_colorKey[2]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color4", ParticleSystemTemplate, m_colorKey[3]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color5", ParticleSystemTemplate, m_colorKey[4]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color6", ParticleSystemTemplate, m_colorKey[5]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color7", ParticleSystemTemplate, m_colorKey[6]),
     FIELD_PARSE_RGB_COLOR_KEYFRAME("Color8", ParticleSystemTemplate, m_colorKey[7]),
     FIELD_PARSE_RANDOM_VARIABLE("ColorScale", ParticleSystemTemplate, m_colorScale),
     FIELD_PARSE_RANDOM_VARIABLE("BurstDelay", ParticleSystemTemplate, m_burstDelay),
     FIELD_PARSE_RANDOM_VARIABLE("BurstCount", ParticleSystemTemplate, m_burstCount),
     FIELD_PARSE_RANDOM_VARIABLE("InitialDelay", ParticleSystemTemplate, m_initialDelay),
     FIELD_PARSE_COORD3D("DriftVelocity", ParticleSystemTemplate, m_driftVelocity),
     FIELD_PARSE_INDEX_LIST("VelocityType", EmissionVelocityTypeNames, ParticleSystemTemplate, m_emissionVelocityType),
     FIELD_PARSE_RANDOM_VARIABLE("VelOrthoX", ParticleSystemTemplate, m_emissionVelocity.ortho.x),
     FIELD_PARSE_RANDOM_VARIABLE("VelOrthoY", ParticleSystemTemplate, m_emissionVelocity.ortho.y),
     FIELD_PARSE_RANDOM_VARIABLE("VelOrthoZ", ParticleSystemTemplate, m_emissionVelocity.ortho.z),
     FIELD_PARSE_RANDOM_VARIABLE("VelSpherical", ParticleSystemTemplate, m_emissionVelocity.spherical),
     FIELD_PARSE_RANDOM_VARIABLE("VelHemispherical", ParticleSystemTemplate, m_emissionVelocity.hemispherical),
     FIELD_PARSE_RANDOM_VARIABLE("VelCylindricalRadial", ParticleSystemTemplate, m_emissionVelocity.cylindrical.radial),
     FIELD_PARSE_RANDOM_VARIABLE("VelCylindricalNormal", ParticleSystemTemplate, m_emissionVelocity.cylindrical.normal),
     FIELD_PARSE_RANDOM_VARIABLE("VelOutward", ParticleSystemTemplate, m_emissionVelocity.outward.outward),
     FIELD_PARSE_RANDOM_VARIABLE("VelOutwardOther", ParticleSystemTemplate, m_emissionVelocity.outward.other),
     FIELD_PARSE_INDEX_LIST("VolumeType", EmssionVolumeTypeNames, ParticleSystemTemplate, m_emissionVolumeType),
     FIELD_PARSE_COORD3D("VolLineStart", ParticleSystemTemplate, m_emissionVolume.line.start),
     FIELD_PARSE_COORD3D("VolLineEnd", ParticleSystemTemplate, m_emissionVolume.line.end),
     FIELD_PARSE_COORD3D("VolBoxHalfSize", ParticleSystemTemplate, m_emissionVolume.box),
     FIELD_PARSE_REAL("VolSphereRadius", ParticleSystemTemplate, m_emissionVolume.sphere),
     FIELD_PARSE_REAL("VolCylinderRadius", ParticleSystemTemplate, m_emissionVolume.cylinder.radius),
     FIELD_PARSE_REAL("VolCylinderLength", ParticleSystemTemplate, m_emissionVolume.cylinder.length),
     FIELD_PARSE_BOOL("IsHollow", ParticleSystemTemplate, m_isEmissionVolumeHollow),
     FIELD_PARSE_BOOL("IsGroundAligned", ParticleSystemTemplate, m_isGroundAligned),
     FIELD_PARSE_BOOL("IsEmitAboveGroundOnly", ParticleSystemTemplate, m_isEmitAboveGroundOnly),
     FIELD_PARSE_BOOL("IsParticleUpTowardsEmitter", ParticleSystemTemplate, m_isParticleUpTowardsEmitter),
     FIELD_PARSE_INDEX_LIST("WindMotion", WindMotionTypeNames, ParticleSystemTemplate, m_windMotion),
     FIELD_PARSE_REAL("WindAngleChangeMin", ParticleSystemTemplate, m_windAngleChangeMin),
     FIELD_PARSE_REAL("WindAngleChangeMax", ParticleSystemTemplate, m_windAngleChangeMax),
     FIELD_PARSE_REAL("WindPingPongStartAngleMin", ParticleSystemTemplate, m_windMotionStartAngleMin),
     FIELD_PARSE_REAL("WindPingPongStartAngleMax", ParticleSystemTemplate, m_windMotionStartAngleMax),
     FIELD_PARSE_REAL("WindPingPongEndAngleMin", ParticleSystemTemplate, m_windMotionEndAngleMin),
     FIELD_PARSE_REAL("WindPingPongEndAngleMax", ParticleSystemTemplate, m_windMotionEndAngleMax),
     FIELD_PARSE_LAST
};
// clang-format on

/**
 * @brief Parses random keyframe data.
 *
 * zh: 0x004D1540;
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
 * zh: 0x004D15A0;
 */
void ParticleSystemTemplate::Parse_RGB_Color_Keyframe(INI *ini, void *formal, void *store, const void *user_data)
{
    RGBColorKeyframe *rkf = static_cast<RGBColorKeyframe *>(store);
    INI::Parse_RGB_Color(ini, formal, &rkf->color, user_data);
    INI::Parse_Int(ini, formal, &rkf->frame, user_data);
}

// zh: 0x0041CD00 wb: 0x007A3AE0
void ParticleSystemTemplate::Parse(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String template_name = ini->Get_Next_Token();

    auto *particle_template = g_theParticleSystemManager->Find_Template(template_name);

    captainslog_dbgassert(
        particle_template != nullptr || template_name.Is_None(), "ParticleSystem %s not found!\n", template_name.Str());

    *static_cast<ParticleSystemTemplate **>(store) = particle_template;
}

/**
 * @brief Creates a new particle system from a slave template.
 */
ParticleSystem *ParticleSystemTemplate::Create_Slave_System(bool create_slaves) const
{
    if (m_slaveTemplate == nullptr && m_slaveSystemName.Is_Not_Empty()) {
        m_slaveTemplate = g_theParticleSystemManager->Find_Template(m_slaveSystemName);
    }

    if (m_slaveTemplate != nullptr) {
        return g_theParticleSystemManager->Create_Particle_System(m_slaveTemplate, create_slaves);
    }

    return nullptr;
}
