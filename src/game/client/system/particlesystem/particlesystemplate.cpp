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
     { "AngleX", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angleX) },
     { "AngleY", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angleY) },
#endif
     { "AngleZ", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angleZ) },
#ifndef GAME_DLL
     { "AngularRateX", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angularRateX) },
     { "AngularRateY", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angularRateY) },
#endif
     { "AngularRateZ", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angularRateZ) },
     { "AngularDamping", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_angularDamping) },
     { "VelocityDamping", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_velDamping) },
     FIELD_PARSE_REAL("Gravity", ParticleSystemTemplate, m_gravity),
     FIELD_PARSE_ASCIISTRING("SlaveSystem", ParticleSystemTemplate, m_slaveSystemName),
     FIELD_PARSE_COORD3D("SlavePosOffset", ParticleSystemTemplate, m_slavePosOffset),
     FIELD_PARSE_ASCIISTRING("PerParticleAttachedSystem", ParticleSystemTemplate, m_attachedSystemName),
     { "Lifetime", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_lifetime) },
     FIELD_PARSE_UNSIGNED_INT("SystemLifetime", ParticleSystemTemplate, m_systemLifetime),
     { "Size", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_startSize) },
     { "StartSizeRate", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_startSizeRate) },
     { "SizeRate", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_sizeRate) },
     { "SizeRateDamping", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_sizeRateDamping) },
     { "Alpha1", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[0]) },
     { "Alpha2", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[1]) },
     { "Alpha3", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[2]) },
     { "Alpha4", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[3]) },
     { "Alpha5", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[4]) },
     { "Alpha6", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[5]) },
     { "Alpha7", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[6]) },
     { "Alpha8", &ParticleSystemTemplate::Parse_Random_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_alphaKey[7]) },
     { "Color1", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[0]) },
     { "Color2", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[1]) },
     { "Color3", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[2]) },
     { "Color4", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[3]) },
     { "Color5", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[4]) },
     { "Color6", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[5]) },
     { "Color7", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[6]) },
     { "Color8", &ParticleSystemTemplate::Parse_RGB_Color_Keyframe, nullptr, offsetof(ParticleSystemTemplate, m_colorKey[7]) },
     { "ColorScale", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_colorScale) },
     { "BurstDelay", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_burstDelay) },
     { "BurstCount", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_burstCount) },
     { "InitialDelay", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_initialDelay) },
     FIELD_PARSE_COORD3D("DriftVelocity", ParticleSystemTemplate, m_driftVelocity),
     FIELD_PARSE_INDEX_LIST("VelocityType", EmissionVelocityTypeNames, ParticleSystemTemplate, m_emissionVelocityType),
     { "VelOrthoX", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.ortho.x) },
     { "VelOrthoY", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.ortho.y) },
     { "VelOrthoZ", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.ortho.z) },
     { "VelSpherical", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.spherical) },
     { "VelHemispherical", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.hemispherical) },
     { "VelCylindricalRadial", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.cylindrical.radial) },
     { "VelCylindricalNormal", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.cylindrical.normal) },
     { "VelOutward", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.outward.outward) },
     { "VelOutwardOther", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemTemplate, m_emissionVelocity.outward.other) },
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

// zh: 0x0041CD00 wb: 0x007A3AE0
void ParticleSystemTemplate::Parse(INI *ini, void *, void *store, const void *)
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
