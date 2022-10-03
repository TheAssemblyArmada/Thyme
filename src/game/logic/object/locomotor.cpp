/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Locomotor
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "locomotor.h"

LocomotorTemplate::LocomotorTemplate() :
    m_maxSpeedDamaged(-1.0f),
    m_maxTurnRateDamaged(-1.0f),
    m_accelerationDamaged(-1.0f),
    m_liftDamaged(-1.0f),
    m_surfaces(0),
    m_maxSpeed(0.0f),
    m_maxTurnRate(0.0f),
    m_acceleration(0.0f),
    m_lift(0.0f),
    m_braking(99999.0f),
    m_minSpeed(0.0f),
    m_minTurnSpeed(99999.0f),
    m_behaviorZ(Z_NO_Z_MOTIVE_FORCE),
    m_appearance(LOCO_OTHER),
    m_groupMovementPriority(PRIORITY_MOVES_MIDDLE),
    m_preferredHeight(0.0f),
    m_preferredHeightDamping(1.0f),
    m_circlingRadius(0.0f),
    m_maxThrustAngle(0.0f),
    m_speedLimitZ(999999.0f),
    m_extra2DFriction(0.0f),
    m_accelPitchLimit(0.0f),
    m_deaccelPitchLimit(0.0f),
    m_bounceKick(0.0f),
    m_pitchStiffness(0.1f),
    m_rollStiffness(0.1f),
    m_pitchDamping(0.9f),
    m_rollDamping(0.9f),
    m_forwardVelCoef(0.0f),
    m_pitchInDirectionOfZVelFactor(0.0f),
    m_thrustRoll(0.0f),
    m_thrustWobbleRate(0.0f),
    m_thrustMinWobble(0.0f),
    m_thrustMaxWobble(0.0f),
    m_lateralVelCoef(0.0f),
    m_forwardAccelCoef(0.0f),
    m_lateralAccelCoef(0.0f),
    m_uniformAxialDamping(1.0f),
    m_turnPivotOffset(0.0f),
    m_apply2DFrictionWhenAirborne(false),
    m_downhillOnly(false),
    m_allowMotiveForceWhileAirborne(false),
    m_locomotorWorksWhenDead(false),
    m_airborneTargetingHeight(INT_MAX),
    m_stickToGround(false),
    m_canMoveBackwards(false),
    m_hasSuspension(false),
    m_wheelTurnAngle(0.0f),
    m_maximumWheelExtension(0.0f),
    m_maximumWheelCompression(0.0f),
    m_closeEnoughDist(1.0f),
    m_closeEnoughDist3D(false),
    m_slideIntoPlaceTime(0.0f),
    m_wanderWidthFactor(0.0f),
    m_wanderLengthFactor(1.0f),
    m_wanderAboutPointRadius(0.0f),
    m_rudderCorrectionDegree(0.0f),
    m_rudderCorrectionRate(0.0f),
    m_elevatorCorrectionDegree(0.0f),
    m_elevatorCorrectionRate(0.0f)
{
}

void LocomotorTemplate::Validate()
{
    if (m_maxSpeedDamaged < 0.0f) {
        m_maxSpeedDamaged = m_maxSpeed;
    }
    if (m_maxTurnRateDamaged < 0.0f) {
        m_maxTurnRateDamaged = m_maxTurnRate;
    }
    if (m_accelerationDamaged < 0.0f) {
        m_accelerationDamaged = m_acceleration;
    }
    if (m_liftDamaged < 0.0f) {
        m_liftDamaged = m_lift;
    }

    switch (m_appearance) {
        case LOCO_WINGS:
            if (m_minSpeed <= 0.0f) {
                captainslog_dbgassert(false, "WINGS should always have positive minSpeeds (otherwise they hover)");
                m_minSpeed = 0.01f;
            }

            if (m_minTurnSpeed <= 0.0f) {
                captainslog_dbgassert(false, "WINGS should always have positive minTurnSpeeds");
                m_minTurnSpeed = 0.01f;
            }

            break;
        case LOCO_THRUST:
            captainslog_relassert(
                m_behaviorZ != LocomotorBehaviorZ::Z_NO_Z_MOTIVE_FORCE || m_lift == 0.0f || m_liftDamaged == 0.0f,
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

const char *g_theSurfaceBitNames[] = {
    "GROUND",
    "WATER",
    "CLIFF",
    "AIR",
    "RUBBLE",
    nullptr,
};

const char *g_theZAxisBehaviourNames[] = {
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

const char *g_theLocomotorAppearanceNames[] = {
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

const char *g_theLocomotorGroupMovementNames[] = {
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
    { "Surfaces", &INI::Parse_Bitstring32, g_theSurfaceBitNames, offsetof(LocomotorTemplate, m_surfaces) },
    { "Speed", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxSpeed) },
    { "SpeedDamaged", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxSpeedDamaged) },
    { "MinSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_minSpeed) },
    { "TurnRate", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxTurnRate) },
    { "TurnRateDamaged", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxTurnRateDamaged) },
    { "Acceleration", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_acceleration) },
    { "AccelerationDamaged", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_accelerationDamaged) },
    { "Lift", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_lift) },
    { "LiftDamaged", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_liftDamaged) },
    { "Braking", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_braking) },
    { "MinTurnSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_minTurnSpeed) },
    { "PreferredHeight", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_preferredHeight) },
    { "PreferredHeightDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_preferredHeightDamping) },
    { "CirclingRadius", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_circlingRadius) },
    { "SpeedLimitZ", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_speedLimitZ) },
    { "Extra2DFriction", &Parse_Friction_Per_Sec, nullptr, offsetof(LocomotorTemplate, m_extra2DFriction) },
    { "MaxThrustAngle", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_maxThrustAngle) },
    { "ZAxisBehavior", &INI::Parse_Index_List, g_theZAxisBehaviourNames, offsetof(LocomotorTemplate, m_behaviorZ) },
    { "Appearance", &INI::Parse_Index_List, g_theLocomotorAppearanceNames, offsetof(LocomotorTemplate, m_appearance) },
    { "GroupMovementPriority", &INI::Parse_Index_List, g_theLocomotorGroupMovementNames, offsetof(LocomotorTemplate, m_groupMovementPriority) },
    { "AccelerationPitchLimit", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_accelPitchLimit) },
    { "DecelerationPitchLimit", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_deaccelPitchLimit) },
    { "BounceAmount", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_bounceKick) },
    { "PitchStiffness", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_pitchStiffness) },
    { "RollStiffness", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rollStiffness) },
    { "PitchDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_pitchDamping) },
    { "RollDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rollDamping) },
    { "PitchInDirectionOfZVelFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_pitchInDirectionOfZVelFactor) },
    { "ThrustRoll", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustRoll) },
    { "ThrustWobbleRate", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustWobbleRate) },
    { "ThrustMinWobble", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustMinWobble) },
    { "ThrustMaxWobble", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustMaxWobble) },
    { "ForwardVelocityPitchFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_forwardVelCoef) },
    { "LateralVelocityRollFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_lateralVelCoef) },
    { "ForwardAccelerationPitchFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_forwardAccelCoef) },
    { "LateralAccelerationRollFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_lateralAccelCoef) },
    { "UniformAxialDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_uniformAxialDamping) },
    { "TurnPivotOffset", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_turnPivotOffset) },
    { "AirborneTargetingHeight", &INI::Parse_Int, nullptr, offsetof(LocomotorTemplate, m_airborneTargetingHeight) },
    { "CloseEnoughDist", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_closeEnoughDist) },
    { "CloseEnoughDist3D", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_closeEnoughDist3D) },
    { "SlideIntoPlaceTime", &INI::Parse_Duration_Real, nullptr, offsetof(LocomotorTemplate, m_slideIntoPlaceTime) },
    { "LocomotorWorksWhenDead", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_locomotorWorksWhenDead) },
    { "AllowAirborneMotiveForce", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_allowMotiveForceWhileAirborne) },
    { "Apply2DFrictionWhenAirborne", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_apply2DFrictionWhenAirborne) },
    { "DownhillOnly", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_downhillOnly) },
    { "StickToGround", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_stickToGround) },
    { "CanMoveBackwards", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_canMoveBackwards) },
    { "HasSuspension", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_hasSuspension) },
    { "MaximumWheelExtension", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_maximumWheelExtension) },
    { "MaximumWheelCompression", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_maximumWheelCompression) },
    { "FrontWheelTurnAngle", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_wheelTurnAngle) },
    { "WanderWidthFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_wanderWidthFactor) },
    { "WanderLengthFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_wanderLengthFactor) },
    { "WanderAboutPointRadius", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_wanderAboutPointRadius) },
    { "RudderCorrectionDegree", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rudderCorrectionDegree) },
    { "RudderCorrectionRate", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rudderCorrectionRate) },
    { "ElevatorCorrectionDegree", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_elevatorCorrectionDegree) },
    { "ElevatorCorrectionRate", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_elevatorCorrectionRate) },
    { nullptr, nullptr, nullptr, 0 },
};
// clang-format on
