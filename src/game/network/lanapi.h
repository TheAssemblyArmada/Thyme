/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Provides an networking implementation.
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
#include "lanapiinterface.h"

class Transport;
class LANPlayer;

class LANAPI : public LANAPIInterface
{
    enum PendingActionType : int32_t
    {
        ACT_NONE,
        ACT_JOIN,
        ACT_LEAVE,
        ACT_MAX,
    };

    enum
    {
        LANAPI_PORT = 8086,
    };

public:
    LANAPI();
    ~LANAPI() override;

    void Init() override;
    void Reset() override;
    void Update() override;

    void Set_Is_Active(bool active) override;
    void Request_Locations() override;
    void Request_Game_Join(LANGameInfo *game, uint32_t addr) override;
    void Request_Game_Join_Direct(uint32_t addr) override;
    void Request_Game_Leave() override;
    void Request_Accept() override;
    void Request_Has_Map() override;
    void Request_Chat(const Utf16String &msg, ChatType format) override;
    void Request_Game_Start() override;
    void Request_Game_Start_Timer() override;
    void Request_Game_Options(Utf8String options, bool is_public, uint32_t addr) override;
    void Request_Game_Create(const Utf16String &name, bool unk) override;
    void Request_Game_Announce() override;
    void Request_Set_Name(Utf16String name) override;
    void Request_Lobby_Leave() override;
    void Reset_Game_Start_Timer() override;
    void On_Game_List(LANGameInfo *game) override;
    void On_Player_List(LANPlayer *player) override;
    void On_Game_Join(ReturnType ret, LANGameInfo *game) override;
    void On_Player_Join(uint32_t slot, Utf16String player) override;
    void On_Host_Leave() override;
    void On_Player_Leave(Utf16String player) override;
    void On_Accept(uint32_t player_ip, bool status) override;
    void On_Has_Map(uint32_t player_ip, bool status) override;
    void On_Chat(Utf16String player, uint32_t ip, Utf16String message, ChatType format) override;
    void On_Game_Start() override;
    void On_Game_Start_Timer(int time) override;
    void On_Game_Options(uint32_t player_addr, int player_slot, Utf8String options) override;
    void On_Game_Create(ReturnType type) override;
    void On_Name_Change(uint32_t ip, Utf16String name) override;
    LANGameInfo *Lookup_Game(Utf16String name) override;
    LANGameInfo *Lookup_Game_By_Offset(int offset) override;
    void Set_Local_IP(uint32_t address) override;
    void Set_Local_IP(Utf8String address) override;
    bool Am_I_Host() override;
    Utf16String Get_My_Name() override { return m_name; }
    LANGameInfo *Get_My_Game() override { return m_currentGame; }
    void Fill_In_Message(LANMessage *msg) override;
    void Check_MOTD() override {}
    void On_In_Active() override {}
    LANPlayer *Lookup_Player(uint32_t ip) override;
    uint32_t Get_Local_IP() override { return m_localIP; }

private:
    void Send_Message(LANMessage *msg, uint32_t addr);
    static uint32_t Resolve_IP(const Utf8String &addr);

private:
    LANPlayer *m_lobbyPlayers;
    LANGameInfo *m_games;
    Utf16String m_name;
    Utf8String m_userName;
    Utf8String m_hostName;
    int32_t m_nextStartTimerTime;
    int32_t m_startTimerTicksRemaining;
    PendingActionType m_pendingAction;
    uint32_t m_expiration;
    uint32_t m_actionTimeout;
    uint32_t m_destAddr;
    uint32_t m_lastResendTime;
    bool m_unkbool1;
    bool m_inLobby;
    LANGameInfo *m_currentGame;
    uint32_t m_localIP;
    Transport *m_transport;
    uint32_t m_broadcastAddr;
    uint32_t m_lastUpdate;
    Utf8String m_lastGame;
    bool m_isActive;
};