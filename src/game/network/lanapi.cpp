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
#include "lanapi.h"
#include "rtsutils.h"
#include "stringex.h"
#include "transport.h"

#ifdef PLATFORM_WINDOWS
#include <lmcons.h>
#elif defined PLATFORM_LINUX
#include <limits.h>
#include <unistd.h>
#endif

/**
 * 0x00728FB0
 */
LANAPI::LANAPI() :
    m_lobbyPlayers(0),
    m_games(nullptr),
    m_name(),
    m_userName(),
    m_hostName(),
    m_nextStartTimerTime(0),
    m_startTimerTicksRemaining(0),
    m_pendingAction(ACT_NONE),
    m_expiration(0),
    m_actionTimeout(5000),
    m_destAddr(0),
    m_lastResendTime(0),
    m_unkbool1(true),
    m_inLobby(true),
    m_currentGame(nullptr),
    m_localIP(0),
    m_transport(nullptr),
    m_broadcastAddr(UINT32_MAX),
    m_lastUpdate(0),
    m_lastGame(),
    m_isActive(true)
{
    m_transport = new Transport;
}

/**
 * 0x007290D0
 */
LANAPI::~LANAPI()
{
    Reset();
    delete m_transport;
}

/**
 * Initialise this subsystem.
 *
 * 0x007291D0
 */
void LANAPI::Init()
{
    m_nextStartTimerTime = 0;
    m_startTimerTicksRemaining = 0;
    m_transport->Reset();
    m_transport->Init(m_localIP, LANAPI_PORT);
    m_transport->Allow_Broadcast(true);
    m_pendingAction = ACT_NONE;
    m_expiration = 0;
    m_inLobby = true;
    m_unkbool1 = true;
    m_currentGame = nullptr;
    m_destAddr = 0;
    m_lastGame.Clear();

#ifdef PLATFORM_WINDOWS
    char user_name[UNLEN + 1];
    DWORD un_size = UNLEN + 1;

    if (!GetUserNameA(user_name, &un_size)) {
        if (getenv("USERNAME") != nullptr) {
            strlcpy(user_name, getenv("USERNAME"), sizeof(user_name));
        } else {
            strcpy(user_name, "unknown");
        }
    }
#elif PLATFORM_LINUX
    char user_name[LOGIN_NAME_MAX];

    if (getlogin_r(user_name, sizeof(user_name)) != 0) {
        if (getenv("USER") != nullptr) {
            strlcpy(user_name, getenv("USER"), sizeof(user_name));
        } else {
            strcpy(user_name, "unknown");
        }
    }
#else
#error Implement fetching username in lanapi.cpp for this platform
#endif

    m_userName = user_name;

#ifdef PLATFORM_WINDOWS
    char host_name[CNLEN + 1];
    DWORD hn_size = CNLEN + 1;

    if (!GetComputerNameA(host_name, &hn_size)) {
        if (getenv("COMPUTERNAME") != nullptr) {
            strlcpy(host_name, getenv("COMPUTERNAME"), sizeof(host_name));
        } else {
            strcpy(host_name, "unknown");
        }
    }
#elif PLATFORM_LINUX
    char host_name[HOST_NAME_MAX];

    if (gethostname(host_name, sizeof(host_name)) != 0) {
        if (getenv("HOSTNAME") != nullptr) {
            strlcpy(host_name, getenv("HOSTNAME"), sizeof(host_name));
        } else {
            strcpy(host_name, "unknown");
        }
    }
#else
#error Implement fetching hostname in lanapi.cpp for this platform
#endif

    // Just get the hostname portion of the name
    strtok(host_name, ".");
    m_hostName = host_name;
}

/**
 * Reset this subsystem.
 *
 * 0x007293D0
 */
void LANAPI::Reset()
{
	// Needs LANGameInfo, LANPlayer
#ifndef	THYME_STANDALONE
    Call_Method<void, LANAPI>(0x007293D0, this);
#endif
}

/**
 * Update this subsystem.
 *
 * 0x007295E0
 */
void LANAPI::Update()
{
	// Needs some utlility functions
#ifndef THYME_STANDALONE
    Call_Method<void, LANAPI>(0x007295E0, this);
#endif
}

/**
 * Marks LANAPI object as active.
 *
 * 0x0072B980
 */
void LANAPI::Set_Is_Active(bool active)
{
    LANMessage msg;

    if (active != m_isActive && !active && !m_inLobby && m_currentGame != nullptr) {
        Fill_In_Message(&msg);
        msg.message_type = LANMessage::MSG_SET_ACTIVE;
        Send_Message(&msg, 0);
	}

	m_isActive = active;
}

/**
 * Request location of some kind.
 *
 * 0x00729EB0
 */
void LANAPI::Request_Locations()
{
    LANMessage msg;
    msg.message_type = LANMessage::MSG_REQUEST_LOCATIONS;
    Fill_In_Message(&msg);
    Send_Message(&msg, 0);
}

void LANAPI::Request_Game_Join(LANGameInfo *game, uint32_t addr)
{
	// Needs LANGameInfo
#ifndef THYME_STANDALONE
    Call_Method<void, LANAPI, LANGameInfo *, uint32_t>(0x00729EF0, this, game, addr);
#endif
}

void LANAPI::Request_Game_Join_Direct(uint32_t addr)
{
    if (m_pendingAction) {
        On_Game_Join(RET_UNKNOWN, nullptr);
	} else if(addr != 0) {
        LANMessage msg;
        msg.message_type = LANMessage::MSG_REQUEST_GAME_INFO;
        Fill_In_Message(&msg);
        msg.direct_join.addr = Get_Local_IP();
        u_strlcpy(msg.direct_join.name, (const unichar_t*)m_name.Str(), m_name.Get_Length() + 1);
        Send_Message(&msg, addr);
        m_pendingAction = ACT_LEAVE;
        m_expiration = m_actionTimeout + rts::Get_Time();
	} else {
        On_Game_Join(RET_BUSY, nullptr);
	}
}

void LANAPI::Request_Game_Leave() {}

void LANAPI::Request_Accept() {}

void LANAPI::Request_Has_Map() {}

void LANAPI::Request_Chat(const Utf16String &msg, ChatType format) {}

void LANAPI::Request_Game_Start() {}

void LANAPI::Request_Game_Start_Timer() {}

void LANAPI::Request_Game_Options(Utf8String options, bool is_public, uint32_t addr) {}

void LANAPI::Request_Game_Create(const Utf16String &name, bool unk) {}

void LANAPI::Request_Game_Announce() {}

void LANAPI::Request_Set_Name(Utf16String name) {}

void LANAPI::Request_Lobby_Leave() {}

void LANAPI::Reset_Game_Start_Timer() {}

void LANAPI::On_Game_List(LANGameInfo *game) {}

void LANAPI::On_Player_List(LANPlayer *player) {}

void LANAPI::On_Game_Join(ReturnType ret, LANGameInfo *game) {}

void LANAPI::On_Player_Join(uint32_t slot, Utf16String player) {}

void LANAPI::On_Host_Leave() {}

void LANAPI::On_Player_Leave(Utf16String player) {}

void LANAPI::On_Accept(uint32_t player_ip, bool status) {}

void LANAPI::On_Has_Map(uint32_t player_ip, bool status) {}

void LANAPI::On_Chat(Utf16String player, uint32_t ip, Utf16String message, ChatType format) {}

void LANAPI::On_Game_Start() {}

void LANAPI::On_Game_Start_Timer(int time) {}

void LANAPI::On_Game_Options(uint32_t player_addr, int player_slot, Utf8String options) {}

void LANAPI::On_Game_Create(ReturnType type) {}

void LANAPI::On_Name_Change(uint32_t ip, Utf16String name) {}

LANGameInfo *LANAPI::Lookup_Game(Utf16String name)
{
    return nullptr;
}

LANGameInfo *LANAPI::Lookup_Game_By_Offset(int offset)
{
    return nullptr;
}

void LANAPI::Fill_In_Message(LANMessage *msg)
{
    u_strlcpy(msg->name, (const unichar_t *)m_name.Str(), sizeof(msg->name));
    strlcpy(msg->user_name, m_userName, sizeof(msg->user_name));
    strlcpy(msg->host_name, m_hostName, sizeof(msg->host_name));
}

void LANAPI::Check_MOTD()
{
	// Empty in known binaries
}

LANPlayer *LANAPI::Lookup_Player(uint32_t ip)
{
    return nullptr;
}

uint32_t LANAPI::Get_Local_IP()
{
    return uint32_t();
}

/**
 * Sends a message to the specified recipient.
 *
 * 0x00729530
 */
void LANAPI::Send_Message(LANMessage *msg, uint32_t addr)
{
    // Needs LANGameInfo, LANPlayer
#ifndef THYME_STANDALONE
    Call_Method<void, LANAPI>(0x00729530, this);
#endif
}
