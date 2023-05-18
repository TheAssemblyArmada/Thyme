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
#include "ini.h"
#include "namekeygenerator.h"
#include "overridable.h"
#include "physicsupdate.h"
#include "snapshot.h"
#include <map>

class Object;
class Locomotor;

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

enum LocomotorSurfaceType
{
    LOCOMOTOR_SURFACE_GROUND = 1,
    LOCOMOTOR_SURFACE_WATER = 2,
    LOCOMOTOR_SURFACE_CLIFF = 4,
    LOCOMOTOR_SURFACE_AIR = 8,
    LOCOMOTOR_SURFACE_RUBBLE = 16,
};

class LocomotorTemplate : public Overridable
{
    friend class Locomotor;

    IMPLEMENT_POOL(LocomotorTemplate);

public:
    LocomotorTemplate();
    virtual ~LocomotorTemplate() override {}

    void Validate();

    void Set_Name(const Utf8String &name) { m_name = name; }

    static FieldParse *Get_Field_Parse() { return s_fieldParseTable; }

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
    float m_decelPitchLimit;
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

    static FieldParse s_fieldParseTable[];
};

class LocomotorStore : public SubsystemInterface
{
public:
    LocomotorStore() {}
    virtual ~LocomotorStore() override;

    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

#ifdef GAME_DLL
    LocomotorStore *Hook_Ctor() { return new (this) LocomotorStore; }
#endif

    LocomotorTemplate *Find_Locomotor_Template(NameKeyType namekey);
    const LocomotorTemplate *Find_Locomotor_Template(NameKeyType namekey) const;
    Locomotor *New_Locomotor(const LocomotorTemplate *tmpl);
    LocomotorTemplate *New_Override(LocomotorTemplate *tmpl);

    static void Parse_Locomotor_Template_Definition(INI *ini);

private:
    std::map<NameKeyType, LocomotorTemplate *> m_locomotorTemplates;
};

class Locomotor : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Locomotor);

public:
    enum LocoFlag // not 100% confirmed
    {
        IS_BRAKING,
        ALLOW_INVALID_POSITION,
        MAINTAIN_POS_IS_VALID,
        PRECISE_Z_POS,
        NO_SLOW_DOWN_AS_APPROACHING_DEST,
        OVER_WATER,
        ULTRA_ACCURATE,
        MOVING_BACKWARDS,
        TURN_AROUND,
        CLIMB,
        CLOSE_ENOUGH_DIST_3D,
        WANDER_DIRECTION,
    };

#ifdef GAME_DLL
    Locomotor *Hook_Ctor(const LocomotorTemplate *tmpl) { return new (this) Locomotor(tmpl); }
#endif

    Locomotor(const LocomotorTemplate *tmpl);
    Locomotor(const Locomotor &that);
    Locomotor &operator=(const Locomotor &that);

    virtual ~Locomotor() override {}
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    float Get_Max_Speed_For_Condition(BodyDamageType condition) const;
    float Get_Max_Turn_Rate(BodyDamageType condition) const;
    float Get_Max_Acceleration(BodyDamageType condition) const;
    float Get_Max_Lift(BodyDamageType condition) const;
    float Get_Surface_Ht_At_Pt(float x, float y);
    float Get_Braking() const;

    void Loco_Update_Move_Towards_Position(
        Object *obj, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed, bool *blocked);
    void Loco_Update_Move_Towards_Angle(Object *obj, float goal_angle);
    void Move_Towards_Position_Legs(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Wheels(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Treads(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Other(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Hover(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Thrust(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Wings(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);
    void Move_Towards_Position_Climb(
        Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos, float on_path_dist_to_goal, float desired_speed);

    bool Loco_Update_Maintain_Current_Position(Object *obj);
    void Maintain_Current_Position_Thrust(Object *obj, PhysicsBehavior *physics);
    void Maintain_Current_Position_Other(Object *obj, PhysicsBehavior *physics);
    void Maintain_Current_Position_Hover(Object *obj, PhysicsBehavior *physics);
    void Maintain_Current_Position_Wings(Object *obj, PhysicsBehavior *physics);

    void Maintain_Current_Position_Legs(Object *obj, PhysicsBehavior *physics)
    {
        Maintain_Current_Position_Other(obj, physics);
    }

    void Maintain_Current_Position_Wheels(Object *obj, PhysicsBehavior *physics)
    {
        Maintain_Current_Position_Other(obj, physics);
    }

    void Maintain_Current_Position_Treads(Object *obj, PhysicsBehavior *physics)
    {
        Maintain_Current_Position_Other(obj, physics);
    }

    float Calc_Min_Turn_Radius(BodyDamageType condition, float *time_to_travel_that_dist) const;
    float Calc_Lift_To_Use_At_Pt(
        Object *obj, PhysicsBehavior *physics, float cur_z, float surface_at_pt, float preferred_height);
    bool Fix_Invalid_Position(Object *obj, PhysicsBehavior *physics);
    void Set_Physics_Options(Object *obj);
    void Start_Move();
    bool Handle_Behavior_Z(Object *obj, PhysicsBehavior *physics, const Coord3D &goal_pos);
    PhysicsTurningType Rotate_Obj_Around_Loco_Pivot(Object *obj, const Coord3D &position, float rate, float *angle);
    PhysicsTurningType Rotate_Towards_Position(Object *obj, const Coord3D &position, float *angle);

    LocomotorAppearance Get_Appearance() const { return m_template->m_appearance; }
    float Get_Thrust_Roll() const { return m_template->m_thrustRoll; }
    float Get_Thrust_Wobble_Rate() const { return m_template->m_thrustWobbleRate; }
    float Get_Thrust_Max_Wobble() const { return m_template->m_thrustMaxWobble; }
    float Get_Thrust_Min_Wobble() const { return m_template->m_thrustMinWobble; }

    float Get_Accel_Pitch_Limit() const { return m_template->m_accelPitchLimit; }
    float Get_Deaccel_Pitch_Limit() const { return m_template->m_decelPitchLimit; }
    float Get_Pitch_Stiffness() const { return m_template->m_pitchStiffness; }
    float Get_Roll_Stiffness() const { return m_template->m_rollStiffness; }
    float Get_Pitch_Damping() const { return m_template->m_pitchDamping; }
    float Get_Roll_Damping() const { return m_template->m_rollDamping; }
    float Get_Pitch_In_Direction_Of_Z_Vel_Factor() const { return m_template->m_pitchInDirectionOfZVelFactor; }

    float Get_Forward_Vel_Coef() const { return m_template->m_forwardVelCoef; }
    float Get_Lateral_Vel_Coef() const { return m_template->m_lateralVelCoef; }
    float Get_Forward_Accel_Coef() const { return m_template->m_forwardAccelCoef; }
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

    float Get_Preferred_Height_Damping() const { return m_preferredHeightDamping; }
    int Get_Legal_Surfaces() const { return m_template->m_surfaces; }
    float Get_Min_Speed() const { return m_template->m_minSpeed; }
    float Get_Turn_Pivot_Offset() const { return m_template->m_turnPivotOffset; }
    int Get_Airborne_Targeting_Height() const { return m_template->m_airborneTargetingHeight; }
    Utf8String Get_Template_Name() const { return m_template->m_name; }
    float Get_Wander_Width_Factor() const { return m_template->m_wanderWidthFactor; }
    float Get_Wander_About_Point_Radius() const { return m_template->m_wanderAboutPointRadius; }
    LocomotorGroupMovementPriority Get_Group_Movement_Priority() const { return m_template->m_groupMovementPriority; }

    bool Get_Apply_2D_Friction_When_Airborne() const { return m_template->m_apply2DFrictionWhenAirborne; }
    bool Get_Allow_Motive_Force_While_Airborne() const { return m_template->m_allowMotiveForceWhileAirborne; }
    bool Get_Locomotor_Works_When_Dead() const { return m_template->m_locomotorWorksWhenDead; }
    bool Get_Stick_To_Ground() const { return m_template->m_stickToGround; }
    bool Get_Downhill_Only() const { return m_template->m_downhillOnly; }

    bool Get_Flag(LocoFlag flag) const { return ((1 << flag) & m_flags) != 0; }
    bool Is_Moving_Backwards() const { return Get_Flag(MOVING_BACKWARDS); }
    bool Is_Ultra_Accurate() const { return Get_Flag(ULTRA_ACCURATE); }
    bool Is_Close_Enough_Dist_3D() const { return Get_Flag(CLOSE_ENOUGH_DIST_3D); }
    bool Is_Allow_Invalid_Position() const { return Get_Flag(ALLOW_INVALID_POSITION); }

    void Set_Flag(LocoFlag f, bool set)
    {
        if (set) {
            m_flags |= 1 << f;
        } else {
            m_flags &= ~(1 << f);
        }
    }

    void Set_Use_Precise_Z_Pos(bool set) { Set_Flag(PRECISE_Z_POS, set); }
    void Set_Allow_Invalid_Position(bool set) { Set_Flag(ALLOW_INVALID_POSITION, set); }
    void Set_No_Slow_Down_As_Approaching_Dest(bool set) { Set_Flag(NO_SLOW_DOWN_AS_APPROACHING_DEST, set); }
    void Set_Ultra_Accurate(bool set) { Set_Flag(ULTRA_ACCURATE, set); }
    void Set_Close_Enough_Dist_3D(bool set) { Set_Flag(CLOSE_ENOUGH_DIST_3D, set); }
    void Set_Close_Enough_Dist(float dist) { m_closeEnoughDist = dist; }

private:
    Override<const LocomotorTemplate> m_template;
    Coord3D m_maintainPos;
    float m_brakingFactor;
    float m_maxLift;
    float m_maxSpeed;
    float m_maxAccel;
    float m_maxBraking;
    float m_maxTurnRate;
    float m_closeEnoughDist;
    unsigned int m_flags;
    float m_preferredHeight;
    float m_preferredHeightDamping;
    float m_wanderAngle; // not 100% confirmed
    float m_wanderLength; // not 100% confirmed
    unsigned int m_moveFrame; // not 100% confirmed
};

class LocomotorSet : public SnapShot
{
public:
    LocomotorSet();
    LocomotorSet(const LocomotorSet &that);

#ifdef GAME_DLL
    LocomotorSet *Hook_Ctor() { return new (this) LocomotorSet; }
    void Hook_Dtor() { LocomotorSet::~LocomotorSet(); }
#endif

    virtual ~LocomotorSet();
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Xfer_Self_And_Cur_Loco_Ptr(Xfer *xfer, Locomotor **loco);

    LocomotorSet &operator=(const LocomotorSet &that);
    void Add_Locomotor(const LocomotorTemplate *lt);
    void Clear();
    Locomotor *Find_Locomotor(int t);

    bool Get_Downhill_Only() { return m_downhillOnly; }
    int Get_Valid_Surfaces() const { return m_validLocomotorSurfaces; }

private:
    std::vector<Locomotor *> m_locomotors;
    int m_validLocomotorSurfaces;
    bool m_downhillOnly;
};

#ifdef GAME_DLL
extern LocomotorStore *&g_theLocomotorStore;
#else
extern LocomotorStore *g_theLocomotorStore;
#endif
