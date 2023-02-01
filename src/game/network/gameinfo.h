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
#pragma once
#include "always.h"
#include "asciistring.h"
#include "money.h"
#include "unicodestring.h"

enum SlotState
{
    SLOT_OPEN,
    SLOT_CLOSED,
    SLOT_EASY_AI,
    SLOT_MED_AI,
    SLOT_HARD_AI,
    SLOT_PLAYER,
};

class GameSlot
{
public:
    virtual void Reset();

private:
    SlotState m_state;
    bool m_isAccepted;
    bool m_hasMap;
    bool m_unk;
    int m_color;
    int m_startPos;
    int m_playerTemplate;
    int m_teamNumber;
    int m_originalColor;
    int m_originalStartPos;
    int m_originalPlayerTemplate;
    Utf16String m_name;
    unsigned int m_IP;
    unsigned short m_port;
    int m_NATBehavior; // Appears to actually be FirewallHelperClass::tFirewallBehaviorType
    int m_unk2;
    bool m_unk3;
};

class GameInfo
{
public:
    virtual void Reset();
    virtual void Start_Game(int game_id);
    virtual bool Am_I_Host() const;
    virtual int Get_Local_Slot_Num() const;
    virtual void Reset_Accepted();
    virtual void Reset_Start_Spots();
    virtual void Adjust_Slots_For_Map(int i);
    virtual void Close_Open_Slots();

private:
    int m_isPlayerPreorder;
    int m_crcInterval;
    bool m_inGame;
    bool m_inProgress;
    bool m_unk;
    int m_gameID;
    GameSlot *m_slot[8];
    unsigned int m_localIP;
    Utf8String m_mapName;
    unsigned int m_mapCRC;
    unsigned int m_mapSize;
    int m_mapContentsMask;
    int m_seed;
    int m_useStats;
    Money m_money;
    short m_superweaponRestrictions;
    bool m_originalArmies;
};
