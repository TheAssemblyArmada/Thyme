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
    GameSlot() { Reset(); }
    ~GameSlot() {}
    virtual void Reset();

    bool Is_Occupied() const;
    bool Is_Open() const;
    bool Is_AI() const;
    bool Is_Player(Utf8String name) const;
    bool Is_Player(Utf16String name) const;
    bool Is_Player(unsigned int ip) const;
    bool Is_Human() const;
    void Save_Off_Original_Info();
    void Un_Accept();
    Utf16String Get_Apparent_Player_Template_Display_Name() const;
    int Get_Apparent_Player_Template() const;
    int Get_Apparent_Color() const;
    int Get_Apparent_Start_Pos() const;
    void Set_Map_Availability(bool available);
    void Set_State(SlotState state, Utf16String name, unsigned int IP);

    int Get_Color() const { return m_color; }
    int Get_Player_Template() const { return m_playerTemplate; }
    int Get_Start_Pos() const { return m_startPos; }
    int Get_Team_Number() const { return m_teamNumber; }
    Utf16String Get_Name() const { return m_name; }
    SlotState Get_State() const { return m_state; }
    int Get_Original_Player_Template() const { return m_originalPlayerTemplate; }
    unsigned int Get_IP() const { return m_IP; }
    bool Is_Accepted() const { return m_isAccepted; }
    bool Has_Map() const { return m_hasMap; }
    unsigned short Get_Port() const { return m_port; }
    int Get_NAT_Behavior() const { return m_NATBehavior; }
    bool Get_Unk() const { return m_unk; }
    int Get_Original_Start_Pos() const { return m_originalStartPos; }
    int Get_Original_Color() const { return m_originalColor; }

    void Set_Color(int color) { m_color = color; }
    void Set_Accepted() { m_isAccepted = true; }
    void Set_Start_Pos(int pos) { m_startPos = pos; }
    void Set_Player_Template(int tmplate)
    {
        m_playerTemplate = tmplate;

        if (tmplate <= -2) {
            m_startPos = -1;
        }
    }
    void Set_Port(unsigned short port) { m_port = port; }
    void Set_NAT_Behavior(unsigned int behavior) { m_NATBehavior = behavior; }
    void Set_IP(unsigned int IP) { m_IP = IP; }
    void Set_Name(Utf16String name) { m_name = name; }
    void Set_Team_Number(int team) { m_teamNumber = team; }
    void Set_Unk(bool unk) { m_unk = unk; }

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
    enum
    {
        MAX_SLOTS = 8
    };

    GameInfo();
    ~GameInfo() {}
    virtual void Reset();
    virtual void Start_Game(int game_id);
    virtual bool Am_I_Host() const;
    virtual int Get_Local_Slot_Num() const;
    virtual void Reset_Accepted();
    virtual void Reset_Start_Spots();
    virtual void Adjust_Slots_For_Map(int i);
    virtual void Close_Open_Slots();

    const Money *Get_Money() const { return &m_money; }
    unsigned short Get_Superweapon_Restriction() const { return m_superweaponRestriction; }
    bool Get_Original_Armies() const { return m_originalArmies; }
    Utf8String Get_Map() const { return m_mapName; }
    int Get_CRC_Interval() const { return m_crcInterval; }
    unsigned int Get_Map_CRC() const { return m_mapCRC; }
    unsigned int Get_Map_Size() const { return m_mapSize; }
    int Get_Map_Contents_Mask() const { return m_mapContentsMask; }
    int Get_Seed() const { return m_seed; }
    int Get_Use_Stats() const { return m_useStats; }
    bool Is_In_Progress() const { return m_inProgress; }

    void Set_CRC_Interval(int interval)
    {
        if (interval >= 100) {
            interval = 100;
        }

        m_crcInterval = interval;
    }

    void Set_Use_Stats(int stats) { m_useStats = stats; }
    void Set_Original_Armies(bool set) { m_originalArmies = set; }
    void Set_Local_IP(unsigned int IP) { m_localIP = IP; }

    void Init();
    GameSlot *Get_Slot(int num);
    int Get_Slot_Num(Utf8String name) const;
    const GameSlot *Get_Const_Slot(int num) const;
    bool Is_Color_Taken(int index, int ignore) const;
    bool Is_Start_Position_Taken(int index, int ignore) const;
    bool Is_Player_Preorder(int player) const;
    void Mark_Player_As_Preorder(int player);
    int Get_Num_Players() const;
    int Get_Num_Non_Observer_Players() const;
    int Get_Max_Players() const;
    void Enter_Game();
    void Leave_Game();
    void End_Game();
    void Set_Slot(int index, GameSlot slot);
    void Set_Map(Utf8String map_name);
    void Set_Map_Contents_Mask(int mask);
    void Set_Map_CRC(unsigned int crc);
    void Set_Map_Size(unsigned int size);
    void Set_Seed(int seed);
    void Set_Slot_Pointer(int index, GameSlot *slot);
    void Set_Superweapon_Restriction(unsigned short restriction);
    void Set_Starting_Cash(const Money &cash);
    bool Is_Skirmish();
    bool Is_Multi_Player();
    bool Is_Sandbox();

protected:
    int m_isPlayerPreorder;
    int m_crcInterval;
    bool m_inGame;
    bool m_inProgress;
    bool m_unk;
    int m_gameID;
    GameSlot *m_slot[MAX_SLOTS];
    unsigned int m_localIP;
    Utf8String m_mapName;
    unsigned int m_mapCRC;
    unsigned int m_mapSize;
    int m_mapContentsMask;
    int m_seed;
    int m_useStats;
    Money m_money;
    unsigned short m_superweaponRestriction;
    bool m_originalArmies;
};

class SkirmishGameInfo : public GameInfo, public SnapShot
{
public:
    SkirmishGameInfo();
    ~SkirmishGameInfo() {}
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    GameSlot m_gameSlot[MAX_SLOTS];
};

#ifdef GAME_DLL
extern GameInfo *&g_theGameInfo;
extern SkirmishGameInfo *&g_theSkirmishGameInfo;
extern SkirmishGameInfo *&g_theChallengeGameInfo;
#else
extern GameInfo *g_theGameInfo;
extern SkirmishGameInfo *g_theSkirmishGameInfo;
extern SkirmishGameInfo *g_theChallengeGameInfo;
#endif
