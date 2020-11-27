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
#include "module.h"

class AIUpdateInterface;
class BodyModuleInterface;
class BridgeBehaviorInterface;
class BridgeScaffoldBehaviorInterface;
class BridgeTowerBehaviorInterface;
class CaveInterface;
class CollideModuleInterface;
class ContainModuleInterface;
class CountermeasuresBehaviorInterface;
class CreateModuleInterface;
class DamageModuleInterface;
class DestroyModuleInterface;
class DieModuleInterface;
class DockUpdateInterface;
class ExitInterface;
class HordeUpdateInterface;
class LandMineInterface;
class OverchargeBehaviorInterface;
class ParkingPlaceBehaviorInterface;
class PowerPlantUpdateInterface;
class ProductionUpdateInterface;
class ProjectileUpdateInterface;
class RailedTransportDockUpdateInterface;
class RebuildHoleBehaviorInterface;
class SlavedUpdateInterface;
class SlowDeathBehaviorInterface;
class SpawnBehaviorInterface;
class SpecialPowerModuleInterface;
class SpecialPowerUpdateInterface;
class SpyVisionUpdate;
class StealthUpdate;
class TransportPassengerInterface;
class UpdateModuleInterface;
class UpgradeModuleInterface;

class BehaviorModuleData : public ModuleData
{
public:
    BehaviorModuleData() {}
    virtual ~BehaviorModuleData() {}

    static void Build_Field_Parse(MultiIniFieldParse &p) {}
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
    virtual CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() const = 0;
};

class BehaviorModule : public ObjectModule, public BehaviorModuleInterface
{
public:
    BehaviorModule(Thing *thing, ModuleData *module_data);
    virtual ~BehaviorModule() override;

    virtual StealthUpdate *Get_Steath();
    virtual SpyVisionUpdate *Get_Spy_Vision_Update();

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

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
    virtual CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() const override;
};
