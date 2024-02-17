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
#pragma once

#include "always.h"
#include "player.h"
#include "snapshot.h"
#include "subsysteminterface.h"

class Team;

enum PlayerRelationshipFlags : uint16_t
{
    PLAYER_RELATIONSHIP_FLAGS_INCLUDE_SELF = (1U << 0),
    PLAYER_RELATIONSHIP_FLAGS_ALLIES = (1U << 1),
    PLAYER_RELATIONSHIP_FLAGS_ENEMIES = (1U << 2),
    PLAYER_RELATIONSHIP_FLAGS_NEUTRALS = (1U << 3),
};
DEFINE_ENUMERATION_BITWISE_OPERATORS(PlayerRelationshipFlags);

class PlayerList : public SubsystemInterface, public SnapShot
{
public:
    PlayerList();
    virtual ~PlayerList() override;

    // SubsystemInterface interface
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void New_Game();
    virtual void New_Map();

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    Player *Find_Player_With_NameKey(NameKeyType key);
    void Team_About_To_Be_Deleted(Team *team);
    void Update_Team_States();
    Team *Validate_Team(Utf8String name);
    void Set_Local_Player(Player *player);
    Player *Get_Player_From_Mask(uint16_t mask);
    Player *Get_Each_Player_From_Mask(uint16_t &mask);
    uint16_t Get_Players_With_Relationship(int32_t player_idx, PlayerRelationshipFlags relationship_mask);

    // wb: 0x0061D780
    Player *Get_Local_Player()
    {
        captainslog_dbgassert(m_local, "null m_local");
        return m_local;
    }

    // wb: 0x0075A420
    Player *Get_Neutral_Player()
    {
        captainslog_dbgassert(m_players[0], "null neutral");
        return m_players[0];
    }

    // zh: 0x0045A9C0 wb: 0x007BEFE7
    Player *Get_Nth_Player(int32_t i)
    {
        if (i < 0 || i >= MAX_PLAYER_COUNT) {
            return nullptr;
        }

        return m_players[i];
    }

    // wb: 0x0071C510
    int32_t Get_Player_Count() { return m_playerCount; }

    PlayerList *Hook_Ctor() { return new (this) PlayerList; }

private:
    Player *m_local;
    int m_playerCount;
    Player *m_players[MAX_PLAYER_COUNT];
};

#ifdef GAME_DLL
#include "hooker.h"

extern PlayerList *&g_thePlayerList;
#else
extern PlayerList *g_thePlayerList;
#endif