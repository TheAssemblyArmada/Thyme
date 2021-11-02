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
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "overridable.h"
#include "subsysteminterface.h"
#include <map>

class INI;

class Locomotor;

enum LocomotorBehaviorZ
{
    Z_AXIS_BEHAVIOUR_NO_Z_MOTIVE_FORCE,
    Z_AXIS_BEHAVIOUR_SEA_LEVEL,
    Z_AXIS_BEHAVIOUR_SURFACE_RELATIVE_HEIGHT,
    Z_AXIS_BEHAVIOUR_ABSOLUTE_HEIGHT,
    Z_AXIS_BEHAVIOUR_FIXED_SURFACE_RELATIVE_HEIGHT,
    Z_AXIS_BEHAVIOUR_FIXED_ABSOLUTE_HEIGHT,
    Z_AXIS_BEHAVIOUR_FIXED_RELATIVE_TO_GROUND_AND_BUILDINGS,
    Z_AXIS_BEHAVIOUR_RELATIVE_TO_HIGHEST_LAYER,
};

enum LocomotorAppearance
{
    LOCOMOTOR_APPEARANCE_TWO_LEGS,
    LOCOMOTOR_APPEARANCE_FOUR_WHEELS,
    LOCOMOTOR_APPEARANCE_TREADS,
    LOCOMOTOR_APPEARANCE_HOVER,
    LOCOMOTOR_APPEARANCE_THRUST,
    LOCOMOTOR_APPEARANCE_WINGS,
    LOCOMOTOR_APPEARANCE_CLIMBER,
    LOCOMOTOR_APPEARANCE_OTHER,
    LOCOMOTOR_APPEARANCE_MOTORCYCLE,
};

enum LocomotorGroupMovementPriority
{
    LOCOMOTOR_GROUP_MOVEMENT_MOVES_BACK,
    LOCOMOTOR_GROUP_MOVEMENT_MOVES_MIDDLE,
    LOCOMOTOR_GROUP_MOVEMENT_MOVES_FRONT,
};

class LocomotorTemplate : public Overridable
{
    IMPLEMENT_POOL(LocomotorTemplate);

public:
    virtual ~LocomotorTemplate() override{};
    LocomotorTemplate();

    void Set_Name(const Utf8String &newName) { m_name = newName; }
    void Validate();

    static FieldParse *Get_Field_Parse() { return s_fieldParseTable; }

private:
    Utf8String m_name;
    int32_t m_surfaces;
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
    int32_t m_airborneTargetingHeight;
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

    static FieldParse s_fieldParseTable[];
};

class LocomotorStore : public SubsystemInterface
{
public:
    virtual ~LocomotorStore() override;

    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

    // zh: 0x004B81B0 wb: 0x0074E5D8
    LocomotorStore *Hook_Ctor() { return new (this) LocomotorStore; }

    LocomotorTemplate *Find_Locomotor_Template(NameKeyType namekey);
    Locomotor *New_Locomotor(LocomotorTemplate *tmpl);
    LocomotorTemplate *New_Override(LocomotorTemplate *locoTemplate);

    static void Parse_Locomotor_Template_Definition(INI *ini);

private:
    std::map<NameKeyType, LocomotorTemplate *> m_locomotorTemplates;
};

#ifdef GAME_DLL
extern LocomotorStore *&g_theLocomotorStore;
#else
extern LocomotorStore *g_theLocomotorStore;
#endif
