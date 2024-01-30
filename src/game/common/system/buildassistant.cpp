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
#include "buildassistant.h"
#include "globaldata.h"
#include "xfer.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
BuildAssistant *g_theBuildAssistant;
#endif

BuildAssistant::BuildAssistant() : m_buildPositions(nullptr), m_buildPositionSize(0)
{
    m_sellList.clear();
}

BuildAssistant::~BuildAssistant()
{
    if (m_buildPositions != nullptr) {
        delete[] m_buildPositions;
        m_buildPositions = nullptr;
        m_buildPositionSize = 0;
    }
}

void BuildAssistant::Init()
{
    m_buildPositionSize = g_theWriteableGlobalData->m_maxLineBuildObjects;
    m_buildPositions = new Coord3D[m_buildPositionSize];
}

void BuildAssistant::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, BuildAssistant>(PICK_ADDRESS(0x004B3170, 0x009F876F), this);
#endif
}

void BuildAssistant::Update()
{
#ifdef GAME_DLL
    Call_Method<void, BuildAssistant>(PICK_ADDRESS(0x004B31E0, 0x009F87E5), this);
#endif
}

Object *BuildAssistant::Build_Object_Now(
    Object *constructor_object, const ThingTemplate *what, const Coord3D *pos, float angle, Player *owning_player)
{
#ifdef GAME_DLL
    return Call_Method<Object *, BuildAssistant, Object *, const ThingTemplate *, const Coord3D *, float, Player *>(
        PICK_ADDRESS(0x004B3590, 0x009F8C69), this, constructor_object, what, pos, angle, owning_player);
#else
    return nullptr;
#endif
}

void BuildAssistant::Build_Object_Line_Now(Object *constructor_object,
    const ThingTemplate *what,
    const Coord3D *start,
    const Coord3D *end,
    float angle,
    Player *owning_player)
{
#ifdef GAME_DLL
    Call_Method<void, BuildAssistant, Object *, const ThingTemplate *, const Coord3D *, const Coord3D *, float, Player *>(
        PICK_ADDRESS(0x004B38D0, 0x009F9019), this, constructor_object, what, start, end, angle, owning_player);
#endif
}

LegalBuildCode BuildAssistant::Is_Location_Legal_To_Build(const Coord3D *pos,
    const ThingTemplate *what,
    float angle,
    unsigned int options,
    Object *constructor_object,
    Player *owning_player)
{
#ifdef GAME_DLL
    return Call_Method<LegalBuildCode,
        BuildAssistant,
        const Coord3D *,
        const ThingTemplate *,
        float,
        unsigned int,
        Object *,
        Player *>(PICK_ADDRESS(0x004B4910, 0x009FA0AD), this, pos, what, angle, options, constructor_object, owning_player);
#else
    return LBC_OK;
#endif
}

bool BuildAssistant::Is_Location_Clear_Of_Objects(const Coord3D *pos,
    const ThingTemplate *what,
    float angle,
    Object *constructor_object,
    unsigned int options,
    Player *owning_player)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        BuildAssistant,
        const Coord3D *,
        const ThingTemplate *,
        float,
        Object *,
        unsigned int,
        Player *>(PICK_ADDRESS(0x004B3C10, 0x009F937F), this, pos, what, angle, constructor_object, options, owning_player);
#else
    return false;
#endif
}

void BuildAssistant::Add_Bibs(const Coord3D *pos, const ThingTemplate *what)
{
#ifdef GAME_DLL
    Call_Method<void, BuildAssistant, const Coord3D *, const ThingTemplate *>(
        PICK_ADDRESS(0x004B4FB0, 0x009FA759), this, pos, what);
#endif
}

BuildAssistant::TileBuildInfo *BuildAssistant::Build_Tiled_Locations(const ThingTemplate *what,
    float angle,
    const Coord3D *start,
    const Coord3D *end,
    float tiling_size,
    int max_tiles,
    Object *constructor_object)
{
#ifdef GAME_DLL
    return Call_Method<BuildAssistant::TileBuildInfo *,
        BuildAssistant,
        const ThingTemplate *,
        float,
        const Coord3D *,
        const Coord3D *,
        float,
        int,
        Object *>(
        PICK_ADDRESS(0x004B5110, 0x009FA87E), this, what, angle, start, end, tiling_size, max_tiles, constructor_object);
#else
    return nullptr;
#endif
}

Coord3D *BuildAssistant::Get_Build_Locations()
{
    return m_buildPositions;
}

bool BuildAssistant::Is_Line_Build_Template(const ThingTemplate *what)
{
#ifdef GAME_DLL
    return Call_Method<bool, BuildAssistant, const ThingTemplate *>(PICK_ADDRESS(0x004B52E0, 0x009FAA93), this, what);
#else
    return false;
#endif
}

CanMakeType BuildAssistant::Can_Make_Unit(Object *constructor_object, const ThingTemplate *what) const
{
#ifdef GAME_DLL
    return Call_Method<CanMakeType, const BuildAssistant, Object *, const ThingTemplate *>(
        PICK_ADDRESS(0x004B53E0, 0x009FAC20), this, constructor_object, what);
#else
    return CAN_MAKE_SUCCESS;
#endif
}

bool BuildAssistant::Is_Possible_To_Make_Unit(Object *constructor_object, const ThingTemplate *what) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const BuildAssistant, Object *, const ThingTemplate *>(
        PICK_ADDRESS(0x004B5300, 0x009FAAC3), this, constructor_object, what);
#else
    return false;
#endif
}

void BuildAssistant::Sell_Object(Object *object)
{
#ifdef GAME_DLL
    Call_Method<void, BuildAssistant, Object *>(PICK_ADDRESS(0x004B5930, 0x009FB1A0), this, object);
#endif
}

void BuildAssistant::Xfer_The_Sell_List(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, BuildAssistant, Xfer *>(PICK_ADDRESS(0x004B3430, 0x009F8A53), this, xfer);
#endif
}
