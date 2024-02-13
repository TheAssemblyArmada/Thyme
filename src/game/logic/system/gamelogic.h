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
#include "bitflags.h"
#include "gametype.h"
#include "rtsutils.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif
#include <list>
#include <map>

class CommandButton;
class CommandList;
class Drawable;
class GameMessage;
class GhostObjectManager;
class LoadScreen;
class Object;
class Team;
class TerrainLogic;
class ThingTemplate;
class UpdateModule;
class WindowLayout;

class GameLogic : public SubsystemInterface, public SnapShot
{
public:
    struct ObjectTOCEntry
    {
        Utf8String name;
        unsigned short id;
    };

    GameLogic();
    virtual ~GameLogic() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual TerrainLogic *Create_Terrain_Logic();
    virtual GhostObjectManager *Create_Ghost_Object_Manager();

    unsigned int Get_Frame() { return m_frame; }
    GameMode Get_Game_Mode() { return m_gameMode; }
    unsigned short Get_Max_Simultaneous_Of_Type() { return m_maxSimultaneousOfType; }
    unsigned int Get_Sleepy_Module_Count() { return m_sleepingUpdateModules.size(); }
    int Get_Next_Obj_ID() { return m_nextObjID; }
    int Get_Rank_Level_Limit() { return m_rankLevelLimit; }
    int Get_Hulk_Lifetime_Override() { return m_hulkLifetimeOverride; }
    int Get_Rank_Points_To_Add_At_Game_Start() { return m_rankPointsToAddAtGameStart; }
    int Get_Unk_Frame() { return m_frameTriggerAreasChanged; }

    bool Get_Occlusion_Enabled() const { return m_occlusionEnabled; }
    bool Get_In_Game_Logic_Update() const { return m_inGameLogicUpdate; }
    bool Get_Dynamic_LOD() const { return m_dynamicLOD; }
    bool Get_Start_New_Game() const { return m_startNewGame; }
    bool Get_Draw_Icon_UI() const { return m_drawIconUI; }
    bool Get_Enable_Scoring() const { return m_scoringEnabled; }
    bool Get_Prepare_New_Game() const { return m_prepareNewGame; }

    void Set_Width(float width) { m_width = width; }
    void Set_Height(float height) { m_height = height; }
    void Set_Hulk_Lifetime_Override(int lifetime) { m_hulkLifetimeOverride = lifetime; }
    void Set_Game_Mode(GameMode mode) { m_gameMode = mode; }
    void Set_Next_Obj_ID(ObjectID id) { m_nextObjID = id; }

    void Set_Rank_Level_Limit(int limit)
    {
        if (limit < 1) {
            limit = 1;
        }

        m_rankLevelLimit = limit;
    }

    void Set_Prepare_New_Game(bool b) { m_prepareNewGame = b; }
    void Set_Occlusion_Enabled(bool b) { m_occlusionEnabled = b; }
    void Set_Draw_Icon_UI(bool b) { m_drawIconUI = b; }
    void Set_Dynamic_LOD(bool b) { m_dynamicLOD = b; }
    void Set_Clear_Game_Data(bool b) { m_clearingGameData = b; }
    void Set_Loading_Game_State_Map(bool b) { m_loadingGameStateMap = b; }

    Object *Find_Object_By_ID(ObjectID id)
    {
        if (!id) {
            return nullptr;
        }

        if ((unsigned)id >= m_objectLookupTable.size()) {
            return nullptr;
        }

        return m_objectLookupTable[id];
    }

    void Save_Frame() { m_frameTriggerAreasChanged = m_frame; }
#ifdef GAME_DEBUG_STRUCTS
    void Add_Failed_Pathfind() { m_failedPathFinds++; }
#endif

    void Enable_Scoring(bool b) { m_scoringEnabled = b; }

    bool Is_In_Game() { return m_gameMode != GAME_NONE; }
    bool Is_In_Replay_Game() { return m_gameMode == GAME_REPLAY; }
    bool Is_In_Shell_Game() { return m_gameMode == GAME_SHELL; }
    bool Is_In_Multiplayer_Game() { return m_gameMode == GAME_LAN || m_gameMode == GAME_INTERNET; }
    bool Is_In_Internet_Game() { return m_gameMode == GAME_INTERNET; }
    bool Is_In_Skirmish_Game() { return m_gameMode == GAME_SKIRMISH; }
    bool Is_In_Lan_Game() { return m_gameMode == GAME_LAN; }

    bool Is_In_Single_Player_Game();
    bool Is_Intro_Movie_Playing();

    unsigned int Get_CRC(int crc_source, Utf8String deep_crc_name);
#ifdef GAME_DEBUG_STRUCTS
    void Get_AI_Stats(
        int *objects, int *moving_objects, int *attacking_objects, int *waiting_objects, int *failed_pathfinds);
#endif

    ObjectTOCEntry *Find_TOC_Entry_By_Name(Utf8String name);
    ObjectTOCEntry *Find_TOC_Entry_By_ID(unsigned short id);
    void Add_TOC_Entry(Utf8String name, unsigned short id);
    void Xfer_Object_TOC(Xfer *xfer);

    void Set_Defaults(bool keep_obj_id);

    void Set_Buildable_Status_Override(ThingTemplate const *thing, BuildableStatus status);
    bool Find_Buildable_Status_Override(ThingTemplate const *thing, BuildableStatus &status) const;

    void Set_Control_Bar_Override(Utf8String const &s, int i, CommandButton const *button);
    bool Find_Control_Bar_Override(Utf8String const &s, int i, CommandButton const *&button) const;

    LoadScreen *Get_Load_Screen(bool b);
    void Update_Load_Progress(int percentage);
    void Delete_Load_Screen();
    void Process_Progress(int player_id, int percentage);
    void Process_Progress_Complete(int player_id);
    bool Is_Progress_Complete();
    void Last_Heard_From(int player_id);

    void Test_Time_Out();
    void Time_Out_Game_Start();
    void Init_Time_Out_Values();

    void Start_New_Game(bool restart);
    void Load_Map_INI(Utf8String map_file);

    bool Is_Game_Paused();
    void Set_Game_Paused(bool pause, bool pause_music);

    void Process_Destroy_List();
    void Process_Command_List(CommandList *list);

    void Destroy_All_Objects_Immediate();
    void Select_Object(Object *obj, bool force_select, unsigned short player_mask, bool select_ui);
    void Deselect_Object(Object *obj, unsigned short player_mask, bool deselect_ui);
    ObjectID Allocate_Object_ID();
    void Add_Object_To_Lookup_Table(Object *obj);
    void Remove_Object_From_Lookup_Table(Object *obj);
    void Register_Object(Object *obj);
    Object *Friend_Create_Object(ThingTemplate const *thing, BitFlags<OBJECT_STATUS_COUNT> &status_bits, Team *team);
    void Destroy_Object(Object *obj);
    void Send_Object_Created(Object *obj);
    void Bind_Object_And_Drawable(Object *obj, Drawable *d);
    void Send_Object_Destroyed(Object *obj);
    unsigned int Get_Object_Count();
    void Prepare_Logic_For_Object_Load();
    Object *Get_First_Object();

    void Erase_Sleepy_Update(int index);
    int Rebalance_Parent_Sleepy_Update(int index);
    int Rebalance_Child_Sleepy_Update(int index);
    void Rebalance_Sleepy_Update(int index);
    void Remake_Sleepy_Update();
    void Push_Sleepy_Update(UpdateModule *module);
    UpdateModule *Peek_Sleepy_Update();
    void Pop_Sleepy_Update();
    void Friend_Awaken_Update_Module(Object *object, UpdateModule *module, unsigned int wakeup_frame);

    // per Mac, these are in gamelogicdispatch.cpp
    void Close_Windows();
    void Clear_Game_Data(bool b);
    void Prepare_New_Game(int mode, GameDifficulty difficulty, int rank_points);
    void Logic_Message_Dispatcher(GameMessage *msg, void *user_data);

private:
#ifdef THYME_USE_STLPORT
    std::hash_map<Utf8String, BuildableStatus, rts::hash<Utf8String>, std::equal_to<Utf8String>>
        m_thingTemplateBuildableOverrides;
    std::hash_map<Utf8String, CommandButton const *, rts::hash<Utf8String>, std::equal_to<Utf8String>> m_controlBarOverrides;
#else
    std::unordered_map<Utf8String, BuildableStatus, rts::hash<Utf8String>, std::equal_to<Utf8String>>
        m_thingTemplateBuildableOverrides;
    std::unordered_map<Utf8String, CommandButton const *, rts::hash<Utf8String>, std::equal_to<Utf8String>>
        m_controlBarOverrides;
#endif
    float m_width;
    float m_height;
    unsigned int m_frame;
    unsigned int m_crc;
    std::map<int, unsigned int> m_crcCheckList;
    bool m_checkCRCs;
    bool m_prepareNewGame;
    bool m_loadingGameStateMap;
    bool m_clearingGameData;
    bool m_inGameLogicUpdate;
    int m_rankPointsToAddAtGameStart;
    bool m_scoringEnabled;
    bool m_occlusionEnabled;
    bool m_drawIconUI;
    bool m_dynamicLOD;
    int m_hulkLifetimeOverride;
    bool m_startNewGame;
    WindowLayout *m_background;
    Object *m_objList;
    std::vector<Object *> m_objectLookupTable;
    std::vector<UpdateModule *> m_sleepingUpdateModules;
    UpdateModule *m_currentUpdateModule;
    std::list<Object *> m_objectsToDestroy;
    ObjectID m_nextObjID;
    GameMode m_gameMode;
    int m_rankLevelLimit;
    unsigned short m_maxSimultaneousOfType;
    LoadScreen *m_loadScreen;
    bool m_gamePaused;
    bool m_inputEnabled;
    bool m_mouseVisible;
    bool m_progressComplete[8];
    int m_progressCompleteTimeout[8];
    bool m_forceGameStartByTimeOut;
#ifdef GAME_DEBUG_STRUCTS
    int m_failedPathFinds;
#endif
    int m_frameTriggerAreasChanged;
    std::list<ObjectTOCEntry> m_objectTOCEntries;
};

#ifdef GAME_DLL
extern GameLogic *&g_theGameLogic;
#else
extern GameLogic *g_theGameLogic;
#endif
