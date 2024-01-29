/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Build Assistant
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
#include "subsysteminterface.h"
#include <list>

enum LegalBuildCode
{
    LBC_OK,
    LBC_RESTRICTED_TERRAIN,
    LBC_NOT_FLAT_ENOUGH,
    LBC_OBJECTS_IN_THE_WAY,
    LBC_NO_CLEAR_PATH,
    LBC_SHROUD,
    LBC_TOO_CLOSE_TO_SUPPLIES,
};

class ObjectSellInfo;
class Coord3D;
class Object;
class ThingTemplate;
class Player;

enum CanMakeType
{
    CAN_MAKE_SUCCESS,
    CAN_MAKE_IMPOSSIBLE,
    CAN_MAKE_NOT_ENOUGH_MONEY,
    CAN_MAKE_NOT_ENABLED,
    CAN_MAKE_QUEUE_FULL,
    CAN_MAKE_PARKING_FULL,
    CAN_MAKE_MAXIMUM_NUMBER,
};

class BuildAssistant : public SubsystemInterface
{
public:
    struct TileBuildInfo
    {
        int tiles_used;
        Coord3D *positions;
    };

    BuildAssistant();
    BuildAssistant *Hook_Ctor() { return new (this) BuildAssistant(); }

    virtual ~BuildAssistant() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual Object *Build_Object_Now(
        Object *constructor_object, const ThingTemplate *what, const Coord3D *pos, float angle, Player *owning_player);
    virtual void Build_Object_Line_Now(Object *constructor_object,
        const ThingTemplate *what,
        const Coord3D *start,
        const Coord3D *end,
        float angle,
        Player *owning_player);
    virtual LegalBuildCode Is_Location_Legal_To_Build(const Coord3D *pos,
        const ThingTemplate *what,
        float angle,
        unsigned int options,
        Object *constructor_object,
        Player *owning_player);
    virtual bool Is_Location_Clear_Of_Objects(const Coord3D *pos,
        const ThingTemplate *what,
        float angle,
        Object *constructor_object,
        unsigned int options,
        Player *owning_player);
    virtual void Add_Bibs(const Coord3D *pos, const ThingTemplate *what);
    virtual TileBuildInfo *Build_Tiled_Locations(const ThingTemplate *what,
        float angle,
        const Coord3D *start,
        const Coord3D *end,
        float tiling_size,
        int max_tiles,
        Object *constructor_object);
    virtual Coord3D *Get_Build_Locations();
    virtual bool Is_Line_Build_Template(const ThingTemplate *what);
    virtual CanMakeType Can_Make_Unit(Object *constructor_object, const ThingTemplate *what) const;
    virtual bool Is_Possible_To_Make_Unit(Object *constructor_object, const ThingTemplate *what) const;
    virtual void Sell_Object(Object *object);

    void Xfer_The_Sell_List(Xfer *xfer);

private:
    Coord3D *m_buildPositions;
    int m_buildPositionSize;
    std::list<ObjectSellInfo *> m_sellList;
};

#ifdef GAME_DLL
extern BuildAssistant *&g_theBuildAssistant;
#else
extern BuildAssistant *g_theBuildAssistant;
#endif
