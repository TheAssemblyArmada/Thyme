#include "locomotor.h"
/**
 * @file LocomotorStore
 *
 * @author Duncans_pumpkin
 *
 * @brief LocomotorStore
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#ifndef GAME_DLL
LocomotorStore *g_theLocomotorStore = nullptr;
#endif

// zh: 0x004B8260 wb: 0x0074E62F
LocomotorStore::~LocomotorStore()
{
    for (auto &iter : m_locomotorTemplates) {
        iter.second->Delete_Instance();
    }
    m_locomotorTemplates.clear();
}

// zh: 0x004B84B0 wb: 0x0074E7B0
void LocomotorStore::Reset()
{
    for (auto it = m_locomotorTemplates.begin(); it != m_locomotorTemplates.end();) {
        it->second = static_cast<LocomotorTemplate *>(it->second->Delete_Overrides());
        if (it->second == nullptr) {
            m_locomotorTemplates.erase(it);
        } else {
            ++it;
        }
    }
}

// zh: 0x004B8460 wb: 0x0074E6ED
LocomotorTemplate *LocomotorStore::Find_Locomotor_Template(NameKeyType namekey)
{
    if (namekey == NameKeyType::NAMEKEY_INVALID) {
        return nullptr;
    }
    auto res = m_locomotorTemplates.find(namekey);
    if (res == m_locomotorTemplates.end()) {
        return nullptr;
    }
    return res->second;
}

// wb: 0x0074E836
LocomotorTemplate *LocomotorStore::New_Override(LocomotorTemplate *locoTemplate)
{
    if (locoTemplate == nullptr) {
        return nullptr;
    }

    auto *override_template = NEW_POOL_OBJ(LocomotorTemplate);
    *override_template = *locoTemplate;
    locoTemplate->Set_Next(override_template);
    override_template->Set_Is_Allocated();

    return override_template;
}

// zh: 0x004B8570 wb: 0x0074E8D7
void LocomotorStore::Parse_Locomotor_Template_Definition(INI *ini)
{
    captainslog_assert(g_theLocomotorStore != nullptr);

    const auto *tmpl_name = ini->Get_Next_Token();
    const auto key = g_theNameKeyGenerator->Name_To_Key(tmpl_name);

    auto *tmpl = g_theLocomotorStore->Find_Locomotor_Template(key);
    bool is_new_tmpl = false;
    if (tmpl == nullptr) {
        tmpl = NEW_POOL_OBJ(LocomotorTemplate);
        is_new_tmpl = true;
        if (ini->Get_Load_Type() == INILoadType::INI_LOAD_CREATE_OVERRIDES) {
            tmpl->Set_Is_Allocated();
        }
    } else {
        if (ini->Get_Load_Type() == INILoadType::INI_LOAD_CREATE_OVERRIDES) {
            auto *final_override = static_cast<LocomotorTemplate *>(tmpl->Friend_Get_Final_Override());
            tmpl = g_theLocomotorStore->New_Override(final_override);
        }
    }

    tmpl->Set_Name(tmpl_name);

    ini->Init_From_INI(tmpl, LocomotorTemplate::Get_Field_Parse());

    tmpl->Validate();

    if (is_new_tmpl) {
        g_theLocomotorStore->m_locomotorTemplates[key] = tmpl;
    }
}

// zh: 0x004B7E00 wb: 0x0074E001
LocomotorTemplate::LocomotorTemplate() :
    m_surfaces(0),
    m_maxSpeed(0.0f),
    m_maxSpeedDamaged(-1.0f),
    m_minSpeed(0.0f),
    m_maxTurnRate(0.0f),
    m_maxTurnRateDamaged(-1.0f),
    m_acceleration(0.0f),
    m_accelerationDamaged(-1.0f),
    m_lift(0.0f),
    m_liftDamaged(-1.0f),
    m_braking(99999.0f),
    m_minTurnSpeed(99999.0f),
    m_preferredHeight(0.0f),
    m_preferredHeightDamping(1.0f),
    m_circlingRadius(0.0f),
    m_speedLimitZ(999999.0f),
    m_extra2DFriction(0.0f),
    m_maxThrustAngle(0.0f),
    m_behaviorZ(Z_AXIS_BEHAVIOUR_NO_Z_MOTIVE_FORCE),
    m_appearance(LOCOMOTOR_APPEARANCE_OTHER),
    m_groupMovementPriority(LOCOMOTOR_GROUP_MOVEMENT_MOVES_MIDDLE),
    m_accelPitchLimit(0.0f),
    m_deaccelPitchLimit(0.0f),
    m_bounceKick(0.0f),
    m_pitchStiffness(0.1f),
    m_rollStiffness(0.1f),
    m_pitchDamping(0.9f),
    m_rollDamping(0.9f),
    m_pitchInDirectionOfZVelFactor(0.0f),
    m_thrustRoll(0.0f),
    m_thrustWobbleRate(0.0f),
    m_thrustMinWobble(0.0f),
    m_thrustMaxWobble(0.0f),
    m_forwardVelCoef(0.0f),
    m_lateralVelCoef(0.0f),
    m_forwardAccelCoef(0.0f),
    m_lateralAccelCoef(0.0f),
    m_uniformAxialDamping(1.0f),
    m_turnPivotOffset(0.0f),
    m_airborneTargetingHeight(INT_MAX),
    m_closeEnoughDist(1.0f),
    m_closeEnoughDist3D(false),
    m_slideIntoPlaceTime(0.0f),
    m_locomotorWorksWhenDead(false),
    m_allowMotiveForceWhileAirborne(false),
    m_apply2DFrictionWhenAirborne(false),
    m_downhillOnly(false),
    m_stickToGround(false),
    m_canMoveBackwards(false),
    m_hasSuspension(false),
    m_maximumWheelExtension(0.0f),
    m_maximumWheelCompression(0.0f),
    m_wheelTurnAngle(0.0f),
    m_wanderWidthFactor(0.0f),
    m_wanderLengthFactor(1.0f),
    m_wanderAboutPointRadius(0.0f),
    m_rudderCorrectionDegree(0.0f),
    m_rudderCorrectionRate(0.0f),
    m_elevatorCorrectionDegree(0.0f),
    m_elevatorCorrectionRate(0.0f)
{
}

// zh: 0x004B8070 wb: 0x0074E350
void LocomotorTemplate::Validate()
{
    if (m_maxSpeedDamaged < 0.0) {
        m_maxSpeedDamaged = m_maxSpeed;
    }
    if (m_maxTurnRateDamaged < 0.0) {
        m_maxTurnRateDamaged = m_maxTurnRate;
    }
    if (m_accelerationDamaged < 0.0) {
        m_accelerationDamaged = m_acceleration;
    }
    if (m_liftDamaged < 0.0) {
        m_liftDamaged = m_lift;
    }

    switch (m_appearance) {
        case LocomotorAppearance::LOCOMOTOR_APPEARANCE_WINGS:
            if (m_minSpeed <= 0.0f) {
                captainslog_dbgassert(false, "WINGS should always have positive minSpeeds (otherwise they hover)");
                m_minSpeed = 0.01f;
            }
            if (m_minTurnSpeed <= 0.0f) {
                captainslog_dbgassert(false, "WINGS should always have positive minTurnSpeeds");
                m_minTurnSpeed = 0.01f;
            }
            break;
        case LocomotorAppearance::LOCOMOTOR_APPEARANCE_THRUST:
            captainslog_relassert(m_behaviorZ != LocomotorBehaviorZ::Z_AXIS_BEHAVIOUR_NO_Z_MOTIVE_FORCE || m_lift == 0.0f
                    || m_liftDamaged == 0.0f,
                0xDEAD0006,
                "THRUST locos may not use ZAxisBehaviour or lift!");
            if (m_maxSpeed <= 0.0f) {
                captainslog_debug("THRUST locos may not have zero m_maxSpeed; healing...");
                m_maxSpeed = 0.01f;
            }
            if (m_maxSpeedDamaged <= 0.0f) {
                captainslog_debug("THRUST locos may not have zero m_maxSpeedDamaged; healing...");
                m_maxSpeedDamaged = 0.01f;
            }
            if (m_minSpeed <= 0.0f) {
                captainslog_debug("THRUST locos may not have zero m_minSpeed; healing...");
                m_minSpeed = 0.01f;
            }
            break;
    }
}

constexpr const char *TheSurfaceBitNames[] = {
    "GROUND",
    "WATER",
    "CLIFF",
    "AIR",
    "RUBBLE",
    nullptr,
};

constexpr const char *TheZAxisBehaviourNames[] = {
    "NO_Z_MOTIVE_FORCE",
    "SEA_LEVEL",
    "SURFACE_RELATIVE_HEIGHT",
    "ABSOLUTE_HEIGHT",
    "FIXED_SURFACE_RELATIVE_HEIGHT",
    "FIXED_ABSOLUTE_HEIGHT",
    "FIXED_RELATIVE_TO_GROUND_AND_BUILDINGS",
    "RELATIVE_TO_HIGHEST_LAYER",
    nullptr,
};

constexpr const char *TheLocomotorAppearanceNames[] = {
    "TWO_LEGS",
    "FOUR_WHEELS",
    "TREADS",
    "HOVER",
    "THRUST",
    "WINGS",
    "CLIMBER",
    "OTHER",
    "MOTORCYCLE",
    nullptr,
};

constexpr const char *TheLocomotorGroupMovementNames[] = {
    "MOVES_BACK",
    "MOVES_MIDDLE",
    "MOVES_FRONT",
    nullptr,
};

// zh: 0x004B8180 wb: 0x0074E5A7
void Parse_Friction_Per_Sec(INI *ini, void *, void *store, const void *)
{
    constexpr float friction_update_rate = 1.0f / 30.0f;
    const auto friction_rate = ini->Scan_Real(ini->Get_Next_Token()) * friction_update_rate;
    *static_cast<float *>(store) = friction_rate;
}

// clang-format off
FieldParse LocomotorTemplate::s_fieldParseTable[] = {
    { "Surfaces",                       &INI::Parse_Bitstring32,            TheSurfaceBitNames,             offsetof(LocomotorTemplate, m_surfaces) },
    { "Speed",                          &INI::Parse_Velocity_Real,          nullptr,                        offsetof(LocomotorTemplate, m_maxSpeed) },
    { "SpeedDamaged",                   &INI::Parse_Velocity_Real,          nullptr,                        offsetof(LocomotorTemplate, m_maxSpeedDamaged) },
    { "MinSpeed",                       &INI::Parse_Velocity_Real,          nullptr,                        offsetof(LocomotorTemplate, m_minSpeed) },
    { "TurnRate",                       &INI::Parse_Angular_Velocity_Real,  nullptr,                        offsetof(LocomotorTemplate, m_maxTurnRate) },
    { "TurnRateDamaged",                &INI::Parse_Angular_Velocity_Real,  nullptr,                        offsetof(LocomotorTemplate, m_maxTurnRateDamaged) },
    { "Acceleration",                   &INI::Parse_Acceleration_Real,      nullptr,                        offsetof(LocomotorTemplate, m_acceleration) },
    { "AccelerationDamaged",            &INI::Parse_Acceleration_Real,      nullptr,                        offsetof(LocomotorTemplate, m_accelerationDamaged) },
    { "Lift",                           &INI::Parse_Acceleration_Real,      nullptr,                        offsetof(LocomotorTemplate, m_lift) },
    { "LiftDamaged",                    &INI::Parse_Acceleration_Real,      nullptr,                        offsetof(LocomotorTemplate, m_liftDamaged) },
    { "Braking",                        &INI::Parse_Acceleration_Real,      nullptr,                        offsetof(LocomotorTemplate, m_braking) },
    { "MinTurnSpeed",                   &INI::Parse_Velocity_Real,          nullptr,                        offsetof(LocomotorTemplate, m_minTurnSpeed) },
    { "PreferredHeight",                &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_preferredHeight) },
    { "PreferredHeightDamping",         &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_preferredHeightDamping) },
    { "CirclingRadius",                 &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_circlingRadius) },
    { "SpeedLimitZ",                    &INI::Parse_Velocity_Real,          nullptr,                        offsetof(LocomotorTemplate, m_speedLimitZ) },
    { "Extra2DFriction",                &Parse_Friction_Per_Sec,            nullptr,                        offsetof(LocomotorTemplate, m_extra2DFriction) },
    { "MaxThrustAngle",                 &INI::Parse_Angle_Real,             nullptr,                        offsetof(LocomotorTemplate, m_maxThrustAngle) },
    { "ZAxisBehavior",                  &INI::Parse_Index_List,             TheZAxisBehaviourNames,         offsetof(LocomotorTemplate, m_behaviorZ) },
    { "Appearance",                     &INI::Parse_Index_List,             TheLocomotorAppearanceNames,    offsetof(LocomotorTemplate, m_appearance) },
    { "GroupMovementPriority",          &INI::Parse_Index_List,             TheLocomotorGroupMovementNames, offsetof(LocomotorTemplate, m_groupMovementPriority) },
    { "AccelerationPitchLimit",         &INI::Parse_Angle_Real,             nullptr,                        offsetof(LocomotorTemplate, m_accelPitchLimit) },
    { "DecelerationPitchLimit",         &INI::Parse_Angle_Real,             nullptr,                        offsetof(LocomotorTemplate, m_deaccelPitchLimit) },
    { "BounceAmount",                   &INI::Parse_Angular_Velocity_Real,  nullptr,                        offsetof(LocomotorTemplate, m_bounceKick) },
    { "PitchStiffness",                 &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_pitchStiffness) },
    { "RollStiffness",                  &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_rollStiffness) },
    { "PitchDamping",                   &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_pitchDamping) },
    { "RollDamping",                    &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_rollDamping) },
    { "PitchInDirectionOfZVelFactor",   &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_pitchInDirectionOfZVelFactor) },
    { "ThrustRoll",                     &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_thrustRoll) },
    { "ThrustWobbleRate",               &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_thrustWobbleRate) },
    { "ThrustMinWobble",                &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_thrustMinWobble) },
    { "ThrustMaxWobble",                &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_thrustMaxWobble) },
    { "ForwardVelocityPitchFactor",     &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_forwardVelCoef) },
    { "LateralVelocityRollFactor",      &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_lateralVelCoef) },
    { "ForwardAccelerationPitchFactor", &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_forwardAccelCoef) },
    { "LateralAccelerationRollFactor",  &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_lateralAccelCoef) },
    { "UniformAxialDamping",            &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_uniformAxialDamping) },
    { "TurnPivotOffset",                &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_turnPivotOffset) },
    { "AirborneTargetingHeight",        &INI::Parse_Int,                    nullptr,                        offsetof(LocomotorTemplate, m_airborneTargetingHeight) },
    { "CloseEnoughDist",                &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_closeEnoughDist) },
    { "CloseEnoughDist3D",              &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_closeEnoughDist3D) },
    { "SlideIntoPlaceTime",             &INI::Parse_Duration_Real,          nullptr,                        offsetof(LocomotorTemplate, m_slideIntoPlaceTime) },
    { "LocomotorWorksWhenDead",         &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_locomotorWorksWhenDead) },
    { "AllowAirborneMotiveForce",       &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_allowMotiveForceWhileAirborne) },
    { "Apply2DFrictionWhenAirborne",    &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_apply2DFrictionWhenAirborne) },
    { "DownhillOnly",                   &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_downhillOnly) },
    { "StickToGround",                  &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_stickToGround) },
    { "CanMoveBackwards",               &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_canMoveBackwards) },
    { "HasSuspension",                  &INI::Parse_Bool,                   nullptr,                        offsetof(LocomotorTemplate, m_hasSuspension) },
    { "MaximumWheelExtension",          &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_maximumWheelExtension) },
    { "MaximumWheelCompression",        &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_maximumWheelCompression) },
    { "FrontWheelTurnAngle",            &INI::Parse_Angle_Real,             nullptr,                        offsetof(LocomotorTemplate, m_wheelTurnAngle) },
    { "WanderWidthFactor",              &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_wanderWidthFactor) },
    { "WanderLengthFactor",             &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_wanderLengthFactor) },
    { "WanderAboutPointRadius",         &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_wanderAboutPointRadius) },
    { "RudderCorrectionDegree",         &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_rudderCorrectionDegree) },
    { "RudderCorrectionRate",           &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_rudderCorrectionRate) },
    { "ElevatorCorrectionDegree",       &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_elevatorCorrectionDegree) },
    { "ElevatorCorrectionRate",         &INI::Parse_Real,                   nullptr,                        offsetof(LocomotorTemplate, m_elevatorCorrectionRate) },
    { nullptr,                          nullptr,                            nullptr,                        0 },
};
// clang-format on
