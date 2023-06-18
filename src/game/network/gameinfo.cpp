/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game Info
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameinfo.h"
#include "captainslog.h"
#include "filesystem.h"
#include "filetransfer.h"
#include "gametext.h"
#include "globaldata.h"
#include "maputil.h"
#include "multiplayersettings.h"
#include "peerdefs.h"
#include "playertemplate.h"
#include "xfer.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
GameInfo *g_theGameInfo;
SkirmishGameInfo *g_theSkirmishGameInfo;
SkirmishGameInfo *g_theChallengeGameInfo;
#endif

int Get_Slot_Index(const GameSlot *slot)
{
    for (int i = 0; i < GameInfo::MAX_SLOTS; i++) {
        if (g_theGameInfo->Get_Const_Slot(i) == slot) {
            return i;
        }
    }

    return -1;
}

bool Is_Slot_Local_Ally(const GameSlot *slot)
{
    int index = Get_Slot_Index(slot);
    int local_index = g_theGameInfo->Get_Local_Slot_Num();
    const GameSlot *local_slot = g_theGameInfo->Get_Const_Slot(local_index);

    if (index < 0 || local_index < 0) {
        return false;
    }

    if (index == local_index) {
        return true;
    }

    return (slot->Get_Team_Number() == local_slot->Get_Team_Number() && slot->Get_Team_Number() >= 0)
        || local_slot->Get_Original_Player_Template() == -2;
}

Utf16String GameSlot::Get_Apparent_Player_Template_Display_Name() const
{
    if (g_theMultiplayerSettings != nullptr && g_theMultiplayerSettings->Is_Show_Random_Player_Template()
        && m_originalPlayerTemplate == -1 && !Is_Slot_Local_Ally(this)) {
        return g_theGameText->Fetch("GUI:Random");
    } else if (m_originalPlayerTemplate == -2) {
        return g_theGameText->Fetch("GUI:Observer");
    } else {
        captainslog_debug("Fetching player template display name for player template %d (orig is %d)",
            m_playerTemplate,
            m_originalPlayerTemplate);

        if (m_playerTemplate >= 0) {
            return g_thePlayerTemplateStore->Get_Nth_Player_Template(m_playerTemplate)->Get_Display_Name();
        } else {
            return g_theGameText->Fetch("GUI:Random");
        }
    }
}

int GameSlot::Get_Apparent_Player_Template() const
{
    if (g_theMultiplayerSettings != nullptr && g_theMultiplayerSettings->Is_Show_Random_Player_Template()
        && !Is_Slot_Local_Ally(this)) {
        return m_originalPlayerTemplate;
    } else {
        return m_playerTemplate;
    }
}

int GameSlot::Get_Apparent_Start_Pos() const
{
    if (g_theMultiplayerSettings != nullptr && g_theMultiplayerSettings->Is_Show_Random_Start_Pos()
        && !Is_Slot_Local_Ally(this)) {
        return m_originalStartPos;
    } else {
        return m_startPos;
    }
}

int GameSlot::Get_Apparent_Color() const
{
    if (g_theMultiplayerSettings != nullptr && m_originalPlayerTemplate == -2) {
        return g_theMultiplayerSettings->Get_Color(-2)->Get_Color();
    } else if (g_theMultiplayerSettings != nullptr && g_theMultiplayerSettings->Is_Show_Random_Color()
        && !Is_Slot_Local_Ally(this)) {
        return m_originalColor;
    } else {
        return m_color;
    }
}

bool GameSlot::Is_Occupied() const
{
    return m_state == SLOT_PLAYER || m_state == SLOT_EASY_AI || m_state == SLOT_MED_AI || m_state == SLOT_HARD_AI;
}

bool GameSlot::Is_Open() const
{
    return m_state == SLOT_OPEN;
}

void GameSlot::Save_Off_Original_Info()
{
    captainslog_debug("GameSlot::Save_Off_Original_Info() - orig was color=%d, pos=%d, house=%d",
        m_originalColor,
        m_originalStartPos,
        m_originalPlayerTemplate);
    m_originalPlayerTemplate = m_playerTemplate;
    m_originalStartPos = m_startPos;
    m_originalColor = m_color;
    captainslog_debug(
        "GameSlot::Save_Off_Original_Info() - color=%d, pos=%d, house=%d", m_color, m_startPos, m_playerTemplate);
}

bool GameSlot::Is_AI() const
{
    return m_state == SLOT_EASY_AI || m_state == SLOT_MED_AI || m_state == SLOT_HARD_AI;
}

void GameSlot::Reset()
{
    m_state = SLOT_CLOSED;
    m_isAccepted = false;
    m_hasMap = true;
    m_color = -1;
    m_startPos = -1;
    m_playerTemplate = -1;
    m_teamNumber = -1;
    m_NATBehavior = 1;
    m_unk2 = 0;
    m_unk3 = false;
    m_port = 0;
    m_unk = false;
    m_originalPlayerTemplate = -1;
    m_originalStartPos = -1;
    m_originalColor = -1;
}

bool GameSlot::Is_Player(unsigned int ip) const
{
    return m_state == SLOT_PLAYER && m_IP == ip;
}

bool GameSlot::Is_Player(Utf8String name) const
{
    Utf16String str;
    str.Translate(name);
    return m_state == SLOT_PLAYER && m_name.Compare_No_Case(str) == 0;
}

bool GameSlot::Is_Player(Utf16String name) const
{
    return m_state == SLOT_PLAYER && m_name.Compare_No_Case(name) == 0;
}

void GameSlot::Un_Accept()
{
    if (Is_Human()) {
        m_isAccepted = false;
    }
}

bool GameSlot::Is_Human() const
{
    return m_state == SLOT_PLAYER;
}

void GameSlot::Set_Map_Availability(bool available)
{
    if (Is_Human()) {
        m_hasMap = available;
    }
}

void GameSlot::Set_State(SlotState state, Utf16String name, unsigned int IP)
{
    if (!Is_AI() || (state != SLOT_EASY_AI && state != SLOT_MED_AI && state != SLOT_HARD_AI)) {
        m_color = -1;
        m_startPos = -1;
        m_playerTemplate = -1;
        m_teamNumber = -1;

        captainslog_dbgassert(
            state != SLOT_OPEN || g_theGameSpyGame == nullptr || g_theGameSpyGame->Get_Const_Slot(0) != this,
            "Game Is Hosed!");
    }

    if (state == SLOT_PLAYER) {
        Reset();
        m_state = SLOT_PLAYER;
        m_name = name;
    } else {
        m_state = state;
        m_isAccepted = true;
        m_hasMap = true;

        switch (state) {
            case SLOT_OPEN:
                m_name = g_theGameText->Fetch("GUI:Open");
                break;
            case SLOT_EASY_AI:
                m_name = g_theGameText->Fetch("GUI:EasyAI");
                break;
            case SLOT_MED_AI:
                m_name = g_theGameText->Fetch("GUI:MediumAI");
                break;
            case SLOT_HARD_AI:
                m_name = g_theGameText->Fetch("GUI:HardAI");
                break;
            default:
                m_name = g_theGameText->Fetch("GUI:Closed");
                break;
        }
    }

    m_IP = IP;
}

GameInfo::GameInfo()
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        m_slot[i] = nullptr;
    }

    Reset();
}

void GameInfo::Init()
{
    Reset();
}

bool GameInfo::Is_Player_Preorder(int player) const
{
    return player <= 7 && ((1 << player) & m_isPlayerPreorder) != 0;
}

void GameInfo::Mark_Player_As_Preorder(int player)
{
    if (player <= 7) {
        m_isPlayerPreorder |= 1 << player;
    }
}

GameSlot *GameInfo::Get_Slot(int num)
{
    captainslog_dbgassert(num < MAX_SLOTS, "GameInfo::Get_Slot - Invalid slot number");

    if (num < MAX_SLOTS) {
        return m_slot[num];
    } else {
        return nullptr;
    }
}

const GameSlot *GameInfo::Get_Const_Slot(int num) const
{
    captainslog_dbgassert(num < MAX_SLOTS, "GameInfo::Get_Slot - Invalid slot number");

    if (num < MAX_SLOTS) {
        return m_slot[num];
    } else {
        return nullptr;
    }
}

bool GameInfo::Is_Color_Taken(int index, int ignore) const
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        const GameSlot *slot = Get_Const_Slot(i);

        if (slot != nullptr && slot->Get_Color() == index && i != ignore) {
            return true;
        }
    }

    return false;
}

const int NET_CRC_INTERVAL = 100;

void GameInfo::Reset()
{
    m_crcInterval = NET_CRC_INTERVAL;
    m_inGame = false;
    m_inProgress = false;
    m_gameID = 0;
    m_mapName = "NOMAP";
    m_mapContentsMask = 0;
#ifdef PLATFORM_WINDOWS
    m_seed = GetTickCount();
#else
    // TODO linux
#endif
    m_useStats = true;
    m_unk = false;
    m_originalArmies = false;
    m_mapCRC = 0;
    m_mapSize = 0;
    m_superweaponRestriction = 0;
    m_money = g_theWriteableGlobalData->m_defaultStartingCash;
}

void GameInfo::Start_Game(int game_id)
{
    captainslog_dbgassert(m_inGame && !m_inProgress, "Starting game at a bad time!");
    m_gameID = game_id;
    Close_Open_Slots();
    m_inProgress = true;
}

void GameInfo::End_Game()
{
    captainslog_dbgassert(m_inGame && m_inProgress, "Ending game without playing one!");
    m_inGame = false;
    m_inProgress = false;
}

bool GameInfo::Am_I_Host() const
{
    captainslog_dbgassert(m_inGame, "Looking for game slot while not in game");

    if (!m_inGame) {
        return false;
    }

    return Get_Const_Slot(0)->Is_Player(m_localIP);
}

int GameInfo::Get_Local_Slot_Num() const
{
    captainslog_dbgassert(m_inGame, "Looking for game slot while not in game");

    if (!m_inGame) {
        return -1;
    }

    for (int i = 0; i < MAX_SLOTS; i++) {
        const GameSlot *slot = Get_Const_Slot(i);

        if (slot != nullptr && slot->Is_Player(m_localIP)) {
            return i;
        }
    }

    return -1;
}

void GameInfo::Reset_Accepted()
{
    GameSlot *slot = Get_Slot(0);

    if (slot != nullptr) {
        slot->Set_Accepted();
    }

    for (int i = 1; i < MAX_SLOTS; i++) {
        slot = Get_Slot(i);

        if (slot != nullptr) {
            slot->Un_Accept();
        }
    }
}

void GameInfo::Reset_Start_Spots()
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        GameSlot *slot = Get_Slot(i);

        if (slot != nullptr) {
            slot->Set_Start_Pos(-1);
        }
    }
}

void GameInfo::Adjust_Slots_For_Map(int i)
{
    MapMetaData *meta_data = g_theMapCache->Find_Map(m_mapName);

    if (meta_data != nullptr) {
        int num_players = meta_data->m_numPlayers;
        int occupied_count = 0;

        for (int i = 0; i < MAX_SLOTS; i++) {
            GameSlot *slot = Get_Slot(i);

            if (slot->Is_Occupied()) {
                occupied_count++;
            }
        }

        for (int i = 0; i < MAX_SLOTS; i++) {
            GameSlot *slot = Get_Slot(i);

            if (num_players <= occupied_count) {
                if (!slot->Is_Occupied()) {
                    GameSlot new_slot;
                    new_slot.Set_State(SLOT_CLOSED, Utf16String::s_emptyString, 0);
                    Set_Slot(i, new_slot);
                }
            } else {
                if (!slot->Is_Occupied()) {
                    GameSlot new_slot;
                    new_slot.Set_State(SLOT_OPEN, Utf16String::s_emptyString, 0);
                    Set_Slot(i, new_slot);
                    occupied_count++;
                }
            }
        }
    }
}

void GameInfo::Close_Open_Slots()
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        GameSlot *slot = Get_Slot(i);

        if (!slot->Is_Occupied()) {
            GameSlot new_slot;
            new_slot.Set_State(SLOT_CLOSED, Utf16String::s_emptyString, 0);
            Set_Slot(i, new_slot);
        }
    }
}

bool GameInfo::Is_Start_Position_Taken(int index, int ignore) const
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        const GameSlot *slot = Get_Const_Slot(i);

        if (slot != nullptr && slot->Get_Start_Pos() == index && i != ignore) {
            return true;
        }
    }

    return false;
}

int GameInfo::Get_Num_Players() const
{
    int num_players = 0;

    for (int i = 0; i < MAX_SLOTS; i++) {
        if (m_slot[i] != nullptr) {
            if (m_slot[i]->Is_Occupied()) {
                num_players++;
            }
        }
    }

    return num_players;
}

int GameInfo::Get_Num_Non_Observer_Players() const
{
    int num_players = 0;

    for (int i = 0; i < MAX_SLOTS; i++) {
        if (m_slot[i] != nullptr && m_slot[i]->Is_Occupied() && m_slot[i]->Get_Player_Template() != -2) {
            num_players++;
        }
    }

    return num_players;
}

int GameInfo::Get_Max_Players() const
{
    if (g_theMapCache == nullptr) {
        return -1;
    }

    Utf8String str(m_mapName);
    str.To_Lower();

    auto it = g_theMapCache->find(str);

    if (it == g_theMapCache->end()) {
        return -1;
    } else {
        MapMetaData data = it->second;
        return data.m_numPlayers;
    }
}

void GameInfo::Enter_Game()
{
    captainslog_dbgassert(!m_inGame && !m_inProgress, "Entering game at a bad time!");
    Reset();
    m_inGame = true;
    m_inProgress = false;
}

void GameInfo::Leave_Game()
{
    captainslog_dbgassert(m_inGame && !m_inProgress, "Leaving game at a bad time!");
    Reset();
}

void GameInfo::Set_Slot(int index, GameSlot slot)
{
    captainslog_dbgassert(index < MAX_SLOTS, "GameInfo::setSlot - Invalid slot number");

    if (index >= MAX_SLOTS) {
        return;
    }

    captainslog_dbgassert(m_slot[index] != nullptr, "NULL slot pointer");

    if (m_slot[index] != nullptr) {
        if (index == 0) {
            slot.Set_Accepted();
            slot.Set_Map_Availability(true);
        }

        *(m_slot[index]) = slot;
        unsigned int IP = slot.Get_IP();
        Utf16String name = slot.Get_Name();
        captainslog_debug("GameInfo::setSlot - setting slot %d to be player %ls with IP %d.%d.%d.%d",
            index,
            name.Str(),
            (IP & 0xFF000000) >> 24,
            (IP & 0xFF0000) >> 16,
            (IP & 0xFF00) >> 8,
            (IP & 0xFF));
    }
}

int GameInfo::Get_Slot_Num(Utf8String name) const
{
    captainslog_dbgassert(m_inGame, "Looking for game slot while not in game");

    if (m_inGame) {
        Utf16String str;
        str.Translate(name);

        for (int i = 0; i < MAX_SLOTS; i++) {
            const GameSlot *slot = Get_Const_Slot(i);

            if (slot->Is_Player(str)) {
                return i;
            }
        }

        return -1;
    } else {
        return -1;
    }
}

void GameInfo::Set_Map(Utf8String map_name)
{
    m_mapName = map_name;

    if (m_inGame && Am_I_Host()) {
        if (g_theMapCache->Find_Map(map_name) != nullptr) {
            m_mapContentsMask = 1;
            Utf8String image_name = map_name;
            image_name.Remove_Last_Char();
            image_name.Remove_Last_Char();
            image_name.Remove_Last_Char();
            image_name.Concat("tga");
            captainslog_debug("GameInfo::Set_Map() - Looking for '%s'", image_name.Str());
            File *file = g_theFileSystem->Open_File(image_name.Str(), 0);

            if (file != nullptr) {
                m_mapContentsMask |= 2;
                file->Close();
                file = nullptr;
            }

            Utf8String ini_name;

            if (map_name.Get_Length() > 0) {
                Utf8String str3;
                map_name.Next_Token(&str3, "\\/");

                while (map_name.Find('\\')) {
                    if (ini_name.Get_Length() > 0) {
                        ini_name.Concat('/');
                    }

                    ini_name += str3;
                    map_name.Next_Token(&str3, "\\/");
                }
            }

            ini_name.Concat("/map.ini");
            captainslog_debug("GameInfo::Set_Map() - Looking for '%s'", ini_name.Str());
            file = g_theFileSystem->Open_File(ini_name.Str(), 0);

            if (file != nullptr) {
                m_mapContentsMask |= 4;
                file->Close();
                file = nullptr;
            }

            image_name = Get_Str_File_From_Map(m_mapName);
            captainslog_debug("GameInfo::Set_Map() - Looking for '%s'", image_name.Str());
            file = g_theFileSystem->Open_File(image_name.Str(), 0);

            if (file != nullptr) {
                m_mapContentsMask |= 8;
                file->Close();
                file = nullptr;
            }

            image_name = Get_Solo_INI_From_Map(m_mapName);
            captainslog_debug("GameInfo::Set_Map() - Looking for '%s'", image_name.Str());
            file = g_theFileSystem->Open_File(image_name.Str(), 0);

            if (file != nullptr) {
                m_mapContentsMask |= 0x10;
                file->Close();
                file = nullptr;
            }

            image_name = Get_Asset_Usage_From_Map(m_mapName);
            captainslog_debug("GameInfo::Set_Map() - Looking for '%s'", image_name.Str());
            file = g_theFileSystem->Open_File(image_name.Str(), 0);

            if (file != nullptr) {
                m_mapContentsMask |= 0x20;
                file->Close();
                file = nullptr;
            }

            image_name = Get_Readme_From_Map(m_mapName);
            captainslog_debug("GameInfo::Set_Map() - Looking for '%s'", image_name.Str());
            file = g_theFileSystem->Open_File(image_name.Str(), 0);

            if (file != nullptr) {
                m_mapContentsMask |= 0x40;
                file->Close();
                file = nullptr;
            }
        } else {
            m_mapContentsMask = 0;
        }
    }
}

void GameInfo::Set_Map_Contents_Mask(int mask)
{
    m_mapContentsMask = mask;
}

void GameInfo::Set_Map_CRC(unsigned int crc)
{
    m_mapCRC = crc;

    if (g_theMapCache != nullptr && m_inGame && Get_Local_Slot_Num() >= 0) {
        Utf8String name(m_mapName);
        name.To_Lower();
        auto it = g_theMapCache->find(name);

        if (it == g_theMapCache->end()) {
            Get_Slot(Get_Local_Slot_Num())->Set_Map_Availability(false);
        } else if (m_mapCRC == it->second.m_CRC) {
            Get_Slot(Get_Local_Slot_Num())->Set_Map_Availability(true);
        } else {
            captainslog_debug("GameInfo::Set_Map_CRC - map CRC's do not match (%X/%X).", m_mapCRC, it->second.m_CRC);
            Get_Slot(Get_Local_Slot_Num())->Set_Map_Availability(false);
        }
    }
}

void GameInfo::Set_Map_Size(unsigned int size)
{
    m_mapSize = size;

    if (g_theMapCache != nullptr && m_inGame && Get_Local_Slot_Num() >= 0) {
        Utf8String name(m_mapName);
        name.To_Lower();
        auto it = g_theMapCache->find(name);

        if (it == g_theMapCache->end()) {
            captainslog_debug("GameInfo::Set_Map_Size - could not find map file.");
            Get_Slot(Get_Local_Slot_Num())->Set_Map_Availability(false);
        } else if (m_mapCRC == it->second.m_CRC) {
            Get_Slot(Get_Local_Slot_Num())->Set_Map_Availability(true);
        } else {
            captainslog_debug("GameInfo::Set_Map_Size - map CRC's do not match.");
            Get_Slot(Get_Local_Slot_Num())->Set_Map_Availability(false);
        }
    }
}

void GameInfo::Set_Seed(int seed)
{
    m_seed = seed;
}

void GameInfo::Set_Slot_Pointer(int index, GameSlot *slot)
{
    if (index < MAX_SLOTS) {
        m_slot[index] = slot;
    }
}

void GameInfo::Set_Superweapon_Restriction(unsigned short restriction)
{
    m_superweaponRestriction = restriction;
}

void GameInfo::Set_Starting_Cash(const Money &cash)
{
    m_money = cash;
}

bool GameInfo::Is_Skirmish()
{
    bool is_skirmish = false;

    for (int i = 0; i < MAX_SLOTS; i++) {
        if (i != Get_Local_Slot_Num()) {
            if (Get_Const_Slot(i)->Is_Human()) {
                return false;
            }

            if (Get_Const_Slot(i)->Is_AI()) {
                if (Is_Slot_Local_Ally(Get_Const_Slot(i))) {
                    return false;
                }

                is_skirmish = true;
            }
        }
    }

    return is_skirmish;
}

bool GameInfo::Is_Multi_Player()
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (i != Get_Local_Slot_Num()) {
            if (Get_Const_Slot(i)->Is_Human()) {
                return true;
            }
        }
    }

    return false;
}

bool GameInfo::Is_Sandbox()
{
    int local_index = Get_Local_Slot_Num();
    int team = Get_Const_Slot(local_index)->Get_Team_Number();

    for (int i = 0; i < MAX_SLOTS; i++) {
        if (i != local_index) {
            const GameSlot *slot = Get_Const_Slot(i);

            if (slot->Is_Occupied() && (slot->Get_Team_Number() < 0 || slot->Get_Team_Number() != team)) {
                return false;
            }
        }
    }

    return true;
}

SkirmishGameInfo::SkirmishGameInfo()
{
    for (int i = 0; i < MAX_SLOTS; i++) {
        Set_Slot_Pointer(i, &m_gameSlot[i]);
    }
}

void SkirmishGameInfo::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 4;
    xfer->xferVersion(&version, 4);
    xfer->xferInt(&m_isPlayerPreorder);
    xfer->xferInt(&m_crcInterval);
    xfer->xferBool(&m_inGame);
    xfer->xferBool(&m_inProgress);
    xfer->xferBool(&m_unk);
    xfer->xferInt(&m_gameID);
    int slot_count = MAX_SLOTS;
    xfer->xferInt(&slot_count);
    captainslog_dbgassert(slot_count == MAX_SLOTS, "MAX_SLOTS changed, need to change version.");

    for (int i = 0; i < MAX_SLOTS; i++) {
        SlotState state = m_slot[i]->Get_State();
        xfer->xferInt(reinterpret_cast<int *>(&state));
        Utf16String name = m_slot[i]->Get_Name();

        if (version >= 2) {
            xfer->xferUnicodeString(&name);
        }

        bool accepted = m_slot[i]->Is_Accepted();
        xfer->xferBool(&accepted);
        bool unk = m_slot[i]->Get_Unk();
        xfer->xferBool(&unk);
        m_slot[i]->Set_Unk(unk);
        int color = m_slot[i]->Get_Color();
        xfer->xferInt(&color);
        int start_pos = m_slot[i]->Get_Start_Pos();
        xfer->xferInt(&start_pos);
        int player_template = m_slot[i]->Get_Player_Template();
        xfer->xferInt(&player_template);
        int team = m_slot[i]->Get_Team_Number();
        xfer->xferInt(&team);
        int original_color = m_slot[i]->Get_Original_Color();
        xfer->xferInt(&original_color);
        int original_start_pos = m_slot[i]->Get_Original_Start_Pos();
        xfer->xferInt(&original_start_pos);
        int original_player_template = m_slot[i]->Get_Original_Player_Template();
        xfer->xferInt(&original_player_template);

        if (xfer->Get_Mode() == XFER_LOAD) {
            m_slot[i]->Set_State(state, name, 0);

            if (accepted) {
                m_slot[i]->Set_Accepted();
            }

            m_slot[i]->Set_Player_Template(original_player_template);
            m_slot[i]->Set_Start_Pos(original_start_pos);
            m_slot[i]->Set_Color(original_color);
            m_slot[i]->Save_Off_Original_Info();
            m_slot[i]->Set_Team_Number(team);
            m_slot[i]->Set_Color(color);
            m_slot[i]->Set_Start_Pos(start_pos);
            m_slot[i]->Set_Player_Template(player_template);
        }
    }

    xfer->xferUnsignedInt(&m_localIP);
    xfer->xferMapName(&m_mapName);
    xfer->xferUnsignedInt(&m_mapCRC);
    xfer->xferUnsignedInt(&m_mapSize);
    xfer->xferInt(&m_mapContentsMask);
    xfer->xferInt(&m_seed);

    if (version < 3) {
        if (xfer->Get_Mode() == XFER_LOAD) {
            m_superweaponRestriction = 0;
            m_money = g_theWriteableGlobalData->m_defaultStartingCash;
        }
    } else {
        xfer->xferUnsignedShort(&m_superweaponRestriction);

        if (version == 3) {
            bool b;
            xfer->xferBool(&b);
        }

        xfer->xferSnapshot(&m_money);
    }
}
