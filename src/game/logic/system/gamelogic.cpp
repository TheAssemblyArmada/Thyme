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
#include "ai.h"
#include "aipathfind.h"
#include "buddythread.h"
#include "cachedfileinputstream.h"
#include "campaignmanager.h"
#include "cavesystem.h"
#include "challengegenerals.h"
#include "commandlist.h"
#include "display.h"
#include "drawable.h"
#include "filesystem.h"
#include "fpusetting.h"
#include "gameclient.h"
#include "gameengine.h"
#include "gamelod.h"
#include "gamestate.h"
#include "gametext.h"
#include "gamewindowmanager.h"
#include "gamewindowtransitions.h"
#include "ghostobject.h"
#include "lanapiinterface.h"
#include "langameinfo.h"
#include "latchrestore.h"
#include "loadscreen.h"
#include "mapobject.h"
#include "maputil.h"
#include "multiplayersettings.h"
#include "network.h"
#include "object.h"
#include "peerdefs.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "polygontrigger.h"
#include "radar.h"
#include "recorder.h"
#include "rtsutils.h"
#include "scriptactions.h"
#include "scriptconditions.h"
#include "scriptengine.h"
#include "shell.h"
#include "sideslist.h"
#include "snow.h"
#include "staticnamekey.h"
#include "statscollector.h"
#include "team.h"
#include "terrainlogic.h"
#include "terrainvisual.h"
#include "thingfactory.h"
#include "threadutils.h"
#include "updatemodule.h"
#include "victoryconditions.h"
#include "view.h"
#include "water.h"
#include "windowlayout.h"
#include "xfer.h"
#include "xfercrc.h"

#ifndef GAME_DLL
GameLogic *g_theGameLogic;
#endif

GameLogic::GameLogic() :
    m_width(0.0f),
    m_height(0.0f),
    m_frame(0),
    m_crc(0),
    m_checkCRCs(false),
    m_prepareNewGame(false),
    m_loadingGameStateMap(false),
    m_clearingGameData(false),
    m_inGameLogicUpdate(false),
    m_rankPointsToAddAtGameStart(0),
    m_hulkLifetimeOverride(0),
    m_startNewGame(false),
    m_background(nullptr),
    m_objList(nullptr),
    m_currentUpdateModule(nullptr),
    m_nextObjID(INVALID_OBJECT_ID),
    m_gameMode(GAME_NONE),
    m_rankLevelLimit(1000),
    m_maxSimultaneousOfType(0),
    m_loadScreen(nullptr),
    m_gamePaused(false),
    m_inputEnabled(true),
    m_mouseVisible(true),
    m_forceGameStartByTimeOut(false),
#ifdef GAME_DEBUG_STRUCTS
    m_failedPathFinds(0),
#endif
    m_frameTriggerAreasChanged(0)
{
    memset(m_progressComplete, 0, sizeof(m_progressComplete));
    memset(m_progressCompleteTimeout, 0, sizeof(m_progressCompleteTimeout));
}

GameLogic::~GameLogic()
{
    m_objectTOCEntries.clear();

    if (m_background != nullptr) {
        m_background->Destroy_Windows();
        m_background->Delete_Instance();
    }

    Destroy_All_Objects_Immediate();

    if (g_theTerrainLogic != nullptr) {
        delete g_theTerrainLogic;
        g_theTerrainLogic = nullptr;
    }

    if (g_theGhostObjectManager != nullptr) {
        delete g_theGhostObjectManager;
        g_theGhostObjectManager = nullptr;
    }

    if (g_thePartitionManager != nullptr) {
        delete g_thePartitionManager;
        g_thePartitionManager = nullptr;
    }

    if (g_theScriptActions != nullptr) {
        delete g_theScriptActions;
        g_theScriptActions = nullptr;
    }

    if (g_theScriptConditions != nullptr) {
        delete g_theScriptConditions;
        g_theScriptConditions = nullptr;
    }

    if (g_theScriptEngine != nullptr) {
        delete g_theScriptEngine;
        g_theScriptEngine = nullptr;
    }

    g_theGameLogic = nullptr;
}

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
        m_nextObjID = ObjectID(1);
    }
}

void Handle_Name_Change(MapObject *obj)
{
    if (obj->Get_Name().Compare("AmericaTankLeopard") == 0) {
        obj->Set_Name("AmericaTankCrusader");
        obj->Set_Thing_Template(g_theThingFactory->Find_Template(obj->Get_Name(), true));
    }

    if (obj->Get_Name().Compare("AmericaVehicleHumVee") == 0) {
        obj->Set_Name("AmericaVehicleHumvee");
        obj->Set_Thing_Template(g_theThingFactory->Find_Template(obj->Get_Name(), true));
    }
}

bool GameLogic::Is_Intro_Movie_Playing()
{
    return m_startNewGame && g_theDisplay->Is_Movie_Playing();
}

int GameLogic::Rebalance_Parent_Sleepy_Update(int index)
{
    captainslog_dbgassert(index >= 0 && (size_t)index < m_sleepingUpdateModules.size(), "bad sleepy idx");

    for (int i = ((index + 1) >> 1) - 1; i >= 0; i = ((i + 1) >> 1) - 1) {

        captainslog_dbgassert((size_t)i < m_sleepingUpdateModules.size(), "bad idx");

        UpdateModule *a = m_sleepingUpdateModules[i];
        UpdateModule *b = m_sleepingUpdateModules[index];

        if (!UpdateModule::Compare_Update_Modules(a, b)) {
            break;
        }

        m_sleepingUpdateModules[index] = a;
        m_sleepingUpdateModules[i] = b;
        a->Set_Index_In_Logic(index);
        b->Set_Index_In_Logic(i);
        index = i;
    }

    return index;
}

int GameLogic::Rebalance_Child_Sleepy_Update(int index)
{
    captainslog_assert(!m_sleepingUpdateModules.empty());

    captainslog_dbgassert(index >= 0 && (size_t)index < m_sleepingUpdateModules.size(), "bad sleepy idx");

    int next_index = 2 * index + 1;

    // #BUGFIX Original dereferenced elements past end, which is undefined behavior.
    // Here we calculate pointers which are safe to go past the end and beyond.
    UpdateModule **curr = &m_sleepingUpdateModules[0] + index;
    UpdateModule **next = &m_sleepingUpdateModules[0] + next_index;
    UpdateModule **end = &m_sleepingUpdateModules[0] + m_sleepingUpdateModules.size();

    while (next < end) {
        if (next < end - 1 && UpdateModule::Compare_Update_Modules(next[0], next[1])) {
            ++next;
            ++next_index;
        }
        if (!UpdateModule::Compare_Update_Modules(*curr, *next)) {
            break;
        }
        UpdateModule *n = *next;
        UpdateModule *c = *curr;
        *curr = n;
        *next = c;
        n->Set_Index_In_Logic(index);
        c->Set_Index_In_Logic(next_index);

        index = next_index;
        curr = next;
        next_index = 2 * next_index + 1;
        next = &m_sleepingUpdateModules[0] + next_index;
    }
    return index;
}

void GameLogic::Rebalance_Sleepy_Update(int index)
{
    int parent_index = Rebalance_Parent_Sleepy_Update(index);
    Rebalance_Child_Sleepy_Update(parent_index);
}

void GameLogic::Push_Sleepy_Update(UpdateModule *module)
{
    captainslog_dbgassert(module != nullptr, "You may not pass null for sleepy update info");

    m_sleepingUpdateModules.push_back(module);
    module->Set_Index_In_Logic(m_sleepingUpdateModules.size() - 1);
    Rebalance_Parent_Sleepy_Update(m_sleepingUpdateModules.size() - 1);
}

UpdateModule *GameLogic::Peek_Sleepy_Update()
{
    UpdateModule *module = m_sleepingUpdateModules.front();
    captainslog_dbgassert(
        0 == module->Get_Index_In_Logic(), "index mismatch: expected %d, got %d", 0, module->Get_Index_In_Logic());
    return module;
}

void GameLogic::Friend_Awaken_Update_Module(Object *object, UpdateModule *module, unsigned int wakeup_frame)
{
    unsigned int cur_frame = g_theGameLogic->Get_Frame();

    captainslog_dbgassert(wakeup_frame >= cur_frame, "Set_Wake_Frame frame is in the past");

    if (module == m_currentUpdateModule) {
        captainslog_dbgassert(false,
            "Set_Wake_Frame() should not be called from inside the Update(), because it will be ignored, in favor "
            "of the return code from update");

    } else if (wakeup_frame != module->Decode_Frame()
        && (0 == cur_frame || module->Decode_Frame() != cur_frame || wakeup_frame != cur_frame + 1)) {

        int index = module->Get_Index_In_Logic();

        if (object->Is_In_List(&m_objList)) {
            if (index < 0 || static_cast<size_t>(index) >= m_sleepingUpdateModules.size()) {
                captainslog_fatal("fatal error! sleepy update module illegal index.");
            } else {
                if (m_sleepingUpdateModules[index] != module) {
                    captainslog_fatal("fatal error! sleepy update module index mismatch.");
                } else {
                    module->Encode_Frame(wakeup_frame);
                    Rebalance_Sleepy_Update(index);
                }
            }
        } else if (index != -1) {
            captainslog_fatal("fatal error! sleepy update module index mismatch.");
        } else {
            module->Encode_Frame(wakeup_frame);
        }
    }
}

Object *GameLogic::Get_First_Object()
{
    return m_objList;
}

ObjectID GameLogic::Allocate_Object_ID()
{
    ObjectID id = m_nextObjID;
    m_nextObjID++;
    return id;
}

bool GameLogic::Is_Game_Paused()
{
    return m_gamePaused;
}

void GameLogic::Process_Progress_Complete(int player_id)
{
    if (player_id >= 0 && player_id < GameInfo::MAX_SLOTS) {
        if (m_progressComplete[player_id]) {
            captainslog_debug("GameLogic::Process_Progress_Complete, playerId %d is marked true already yet we're trying to "
                              "mark him as true again\n",
                player_id);
        } else {
            captainslog_debug("Progress Complete for Player %d\n", player_id);
        }
        m_progressComplete[player_id] = true;
        Last_Heard_From(player_id);
    } else {
        captainslog_debug("GameLogic::Process_Progress_Complete, Invalid playerid was passed in %d\n", player_id);
    }
}

void GameLogic::Last_Heard_From(int player_id)
{
    if (player_id >= 0 && player_id < GameInfo::MAX_SLOTS) {
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
    strcpy(&str[1], s.Str());
    m_controlBarOverrides[str] = button;
}

bool GameLogic::Find_Control_Bar_Override(Utf8String const &s, int i, CommandButton const *&button) const
{
    char str[256];
    str[0] = i + '0';
    strcpy(&str[1], s.Str());
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

unsigned int GameLogic::Get_Object_Count()
{
    int count = 0;

    for (Object *o = Get_First_Object(); o != nullptr; o = o->Get_Next_Object()) {
        count++;
    }

    return count;
}

void GameLogic::Destroy_Object(Object *obj)
{
    captainslog_dbgassert(obj != nullptr, "destroying null object");

    if (obj != nullptr && !obj->Is_Destroyed()) {
        for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
            DestroyModuleInterface *destroy = (*module)->Get_Destroy();

            if (destroy != nullptr) {
                destroy->On_Destroy();
            }
        }

        obj->Set_Status(BitFlags<OBJECT_STATUS_COUNT>(BitFlags<OBJECT_STATUS_COUNT>::kInit, OBJECT_STATUS_DESTROYED), true);
        AIUpdateInterface *update = obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            update->Set_Locomotor_Goal_None();
            update->Destroy_Path();
        }

        m_objectsToDestroy.push_back(obj);
        obj->On_Destroy();

        if (obj->Is_KindOf(KINDOF_WALK_ON_TOP_OF_WALL)) {
            g_theAI->Get_Pathfinder()->Remove_Wall_Piece(obj);
        }

        if (obj->Has_Any_Special_Power()) {
            if (g_thePlayerList->Get_Local_Player() == obj->Get_Controlling_Player()) {
                g_theControlBar->Mark_UI_Dirty();
            }
        }
    }
}

Object *GameLogic::Friend_Create_Object(ThingTemplate const *thing, BitFlags<OBJECT_STATUS_COUNT> &status_bits, Team *team)
{
    return new Object(thing, status_bits, team);
}

bool GameLogic::Is_In_Single_Player_Game()
{
    return m_gameMode == GAME_SINGLE_PLAYER
        || (g_theRecorder != nullptr && g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK
            && g_theRecorder->Get_Org_Game_Mode() == GAME_SINGLE_PLAYER);
}

void GameLogic::Remove_Object_From_Lookup_Table(Object *obj)
{
    if (obj != nullptr) {
        m_objectLookupTable[obj->Get_ID()] = nullptr;
    }
}

void GameLogic::Add_Object_To_Lookup_Table(Object *obj)
{
    if (obj != nullptr) {
        ObjectID id = obj->Get_ID();

        for (;;) {
            if (id < m_objectLookupTable.size()) {
                break;
            }

            m_objectLookupTable.resize(m_objectLookupTable.size() * 2, nullptr);
        }

        m_objectLookupTable[id] = obj;
    }
}

void GameLogic::Register_Object(Object *obj)
{
    obj->Prepend_To_List(&m_objList);
    Add_Object_To_Lookup_Table(obj);
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (frame == 0) {
        frame = 1;
    }

    for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
        UpdateModuleInterface *update_interface = (*module)->Get_Update();
        UpdateModule *update;

        if (update_interface != nullptr) {
            update = static_cast<UpdateModule *>(update_interface);
        } else {
            update = nullptr;
        }

        if (update != nullptr) {
            if (update->Decode_Frame() == 0) {
                update->Encode_Frame(frame);
            }

            captainslog_dbgassert(update->Decode_Frame() >= frame,
                "you may not specify a zero initial sleep time for sleepy modules (%d %d)",
                update->Decode_Frame(),
                frame);
            Push_Sleepy_Update(update);
        }
    }
}

void GameLogic::Send_Object_Destroyed(Object *obj)
{
    if (g_theGameClient != nullptr) {
        Drawable *drawable = obj->Get_Drawable();

        if (drawable != nullptr) {
            g_theGameClient->Destroy_Drawable(drawable);
        }

        obj->Friend_Bind_To_Drawable(nullptr);
    }
}

void GameLogic::Send_Object_Created(Object *obj)
{
    Bind_Object_And_Drawable(obj, g_theThingFactory->New_Drawable(obj->Get_Template(), DRAWABLE_STATUS_UNK));
}

void GameLogic::Bind_Object_And_Drawable(Object *obj, Drawable *d)
{
    d->Friend_Bind_To_Object(obj);
    obj->Friend_Bind_To_Drawable(d);
}

void GameLogic::Deselect_Object(Object *obj, unsigned short player_mask, bool deselect_ui)
{
    if (obj != nullptr) {
        while (player_mask != 0) {
            Player *player = g_thePlayerList->Get_Each_Player_From_Mask(player_mask);

            if (player == nullptr) {
                break;
            }

#ifdef GAME_DEBUG_STRUCTS
            // TODO unknown CRC stuff
#endif
            AIGroup *group = g_theAI->Create_Group();
            player->Get_Current_Selection_As_AI_Group(group);
            bool is_empty_group = false;
            bool is_in_group = false;

            if (group != nullptr) {
                is_empty_group = group->Remove(obj);
                is_in_group = true;
            }

            if (is_in_group) {
                if (is_empty_group) {
                    player->Set_Currently_Selected_AIGroup(nullptr);
                } else {
                    player->Set_Currently_Selected_AIGroup(group);
                    g_theAI->Destroy_Group(group);
                }
            }

            if (deselect_ui) {
                Drawable *drawable = obj->Get_Drawable();

                if (drawable != nullptr) {
                    g_theInGameUI->Deselect_Drawable(drawable);
                }
            }
        }
    }
}

TerrainLogic *GameLogic::Create_Terrain_Logic()
{
    return new TerrainLogic();
}

GhostObjectManager *GameLogic::Create_Ghost_Object_Manager()
{
    return new GhostObjectManager();
}

void GameLogic::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 10;
    xfer->xferVersion(&version, 10);
    xfer->xferUnsignedInt(&m_frame);
    Xfer_Object_TOC(xfer);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Prepare_Logic_For_Object_Load();
    }

    unsigned int obj_count = Get_Object_Count();
    xfer->xferUnsignedInt(&obj_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (Object *obj = Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
            ObjectTOCEntry *toc = Find_TOC_Entry_By_Name(obj->Get_Template()->Get_Name());
            captainslog_relassert(toc != nullptr,
                CODE_06,
                "GameLogic::Xfer_Snapshot - Object TOC entry not found for '%s'",
                obj->Get_Template()->Get_Name().Str());
            xfer->xferUnsignedShort(&toc->id);
            xfer->Begin_Block();
            xfer->xferSnapshot(obj);
            xfer->End_Block();
        }
    } else {
        Team *team = g_thePlayerList->Get_Neutral_Player()->Get_Default_Team();

        for (unsigned int i = 0; i < obj_count; i++) {
            unsigned short id;
            xfer->xferUnsignedShort(&id);
            ObjectTOCEntry *toc = Find_TOC_Entry_By_ID(id);
            captainslog_relassert(
                toc != nullptr, CODE_06, "GameLogic::Xfer_Snapshot - Object TOC entry not found for '%s'", toc->name.Str());
            int offset = xfer->Begin_Block();
            ThingTemplate *tmplate = g_theThingFactory->Find_Template(toc->name, true);

            if (tmplate != nullptr) {
                Object *obj = g_theThingFactory->New_Object(tmplate, team, OBJECT_STATUS_MASK_NONE);
                xfer->xferSnapshot(obj);
                xfer->End_Block();

                if (obj->Is_KindOf(KINDOF_WALK_ON_TOP_OF_WALL)) {
                    g_theAI->Get_Pathfinder()->Add_Wall_Piece(obj);
                }
            } else {
                captainslog_dbgassert(false,
                    "GameLogic::Xfer_Snapshot - Unrecognized thing template name '%s', skipping.  ENGINEERS - Are you "
                    "*sure* it's OK "
                    "to be ignoring this object from the save file???  Think hard about it!",
                    toc->name.Str());
                xfer->Skip(offset);
            }
        }
    }

    xfer->xferSnapshot(g_theCampaignManager);
    xfer->xferSnapshot(g_theCaveSystem);

    if (version >= 2) {
        xfer->xferBool(&m_scoringEnabled);
    }

    if (version >= 3) {
        unsigned int trigger_count = 0;

        for (PolygonTrigger *trigger = PolygonTrigger::Get_First_Polygon_Trigger(); trigger != nullptr;
             trigger = trigger->Get_Next()) {
            trigger_count++;
        }

        unsigned int map_trigger_count = trigger_count;
        xfer->xferUnsignedInt(&trigger_count);
        captainslog_relassert(map_trigger_count == trigger_count,
            CODE_06,
            "GameLogic::Xfer_Snapshot - Polygon trigger count mismatch.  Save file has a count of '%d', but map had '%d' "
            "triggers",
            map_trigger_count,
            trigger_count);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (PolygonTrigger *trigger = PolygonTrigger::Get_First_Polygon_Trigger(); trigger != nullptr;
                 trigger = trigger->Get_Next()) {
                int trigger_id = trigger->Get_ID();
                xfer->xferInt(&trigger_id);
                xfer->xferSnapshot(trigger);
            }
        } else {
            for (unsigned int i = 0; i < trigger_count; i++) {
                int trigger_id;
                xfer->xferInt(&trigger_id);
                PolygonTrigger *trigger = PolygonTrigger::Get_Polygon_Trigger_By_ID(trigger_id);
                captainslog_relassert(trigger != nullptr,
                    CODE_06,
                    "GameLogic::Xfer_Snapshot - Unable to find polygon trigger with id '%d'",
                    trigger_id);
                xfer->xferSnapshot(trigger);
            }

            g_theAI->Get_Pathfinder()->New_Map();
        }
    }

    if (version >= 5) {
        xfer->xferInt(&m_rankLevelLimit);
    }

    if (version >= 6) {
        g_theBuildAssistant->Xfer_The_Sell_List(xfer);
    }

    if (version >= 7) {
        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto it = m_thingTemplateBuildableOverrides.begin(); it != m_thingTemplateBuildableOverrides.end(); it++) {
                Utf8String name = it->first;
                BuildableStatus status = it->second;
                xfer->xferAsciiString(&name);
                xfer->xferUser(&status, sizeof(status));
            }

            Utf8String empty_end_str;
            xfer->xferAsciiString(&empty_end_str);
        } else if (xfer->Get_Mode() == XFER_LOAD) {
            captainslog_relassert(m_thingTemplateBuildableOverrides.empty(),
                CODE_06,
                "GameLogic::Xfer_Snapshot - m_thingTemplateBuildableOverrides should be empty, but is not");

            for (;;) {
                Utf8String override_name;
                xfer->xferAsciiString(&override_name);

                if (override_name.Is_Empty()) {
                    break;
                }

                BuildableStatus status;
                xfer->xferUser(&status, sizeof(status));
                m_thingTemplateBuildableOverrides[override_name] = status;
            }
        }
    }

    if (version >= 8) {
        xfer->xferBool(&m_occlusionEnabled);
        xfer->xferBool(&m_drawIconUI);
        xfer->xferBool(&m_dynamicLOD);
        xfer->xferInt(&m_hulkLifetimeOverride);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto it = m_controlBarOverrides.begin(); it != m_controlBarOverrides.end(); it++) {
                Utf8String override_name = it->first;
                Utf8String button_name;

                if (it->second != nullptr) {
                    button_name = it->second->Get_Name();
                } else {
                    button_name = Utf8String::s_emptyString;
                }

                xfer->xferAsciiString(&override_name);
                xfer->xferAsciiString(&button_name);
            }

            Utf8String empty_end_str;
            xfer->xferAsciiString(&empty_end_str);
        } else if (xfer->Get_Mode() == XFER_LOAD) {
            captainslog_relassert(m_controlBarOverrides.empty(),
                CODE_06,
                "GameLogic::Xfer_Snapshot - m_controlBarOverrides should be empty, but is not");

            for (;;) {
                Utf8String name;
                xfer->xferAsciiString(&name);

                if (name.Is_Empty()) {
                    break;
                }

                Utf8String button_name;
                xfer->xferAsciiString(&button_name);
                const CommandButton *button = nullptr;

                if (button_name.Is_Not_Empty()) {
                    button = g_theControlBar->Find_Command_Button(button_name);
                    captainslog_dbgassert(button != nullptr, "Could not find button %s", button_name.Str());
                }

                m_controlBarOverrides[name] = button;
            }
        }
    }

    if (version >= 9) {
        xfer->xferInt(&m_rankPointsToAddAtGameStart);
    }

    if (version < 10) {
        if (xfer->Get_Mode() == XFER_LOAD) {
            m_maxSimultaneousOfType = 0;
        }
    } else {
        xfer->xferUnsignedShort(&m_maxSimultaneousOfType);
    }
}

GameLogic::ObjectTOCEntry *GameLogic::Find_TOC_Entry_By_ID(unsigned short id)
{
    for (auto it = m_objectTOCEntries.begin(); it != m_objectTOCEntries.end(); it++) {
        if ((*it).id == id) {
            return &*it;
        }
    }

    return nullptr;
}

GameLogic::ObjectTOCEntry *GameLogic::Find_TOC_Entry_By_Name(Utf8String name)
{
    for (auto it = m_objectTOCEntries.begin(); it != m_objectTOCEntries.end(); it++) {
        if ((*it).name == name) {
            return &*it;
        }
    }

    return nullptr;
}

void GameLogic::Init()
{
    Set_FP_Mode();
    Set_Defaults(false);
    g_thePartitionManager = new PartitionManager();
    g_thePartitionManager->Init();
    g_thePartitionManager->Set_Name("ThePartitionManager");
    g_theGhostObjectManager = Create_Ghost_Object_Manager();
    g_theTerrainLogic = Create_Terrain_Logic();
    g_theTerrainLogic->Set_Name("TheTerrainLogic");
    g_theScriptActions = new ScriptActions();
    g_theScriptConditions = new ScriptConditions();
    g_theScriptEngine = new ScriptEngine();
    g_theScriptEngine->Init();
    g_theScriptEngine->Set_Name("TheScriptEngine");
    m_crc = 0;
    m_gamePaused = false;
    m_inputEnabled = true;
    m_mouseVisible = true;
    memset(m_progressComplete, 0, sizeof(m_progressComplete));
    memset(m_progressCompleteTimeout, 0, sizeof(m_progressCompleteTimeout));
    m_forceGameStartByTimeOut = false;
    m_scoringEnabled = true;
    m_occlusionEnabled = true;
    m_drawIconUI = true;
    m_dynamicLOD = true;
    m_hulkLifetimeOverride = -1;
    m_inGameLogicUpdate = false;
    m_rankPointsToAddAtGameStart = 0;
}

void GameLogic::Load_Post_Process()
{
    m_nextObjID = INVALID_OBJECT_ID;

    for (Object *obj = Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
        if (obj->Get_ID() >= m_nextObjID) {
            unsigned int next_id = static_cast<unsigned int>(obj->Get_ID()) + 1;
            m_nextObjID = static_cast<ObjectID>(next_id);
        }
    }

    for (auto it = m_sleepingUpdateModules.begin(); it != m_sleepingUpdateModules.end(); it++) {
        (*it)->Set_Index_In_Logic(-1);
    }

    m_sleepingUpdateModules.clear();
    unsigned int frame = Get_Frame();

    if (frame == 0) {
        frame = 1;
    }

    for (Object *obj = Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
        for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
            UpdateModuleInterface *update_interface = (*module)->Get_Update();
            UpdateModule *update = static_cast<UpdateModule *>(update_interface);

            if (update != nullptr) {
                captainslog_dbgassert(update->Get_Index_In_Logic() == -1, "Hmm, expected index to be -1 here");

                if (update->Decode_Frame() == 0) {
                    update->Encode_Frame(frame);
                }

                m_sleepingUpdateModules.push_back(update);
                update->Set_Index_In_Logic(m_sleepingUpdateModules.size() - 1);
            }
        }
    }

    Remake_Sleepy_Update();
}

void GameLogic::Reset()
{
    m_thingTemplateBuildableOverrides.clear();
    m_controlBarOverrides.clear();
    m_objectLookupTable.clear();
    m_objectLookupTable.resize(0x2000);
    m_gamePaused = false;
    m_inputEnabled = true;
    m_mouseVisible = true;
    Set_FP_Mode();
    Destroy_All_Objects_Immediate();
    m_nextObjID = static_cast<ObjectID>(1);
    m_frameTriggerAreasChanged = 0;
    g_theGhostObjectManager->Reset();
    g_thePartitionManager->Reset();
    g_theTerrainLogic->Reset();
    g_theAI->Reset();
    g_theScriptEngine->Reset();
    m_crc = 0;
    memset(m_progressComplete, 0, sizeof(m_progressComplete));
    memset(m_progressCompleteTimeout, 0, sizeof(m_progressCompleteTimeout));
    m_forceGameStartByTimeOut = false;

    if (g_theStatsCollector != nullptr) {
        delete g_theStatsCollector;
        g_theStatsCollector = nullptr;
    }

    m_objectTOCEntries.clear();
    Set_Defaults(false);
    m_scoringEnabled = true;
    m_occlusionEnabled = true;
    m_drawIconUI = true;
    m_dynamicLOD = true;
    m_hulkLifetimeOverride = -1;
    WaterTransparencySetting *w = g_theWaterTransparency;
    g_theWaterTransparency = static_cast<WaterTransparencySetting *>(w->Delete_Overrides());
    WeatherSetting *w2 = g_theWeatherSetting;
    g_theWeatherSetting = static_cast<WeatherSetting *>(w2->Delete_Overrides());
    m_rankPointsToAddAtGameStart = 0;
}

void GameLogic::Update()
{
    bool in_game_logic_update = true;
    LatchRestore<bool> latch(&m_inGameLogicUpdate, &in_game_logic_update);
    Set_FP_Mode();

    if (m_startNewGame && !g_theDisplay->Is_Movie_Playing()) {
        Start_New_Game(false);
        m_startNewGame = false;
#ifdef GAME_DEBUG_STRUCTS
        // TODO processor frequency stuff
#endif
    }

    captainslog_dbgassert(g_theGameLogic == this, "hmm, TheGameLogic is not right");
    unsigned int frame = g_theGameLogic->Get_Frame();
    g_theGameClient->Set_Frame(frame);
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theScriptEngine->Update();

    if ((g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished())
        || g_theScriptEngine->Is_Time_Frozen_Debug() || g_theScriptEngine->Is_Time_Frozen_Script()) {
        if (!g_theCommandList->Contains_Message_Of_Type(GameMessage::MSG_CLEAR_GAME_DATA)) {
            return;
        }

        g_theScriptEngine->Force_Unfreeze_Time();
    }

#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theTerrainLogic->Update();
    bool is_recorded_multiplayer_game = g_theRecorder != nullptr && g_theRecorder->Is_Multiplayer()
        && Get_Game_Mode() != GAME_SHELL && Get_Game_Mode() != GAME_NONE;
    bool is_recorded_single_player_game = g_theRecorder != nullptr && !g_theRecorder->Is_Multiplayer()
        && Get_Game_Mode() != GAME_SHELL && Get_Game_Mode() != GAME_NONE;
    bool is_multiplayer_crc_frame = is_recorded_multiplayer_game && (m_frame % g_theGameInfo->Get_CRC_Interval()) == 0;
#ifdef GAME_DEBUG_STRUCTS
    // TODO unknown CRC stuff
#endif
    bool is_single_player_crc_frame = is_recorded_single_player_game && (m_frame % g_replayCRCInterval) == 0;

    if (is_single_player_crc_frame || is_multiplayer_crc_frame) {
        m_crc = Get_CRC(1, Utf8String::s_emptyString);

        if (is_recorded_multiplayer_game) {
            GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_LOGIC_CRC);
            message->Append_Int_Arg(m_crc);
            message->Append_Bool_Arg(g_theRecorder != nullptr && g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK);
        } else {
            GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_LOGIC_CRC);
            message->Append_Int_Arg(m_crc);
            message->Append_Bool_Arg(g_theRecorder != nullptr && g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK);
        }
    }

    if (g_theStatsCollector != nullptr) {
        g_theStatsCollector->Update();
    }

#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theRecorder->Update();
    Process_Command_List(g_theCommandList);

    while (!m_sleepingUpdateModules.empty()) {
        UpdateModule *module = Peek_Sleepy_Update();

        if (module != nullptr) {
            if (module->Decode_Frame() > frame) {
                break;
            }

            UpdateSleepTime sleep_time = UPDATE_SLEEP_TIME_MIN;
            const BitFlags<DISABLED_TYPE_COUNT> &flags = module->Get_Object()->Get_Disabled_State_Bits();

            if (!flags.Any() || flags.Any_Intersection_With(module->Get_Disabled_Types_To_Process())) {
                m_currentUpdateModule = module;
                sleep_time = module->Update();
                captainslog_dbgassert(sleep_time > 0, "you may not return 0 from update");

                if (sleep_time < UPDATE_SLEEP_TIME_MIN) {
                    sleep_time = UPDATE_SLEEP_TIME_MIN;
                }

                m_currentUpdateModule = nullptr;
            }

            module->Encode_Frame(sleep_time + frame);
            Rebalance_Sleepy_Update(0);
        } else {
            captainslog_dbgassert(false, "Null update. should not happen.");
        }
    }

#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theAI->Update();
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theBuildAssistant->Update();
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_thePartitionManager->Update();
    Process_Destroy_List();
    g_theCommandList->Reset();
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theWeaponStore->Update();
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theLocomotorStore->Update();
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    g_theVictoryConditions->Update();

    // obsolete copy protection code removed

    for (Object *obj = m_objList; obj != nullptr; obj = obj->Get_Next_Object()) {
        if (obj->Is_Disabled()) {
            obj->Check_Disabled_Status();
        }
    }

    if (!m_startNewGame) {
        m_frame++;
    }
}

void GameLogic::Destroy_All_Objects_Immediate()
{
    Object *next;

    for (Object *obj = m_objList; obj != nullptr; obj = next) {
        next = obj->Get_Next_Object();
        Destroy_Object(obj);
    }

    Process_Destroy_List();
    captainslog_dbgassert(m_objList == nullptr, "Destroy_All_Objects_Immediate: Object list not cleared");
}

void GameLogic::Prepare_Logic_For_Object_Load()
{
    Object *next;
    for (Object *obj = Get_First_Object(); obj != nullptr; obj = next) {
        next = obj->Get_Next_Object();

        if (obj->Is_KindOf(KINDOF_BRIDGE)) {
            Bridge *bridge = g_theTerrainLogic->Find_Bridge_At(obj->Get_Position());
            captainslog_dbgassert(bridge != nullptr, "GameLogic::Prepare_Logic_For_Object_Load - Unable to find bridge");
            const BridgeInfo *bridge_info = bridge->Peek_Bridge_Info();
            Object *old_obj = Find_Object_By_ID(bridge_info->bridge_object_id);
            captainslog_dbgassert(
                old_obj != nullptr, "GameLogic::Prepare_Logic_For_Object_Load - Unable to find old bridge object");
            captainslog_dbgassert(old_obj == obj, "GameLogic::Prepare_Logic_For_Object_Load - obj != oldObject");

            for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
                Object *tower_obj = Find_Object_By_ID(bridge_info->tower_object_id[i]);

                if (tower_obj != nullptr) {
                    Destroy_Object(tower_obj);
                }
            }

            Destroy_Object(old_obj);
        } else if (obj->Is_KindOf(KINDOF_WALK_ON_TOP_OF_WALL)) {
            Destroy_Object(obj);
        }
    }

    Process_Destroy_List();

    captainslog_dbgassert(Get_First_Object() == nullptr,
        "GameLogic::Prepare_Logic_For_Object_Load - There are still objects loaded in the engine, but it should be empty "
        "(Top is '%s')",
        Get_First_Object()->Get_Template()->Get_Name().Str());
}

void GameLogic::Process_Destroy_List()
{
    for (auto obj_it = m_objectsToDestroy.begin(); obj_it != m_objectsToDestroy.end(); obj_it++) {
        int update_count = 0;
        UpdateModule *updates[256];
        Object *obj = (*obj_it);

        for (auto update_it = m_sleepingUpdateModules.begin(); update_it != m_sleepingUpdateModules.end(); update_it++) {
            UpdateModule *update = *update_it;

            if (update->Get_Object() == obj && update_count < 256) {
                updates[update_count++] = update;
            }
        }

        update_count--;

        while (update_count >= 0) {
            int index = updates[update_count]->Get_Index_In_Logic();
            captainslog_dbgassert(
                m_sleepingUpdateModules[index] == updates[update_count], "Hmm, expected update mismatch here");
            Erase_Sleepy_Update(index);
            captainslog_dbgassert(updates[update_count]->Get_Index_In_Logic() == -1, "Hmm, expected index to be -1 here");
            --update_count;
        }

        obj->Remove_From_List(&m_objList);
        Remove_Object_From_Lookup_Table(obj);
        obj->Delete();
    }

    m_objectsToDestroy.clear();
}

void GameLogic::Erase_Sleepy_Update(int index)
{
    captainslog_dbgassert(index >= 0 && index < static_cast<int>(m_sleepingUpdateModules.size()), "bad sleepy idx");
    m_sleepingUpdateModules[index]->Set_Index_In_Logic(-1);
    int last_index = m_sleepingUpdateModules.size() - 1;

    if (index >= last_index) {
        m_sleepingUpdateModules.pop_back();
    } else {
        m_sleepingUpdateModules[index] = m_sleepingUpdateModules[last_index];
        m_sleepingUpdateModules[index]->Set_Index_In_Logic(index);
        m_sleepingUpdateModules.pop_back();
        Rebalance_Sleepy_Update(index);
    }
}

void GameLogic::Remake_Sleepy_Update()
{
    for (unsigned int i = m_sleepingUpdateModules.size() >> 1;; i--) {
        Rebalance_Child_Sleepy_Update(i);

        if (i == 0) {
            break;
        }
    }
}

void GameLogic::Xfer_Object_TOC(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    m_objectTOCEntries.clear();
    unsigned int entry_count = 0;

    if (xfer->Get_Mode() == XFER_SAVE) {
        Utf8String entry_name;

        for (Object *obj = Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
            entry_name = obj->Get_Template()->Get_Name();

            if (!Find_TOC_Entry_By_Name(entry_name)) {
                Add_TOC_Entry(obj->Get_Template()->Get_Name(), ++entry_count);
            }
        }

        xfer->xferUnsignedInt(&entry_count);

        for (auto it = m_objectTOCEntries.begin(); it != m_objectTOCEntries.end(); it++) {
            xfer->xferAsciiString(&(*it).name);
            xfer->xferUnsignedShort(&(*it).id);
        }
    } else {
        Utf8String entry_name;
        unsigned short entry_id;
        xfer->xferUnsignedInt(&entry_count);

        for (unsigned int i = 0; i < entry_count; i++) {
            xfer->xferAsciiString(&entry_name);
            xfer->xferUnsignedShort(&entry_id);
            Add_TOC_Entry(entry_name, entry_id);
        }
    }
}

LoadScreen *GameLogic::Get_Load_Screen(bool b)
{
    // needs LoadScreen implementations
#ifdef GAME_DLL
    return Call_Method<LoadScreen *, GameLogic, bool>(PICK_ADDRESS(0x004A2A40, 0x007ABCF7), this, b);
#else
    return nullptr;
#endif
}

void GameLogic::Update_Load_Progress(int percentage)
{
    if (m_loadScreen != nullptr) {
        m_loadScreen->Update(percentage);
    }
}

void GameLogic::Delete_Load_Screen()
{
    if (m_loadScreen != nullptr) {
        delete m_loadScreen;
        m_loadScreen = nullptr;
    }
}

void Check_For_Duplicate_Colors(GameInfo *info)
{
    if (info != nullptr) {
        for (int slot_idx = 7; slot_idx >= 0; slot_idx--) {
            GameSlot *slot = info->Get_Slot(slot_idx);

            if (slot != nullptr) {
                if (slot->Is_Occupied()) {
                    int color = slot->Get_Color();

                    if (color >= 0 && color < g_theMultiplayerSettings->Get_Num_Colors()) {
                        slot->Set_Color(-1);

                        if (info->Is_Color_Taken(color, -1)) {
                            if (color >= 0) {
                                captainslog_debug("Clearing color %d for player %d", color, slot_idx);
                            }
                        } else {
                            slot->Set_Color(color);
                        }
                    }
                }
            }
        }
    }
}

void Populate_Random_Side_And_Color(GameInfo *game)
{
    if (game != nullptr) {
        std::vector<int> indexes;

        for (int i = 0; i < g_thePlayerTemplateStore->Get_Player_Template_Count(); i++) {
            PlayerTemplate *player_template = g_thePlayerTemplateStore->Get_Nth_Player_Template(i);

            if (player_template != nullptr) {
                if (!player_template->Get_Starting_Building().Is_Empty()
                    && (!game->Get_Original_Armies() || player_template->Get_Old_Faction())) {
                    bool b = true;
                    bool starts_disabled;
                    GeneralPersona *persona =
                        g_theChallengeGenerals->Get_Player_General_By_Template_Name(player_template->Get_Name());

                    if (persona != nullptr) {
                        starts_disabled = !persona->Get_Starts_Enabled();
                    } else {
                        starts_disabled = false;
                    }

                    if (!b || !starts_disabled) {
                        indexes.push_back(i);
                    }
                }
            }
        }

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game->Get_Slot(slot_idx);

            if (slot != nullptr && slot->Is_Occupied()) {
                int player_template_idx = slot->Get_Player_Template();
                captainslog_debug("Player %d has playerTemplate index %d", slot_idx, player_template_idx);

                while (player_template_idx != -2) {
                    if (player_template_idx >= 0) {
                        if (player_template_idx < g_thePlayerTemplateStore->Get_Player_Template_Count()) {
                            break;
                        }
                    }

                    captainslog_dbgassert(player_template_idx == -1,
                        "Non-random bad playerTemplate %d in slot %d",
                        player_template_idx,
                        slot_idx);

                    for (unsigned int j = 0; j < Get_Logic_Random_Seed() % 7; j++) {
                        Get_Logic_Random_Value(0, 1);
                    }

                    player_template_idx = indexes[Get_Logic_Random_Value(0, 1000) % indexes.size()];
                    PlayerTemplate *player_template = g_thePlayerTemplateStore->Get_Nth_Player_Template(player_template_idx);

                    if (player_template != nullptr && !player_template->Get_Starting_Building().Is_Empty()) {
                        captainslog_debug("Setting playerTemplateIdx %d to %d", slot_idx, player_template_idx);
                        slot->Set_Player_Template(player_template_idx);
                    } else {
                        captainslog_dbgassert(false, "should not be possible");
                        player_template_idx = -1;
                    }
                }

                int color = slot->Get_Color();

                if (color < 0 || color >= g_theMultiplayerSettings->Get_Num_Colors()) {
                    captainslog_dbgassert(color == -1, "Non-random bad color %d in slot %d", color, slot_idx);

                    while (color == -1) {
                        color = Get_Logic_Random_Value(0, g_theMultiplayerSettings->Get_Num_Colors() - 1);

                        if (game->Is_Color_Taken(color, -1)) {
                            color = -1;
                        }
                    }

                    captainslog_debug("Setting color %d to %d", slot_idx, color);
                    slot->Set_Color(color);
                }
            }
        }
    }
}

void Populate_Random_Start_Position(GameInfo *game)
{
    if (game != nullptr) {
        int player_count = GameInfo::MAX_SLOTS;
        MapMetaData *map = g_theMapCache->Find_Map(game->Get_Map());
        captainslog_dbgassert(map != nullptr, "Could not find map %s in the mapcache", game->Get_Map().Str());

        if (map != nullptr) {
            player_count = map->m_numPlayers;
        }

        float distances[GameInfo::MAX_SLOTS][GameInfo::MAX_SLOTS];

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            for (int slot_idx2 = 0; slot_idx2 < GameInfo::MAX_SLOTS; slot_idx2++) {
                if (slot_idx == slot_idx2 || slot_idx >= player_count || slot_idx2 >= player_count) {
                    distances[slot_idx][slot_idx2] = 0.0f;
                } else {
                    Utf8String start_pos_1;
                    Utf8String start_pos_2;
                    start_pos_1.Format("Player_%d_Start", slot_idx + 1);
                    start_pos_2.Format("Player_%d_Start", slot_idx2 + 1);
                    auto it1 = map->m_waypoints.find(start_pos_1);
                    auto it2 = map->m_waypoints.find(start_pos_2);

                    if (it1 == map->m_waypoints.end() || it2 == map->m_waypoints.end()) {
                        distances[slot_idx][slot_idx2] = 1000000.0f;
                    } else {
                        Coord3D pos1 = it1->second;
                        Coord3D pos2 = it2->second;

                        distances[slot_idx][slot_idx2] =
                            GameMath::Sqrt(GameMath::Square(pos1.x - pos2.x) + GameMath::Square(pos1.y - pos2.y));
                    }
                }
            }
        }

        bool has_used_start_positions = false;
        bool used_start_positions[GameInfo::MAX_SLOTS];

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            used_start_positions[slot_idx] = slot_idx >= player_count;
        }

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game->Get_Slot(slot_idx);

            if (slot != nullptr) {
                if (slot->Is_Occupied() && slot->Get_Player_Template() != -2) {
                    int start_pos = slot->Get_Start_Pos();

                    if (start_pos >= 0 || start_pos >= player_count) {
                        has_used_start_positions = true;
                        used_start_positions[start_pos] = true;
                    }
                }
            }
        }

        int start_positions[GameInfo::MAX_SLOTS];

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            start_positions[slot_idx] = -1;
        }

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game->Get_Slot(slot_idx);

            if (slot != nullptr) {
                if (slot->Is_Occupied() && slot->Get_Player_Template() != -2) {
                    int start_pos = slot->Get_Start_Pos();

                    if (start_pos < 0 || start_pos >= player_count) {
                        captainslog_dbgassert(
                            start_pos == -1, "Non-random bad start position %d in slot %d", start_pos, slot_idx);
                        int team = slot->Get_Team_Number();

                        if (has_used_start_positions) {
                            if (team < 0 || start_positions[team] == -1) {
                                float dist1 = 0.0f;
                                int new_start_pos = -1;

                                for (start_pos = 0; start_pos < player_count; start_pos++) {
                                    if (!used_start_positions[start_pos]) {
                                        if (new_start_pos >= 0) {
                                            float dist2 = 0.0f;

                                            for (int j = 0; j < player_count; j++) {
                                                if (used_start_positions[j] && j != start_pos) {
                                                    dist2 += distances[start_pos][j];
                                                }
                                            }

                                            if (dist2 > dist1) {
                                                dist1 = dist2;
                                                new_start_pos = start_pos;
                                            }
                                        } else {
                                            new_start_pos = start_pos;

                                            for (int j = 0; j < player_count; j++) {
                                                if (used_start_positions[j] && j != start_pos) {
                                                    dist1 += distances[start_pos][j];
                                                }
                                            }
                                        }
                                    }
                                }

                                captainslog_dbgassert(new_start_pos >= 0, "Couldn't find a farthest spot!");
                                slot->Set_Start_Pos(new_start_pos);
                                used_start_positions[new_start_pos] = true;

                                if (team > -1) {
                                    start_positions[team] = new_start_pos;
                                }
                            } else {
                                float dist1 = GAMEMATH_FLOAT_MAX;
                                int new_start_pos = 0;

                                for (int j = 0; j < player_count; j++) {
                                    float dist2 = distances[start_positions[team]][j];

                                    if (!used_start_positions[j] && dist2 < dist1) {
                                        dist1 = dist2;
                                        new_start_pos = j;
                                    }
                                }

                                captainslog_dbgassert(
                                    dist1 < GAMEMATH_FLOAT_MAX, "Couldn't find a closest starting positon!");
                                slot->Set_Start_Pos(new_start_pos);
                                used_start_positions[new_start_pos] = true;
                            }
                        } else {
                            while (start_pos == -1) {
                                start_pos = Get_Logic_Random_Value(0, player_count - 1);

                                if (game->Is_Start_Position_Taken(start_pos, -1)) {
                                    start_pos = -1;
                                }
                            }

                            captainslog_debug("Setting start position %d to %d (random choice)", slot_idx, start_pos);
                            has_used_start_positions = true;
                            slot->Set_Start_Pos(start_pos);
                            used_start_positions[start_pos] = true;

                            if (team > -1) {
                                start_positions[team] = start_pos;
                            }
                        }
                    }
                }
            }
        }

        int occupied_count = 0;

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            const GameSlot *slot = game->Get_Const_Slot(slot_idx);

            if (slot->Is_Occupied() && slot->Get_Player_Template() != -2) {
                occupied_count++;
            }
        }

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game->Get_Slot(slot_idx);

            if (slot && slot->Is_Occupied() && slot->Get_Player_Template() == -2) {
                int new_start_pos = -1;

                if (occupied_count == 0) {
                    new_start_pos = 0;
                }

                while (new_start_pos == -1) {
                    new_start_pos = Get_Logic_Random_Value(0, player_count - 1);

                    if (!game->Is_Start_Position_Taken(new_start_pos, -1)) {
                        new_start_pos = -1;
                    }
                }

                captainslog_debug("Setting observer start position %d to %d", slot_idx, new_start_pos);
                slot->Set_Start_Pos(new_start_pos);
            }
        }
    }
}

Waypoint *Find_Named_Waypoint(Utf8String name)
{
    for (Waypoint *wp = g_theTerrainLogic->Get_First_Waypoint(); wp != nullptr; wp = wp->Get_Next()) {
        if (wp->Get_Name() == name) {
            return wp;
        }
    }

    return nullptr;
}

Object *Place_Object_At_Position(
    int id, Utf8String name, Coord3D &pos, Player *player, const PlayerTemplate *player_template)
{
    ThingTemplate *tmplate = g_theThingFactory->Find_Template(name, true);
    captainslog_dbgassert(tmplate != nullptr, "TheThingFactory didn't find a template in Place_Object_At_Position()");
    Object *obj = g_theThingFactory->New_Object(tmplate, player->Get_Default_Team(), OBJECT_STATUS_MASK_NONE);
    captainslog_dbgassert(obj != nullptr,
        "TheThingFactory didn't give me a valid Object for player %d's (%ls) starting building",
        id,
        player->Get_Player_Display_Name().Str());

    if (obj != nullptr) {
        obj->Set_Orientation(obj->Get_Template()->Get_Placement_View_Angle());
        obj->Set_Position(&pos);
#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif
        Team *team = player->Get_Default_Team();

        for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
            CreateModuleInterface *create = (*module)->Get_Create();

            if (create != nullptr) {
                create->On_Build_Complete();
            }
        }

        if (team != nullptr) {
            team->Set_Active();
        }

        g_theAI->Get_Pathfinder()->Add_Object_To_Pathfind_Map(obj);

        if (obj->Get_AI_Update_Interface() != nullptr) {
            if (!obj->Is_KindOf(KINDOF_IMMOBILE)) {
#ifdef GAME_DEBUG_STRUCTS
                // TODO unknown CRC stuff
#endif
                if (g_theAI->Get_Pathfinder()->Adjust_Destination(
                        obj, *obj->Get_AI_Update_Interface()->Get_Locomotor_Set(), &pos, nullptr)) {
#ifdef GAME_DEBUG_STRUCTS
                    // TODO unknown CRC stuff
#endif
                    g_theAI->Get_Pathfinder()->Update_Goal(obj, &pos, LAYER_GROUND);
                    obj->Set_Position(&pos);
                }
            }
        }
    }

    return obj;
}

void Find_And_Select_Command_Center(Object *obj, void *data)
{
    bool *found = static_cast<bool *>(data);

    if (!*found && obj->Is_KindOf(KINDOF_COMMANDCENTER)) {
        *found = true;
        g_theGameLogic->Select_Object(
            obj, true, obj->Get_Controlling_Player()->Get_Player_Mask(), obj->Is_Locally_Controlled());
    }
}

void Place_Network_Buildings_For_Player(int id, const GameSlot *slot, Player *player, const PlayerTemplate *player_template)
{
    int start_pos = slot->Get_Start_Pos();
    Utf8String start_str;
    start_str.Format("Player_%d_Start", start_pos + 1);
    Utf8String rally_str;
    rally_str.Format("Player_%d_Rally", start_pos + 1);
    Waypoint *start_wp = Find_Named_Waypoint(start_str);
    Waypoint *rally_wp = Find_Named_Waypoint(rally_str);
    captainslog_dbgassert(start_wp != nullptr, "Player %d has no starting waypoint (Player_%d_Start)", id, start_pos);

    if (start_wp != nullptr) {
        Coord3D pos = *start_wp->Get_Location();
        pos.z = g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
        Utf8String start_building = player_template->Get_Starting_Building();
        captainslog_dbgassert(!start_building.Is_Empty(),
            "no starting building type for player %d (playertemplate %ls)",
            id,
            player_template->Get_Display_Name().Str());

        if (start_building.Is_Empty()) {
            return;
        } else {
            captainslog_debug("Placing starting building at waypoint %s", start_str.Str());
            Object *obj = Place_Object_At_Position(id, start_building, pos, player, player_template);

            if (obj != nullptr) {
                player->On_Structure_Created(nullptr, obj);
                player->On_Structure_Construction_Complete(nullptr, obj, false);
                pos.y -= obj->Get_Geometry_Info().Get_Bounding_Sphere_Radius() / 2.0f;
                pos.z = g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);

                if (rally_wp != nullptr) {
                    pos = *rally_wp->Get_Location();
                    pos.z = g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
                }

                for (int i = 0; i < 10; i++) {
                    Utf8String starting_unit = player_template->Get_Starting_Unit(i);

                    if (starting_unit.Is_Not_Empty()) {
                        Coord3D new_pos = pos;
                        FindPositionOptions options;
                        options.min_radius = obj->Get_Geometry_Info().Get_Bounding_Sphere_Radius() * 0.7f;
                        options.max_radius = obj->Get_Geometry_Info().Get_Bounding_Sphere_Radius() * 1.3f;
                        captainslog_debug("Placing starting object %d (%s)", i, starting_unit.Str());
                        g_thePartitionManager->Update();

                        if (g_thePartitionManager->Find_Position_Around(&pos, &options, &new_pos)) {
                            Object *unit = Place_Object_At_Position(id, starting_unit, new_pos, player, player_template);

                            if (unit != nullptr) {
                                player->On_Unit_Created(nullptr, unit);
                            }
                        } else {
                            captainslog_debug("Could not find position");
                        }
                    }
                }
            }
        }
    }
}

void GameLogic::Start_New_Game(bool restart)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif

    Set_Prepare_New_Game(true);

    if (!restart) {
        g_theGameState->Set_Pristine_Map_Name(g_theWriteableGlobalData->m_mapName);
        captainslog_dbgassert(!g_theGameState->Is_In_Save_Dir(g_theWriteableGlobalData->m_mapName),
            "FATAL SAVE/LOAD ERROR! - Setting a pristine map name that refers to a map in the save directory.  The pristine "
            "map should always refer to the ORIGINAL map in the Maps directory, if the pristine map string is corrupt then "
            "map.ini files will not load correctly.");

        if (!m_startNewGame) {
            if (m_gameMode == GAME_SINGLE_PLAYER) {
                if (m_background != nullptr) {
                    m_background->Destroy_Windows();
                    m_background->Delete_Instance();
                    m_background = nullptr;
                }

                m_loadScreen = Get_Load_Screen(false);

                if (m_loadScreen != nullptr) {
                    g_theWriteableGlobalData->m_unkBool17 = true;
                    m_loadScreen->Init(nullptr);
                }
            }

            m_startNewGame = true;
            return;
        }
    }

    m_rankLevelLimit = 1000;
    Set_Defaults(restart);
    g_theWriteableGlobalData->m_unkBool17 = true;
#ifdef GAME_DEBUG_STRUCTS
    g_theWriteableGlobalData->m_TiVOFastMode = false;
#endif
    m_occlusionEnabled = true;
    m_drawIconUI = true;
    m_dynamicLOD = true;
    m_hulkLifetimeOverride = -1;

    bool is_challenge_campaign = m_gameMode == GAME_SINGLE_PLAYER && g_theCampaignManager->Get_Current_Campaign() != nullptr
        && g_theCampaignManager->Get_Current_Campaign()->m_isChallengeCampaign;

    GameInfo *game_info = nullptr;
    g_theGameInfo = nullptr;
    int local_slot_idx = 0;

    if (g_theNetwork != nullptr) {
        if (g_theLAN != nullptr) {
            captainslog_debug("Starting network game");
            game_info = g_theLAN->Get_My_Game();
            g_theGameInfo = game_info;
        } else {
            captainslog_debug("Starting gamespy game");
            game_info = g_theGameSpyGame;
            g_theGameInfo = g_theGameSpyGame;
        }
    } else if (g_theRecorder != nullptr && g_theRecorder->Get_Mode() == RECORDERMODETYPE_PLAYBACK) {
        game_info = g_theRecorder->Get_Game_Info();
        g_theGameInfo = game_info;
    } else if (m_gameMode == GAME_SKIRMISH) {
        game_info = g_theSkirmishGameInfo;
        g_theGameInfo = g_theSkirmishGameInfo;
    } else if (is_challenge_campaign) {
        game_info = g_theChallengeGameInfo;
        g_theGameInfo = g_theChallengeGameInfo;
    }

    if (!restart) {
        if (g_theGameInfo != nullptr) {
            m_maxSimultaneousOfType = g_theGameInfo->Get_Superweapon_Restriction();
        } else {
            m_maxSimultaneousOfType = 0;
        }
    }

    Check_For_Duplicate_Colors(game_info);
    bool has_ai_slot = false;

    if (game_info != nullptr) {
        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game_info->Get_Slot(slot_idx);

            if (!restart) {
                slot->Save_Off_Original_Info();
            }

            if (slot->Is_AI()) {
                has_ai_slot = true;
            }
        }
    } else if (m_gameMode == GAME_SINGLE_PLAYER && g_theSkirmishGameInfo != nullptr) {
        delete g_theSkirmishGameInfo;
        g_theSkirmishGameInfo = nullptr;
    }

    Populate_Random_Side_And_Color(game_info);
    Populate_Random_Start_Position(game_info);

    if (m_loadScreen == nullptr) {
        m_loadScreen = Get_Load_Screen(restart);
        g_theMouse->Set_Visibility(false);
        m_loadScreen->Init(game_info);
        Update_Load_Progress(0);
    }

    if (m_background != nullptr) {
        m_background->Destroy_Windows();
        m_background->Delete_Instance();
        m_background = nullptr;
    }

    Set_FP_Mode();

    if (g_theCampaignManager != nullptr) {
        g_theCampaignManager->Set_Is_Victory(false);
    }

    m_startNewGame = false;

    if (m_loadScreen != nullptr) {
        Update_Load_Progress(1);
    }

    m_frame = 0;
    Load_Map_INI(g_theWriteableGlobalData->m_mapName);
    g_theTerrainLogic->Load_Map(g_theWriteableGlobalData->m_mapName, false);
    Update_Load_Progress(2);
#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif
    int percentage = 3;

    if (game_info != nullptr) {
        if (g_theGameEngine->Is_Multiplayer_Session() || has_ai_slot) {
            g_theSidesList->Prepare_For_MP_Or_Skirmish();
        }

        Dict dict;

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game_info->Get_Slot(slot_idx);

            if (slot == nullptr || !slot->Is_Human()) {
                m_progressComplete[slot_idx] = true;
                Last_Heard_From(slot_idx);
            }

            if (slot != nullptr && slot->Is_Occupied()) {
                dict.Clear();
                Utf8String player_name;
                player_name.Format("player%d", slot_idx);
                dict.Set_AsciiString(g_playerNameKey, player_name);
                dict.Set_Bool(g_playerIsHumanKey, slot->Is_Human());
                dict.Set_UnicodeString(g_playerDisplayNameKey, slot->Get_Name());
                PlayerTemplate *player_template;

                if (slot->Get_Player_Template() < 0) {
                    player_template = g_thePlayerTemplateStore->Find_Player_Template(
                        g_theNameKeyGenerator->Name_To_Key("FactionObserver"));
                } else {
                    player_template = g_thePlayerTemplateStore->Get_Nth_Player_Template(slot->Get_Player_Template());
                }

                if (player_template != nullptr) {
                    dict.Set_AsciiString(
                        g_playerFactionKey, g_theNameKeyGenerator->Key_To_Name(player_template->Get_Name_Key()));
                }

                if (game_info->Is_Player_Preorder(slot_idx)) {
                    dict.Set_Bool(g_playerIsPreorderKey, true);
                }

                Utf8String enemies;
                Utf8String allies;
                int team = slot->Get_Team_Number();
                captainslog_debug("Looking for allies of player %d, team %d", slot_idx, team);

                for (int other_slot_idx = 0; other_slot_idx < GameInfo::MAX_SLOTS; other_slot_idx++) {
                    GameSlot *other_slot = game_info->Get_Slot(other_slot_idx);

                    if (slot_idx != other_slot_idx && other_slot->Is_Occupied()) {
                        captainslog_debug("Player %d is team %d", other_slot_idx, other_slot->Get_Team_Number());
                        Utf8String other_player_name;
                        other_player_name.Format("player%d", other_slot_idx);
                        bool is_enemy = false;

                        if (team == -1 || other_slot->Get_Team_Number() != team) {
                            is_enemy = true;
                        }

                        if (is_enemy) {
                            captainslog_debug("Player %d is %s", other_slot_idx, "enemy");
                        } else {
                            captainslog_debug("Player %d is %s", other_slot_idx, "ally");
                        }

                        if (is_enemy) {
                            if (!enemies.Is_Empty()) {
                                enemies.Concat(" ");
                            }

                            enemies += other_player_name;
                        } else {
                            if (!allies.Is_Empty()) {
                                allies.Concat(" ");
                            }

                            allies += other_player_name;
                        }
                    }
                }

                dict.Set_AsciiString(g_playerAlliesKey, allies);
                dict.Set_AsciiString(g_playerEnemiesKey, enemies);
                captainslog_debug("Player %d's teams are: allies=%s, enemies=%s", slot_idx, allies.Str(), enemies.Str());
                dict.Set_Int(g_playerColorKey, g_theMultiplayerSettings->Get_Color(slot->Get_Color())->Get_Color());
                dict.Set_Int(
                    g_playerNightColorKey, g_theMultiplayerSettings->Get_Color(slot->Get_Color())->Get_Night_Color());
                dict.Set_Int(g_multiplayerStartIndexKey, slot->Get_Start_Pos());
                bool is_local;

                if (!slot->Is_Human()) {
                    is_local = false;
                } else {
                    Utf16String local_slot_name = game_info->Get_Slot(game_info->Get_Local_Slot_Num())->Get_Name();
                    Utf16String slot_name = slot->Get_Name();

                    if (slot_name.Compare(local_slot_name.Str()) == 0) {
                        is_local = true;
                    } else {
                        is_local = false;
                    }
                }

                dict.Set_Bool(g_multiplayerIsLocalKey, is_local);

                if (has_ai_slot) {
                    dict.Set_Bool(g_playerIsSkirmishKey, true);

                    switch (slot->Get_State()) {
                        case SLOT_EASY_AI:
                            dict.Set_Int(g_skirmishDifficultyKey, 0);
                            break;
                        case SLOT_MED_AI:
                            dict.Set_Int(g_skirmishDifficultyKey, 1);
                            break;
                        case SLOT_HARD_AI:
                            dict.Set_Int(g_skirmishDifficultyKey, 2);
                            break;
                    }
                }

                Utf8String slot_name;
                slot_name.Translate(slot->Get_Name());

                if (slot->Is_Human()) {
                    if (game_info->Get_Slot_Num(slot_name) == game_info->Get_Local_Slot_Num()) {
                        local_slot_idx = slot_idx;
                    }
                }

                g_theSidesList->Add_Side(&dict);
                Utf8String team_name;
                team_name.Set("team");
                team_name += player_name;
                dict.Clear();

                dict.Set_AsciiString(g_teamNameKey, team_name);
                dict.Set_AsciiString(g_teamOwnerKey, player_name);
                dict.Set_Bool(g_teamIsSingletonKey, true);
                g_theSidesList->Add_Team(&dict);
                captainslog_debug("Added side %d", slot_idx);
                Update_Load_Progress(slot_idx + percentage);
            }
        }
    }

    Dict dict;
    dict.Set_AsciiString(g_playerNameKey, "ReplayObserver");
    dict.Set_Bool(g_playerIsHumanKey, true);
    Utf16String value = U_CHAR("Observer");
    dict.Set_UnicodeString(g_playerDisplayNameKey, value);
    PlayerTemplate *player_template =
        g_thePlayerTemplateStore->Find_Player_Template(g_theNameKeyGenerator->Name_To_Key("FactionObserver"));

    if (player_template != nullptr) {
        dict.Set_AsciiString(g_playerFactionKey, g_theNameKeyGenerator->Key_To_Name(player_template->Get_Name_Key()));
    }

    dict.Set_AsciiString(g_playerAlliesKey, Utf8String::s_emptyString);
    dict.Set_AsciiString(g_playerEnemiesKey, Utf8String::s_emptyString);
    dict.Set_Int(g_playerColorKey, g_theMultiplayerSettings->Get_Color(0)->Get_Color());
    dict.Set_Int(g_playerNightColorKey, g_theMultiplayerSettings->Get_Color(0)->Get_Night_Color());
    dict.Set_Int(g_multiplayerStartIndexKey, 0);
    dict.Set_Int(g_multiplayerIsLocalKey, 0);
    g_theSidesList->Add_Side(&dict);
    dict.Clear();

    dict.Set_AsciiString(g_teamNameKey, "teamReplayObserver");
    dict.Set_AsciiString(g_teamOwnerKey, "ReplayObserver");
    dict.Set_Bool(g_teamIsSingletonKey, true);
    g_theSidesList->Add_Team(&dict);
    g_theSidesList->Validate_Sides();
    Update_Load_Progress(12);
    g_theTeamFactory->Reset();
    g_thePlayerList->New_Game();
    Update_Load_Progress(13);
    g_theScriptEngine->New_Map();
    Update_Load_Progress(14);

    if (g_theGameEngine->Is_Multiplayer_Session() || has_ai_slot) {
        int team_count = 0;
        int team_index = -1;

        if (game_info != nullptr) {
            for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
                const GameSlot *slot = game_info->Get_Slot(slot_idx);

                if (slot->Is_Occupied() && slot->Get_Player_Template() != -2) {
                    if (slot->Get_Team_Number() == -1 || slot->Get_Team_Number() != team_index) {
                        team_count++;
                        team_index = slot->Get_Team_Number();
                    }
                }
            }
        }

        if (team_count > 1) {
            Utf8String filename("Data\\Scripts\\MultiplayerScripts.scb");
            CachedFileInputStream stream;

            if (stream.Open(filename)) {
                DataChunkInput input(&stream);
                input.Register_Parser(
                    "PlayerScriptsList", Utf8String::s_emptyString, ScriptList::Parse_Scripts_Data_Chunk, nullptr);

                if (!input.Parse(nullptr)) {
                    captainslog_debug("ERROR - Unable to read in multiplayer scripts.");
                    return;
                }
            }

            ScriptList *scripts[MAX_PLAYER_COUNT];
            int script_count = ScriptList::Get_Read_Scripts(scripts);

            if (script_count != 0) {
                ScriptList *script_list = g_theSidesList->Get_Side_Info(0)->Get_Script_List();

                for (Script *script = scripts[0]->Get_Script(); script != nullptr; script = script->Get_Next()) {
                    script_list->Add_Script(script->Duplicate(), 0);
                }
            }

            for (int script_idx = 0; script_idx < script_count; script_idx++) {
                scripts[script_idx]->Delete_Instance();
            }
        }
    }

    Update_Load_Progress(16);
    g_theRadar->New_Map(g_theTerrainLogic);
    g_theInGameUI->Set_No_Radar_Edge_Sound(false);
    g_theVictoryConditions->Cache_Player_Ptrs();
    g_theVictoryConditions->Set_Victory_Conditions(1);
    Update_Load_Progress(17);

    Region3D extent;
    g_theTerrainLogic->Get_Extent(&extent);

    g_theGameLogic->Set_Width(extent.hi.x - extent.lo.x);
    g_theGameLogic->Set_Height(extent.hi.y - extent.lo.y);
    g_thePartitionManager->Init();
    g_thePartitionManager->Refresh_Shroud_For_Local_Player();
    g_theGhostObjectManager->Set_Local_Player_Index(g_thePlayerList->Get_Local_Player()->Get_Player_Index());
    g_theGhostObjectManager->Reset();
    Update_Load_Progress(18);
    g_theTerrainLogic->New_Map(restart);
    Update_Load_Progress(19);
#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif

    for (MapObject *map_obj = MapObject::Get_First_Map_Object(); map_obj != nullptr; map_obj = map_obj->Get_Next()) {
        if (!map_obj->Get_Flag(FLAG_BRIDGE_POINT2 | FLAG_BRIDGE_POINT1)
            && !map_obj->Get_Flag(FLAG_ROAD_POINT2 | FLAG_ROAD_POINT1)) {
            const ThingTemplate *tmplate = map_obj->Get_Thing_Template();

            if (tmplate != nullptr) {
                bool is_bridge_related = tmplate->Is_Bridge();

                if (tmplate->Is_KindOf(KINDOF_WALK_ON_TOP_OF_WALL)) {
                    is_bridge_related = true;
                }

                if (is_bridge_related) {
                    Object *obj = g_theThingFactory->New_Object(
                        tmplate, g_thePlayerList->Get_Neutral_Player()->Get_Default_Team(), OBJECT_STATUS_MASK_NONE);

                    if (obj != nullptr) {
                        Coord3D pos = *map_obj->Get_Location();
                        pos.z += g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
                        obj->Set_Orientation(Normalize_Angle(map_obj->Get_Angle()));
                        obj->Set_Position(&pos);

                        if (tmplate->Is_Bridge()) {
                            g_theTerrainLogic->Add_Landmark_Bridge_To_Logic(obj);
                        }

                        if (tmplate->Is_KindOf(KINDOF_WALK_ON_TOP_OF_WALL)) {
                            g_theAI->Get_Pathfinder()->Add_Wall_Piece(obj);
                        }

                        obj->Update_Obj_Values_From_Map_Properties(map_obj->Get_Properties());
                    }
                }
            }
        }
    }

    Update_Load_Progress(20);
    g_theRadar->Refresh_Terrain(g_theTerrainLogic);
    g_theAI->Get_Pathfinder()->New_Map();
    Update_Load_Progress(21);
    g_thePartitionManager->Reveal_Map_For_Player_Permanently(
        g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key("ReplayObserver"))->Get_Player_Index());
    captainslog_debug("Reveal shroud for %ls whose index is %d",
        g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key("ReplayObserver"))
            ->Get_Player_Display_Name()
            .Str(),
        g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key("ReplayObserver"))->Get_Player_Index());

    if (game_info != nullptr) {
        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game_info->Get_Slot(slot_idx);

            if (slot != nullptr && slot->Is_Occupied()) {
                Utf8String player_name;
                player_name.Format("player%d", slot_idx);
                Player *player =
                    g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(player_name.Str()));

                if (slot->Get_Player_Template() == -2) {
                    captainslog_debug("Clearing shroud for observer %s in playerList slot %d",
                        player_name.Str(),
                        player->Get_Player_Index());
                    g_thePartitionManager->Reveal_Map_For_Player_Permanently(player->Get_Player_Index());
                } else if (!g_theMultiplayerSettings->Is_Use_Shroud()) {
                    g_thePartitionManager->Reveal_Map_For_Player(player->Get_Player_Index());
                }
            }
        }
    }
#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif
    bool use_trees = g_theWriteableGlobalData->m_useTrees;
    bool use_volaile_props = g_theGameLODManager->Get_Static_LOD_Level() != 2;

    if (g_theGameLODManager->Get_Static_LOD_Level() == 3 && g_theWriteableGlobalData->m_shadowVolumes) {
        use_volaile_props = false;
    }

    if (g_theRecorder != nullptr && g_theRecorder->Is_Multiplayer()) {
        use_trees = true;
        use_volaile_props = true;
    }

    percentage = 21;
    unsigned int time = rts::Get_Time();

    if (restart) {
        for (MapObject *map_obj = MapObject::Get_First_Map_Object(); map_obj != nullptr; map_obj = map_obj->Get_Next()) {
            const ThingTemplate *tmplate = map_obj->Get_Thing_Template();

            if (tmplate != nullptr && (!tmplate->Is_KindOf(KINDOF_SHRUBBERY) || use_trees)) {
                Coord3D pos = *map_obj->Get_Location();
                pos.z += g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
                float angle = Normalize_Angle(map_obj->Get_Angle());

                if (tmplate->Is_KindOf(KINDOF_OPTIMIZED_TREE)) {
                    Drawable *drawable = g_theThingFactory->New_Drawable(tmplate, DRAWABLE_STATUS_UNK);

                    if (drawable != nullptr) {
                        drawable->Set_Orientation(angle);
                        drawable->Set_Position(&pos);
                        g_theGameClient->Destroy_Drawable(drawable);
                    }
                }
            }
        }
    } else {
        for (MapObject *map_obj = MapObject::Get_First_Map_Object(); map_obj != nullptr; map_obj = map_obj->Get_Next()) {
            if (!map_obj->Get_Flag(FLAG_BRIDGE_POINT2 | FLAG_BRIDGE_POINT1)
                && !map_obj->Get_Flag(FLAG_ROAD_POINT2 | FLAG_ROAD_POINT1)) {
                Handle_Name_Change(map_obj);
                const ThingTemplate *tmplate = map_obj->Get_Thing_Template();

                if (tmplate != nullptr) {
                    bool is_bridge_related = tmplate->Is_Bridge();

                    if (tmplate->Is_KindOf(KINDOF_WALK_ON_TOP_OF_WALL)) {
                        is_bridge_related = true;
                    }

                    if (!is_bridge_related && (!tmplate->Is_KindOf(KINDOF_SHRUBBERY) || use_trees)) {
                        Coord3D pos = *map_obj->Get_Location();
                        pos.z += g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
                        float angle = Normalize_Angle(map_obj->Get_Angle());

                        if (tmplate->Is_KindOf(KINDOF_OPTIMIZED_TREE)) {
                            Drawable *drawable = g_theThingFactory->New_Drawable(tmplate, DRAWABLE_STATUS_UNK);

                            if (drawable != nullptr) {
                                drawable->Set_Orientation(angle);
                                drawable->Set_Position(&pos);
                                g_theGameClient->Destroy_Drawable(drawable);
                            }
                        } else {
                            bool is_prop = tmplate->Is_KindOf(KINDOF_PROP);
                            bool is_volatile_prop = false;

                            if (tmplate->Is_KindOf(KINDOF_CLEARED_BY_BUILD) && tmplate->Get_Fence_Width() == 0.0f) {
                                is_volatile_prop = true;
                            }

                            if (is_prop || (is_volatile_prop && use_volaile_props)) {
                                g_theTerrainVisual->Add_Prop(tmplate, &pos, angle);
                            } else {
                                captainslog_dbgassert(
                                    map_obj->Get_Properties()->Get_Type(g_originalOwnerKey) == Dict::DICT_ASCIISTRING,
                                    "unit %s has no original owner specified (obsolete map file)",
                                    map_obj->Get_Name().Str());
                                bool exists = false;
                                Utf8String owner = map_obj->Get_Properties()->Get_AsciiString(g_originalOwnerKey, &exists);
                                Team *team = g_thePlayerList->Validate_Team(owner);
                                Object *obj = g_theThingFactory->New_Object(tmplate, team, OBJECT_STATUS_MASK_NONE);

                                if (obj != nullptr) {
                                    if (map_obj->Get_Flag(FLAG_DRAWS_IN_MIRROR)
                                        || obj->Is_KindOf(KINDOF_CAN_CAST_REFLECTIONS)) {
                                        Drawable *drawable = obj->Get_Drawable();

                                        if (drawable != nullptr) {
                                            drawable->Set_Status_Bit(DRAWABLE_STATUS_DRAWS_IN_MIRROR);
                                        }
                                    }

                                    obj->Set_Orientation(angle);
                                    obj->Set_Position(&pos);
                                    obj->Update_Obj_Values_From_Map_Properties(map_obj->Get_Properties());
                                    obj->Set_Layer(g_theTerrainLogic->Get_Layer_For_Destination(&pos));

                                    for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
                                        CreateModuleInterface *create = (*module)->Get_Create();

                                        if (create != nullptr) {
                                            create->On_Build_Complete();
                                        }
                                    }

                                    team->Set_Active();
                                    g_theAI->Get_Pathfinder()->Add_Object_To_Pathfind_Map(obj);
                                }

                                if (rts::Get_Time() > time + 500) {
                                    if (percentage < 80) {
                                        percentage++;
                                    }

                                    Update_Load_Progress(percentage);
                                    time = rts::Get_Time();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif

    percentage = 81;

    if (game_info != nullptr && !restart) {
        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            GameSlot *slot = game_info->Get_Slot(slot_idx);

            if (slot != nullptr && slot->Is_Occupied()) {
                Utf8String player_name;
                player_name.Format("player%d", slot_idx);
                Player *player =
                    g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(player_name.Str()));

                if (slot->Get_Player_Template() == -2) {
                    slot->Set_Player_Template(0);

                    PlayerTemplate *observer = g_thePlayerTemplateStore->Find_Player_Template(
                        g_theNameKeyGenerator->Name_To_Key("FactionObserver"));

                    if (observer != nullptr) {
                        for (int m = 0; m < g_thePlayerTemplateStore->Get_Player_Template_Count(); m++) {
                            if (observer == g_thePlayerTemplateStore->Get_Nth_Player_Template(slot_idx)) {
                                slot->Set_Player_Template(m);
                                break;
                            }
                        }
                    }

                    captainslog_debug(
                        "Setting observer's playerTemplate to %d in slot %d", slot->Get_Player_Template(), slot_idx);
                } else {
                    PlayerTemplate *nth_player_template =
                        g_thePlayerTemplateStore->Get_Nth_Player_Template(slot->Get_Player_Template());
                    bool b8 = true;
                    bool starts_disabled;
                    GeneralPersona *general =
                        g_theChallengeGenerals->Get_Player_General_By_Template_Name(nth_player_template->Get_Name());

                    if (general != nullptr) {
                        starts_disabled = !general->Get_Starts_Enabled();
                    } else {
                        starts_disabled = false;
                    }

                    if (b8 && starts_disabled
                        || Is_In_Internet_Game() && game_info->Get_Original_Armies()
                            && !nth_player_template->Get_Old_Faction()) {
                        continue;
                    }

                    Place_Network_Buildings_For_Player(slot_idx, slot, player, nth_player_template);
                }

                percentage++;
                Update_Load_Progress(percentage);
            }
        }
    }

    Update_Load_Progress(90);

    if (g_theWriteableGlobalData->m_preloadAssets) {
        if (g_theWriteableGlobalData->m_preloadEverything) {
            for (int n = TIME_OF_DAY_FIRST; n < TIME_OF_DAY_COUNT; n++) {
                g_theGameClient->Preload_Assets(static_cast<TimeOfDayType>(n));
            }
        } else {
            g_theGameClient->Preload_Assets(g_theWriteableGlobalData->m_timeOfDay);
        }
    }

    g_theControlBar->Hide_Communicator(false);
    Update_Load_Progress(91);
    g_theTacticalView->Set_Angle_And_Pitch_To_Default();
    g_theTacticalView->Set_Zoom_To_Default();

    if (g_theRecorder != nullptr) {
        g_theRecorder->Init_Controls();
    }

    Utf8String starting_cam_name = g_theNameKeyGenerator->Key_To_Name(g_theInitialCameraPositionKey);

    if (game_info != nullptr) {
        GameSlot *slot = game_info->Get_Slot(local_slot_idx);
        captainslog_dbgassert(slot != nullptr, "Starting a LAN game without ourselves!");

        if (slot->Is_Human()) {
            starting_cam_name.Format("Player_%d_Start", slot->Get_Start_Pos() + 1);
            captainslog_debug("Using %s as the multiplayer initial camera position", starting_cam_name.Str());
        }
    }

    Update_Load_Progress(92);
    Waypoint *starting_cam = Find_Named_Waypoint(starting_cam_name);

    if (starting_cam != nullptr) {
        g_theTacticalView->Look_At(starting_cam->Get_Location());
    } else {
        Coord3D c;
        c.x = 50.0f;
        c.y = 50.0f;
        c.z = 0.0;
        g_theTacticalView->Look_At(&c);
        captainslog_debug("Failed to find initial camera position waypoint %s", starting_cam_name.Str());
    }

    g_theTacticalView->Init_Height_For_Map();
    g_theTacticalView->Set_Angle_And_Pitch_To_Default();
    g_theTacticalView->Set_Zoom_To_Default();
    Update_Load_Progress(93);
    g_thePartitionManager->Update();
#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif

    if (!restart) {
        g_thePlayerList->New_Map();
    }

    if (is_challenge_campaign) {
        Player *local_player = g_thePlayerList->Get_Local_Player();
        captainslog_dbgassert(local_player != nullptr, "Local player has not been established for Challenge map.");
        Player *player = g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key("ThePlayer"));
        captainslog_dbgassert(player != nullptr,
            "Challenge maps without player \"ThePlayer\" assume that the local player is mutual enemies with all other "
            "players except the neutral and civilian players.");

        if (player != nullptr) {
            unsigned short mask = g_thePlayerList->Get_Players_With_Relationship(
                player->Get_Player_Index(), PLAYER_RELATIONSHIP_FLAGS_ENEMIES);

            while (mask != 0) {
                Player *each_player = g_thePlayerList->Get_Each_Player_From_Mask(mask);
                each_player->Set_Player_Relationship(local_player, ENEMIES);
                local_player->Set_Player_Relationship(each_player, ENEMIES);
            }
        } else {
            for (int i = 0; i < g_thePlayerList->Get_Player_Count(); i++) {
                Relationship relationship = NEUTRAL;
                Player *nth_player = g_thePlayerList->Get_Nth_Player(i);

                if (nth_player == local_player) {
                    relationship = ALLIES;
                } else {
                    if (nth_player != g_thePlayerList->Get_Neutral_Player()) {
                        if (nth_player
                            != g_thePlayerList->Find_Player_With_NameKey(
                                g_theNameKeyGenerator->Name_To_Key("PlyrCivilian"))) {
                            relationship = ENEMIES;
                        }
                    }
                }

                nth_player->Set_Player_Relationship(local_player, relationship);
                local_player->Set_Player_Relationship(nth_player, relationship);
            }
        }
    }

    if (!restart && Is_In_Single_Player_Game()) {
        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            Player *nth_player = g_thePlayerList->Get_Nth_Player(i);

            if (nth_player != nullptr && nth_player->Get_Player_Type() != Player::PLAYER_HUMAN) {
                nth_player = nullptr;
            }

            if (nth_player != nullptr) {
                nth_player->Add_Skill_Points(m_rankPointsToAddAtGameStart);
                captainslog_debug("GameLogic::Start_New_Game() - adding m_rankPointsToAddAtGameStart==%d to player %d(%ls)",
                    m_rankPointsToAddAtGameStart,
                    i,
                    nth_player->Get_Player_Display_Name().Str());
            }
        }
    }

    Update_Load_Progress(100);

    if (Is_In_Multiplayer_Game() && g_theNetwork != nullptr) {
        g_theNetwork->Load_Progress_Complete();
        g_theNetwork->Lite_Update();
    }

    while (!Is_Progress_Complete()) {
        Update_Load_Progress(101);
        Test_Time_Out();
        rts::Sleep_Ms(100);
    }

    if (!restart) {
        g_theTransitionHandler->Set_Group("FadeWholeScreen", false);

        while (!g_theTransitionHandler->Is_Finished()) {
            g_theWindowManager->Update();

            if (!g_theTransitionHandler->Is_Finished()) {
                g_theDisplay->Draw();
                Set_FP_Mode();
                rts::Sleep_Ms(33);
            }
        }
    }

    if (m_loadScreen != nullptr) {
        g_theMouse->Set_Visibility(true);
        Delete_Load_Screen();
    }

#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif

    if (m_gameMode == GAME_SHELL) {
        if (g_theShell->Get_Screen_Count() != 0) {
            if (g_theShell->Top() != nullptr) {
                g_theShell->Top()->Hide(false);
                g_theShell->Top()->Bring_Forward();
            }
        } else {
            g_theShell->Push("Menus/MainMenu.wnd", false);
        }

        Hide_Control_Bar(true);
    } else {
        if (g_theStatsCollector != nullptr) {
            g_theStatsCollector->Reset();
        } else if (g_theWriteableGlobalData->m_playerStats > 0) {
            g_theStatsCollector = new StatsCollector;
            g_theStatsCollector->Reset();
        }

        if (m_gameMode == GAME_REPLAY) {
            g_thePlayerList->Set_Local_Player(
                g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key("ReplayObserver")));
            g_theRadar->Force_On(true);
            g_thePartitionManager->Refresh_Shroud_For_Local_Player();
            g_theControlBar->Set_Control_Bar_Scheme_By_Player(g_thePlayerList->Get_Local_Player());
            captainslog_debug("Start of a replay game %ls, %d",
                g_thePlayerList->Get_Local_Player()->Get_Player_Display_Name().Str(),
                g_thePlayerList->Get_Local_Player()->Get_Player_Index());
        } else {
            g_theControlBar->Set_Control_Bar_Scheme_By_Player(g_thePlayerList->Get_Local_Player());
        }
    }

    g_theTacticalView->Set_Ok_To_Adjust_Height(true);

    if (g_theRecorder->Is_Multiplayer()) {
        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            Player *nth_player = g_thePlayerList->Get_Nth_Player(i);

            if (nth_player != nullptr && nth_player->Is_Player_Active()) {
                bool found = false;
                nth_player->Iterate_Objects(Find_And_Select_Command_Center, &found);
            }
        }
    }

    g_theControlBar->Init_Special_Power_Shortcut_Bar(g_thePlayerList->Get_Local_Player());

    if (m_gameMode == GAME_SHELL) {
        Hide_Control_Bar(true);
    } else {
        Show_Control_Bar(false);
    }

    g_theWriteableGlobalData->m_unkBool17 = false;

    if (g_theGameSpyBuddyMessageQueue != nullptr && g_theGameSpyGame != nullptr && Is_In_Internet_Game()) {
        BuddyRequest request;
        request.m_budyRequestType = BuddyRequest::BUDDYREQUEST_SETSTATUS;
        request.m_arg.status.status = GP_RECV_BUDDY_STATUS;
        strcpy(request.m_arg.status.status_string, "Playing");
        sprintf(request.m_arg.status.location_string,
            "%s",
            WideCharStringToMultiByte(g_theGameSpyGame->Get_Game_Name().Str()).c_str());
        g_theGameSpyBuddyMessageQueue->Add_Request(request);
    }

    if (!restart) {
        for (Drawable *drawable = g_theGameClient->Get_Drawable_List(); drawable != nullptr;
             drawable = drawable->Get_Next()) {
            drawable->On_Level_Start();
        }
    }

    Set_Prepare_New_Game(false);
#ifdef GAME_DEBUG_STRUCTS
    // TODO processor frequency stuff
#endif

    if (g_theGameSpyInfo != nullptr) {
        g_theGameSpyInfo->Update_Additional_Game_Spy_Disconnections(1);
    }

    if (Is_In_Replay_Game() && g_theInGameUI != nullptr && g_theGameText != nullptr) {
        g_theInGameUI->Message(g_theGameText->Fetch("GUI:FastForwardInstructions"));
    }
}

void GameLogic::Load_Map_INI(Utf8String map_file)
{
    if (g_theMapCache != nullptr) {
        char map_path[PATH_MAX];
        strcpy(map_path, map_file.Str());

        if (g_theGameState->Is_In_Save_Dir(map_path)) {
            strcpy(map_path, g_theGameState->Get_Save_Info()->m_pristineMapPath.Str());
        }

        size_t map_path_len = strlen(map_path);

        if (map_path_len >= 4) {
            char *c;

            for (c = &map_path[map_path_len - 4]; c > map_path && *c != '\\' && *c != '/'; c--) {
            }

            *c = '\0';
            char load_file[PATH_MAX];
            sprintf(load_file, "%s\\map.ini", map_path);

            if (g_theFileSystem->Does_File_Exist(load_file)) {
                captainslog_debug("Loading map.ini");
                INI map_ini;
                map_ini.Load(load_file, INI_LOAD_CREATE_OVERRIDES, nullptr);
            }

            sprintf(load_file, "%s\\solo.ini", map_path);

            if (g_theFileSystem->Does_File_Exist(load_file)) {
                captainslog_debug("Loading solo.ini");
                INI map_ini;
                map_ini.Load(load_file, INI_LOAD_CREATE_OVERRIDES, nullptr);
            }

            sprintf(load_file, "%s\\map.str", map_path);

            if (g_theFileSystem->Does_File_Exist(load_file)) {
                g_theGameText->Init_Map_String_File(load_file);
            }

            if (g_theDisplay != nullptr) {
                sprintf(load_file, "%s\\%s", map_path, "AssetUsage.txt");
                g_theDisplay->Do_Smart_Asset_Purge_And_Preload(load_file);
            }
        }
    }
}

void GameLogic::Process_Progress(int player_id, int percentage)
{
    if (m_loadScreen != nullptr) {
        m_loadScreen->Process_Progress(player_id, percentage);
    }
}

bool GameLogic::Is_Progress_Complete()
{
    if (!Is_In_Multiplayer_Game() || g_theNetwork == nullptr || m_forceGameStartByTimeOut) {
        return true;
    }

    for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
        if (!m_progressComplete[slot_idx]) {
            return false;
        }
    }

    return true;
}

void GameLogic::Process_Command_List(CommandList *list)
{
    m_crcCheckList.clear();
    m_checkCRCs = false;

    for (GameMessage *message = list->Get_First_Message(); message != nullptr; message = message->Get_Next()) {
        Logic_Message_Dispatcher(message, nullptr);
    }

    if (m_checkCRCs && !g_theNetwork->Saw_CRC_Mismatch()) {
        bool has_crc_mismatch = false;
        unsigned int connected_count = 0;
        captainslog_dbgassert(g_theNetwork != nullptr, "No Network!");

        if (g_theNetwork != nullptr) {
            for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
                if (g_theNetwork->Is_Player_Connected(slot_idx)) {
                    connected_count++;
                }
            }

            if (m_crcCheckList.size() >= connected_count) {
                auto it = m_crcCheckList.begin();
                unsigned int crc = it->second;

                for (;;) {
                    if (!(++it != m_crcCheckList.end())) {
                        break;
                    }

                    if (crc != it->second) {
                        captainslog_dbgassert(false, "CRC mismatch!");
                        has_crc_mismatch = true;
                    }
                }
            } else {
                captainslog_dbgassert(false, "Not enough CRCs!");
                has_crc_mismatch = true;
            }
        }

        if (has_crc_mismatch) {
            captainslog_debug("CRC Mismatch - saw %d CRCs from %d players", m_crcCheckList.size(), connected_count);

            for (auto it = m_crcCheckList.begin(); it != m_crcCheckList.end(); it++) {
                const unichar_t *player_name;
                Player *player = g_thePlayerList->Get_Nth_Player(it->first);

                if (player != nullptr) {
                    player_name = player->Get_Player_Display_Name().Str();
                } else {
                    player_name = U_CHAR("<NONE>");
                }

                captainslog_debug("CRC from player %d (%ls) = %X", it->first, player_name, it->second);
            }

            g_theNetwork->Set_Saw_CRC_Mismatch();
        }
    }
}

void GameLogic::Select_Object(Object *obj, bool force_select, unsigned short player_mask, bool select_ui)
{
    if (obj != nullptr) {
        if (obj->Is_Selectable() || force_select) {
            while (player_mask) {
                Player *player = g_thePlayerList->Get_Each_Player_From_Mask(player_mask);

                if (player == nullptr) {
                    break;
                }

#ifdef GAME_DEBUG_STRUCTS
                // TODO unknown CRC stuff
#endif
                AIGroup *group = g_theAI->Create_Group();
                group->Add(obj);

                if (force_select) {
                    player->Set_Currently_Selected_AIGroup(group);
                } else {
                    player->Add_AIGroup_To_Current_Selection(group);
                }

                g_theAI->Destroy_Group(group);

                if (select_ui) {
                    Drawable *drawable = obj->Get_Drawable();

                    if (drawable != nullptr) {
                        g_theInGameUI->Select_Drawable(drawable);
                    }
                }
            }
        } else {
            captainslog_debug(
                "GameLogic::Select_Object() - Object attempted to be added to selection, but isn't mass-selectable.");
        }
    }
}

void GameLogic::Pop_Sleepy_Update()
{
    int size = m_sleepingUpdateModules.size();

    if (size != 0) {
        m_sleepingUpdateModules[0]->Set_Index_In_Logic(-1);

        if (size <= 1) {
            m_sleepingUpdateModules.pop_back();
        } else {
            m_sleepingUpdateModules[0] = m_sleepingUpdateModules[size - 1];
            m_sleepingUpdateModules[0]->Set_Index_In_Logic(0);
            m_sleepingUpdateModules.pop_back();
            Rebalance_Child_Sleepy_Update(0);
        }
    } else {
        captainslog_dbgassert(false, "should not happen");
    }
}

unsigned int GameLogic::Get_CRC(int crc_source, Utf8String deep_crc_name)
{
    if (crc_source == 1) {
        Set_FP_Mode();
        static bool m_inCRCGen;
        bool in_crc_gen = Get_In_Game_Logic_Update();
        LatchRestore<bool> latch(&m_inCRCGen, &in_crc_gen);
        Utf8String str;
        XferCRC *xfer;

        if (deep_crc_name.Is_Not_Empty()) {
            xfer = new XferDeepCRC();
            xfer->Open(deep_crc_name.Str());
        } else {
            Utf8String name;
#ifdef GAME_DEBUG_STRUCTS
            // TODO deep CRC stuff
#endif
            xfer = new XferCRC();
            name = "lightCRC";
            xfer->Open(name.Str());
        }

        captainslog_dbgassert(this == g_theGameLogic, "Not in GameLogic");

#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif
        str = "MARKER:Objects";
        xfer->xferAsciiString(&str);

        for (Object *obj = m_objList; obj != nullptr; obj = obj->Get_Next_Object()) {
            xfer->xferSnapshot(obj);
        }

        unsigned int logic_crc = Get_Logic_Random_Seed_CRC();

#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif

        if (xfer->Get_Mode() == XFER_CRC) {
            xfer->xferUnsignedInt(&logic_crc);
        }

        str = "MARKER:ThePartitionManager";
        xfer->xferAsciiString(&str);
        xfer->xferSnapshot(g_thePartitionManager);

#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif

        str = "MARKER:ThePlayerList";
        xfer->xferAsciiString(&str);
        xfer->xferSnapshot(g_thePlayerList);

#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif
        str = "MARKER:TheAI";
        xfer->xferAsciiString(&str);
        xfer->xferSnapshot(g_theAI);

#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif

        if (xfer->Get_Mode() == XFER_SAVE) {
            str = "MARKER:GameSave";
            xfer->xferAsciiString(&str);
            g_theGameState->Friend_Xfer_Save_Data_For_CRC(xfer, SNAPSHOT_TYPE_UNK2);
        }

        xfer->Close();
        unsigned int crc = xfer->Get_CRC();
        delete xfer;
        xfer = nullptr;

#ifdef GAME_DEBUG_STRUCTS
        // TODO unknown CRC stuff
#endif

        return crc;
    } else {
        return m_crc;
    }
}

void GameLogic::Set_Game_Paused(bool pause, bool pause_music)
{
    if (pause != m_gamePaused) {
        m_gamePaused = pause;
        int audio_affect =
            (pause_music ? AUDIOAFFECT_MUSIC : 0) | AUDIOAFFECT_SPEECH | AUDIOAFFECT_3DSOUND | AUDIOAFFECT_SOUND;

        if (pause) {
            m_inputEnabled = g_theInGameUI->Get_Input_Enabled();
            m_mouseVisible = g_theMouse->Get_Visibility();
            g_theMouse->Set_Visibility(true);
            g_theMouse->Set_Cursor(CURSOR_ARROW);

            if (m_inputEnabled) {
                g_theInGameUI->Set_Input_Enabled(false);
            }

            g_theAudio->Pause_Audio(static_cast<AudioAffect>(audio_affect));
        } else {
            g_theMouse->Set_Visibility(m_mouseVisible);

            if (m_inputEnabled) {
                g_theInGameUI->Set_Input_Enabled(true);
            }

            g_theAudio->Resume_Audio(static_cast<AudioAffect>(audio_affect));
        }
    }
}

void GameLogic::Test_Time_Out()
{
    if (!Is_Progress_Complete()) {
        int time = rts::Get_Time();

        for (int slot_idx = 0; slot_idx < GameInfo::MAX_SLOTS; slot_idx++) {
            if (!m_progressComplete[slot_idx] && m_progressCompleteTimeout[slot_idx] + 60000 > time) {
                return;
            }
        }

        m_forceGameStartByTimeOut = true;
    }
}

void GameLogic::Init_Time_Out_Values()
{
    if (g_theNetwork != nullptr) {
        for (int i = 0; i < g_theNetwork->Get_Num_Players(); i++) {
            m_progressCompleteTimeout[i] = rts::Get_Time();
        }
    }
}

#ifdef GAME_DEBUG_STRUCTS
void GameLogic::Get_AI_Stats(
    int *objects, int *moving_objects, int *attacking_objects, int *waiting_objects, int *failed_pathfinds)
{
    *objects = 0;
    *moving_objects = 0;
    *attacking_objects = 0;
    *waiting_objects = 0;

    for (Object *obj = Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
        AIUpdateInterface *update = obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            (*objects)++;

            if (update->Is_Moving()) {
                (*moving_objects)++;
            }

            if (update->Is_Waiting_For_Path()) {
                (*waiting_objects)++;
            }

            if (update->Is_Attacking()) {
                (*attacking_objects)++;
            }
        }
    }

    (*failed_pathfinds) = m_failedPathFinds;
}
#endif
