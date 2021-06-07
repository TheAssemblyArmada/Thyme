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
#include "gamelogic.h"
#include "display.h"
#include "mapobject.h"
#include "object.h"
#include "thingfactory.h"
#include "updatemodule.h"
#include "xfer.h"

#ifndef GAME_DLL
GameLogic *g_theGameLogic;
#endif

void GameLogic::Set_Defaults(bool keep_obj_id)
{
    m_frame = 0;
    m_width = 64;
    m_height = 64;
    m_objList = nullptr;

    for (auto i = m_sleepingUpdateModules.begin(); i != m_sleepingUpdateModules.end(); i++) {
        (*i)->Set_Index_In_Logic(-1);
    }

    m_sleepingUpdateModules.clear();

    m_currentUpdateModule = nullptr;

    if (!keep_obj_id) {
        m_nextObjID = 1;
    }
}

void Handle_Name_Change(MapObject *obj)
{
    if (obj->Get_Name().Compare("AmericaTankLeopard") == 0) {
        obj->Set_Name("AmericaTankCrusader");
        obj->Set_Thing_Template(g_theThingFactory->Find_Template(obj->Get_Name()));
    }

    if (obj->Get_Name().Compare("AmericaVehicleHumVee") == 0) {
        obj->Set_Name("AmericaVehicleHumvee");
        obj->Set_Thing_Template(g_theThingFactory->Find_Template(obj->Get_Name()));
    }
}

bool GameLogic::Is_Intro_Movie_Playing()
{
    return m_startNewGame && g_theDisplay->Is_Movie_Playing();
}

int GameLogic::Rebalance_Parent_Sleepy_Update(int update)
{
    captainslog_dbgassert(update >= 0 && (unsigned)update < m_sleepingUpdateModules.size(), "bad sleepy idx");

    for (int i = ((update + 1) >> 1) - 1; i >= 0; i = ((i + 1) >> 1) - 1) {
        if (!Compare_Update_Modules(m_sleepingUpdateModules[i], m_sleepingUpdateModules[update])) {
            break;
        }

        UpdateModule *a = m_sleepingUpdateModules[i];
        UpdateModule *b = m_sleepingUpdateModules[update];
        m_sleepingUpdateModules[update] = a;
        m_sleepingUpdateModules[i] = b;
        a->Set_Index_In_Logic(update);
        b->Set_Index_In_Logic(i);
        update = i;
    }

    return update;
}

void GameLogic::Push_Sleepy_Update(UpdateModule *update)
{
    captainslog_dbgassert(update, "You may not pass null for sleepy update info");
    m_sleepingUpdateModules.push_back(update);
    update->Set_Index_In_Logic(m_sleepingUpdateModules.size() - 1);
    Rebalance_Parent_Sleepy_Update(m_sleepingUpdateModules.size() - 1);
}

UpdateModule *GameLogic::Peek_Sleepy_Update()
{
    UpdateModule *a = m_sleepingUpdateModules.front();
    captainslog_dbgassert(a->Get_Index_In_Logic(), "index mismatch: expected %d, got %d\n", 0, a->Get_Index_In_Logic());
    return a;
}

Object *GameLogic::Get_First_Object()
{
    return m_objList;
}

ObjectID GameLogic::Allocate_Object_ID()
{
    ObjectID id = (ObjectID)m_nextObjID;
    m_nextObjID++;
    return id;
}

bool GameLogic::Is_Game_Paused()
{
    return m_gamePaused;
}

void GameLogic::Process_Progress_Complete(int player_id)
{
    if (player_id >= 0 && player_id < 8) {
        if (m_progressComplete[player_id]) {
            captainslog_debug("GameLogic::processProgressComplete, playerId %d is marked true already yet we're trying to "
                              "mark him as true again\n",
                player_id);
        } else {
            captainslog_debug("Progress Complete for Player %d\n", player_id);
        }
        m_progressComplete[player_id] = true;
        Last_Heard_From(player_id);
    } else {
        captainslog_debug("GameLogic::processProgressComplete, Invalid playerid was passed in %d\n", player_id);
    }
}

void GameLogic::Last_Heard_From(int player_id)
{
    if (player_id >= 0 && player_id < 8) {
        m_progressCompleteTimeout[player_id] = rts::Get_Time();
    }
}

void GameLogic::Time_Out_Game_Start()
{
    captainslog_debug("We got the Force TimeOut Start Message\n");
    m_forceGameStartByTimeOut = true;
}

void GameLogic::Set_Buildable_Status_Override(ThingTemplate const *thing, BuildableStatus status)
{
    if (thing) {
        m_thingTemplateBuildableOverrides[thing->Get_Name()] = status;
    }
}

bool GameLogic::Find_Buildable_Status_Override(ThingTemplate const *thing, BuildableStatus &status) const
{
    if (!thing) {
        return false;
    }

    auto i = m_thingTemplateBuildableOverrides.find(thing->Get_Name());

    if (i != m_thingTemplateBuildableOverrides.end()) {
        status = i->second;
        return true;
    }

    return false;
}

void GameLogic::Set_Control_Bar_Override(Utf8String const &s, int i, CommandButton const *button)
{
    char str[256];
    str[0] = i + '0';
    strcpy(&str[1], s);
    m_controlBarOverrides[str] = button;
}

bool GameLogic::Find_Control_Bar_Override(Utf8String const &s, int i, CommandButton const *&button) const
{
    char str[256];
    str[0] = i + '0';
    strcpy(&str[1], s);
    auto j = m_controlBarOverrides.find(str);

    if (j != m_controlBarOverrides.end()) {
        button = j->second;
        return true;
    }

    return false;
}

void GameLogic::CRC_Snapshot(Xfer *xfer) {}

void GameLogic::Add_TOC_Entry(Utf8String name, unsigned short id)
{
    ObjectTOCEntry entry;
    entry.name = name;
    entry.id = id;
    m_objectTOCEntries.push_back(entry);
}
