/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Contains a list of players in the game.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "playerlist.h"
#include "network.h"
#include "sideslist.h"
#include "staticnamekey.h"
#include "team.h"

#ifdef GAME_DLL
#else
PlayerList *g_thePlayerList = nullptr;
#endif

// zh: 0x0045A7C0 wb: 0x007BEE10
PlayerList::PlayerList() : m_local(nullptr), m_playerCount(0)
{
    for (auto i = 0; i < MAX_PLAYER_COUNT; ++i) {
        m_players[i] = new Player(i);
    }
    Init();
}

// zh: 0x0045A8D0 wb: 0x007BEEFD
PlayerList::~PlayerList()
{
    Init(); // TODO: Investigate why destructor calls init
    for (auto *player : m_players) {
        delete player;
    }
}

// zh: 0x0045B000 wb: 0x007BF549
void PlayerList::Init()
{
    m_playerCount = 1;
    for (auto *player : m_players) {
        player->Init(nullptr);
    }
    Set_Local_Player(m_players[0]);
}

// zh: 0x0045AA20 wb: 0x007BF05E
void PlayerList::Reset()
{
    g_theTeamFactory->Clear();
    Init();
}

// zh: 0x0045B060 wb: 0x007BF5A7
void PlayerList::Update()
{
    for (auto *player : m_players) {
        player->Update();
    }
}

// zh: 0x0045AA40 wb: 0x007BF07F
void PlayerList::New_Game()
{
    captainslog_dbgassert(this != nullptr, "null this");
    g_theTeamFactory->Clear();
    Init();
    bool is_local = false;

    for (int i = 0; i < g_theSidesList->Get_Num_Sides(); i++) {
        Dict &dict = g_theSidesList->Get_Side_Info(i)->Get_Dict();
        Utf8String name = dict.Get_AsciiString(g_playerNameKey.Key());

        if (!name.Is_Empty()) {
            Player *player = m_players[m_playerCount++];
            player->Init_From_Dict(&dict);
            bool exists;

            if (dict.Get_Bool(g_multiplayerIsLocalKey.Key(), &exists)) {
                captainslog_debug("Player %s is multiplayer local", name.Str());
                Set_Local_Player(player);
                is_local = true;
            }

            if (!is_local && g_theNetwork == nullptr) {
                if (dict.Get_Bool(g_playerIsHumanKey.Key())) {
                    Set_Local_Player(player);
                    is_local = true;
                }
            }

            player->Set_Build_List(g_theSidesList->Get_Side_Info(i)->Get_Build_List());
            g_theSidesList->Get_Side_Info(i)->Clear_Build_List();
        }
    }

    if (!is_local) {
        captainslog_dbgassert(
            g_theNetwork != nullptr, "*** Map has no human player... picking first nonneutral player for control");

        for (int i = 0; i < g_theSidesList->Get_Num_Sides(); i++) {
            Player *player = Get_Nth_Player(i);

            if (player != Get_Neutral_Player()) {
                player->Set_Player_Type(Player::PLAYER_HUMAN, false);
                Set_Local_Player(player);
                is_local = true;
                break;
            }
        }
    }

    g_theTeamFactory->Init_From_Sides(g_theSidesList);

    for (int i = 0; i < g_theSidesList->Get_Num_Sides(); i++) {
        Dict &dict = g_theSidesList->Get_Side_Info(i)->Get_Dict();
        Player *player =
            Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(dict.Get_AsciiString(g_playerNameKey.Key()).Str()));
        Utf8String enemies = dict.Get_AsciiString(g_playerEnemiesKey.Key());
        Utf8String enemy;

        while (enemies.Next_Token(&enemy)) {
            Player *enemy_player = Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(enemy.Str()));

            if (enemy_player != nullptr) {
                player->Set_Player_Relationship(enemy_player, ENEMIES);
            } else {
                captainslog_debug("unknown enemy %s", enemy.Str());
            }
        }

        Utf8String allies = dict.Get_AsciiString(g_playerAlliesKey.Key());
        Utf8String ally;

        while (allies.Next_Token(&ally)) {
            Player *ally_player = Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(ally.Str()));

            if (ally_player != nullptr) {
                player->Set_Player_Relationship(ally_player, ALLIES);
            } else {
                captainslog_debug("unknown ally %s", ally.Str());
            }
        }

        player->Set_Player_Relationship(player, ALLIES);

        if (player != Get_Neutral_Player()) {
            player->Set_Player_Relationship(Get_Neutral_Player(), NEUTRAL);
        }

        player->Set_Default_Team();
    }
}

// zh: 0x0045B080 wb: 0x007BF5DD
void PlayerList::New_Map()
{
    for (auto *player : m_players) {
        player->New_Map();
    }
}

// zh: 0x0045B330 wb: 0x007BFA43
void PlayerList::CRC_Snapshot(Xfer *xfer)
{
    xfer->xferInt(&m_playerCount);
    for (auto i = 0; i < m_playerCount; ++i) {
        xfer->xferSnapshot(m_players[i]);
    }
}

// zh: 0x0045B370 wb: 0x007BFA99
void PlayerList::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    auto player_count = m_playerCount;
    xfer->xferInt(&player_count);
    captainslog_dbgassert(
        player_count == m_playerCount, "Invalid player count \'%d\', should be \'%d\'", player_count, m_playerCount);

    for (auto i = 0; i < player_count; ++i) {
        xfer->xferSnapshot(m_players[i]);
    }
}

// zh: 0x0045A9E0 wb: 0x007BF00E
Player *PlayerList::Find_Player_With_NameKey(NameKeyType key)
{
    for (auto i = 0; i < m_playerCount; ++i) {
        auto *player = m_players[i];
        if (player->Get_Player_NameKey() == key) {
            return player;
        }
    }
    return nullptr;
}

// zh: 0x0045B0A0 wb: 0x007BF613
void PlayerList::Team_About_To_Be_Deleted(Team *team)
{
    for (auto *player : m_players) {
        player->Remove_Team_Relationship(team);
    }
}

// zh: 0x0045B0D0 wb: 0x007BF64F
void PlayerList::Update_Team_States()
{
    for (auto *player : m_players) {
        player->Update_Team_States();
    }
}

// zh: 0x0045B0F0 wb: 0x007BF685
Team *PlayerList::Validate_Team(Utf8String name)
{
    Team *team = g_theTeamFactory->Find_Team(name);

    if (team == nullptr) {
        captainslog_dbgassert(false, "no team or player named %s could be found!", name.Str());
        team = Get_Neutral_Player()->Get_Default_Team();
    }

    return team;
}

// zh: 0x0045B170 wb: 0x007BF739
void PlayerList::Set_Local_Player(Player *player)
{
    if (player == nullptr) {
        captainslog_dbgassert(false, "Local player may not be null");
        player = Get_Neutral_Player();
    }

    if (player == m_local) {
        return;
    }

    if (m_local != nullptr) {
        m_local->Becoming_Local_Player(false);
    }
    m_local = player;
    player->Becoming_Local_Player(true);
}

// zh: 0x0045B1B0 wb: 0x007BF7BD
Player *PlayerList::Get_Player_From_Mask(uint16_t mask)
{
    for (auto i = 0; i < MAX_PLAYER_COUNT; ++i) {
        auto *player = Get_Nth_Player(i);
        if (player == nullptr) {
            continue;
        }

        if (player->Get_Player_Mask() & mask) {
            return player;
        }
    }
    captainslog_dbgassert(false, "Player does not exist for mask");
    return nullptr;
}

// zh: 0x0045B1F0 wb: 0x007BF855
Player *PlayerList::Get_Each_Player_From_Mask(uint16_t &mask)
{
    for (auto i = 0; i < MAX_PLAYER_COUNT; ++i) {
        auto *player = Get_Nth_Player(i);
        if (player == nullptr) {
            continue;
        }

        if (player->Get_Player_Mask() & mask) {
            mask &= ~player->Get_Player_Mask();
            return player;
        }
    }
    captainslog_dbgassert(false, "No players found that contain any matching masks.");
    mask = 0;
    return nullptr;
}

// zh: 0x0045B250 wb: 0x007BF914
uint16_t PlayerList::Get_Players_With_Relationship(int32_t player_idx, PlayerRelationshipFlags relationship_mask)
{
    if (relationship_mask == 0) {
        return 0;
    }

    uint16_t player_mask = 0;
    auto *target_player = Get_Nth_Player(player_idx);
    if (target_player == nullptr) {
        return 0;
    }

    if (relationship_mask & PLAYER_RELATIONSHIP_FLAGS_INCLUDE_SELF) {
        player_mask |= target_player->Get_Player_Mask();
    }

    for (auto i = 0; i < Get_Player_Count(); ++i) {
        auto *player = Get_Nth_Player(i);
        if (player == nullptr || player == target_player) {
            continue;
        }

        const auto relationship = target_player->Get_Relationship(player->Get_Default_Team());
        switch (relationship) {
            case Relationship::ENEMIES:
                if (relationship_mask & PLAYER_RELATIONSHIP_FLAGS_ENEMIES) {
                    player_mask |= player->Get_Player_Mask();
                }
                break;
            case Relationship::NEUTRAL:
                if (relationship_mask & PLAYER_RELATIONSHIP_FLAGS_NEUTRALS) {
                    player_mask |= player->Get_Player_Mask();
                }
                break;
            case Relationship::ALLIES:
                if (relationship_mask & PLAYER_RELATIONSHIP_FLAGS_ALLIES) {
                    player_mask |= player->Get_Player_Mask();
                }
                break;
            default:
                break;
        }
    }
    return player_mask;
}
