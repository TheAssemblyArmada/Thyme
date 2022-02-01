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
#pragma once
#include "always.h"
#include "asciistring.h"
#include "coord.h"
#include "overridable.h"
#include "snapshot.h"

enum LocomotorBehaviorZ
{
    Z_NO_Z_MOTIVE_FORCE,
    Z_SEA_LEVEL,
    Z_SURFACE_RELATIVE_HEIGHT,
    Z_ABSOLUTE_HEIGHT,
    Z_FIXED_SURFACE_RELATIVE_HEIGHT,
    Z_FIXED_ABSOLUTE_HEIGHT,
    Z_FIXED_RELATIVE_TO_GROUND_AND_BUILDINGS,
    Z_RELATIVE_TO_HIGHEST_LAYER,
};

enum LocomotorAppearance
{
    LOCO_LEGS_TWO,
    LOCO_WHEELS_FOUR,
    LOCO_TREADS,
    LOCO_HOVER,
    LOCO_THRUST,
    LOCO_WINGS,
    LOCO_CLIMBER,
    LOCO_OTHER,
    LOCO_MOTORCYCLE,
};

enum LocomotorGroupMovementPriority
{
    PRIORITY_MOVES_BACK,
    PRIORITY_MOVES_MIDDLE,
    PRIORITY_MOVES_FRONT,
};

class LocomotorTemplate : public Overridable
{
    friend class Locomotor;

    IMPLEMENT_POOL(LocomotorTemplate);

public:
    virtual ~LocomotorTemplate() override;

private:
    Utf8String m_name;
    int m_surfaces;
    float m_maxSpeed;
    float m_maxSpeedDamaged;
    float m_minSpeed;
    float m_maxTurnRate;
    float m_maxTurnRateDamaged;
    float m_acceleration;
    float m_accelerationDamaged;
    float m_lift;
    float m_liftDamaged;
    float m_braking;
    float m_minTurnSpeed;
    float m_preferredHeight;
    float m_preferredHeightDamping;
    float m_circlingRadius;
    float m_speedLimitZ;
    float m_extra2DFriction;
    float m_maxThrustAngle;
    LocomotorBehaviorZ m_behaviorZ;
    LocomotorAppearance m_appearance;
    LocomotorGroupMovementPriority m_groupMovementPriority;
    float m_accelPitchLimit;
    float m_deaccelPitchLimit;
    float m_bounceKick;
    float m_pitchStiffness;
    float m_rollStiffness;
    float m_pitchDamping;
    float m_rollDamping;
    float m_pitchInDirectionOfZVelFactor;
    float m_thrustRoll;
    float m_thrustWobbleRate;
    float m_thrustMinWobble;
    float m_thrustMaxWobble;
    float m_forwardVelCoef;
    float m_lateralVelCoef;
    float m_forwardAccelCoef;
    float m_lateralAccelCoef;
    float m_uniformAxialDamping;
    float m_turnPivotOffset;
    int m_airborneTargetingHeight;
    float m_closeEnoughDist;
    bool m_closeEnoughDist3D;
    float m_slideIntoPlaceTime;
    bool m_locomotorWorksWhenDead;
    bool m_allowMotiveForceWhileAirborne;
    bool m_apply2DFrictionWhenAirborne;
    bool m_downhillOnly;
    bool m_stickToGround;
    bool m_canMoveBackwards;
    bool m_hasSuspension;
    float m_maximumWheelExtension;
    float m_maximumWheelCompression;
    float m_wheelTurnAngle;
    float m_wanderWidthFactor;
    float m_wanderLengthFactor;
    float m_wanderAboutPointRadius;
    float m_rudderCorrectionDegree;
    float m_rudderCorrectionRate;
    float m_elevatorCorrectionDegree;
    float m_elevatorCorrectionRate;
};

class Locomotor : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Locomotor);

public:
    virtual ~Locomotor() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    LocomotorAppearance Get_Appearance() const { return m_template->m_appearance; }
    float Get_Thrust_Roll() const { return m_template->m_thrustRoll; }
    float Get_Thrust_Wobble_Rate() const { return m_template->m_thrustWobbleRate; }
    float Get_Thrust_Max_Wobble() const { return m_template->m_thrustMaxWobble; }
    float Get_Thrust_Min_Wobble() const { return m_template->m_thrustMinWobble; }

    float Get_Accel_Pitch_Limit() const { return m_template->m_accelPitchLimit; }
    float Get_Deaccel_Pitch_Limit() const { return m_template->m_deaccelPitchLimit; }
    float Get_Pitch_Stiffness() const { return m_template->m_pitchStiffness; }
    float Get_Roll_Stiffness() const { return m_template->m_rollStiffness; }
    float Get_Pitch_Damping() const { return m_template->m_pitchDamping; }
    float Get_Roll_Damping() const { return m_template->m_rollDamping; }
    float Get_Pitch_In_Direction_Of_Z_Vel_Factor() const { return m_template->m_pitchInDirectionOfZVelFactor; }

    float Get_Foward_Vel_Coef() const { return m_template->m_forwardVelCoef; }
    float Get_Lateral_Vel_Coef() const { return m_template->m_lateralVelCoef; }
    float Get_Foward_Accel_Coef() const { return m_template->m_forwardAccelCoef; }
    float Get_Lateral_Accel_Coef() const { return m_template->m_lateralAccelCoef; }
    float Get_Uniform_Axial_Damping() const { return m_template->m_uniformAxialDamping; }
    float Get_Rudder_Correction_Degree() const { return m_template->m_rudderCorrectionDegree; }
    float Get_Rudder_Correction_Rate() const { return m_template->m_rudderCorrectionRate; }

    float Get_Elevator_Correction_Degree() const { return m_template->m_elevatorCorrectionDegree; }
    float Get_Elevator_Correction_Rate() const { return m_template->m_elevatorCorrectionRate; }
    float Get_Bounce_Kick() const { return m_template->m_bounceKick; }
    bool Has_Suspension() const { return m_template->m_hasSuspension; }
    float Get_Max_Wheel_Extension() const { return m_template->m_maximumWheelExtension; }
    float Get_Wheel_Turn_Angle() const { return m_template->m_wheelTurnAngle; }

private:
    Override<LocomotorTemplate> m_template;
    Coord3D m_maintainPos;
    float m_unk1;
    float m_maxLift;
    float m_maxSpeed;
    float m_maxAccel;
    float m_brakingFactor;
    float m_maxTurnRate;
    float m_closeEnoughDist;
    unsigned int m_flags;
    float m_preferredHeight;
    float m_preferredHeightDamping;
    float m_unk2;
    float m_unk3;
    unsigned int m_unk4;
};
