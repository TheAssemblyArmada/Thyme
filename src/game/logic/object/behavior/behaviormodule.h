/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
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
#include "buildassistant.h"
#include "module.h"
#include "opencontain.h"

class AIUpdateInterface;
class BodyModuleInterface;
class BridgeBehaviorInterface;
class BridgeScaffoldBehaviorInterface;
class BridgeTowerBehaviorInterface;
class CaveInterface;
class CollideModuleInterface;
class ContainModuleInterface;
class DamageModuleInterface;
class DieModuleInterface;
class ExitInterface;
class HordeUpdateInterface;
class OverchargeBehaviorInterface;
class ParkingPlaceBehaviorInterface;
class PowerPlantUpdateInterface;
class RailedTransportDockUpdateInterface;
class RebuildHoleBehaviorInterface;
class SlowDeathBehaviorInterface;
class SpawnBehaviorInterface;
class SpecialPowerUpdateInterface;
class SpyVisionUpdate;
class StealthUpdate;
class TransportPassengerInterface;
class UpdateModuleInterface;
class UpgradeModuleInterface;
class WeaponTemplate;
class ParticleSystemTemplate;
class SpecialPowerTemplate;
class Waypoint;
class AudioEventRTS;

enum ProductionID
{
    INVALID_PRODUCTION_ID,
};

class DestroyModuleInterface
{
public:
    virtual void On_Destroy() = 0;
};

class DockUpdateInterface
{
public:
    virtual bool Is_Clear_To_Approach(const Object *docker) const = 0;
    virtual bool Reserve_Approach_Position(Object *docker, Coord3D *pos, int *index) = 0;
    virtual bool Advance_Approach_Position(Object *docker, Coord3D *pos, int *index) = 0;
    virtual bool Is_Clear_To_Enter(const Object *docker) const = 0;
    virtual bool Is_Clear_To_Advance(const Object *docker, int index) const = 0;
    virtual void Get_Enter_Position(Object *docker, Coord3D *pos) = 0;
    virtual void Get_Dock_Position(Object *docker, Coord3D *pos) = 0;
    virtual void Get_Exit_Position(Object *docker, Coord3D *pos) = 0;
    virtual void On_Approach_Reached(Object *docker) = 0;
    virtual void On_Enter_Reached(Object *docker) = 0;
    virtual void On_Dock_Reached(Object *docker) = 0;
    virtual void On_Exit_Reached(Object *docker) = 0;
    virtual bool Action(Object *docker, Object *obj) = 0;
    virtual void Cancel_Dock(Object *docker) = 0;
    virtual bool Is_Dock_Open() = 0;
    virtual void Set_Dock_Open(bool open) = 0;
    virtual void Set_Dock_Crippled(bool crippled) = 0;
    virtual bool Is_Allow_Passthrough_Type() = 0;
    virtual bool Is_Rally_Point_After_Dock_Type() = 0;
};

class SlavedUpdateInterface
{
public:
    virtual ObjectID Get_Slaver_ID() const = 0;
    virtual void On_Enslave(const Object *slaver) = 0;
    virtual void On_Slaver_Die(const DamageInfo *info) = 0;
    virtual void On_Slaver_Damage(const DamageInfo *info) = 0;
    virtual bool Is_Self_Tasking() const = 0;
};

class CountermeasuresBehaviorInterface
{
public:
    virtual void Report_Missile_For_Countermeasures(Object *obj) = 0;
    virtual ObjectID Calculate_Countermeasure_To_Divert_To(const Object &obj) = 0;
    virtual void Reload_Countermeasures() = 0;
    virtual bool Is_Active() const = 0;
};

class ProductionEntry : public MemoryPoolObject
{
    IMPLEMENT_POOL(ProductionEntry)

public:
    enum ProductionType
    {
        PRODUCTION_INVALID,
        PRODUCTION_UNIT,
        PRODUCTION_UPGRADE,
    };

    virtual ~ProductionEntry() override;
    ProductionID Get_Production_ID() const { return m_productionID; }
    const UpgradeTemplate *Get_Production_Upgrade() const { return m_upgradeToResearch; }
    const ThingTemplate *Get_Production_Object() const { return m_objectToProduce; }
    ProductionType Get_Type() const { return m_type; }
    float Get_Percent_Complete() const { return m_percentComplete; }

private:
    ProductionType m_type;
    union
    {
        ThingTemplate *m_objectToProduce;
        UpgradeTemplate *m_upgradeToResearch;
    };
    ProductionID m_productionID;
    float m_percentComplete;
    int m_framesUnderConstruction;
    int m_productionQuantity;
    int m_productionCount;
    int m_exitDoor;
    ProductionEntry *m_next;
    ProductionEntry *m_prev;
};

class ProductionUpdateInterface
{
public:
    virtual CanMakeType Can_Queue_Create_Unit(const ThingTemplate *unit_type) const = 0;
    virtual CanMakeType Can_Queue_Upgrade(const UpgradeTemplate *upgrade) const = 0;
    virtual ProductionID Request_Unique_Unit_ID() = 0;
    virtual bool Queue_Upgrade(const UpgradeTemplate *upgrade) = 0;
    virtual void Cancel_Upgrade(const UpgradeTemplate *upgrade) = 0;
    virtual bool Is_Upgrade_In_Queue(const UpgradeTemplate *upgrade) const = 0;
    virtual unsigned int Count_Unit_Type_In_Queue(const ThingTemplate *unit_type) const = 0;
    virtual bool Queue_Create_Unit(const ThingTemplate *unit_type, ProductionID production_id) = 0;
    virtual void Cancel_Unit_Create(ProductionID production_id) = 0;
    virtual void Cancel_All_Units_Of_Type(const ThingTemplate *unit_type) = 0;
    virtual void Cancel_And_Refund_All_Production() = 0;
    virtual unsigned int Get_Production_Count() const = 0;
    virtual const ProductionEntry *First_Production() const = 0;
    virtual const ProductionEntry *Next_Production(const ProductionEntry *production) const = 0;
    virtual void Set_Hold_Door_Open(ExitDoorType type, bool b) = 0;
    virtual CommandButton *Get_Special_Power_Construction_Command_Button() const = 0;
    virtual void Set_Special_Power_Construction_Command_Button(const CommandButton *button) = 0;
};

class SpecialPowerModuleInterface
{
public:
    virtual bool Is_Module_For_Power(const SpecialPowerTemplate *tmplate) const = 0;
    virtual bool Is_Ready() const = 0;
    virtual float Get_Percent_Ready() const = 0;
    virtual unsigned int Get_Ready_Frame() const = 0;
    virtual Utf8String Get_Power_Name() const = 0;
    virtual const SpecialPowerTemplate *Get_Special_Power_Template() const = 0;
    virtual ScienceType Get_Required_Science() const = 0;
    virtual void On_Special_Power_Creation() = 0;
    virtual void Set_Ready_Frame(unsigned int frame) = 0;
    virtual void Pause_Countdown(bool pause) = 0;
    virtual void Do_Special_Power(unsigned int i) = 0;
    virtual void Do_Special_Power_At_Object(Object *obj, unsigned int i) = 0;
    virtual void Do_Special_Power_At_Location(const Coord3D *pos, float f, unsigned int i) = 0;
    virtual void Do_Special_Power_Using_Waypoints(const Waypoint *waypoint, unsigned int i) = 0;
    virtual void Mark_Special_Power_Triggered(const Coord3D *pos) = 0;
    virtual void Start_Power_Recharge() = 0;
    virtual const AudioEventRTS &Get_Initiate_Sound() const = 0;
    virtual bool Is_Script_Only() const = 0;
    virtual ThingTemplate *Get_Reference_Thing_Template() const = 0;
};

class LandMineInterface
{
public:
    virtual void Set_Scoot_Params(Coord3D const &, Coord3D const &) = 0;
    virtual void Disarm() = 0;
};

class CreateModuleInterface
{
public:
    virtual void On_Create() = 0;
    virtual void On_Build_Complete() = 0;
    virtual bool Should_Do_On_Build_Complete() = 0;
};

class BehaviorModuleData : public ModuleData
{
public:
    BehaviorModuleData() {}
    virtual ~BehaviorModuleData() override {}

    static void Build_Field_Parse(MultiIniFieldParse &p) {}
};

class ProjectileUpdateInterface
{
public:
    virtual void Projectile_Launch_At_Object_Or_Position(const Object *victim,
        const Coord3D *victim_pos,
        const Object *launcher,
        WeaponSlotType slot,
        int ammo_index,
        const WeaponTemplate *det_weap,
        const ParticleSystemTemplate *particle) = 0;
    virtual void Projectile_Fire_At_Object_Or_Position(const Object *victim,
        const Coord3D *victim_pos,
        const WeaponTemplate *det_weap,
        const ParticleSystemTemplate *particle) = 0;
    virtual bool Projectile_Is_Armed() const = 0;
    virtual ObjectID Projectile_Get_Launcher_ID() const = 0;
    virtual bool Projectile_Handle_Collision(Object *obj) = 0;
    virtual void Set_Frames_Till_Countermeasure_Diversion_Occurs(unsigned int frames) = 0;
    virtual void Projectile_Now_Jammed() = 0;
};

class BehaviorModuleInterface
{
public:
    virtual BodyModuleInterface *Get_Body() = 0;
    virtual CollideModuleInterface *Get_Collide() = 0;
    virtual ContainModuleInterface *Get_Contain() = 0;
    virtual CreateModuleInterface *Get_Create() = 0;
    virtual DamageModuleInterface *Get_Damage() = 0;
    virtual DestroyModuleInterface *Get_Destroy() = 0;
    virtual DieModuleInterface *Get_Die() = 0;
    virtual SpecialPowerModuleInterface *Get_Special_Power() = 0;
    virtual UpdateModuleInterface *Get_Update() = 0;
    virtual UpgradeModuleInterface *Get_Upgrade() = 0;
    virtual ParkingPlaceBehaviorInterface *Get_Parking_Place_Behavior_Interface() = 0;
    virtual RebuildHoleBehaviorInterface *Get_Rebuild_Hole_Behavior_Interface() = 0;
    virtual BridgeBehaviorInterface *Get_Bridge_Behavior_Interface() = 0;
    virtual BridgeTowerBehaviorInterface *Get_Bridge_Tower_Behavior_Interface() = 0;
    virtual BridgeScaffoldBehaviorInterface *Get_Bridge_Scaffold_Behavior_Interface() = 0;
    virtual OverchargeBehaviorInterface *Get_Overcharge_Behavior_Interface() = 0;
    virtual TransportPassengerInterface *Get_Transport_Passenger_Interface() = 0;
    virtual CaveInterface *Get_Cave_Interface() = 0;
    virtual LandMineInterface *Get_Land_Mine_Interface() = 0;
    virtual DieModuleInterface *Get_Eject_Pilot_Die_Interface() = 0;
    virtual ProjectileUpdateInterface *Get_Projectile_Update_Interface() = 0;
    virtual AIUpdateInterface *Get_AI_Update_Interface() = 0;
    virtual ExitInterface *Get_Update_Exit_Interface() = 0;
    virtual DockUpdateInterface *Get_Dock_Update_Interface() = 0;
    virtual RailedTransportDockUpdateInterface *Get_Railed_Transport_Dock_Update_Interface() = 0;
    virtual SlowDeathBehaviorInterface *Get_Slow_Death_Behavior_Interface() = 0;
    virtual SpecialPowerUpdateInterface *Get_Special_Power_Update_Interface() = 0;
    virtual SlavedUpdateInterface *Get_Slaved_Update_Interface() = 0;
    virtual ProductionUpdateInterface *Get_Production_Update_Interface() = 0;
    virtual HordeUpdateInterface *Get_Horde_Update_Interface() = 0;
    virtual PowerPlantUpdateInterface *Get_Power_Plant_Update_Interface() = 0;
    virtual SpawnBehaviorInterface *Get_Spawn_Behavior_Interface() = 0;
    virtual CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() = 0;
    virtual const CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() const = 0;
};

class CollideModuleInterface
{
public:
    virtual void On_Collide(Object *other, Coord3D const *loc, Coord3D const *normal) = 0;
    virtual bool Would_Like_To_Collide_With(Object const *other) = 0;
    virtual bool Is_Hijacked_Vehicle_Crate_Collide() = 0;
    virtual bool Is_Sabotage_Building_Crate_Collide() = 0;
    virtual bool Is_Car_Bomb_Crate_Collide() = 0;
    virtual bool Is_Railroad() = 0;
    virtual bool Is_Salvage_Crate_Collide() = 0;
};

class BehaviorModule : public ObjectModule, public BehaviorModuleInterface
{
    IMPLEMENT_ABSTRACT_POOL(BehaviorModule)

protected:
    BehaviorModule(Thing *thing, const ModuleData *module_data);
    virtual ~BehaviorModule() override;

public:
    virtual StealthUpdate *Get_Steath();
    virtual SpyVisionUpdate *Get_Spy_Vision_Update();

    // Snapshot
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    //~Snapshot

    // BehaviorModuleInterface
    virtual BodyModuleInterface *Get_Body() override;
    virtual CollideModuleInterface *Get_Collide() override;
    virtual ContainModuleInterface *Get_Contain() override;
    virtual CreateModuleInterface *Get_Create() override;
    virtual DamageModuleInterface *Get_Damage() override;
    virtual DestroyModuleInterface *Get_Destroy() override;
    virtual DieModuleInterface *Get_Die() override;
    virtual SpecialPowerModuleInterface *Get_Special_Power() override;
    virtual UpdateModuleInterface *Get_Update() override;
    virtual UpgradeModuleInterface *Get_Upgrade() override;
    virtual ParkingPlaceBehaviorInterface *Get_Parking_Place_Behavior_Interface() override;
    virtual RebuildHoleBehaviorInterface *Get_Rebuild_Hole_Behavior_Interface() override;
    virtual BridgeBehaviorInterface *Get_Bridge_Behavior_Interface() override;
    virtual BridgeTowerBehaviorInterface *Get_Bridge_Tower_Behavior_Interface() override;
    virtual BridgeScaffoldBehaviorInterface *Get_Bridge_Scaffold_Behavior_Interface() override;
    virtual OverchargeBehaviorInterface *Get_Overcharge_Behavior_Interface() override;
    virtual TransportPassengerInterface *Get_Transport_Passenger_Interface() override;
    virtual CaveInterface *Get_Cave_Interface() override;
    virtual LandMineInterface *Get_Land_Mine_Interface() override;
    virtual DieModuleInterface *Get_Eject_Pilot_Die_Interface() override;
    virtual ProjectileUpdateInterface *Get_Projectile_Update_Interface() override;
    virtual AIUpdateInterface *Get_AI_Update_Interface() override;
    virtual ExitInterface *Get_Update_Exit_Interface() override;
    virtual DockUpdateInterface *Get_Dock_Update_Interface() override;
    virtual RailedTransportDockUpdateInterface *Get_Railed_Transport_Dock_Update_Interface() override;
    virtual SlowDeathBehaviorInterface *Get_Slow_Death_Behavior_Interface() override;
    virtual SpecialPowerUpdateInterface *Get_Special_Power_Update_Interface() override;
    virtual SlavedUpdateInterface *Get_Slaved_Update_Interface() override;
    virtual ProductionUpdateInterface *Get_Production_Update_Interface() override;
    virtual HordeUpdateInterface *Get_Horde_Update_Interface() override;
    virtual PowerPlantUpdateInterface *Get_Power_Plant_Update_Interface() override;
    virtual SpawnBehaviorInterface *Get_Spawn_Behavior_Interface() override;
    virtual CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() override;
    virtual const CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() const override;
    //~BehaviorModuleInterface
};
