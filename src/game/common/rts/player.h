/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object for tracking player information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "handicap.h"
#include "snapshot.h"
#include "unicodestring.h"

class PlayerTemplate;
class Team;
class SpecialPowerTemplate;
class Object;
class Waypoint;
enum Relationship;

// TODO this is just a skeleton to support other objects at the moment.
class Player : public SnapShot
{
public:
    Player(int32_t player_index);
    virtual ~Player();

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    virtual void Compute_Superweapon_Target(const SpecialPowerTemplate *sp_template, Coord3D *loc, int32_t unk1, float unk2);
    virtual bool Check_Bridges(Object *obj, Waypoint *waypoint);
    virtual bool Get_Ai_Base_Center(Coord3D *center);
    virtual void Repair_Structure(ObjectID obj_id);

    NameKeyType Get_Player_NameKey() { return m_playerNameKey; }
    int Get_Player_Index() { return m_playerIndex; }
    uint16_t Get_Player_Mask() { return 1ULL << m_playerIndex; }
    int Get_Color() { return m_playerColor; }
    // wb: 0x006DA8F0
    Team *Get_Default_Team()
    {
        captainslog_dbgassert(m_defaultTeam != nullptr, "Default team is null.");
        return m_defaultTeam;
    }
    Relationship Get_Relationship(Team *team);

    void On_Power_Brown_Out_Change(bool b);
    void Init(const PlayerTemplate *player_template);
    void Update();
    void New_Map();
    bool Remove_Team_Relationship(Team *team);
    void Update_Team_States();
    void Becoming_Local_Player(bool b);

public:
    int unk1;
    int unk2;
    Handicap m_handicap;
    int unk3;
    NameKeyType m_playerNameKey;
    int m_playerIndex;
    char unk4[0xFC];
    int m_playerColor;
    uint8_t unk5[0x160 - 0x128];
    Team *m_defaultTeam;
    uint8_t unk6[0x44C - 0x164];
#ifdef GAME_DEBUG_STRUCTS
    uint8_t unk7[0x4]; // Unsure of location of 4 extra bytes in WB
#endif
};
