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
#ifdef GAME_DLL
    Call_Method<void, SubsystemInterface>(PICK_ADDRESS(0x0045AA20, 0x007BF05E), this);
#else
    // Requires TeamFactory
    // g_theTeamFactory->Clear();
    Init();
#endif
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
#ifdef GAME_DLL
    Call_Method<void, PlayerList>(PICK_ADDRESS(0x0045AA40, 0x007BF07F), this);
#else
    // Requires TeamFactory, g_theNetwork
#endif
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
#ifdef GAME_DLL
    return Call_Method<Team *, PlayerList, Utf8String>(PICK_ADDRESS(0x0045B0F0, 0x007BF685), this, name);
#else
    // Requires TeamFactory::Find_Team
    return nullptr;
#endif
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
uint16_t PlayerList::Get_Players_With_Relationship(int32_t player_idx, uint16_t relationship_mask)
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
