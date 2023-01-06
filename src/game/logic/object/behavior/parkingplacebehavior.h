/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Parking Place Behavior
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
#include "object.h"

enum RunwayReservationType
{
    RUNWAY_RESERVATION_UNK,
};

class ParkingPlaceBehaviorInterface
{
public:
    class PPInfo;

    virtual bool Should_Reserve_Door_When_Queued(const ThingTemplate *tmplate) const = 0;
    virtual bool Has_Available_Space_For(const ThingTemplate *tmplate) const = 0;
    virtual bool Has_Reserved_Space(ObjectID id) const = 0;
    virtual int Get_Space_Index(ObjectID id) const = 0;
    virtual bool Reserve_Space(ObjectID id, float f, PPInfo *info) = 0;
    virtual void Release_Space(ObjectID id) = 0;
    virtual bool Reserve_Runway(ObjectID id, bool b) = 0;
    virtual void Calc_PP_Info(ObjectID id, PPInfo *info) = 0;
    virtual void Release_Runway(ObjectID id) = 0;
    virtual int Get_Runway_Count() const = 0;
    virtual ObjectID Get_Runway_Reservation(int i, RunwayReservationType type) = 0;
    virtual void Transfer_Runway_Reservation_To_Next_In_Line_For_Takeoff(ObjectID id) = 0;
    virtual float Get_Approach_Height() const = 0;
    virtual float Get_Landing_Deck_Height_Offset() const = 0;
    virtual void Set_Healee(Object *obj, bool b) = 0;
    virtual void Kill_All_Parked_Units() = 0;
    virtual void Defect_All_Parked_Units(Team *team, unsigned int i) = 0;
    virtual bool Calc_Best_Parking_Assignment(ObjectID id, Coord3D *pos, int *i1, int *i2) = 0;
    virtual int Get_Taxi_Locations(ObjectID id) const = 0;
    virtual int Get_Creation_Locations(ObjectID id) const = 0;
};
