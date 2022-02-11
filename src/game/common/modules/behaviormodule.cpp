/**
 * @file
 *
 * @author xezon
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
#include "behaviormodule.h"

BehaviorModule::BehaviorModule(Thing *thing, const ModuleData *module_data) : ObjectModule(thing, module_data) {}

BehaviorModule::~BehaviorModule() {}

void BehaviorModule::CRC_Snapshot(Xfer *xfer)
{
    ObjectModule::CRC_Snapshot(xfer);
}

void BehaviorModule::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    ObjectModule::Xfer_Snapshot(xfer);
}

void BehaviorModule::Load_Post_Process()
{
    ObjectModule::Load_Post_Process();
}

BodyModuleInterface *BehaviorModule::Get_Body()
{
    return nullptr;
}

CollideModuleInterface *BehaviorModule::Get_Collide()
{
    return nullptr;
}

ContainModuleInterface *BehaviorModule::Get_Contain()
{
    return nullptr;
}

CreateModuleInterface *BehaviorModule::Get_Create()
{
    return nullptr;
}

DamageModuleInterface *BehaviorModule::Get_Damage()
{
    return nullptr;
}

DestroyModuleInterface *BehaviorModule::Get_Destroy()
{
    return nullptr;
}

DieModuleInterface *BehaviorModule::Get_Die()
{
    return nullptr;
}

SpecialPowerModuleInterface *BehaviorModule::Get_Special_Power()
{
    return nullptr;
}

UpdateModuleInterface *BehaviorModule::Get_Update()
{
    return nullptr;
}

UpgradeModuleInterface *BehaviorModule::Get_Upgrade()
{
    return nullptr;
}

ParkingPlaceBehaviorInterface *BehaviorModule::Get_Parking_Place_Behavior_Interface()
{
    return nullptr;
}

RebuildHoleBehaviorInterface *BehaviorModule::Get_Rebuild_Hole_Behavior_Interface()
{
    return nullptr;
}

BridgeBehaviorInterface *BehaviorModule::Get_Bridge_Behavior_Interface()
{
    return nullptr;
}

BridgeTowerBehaviorInterface *BehaviorModule::Get_Bridge_Tower_Behavior_Interface()
{
    return nullptr;
}

BridgeScaffoldBehaviorInterface *BehaviorModule::Get_Bridge_Scaffold_Behavior_Interface()
{
    return nullptr;
}

OverchargeBehaviorInterface *BehaviorModule::Get_Overcharge_Behavior_Interface()
{
    return nullptr;
}

TransportPassengerInterface *BehaviorModule::Get_Transport_Passenger_Interface()
{
    return nullptr;
}

CaveInterface *BehaviorModule::Get_Cave_Interface()
{
    return nullptr;
}

LandMineInterface *BehaviorModule::Get_Land_Mine_Interface()
{
    return nullptr;
}

DieModuleInterface *BehaviorModule::Get_Eject_Pilot_Die_Interface()
{
    return nullptr;
}

ProjectileUpdateInterface *BehaviorModule::Get_Projectile_Update_Interface()
{
    return nullptr;
}

AIUpdateInterface *BehaviorModule::Get_AI_Update_Interface()
{
    return nullptr;
}

ExitInterface *BehaviorModule::Get_Update_Exit_Interface()
{
    return nullptr;
}

DockUpdateInterface *BehaviorModule::Get_Dock_Update_Interface()
{
    return nullptr;
}

RailedTransportDockUpdateInterface *BehaviorModule::Get_Railed_Transport_Dock_Update_Interface()
{
    return nullptr;
}

SlowDeathBehaviorInterface *BehaviorModule::Get_Slow_Death_Behavior_Interface()
{
    return nullptr;
}

SpecialPowerUpdateInterface *BehaviorModule::Get_Special_Power_Update_Interface()
{
    return nullptr;
}

SlavedUpdateInterface *BehaviorModule::Get_Slaved_Update_Interface()
{
    return nullptr;
}

ProductionUpdateInterface *BehaviorModule::Get_Production_Update_Interface()
{
    return nullptr;
}

HordeUpdateInterface *BehaviorModule::Get_Horde_Update_Interface()
{
    return nullptr;
}

PowerPlantUpdateInterface *BehaviorModule::Get_Power_Plant_Update_Interface()
{
    return nullptr;
}

SpawnBehaviorInterface *BehaviorModule::Get_Spawn_Behavior_Interface()
{
    return nullptr;
}

CountermeasuresBehaviorInterface *BehaviorModule::Get_Countermeasures_Behavior_Interface()
{
    return nullptr;
}

const CountermeasuresBehaviorInterface *BehaviorModule::Get_Countermeasures_Behavior_Interface() const
{
    return nullptr;
}

StealthUpdate *BehaviorModule::Get_Steath()
{
    return nullptr;
}

SpyVisionUpdate *BehaviorModule::Get_Spy_Vision_Update()
{
    return nullptr;
}
