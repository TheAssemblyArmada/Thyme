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
#include "gametext.h"
#include "rtsutils.h"
#include "transport.h"
#include <cctype>
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include <lmcons.h>
#elif defined PLATFORM_LINUX
#include <limits.h>
#include <unistd.h>
#endif

#ifdef GAME_DLL
#include "hooker.h"
#endif

using std::isdigit;
using std::strcpy;

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

    // Get the current username from the OS.
#ifdef PLATFORM_WINDOWS
    char user_name[UNLEN + 1];
    DWORD un_size = UNLEN + 1;

    if (!GetUserNameA(user_name, &un_size)) {
        if (getenv("USERNAME") != nullptr) {
            strlcpy_tpl(user_name, getenv("USERNAME"));
        } else {
            strcpy(user_name, "unknown");
        }
    }
#elif PLATFORM_LINUX
    char user_name[LOGIN_NAME_MAX];

    if (getlogin_r(user_name, sizeof(user_name)) != 0) {
        if (getenv("USER") != nullptr) {
            strlcpy_tpl(user_name, getenv("USER"));
        } else {
            strcpy(user_name, "unknown");
        }
    }
#else
#error Implement fetching username in lanapi.cpp for this platform
#endif

    m_userName = user_name;

    // Get the current host/computer name from the OS.
#ifdef PLATFORM_WINDOWS
    char host_name[CNLEN + 1];
    DWORD hn_size = CNLEN + 1;

    if (!GetComputerNameA(host_name, &hn_size)) {
        if (getenv("COMPUTERNAME") != nullptr) {
            strlcpy_tpl(host_name, getenv("COMPUTERNAME"));
        } else {
            strcpy(host_name, "unknown");
        }
    }
#elif PLATFORM_LINUX
    char host_name[HOST_NAME_MAX];

    if (gethostname(host_name, sizeof(host_name)) != 0) {
        if (getenv("HOSTNAME") != nullptr) {
            strlcpy_tpl(host_name, getenv("HOSTNAME"));
        } else {
            strcpy(host_name, "unknown");
        }
    }
#else
#error Implement fetching hostname in lanapi.cpp for this platform
#endif

    // Just get the hostname portion of the name, strtok will replace delimiter with null terminator.
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
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x007293D0, 0), this);
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
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x007295E0, 0), this);
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
#ifdef GAME_DLL
    Call_Method<void, LANAPI, LANGameInfo *, uint32_t>(PICK_ADDRESS(0x00729EF0, 0), this, game, addr);
#endif
}

/**
 * Request a direct join game.
 *
 * 0x0072A090
 */
void LANAPI::Request_Game_Join_Direct(uint32_t addr)
{
    if (m_pendingAction) {
        On_Game_Join(RET_UNKNOWN, nullptr);
    } else if (addr != 0) {
        LANMessage msg;
        msg.message_type = LANMessage::MSG_REQUEST_GAME_INFO;
        Fill_In_Message(&msg);
        msg.direct_join.addr = Get_Local_IP();
        // BUGFIX: Originally the target buffer size came from the source string length, which could cause buffer overflow if
        // the source string is longer than the target buffer.
        u_strlcpy_tpl(msg.direct_join.name, m_name.Str());
        Send_Message(&msg, addr);
        m_pendingAction = ACT_LEAVE;
        m_expiration = m_actionTimeout + rts::Get_Time();
    } else {
        On_Game_Join(RET_BUSY, nullptr);
    }
}

void LANAPI::Request_Game_Leave()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(0x0072A190, this);
#endif
}

void LANAPI::Request_Accept()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x0072A440, 0), this);
#endif
}

void LANAPI::Request_Has_Map()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x0072A4D0, 0), this);
#endif
}

void LANAPI::Request_Chat(const Utf16String &msg, ChatType format)
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, const Utf16String &, ChatType>(PICK_ADDRESS(0x0072A850, 0), this, msg, format);
#endif
}

void LANAPI::Request_Game_Start()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x0072A9B0, 0), this);
#endif
}

void LANAPI::Request_Game_Start_Timer()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x0072AA20, 0), this);
#endif
}

void LANAPI::Request_Game_Options(Utf8String options, bool is_public, uint32_t addr)
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, Utf8String, bool, uint32_t>(PICK_ADDRESS(0x0072AAB0, 0), this, options, is_public, addr);
#endif
}

void LANAPI::Request_Game_Create(const Utf16String &name, bool unk)
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, const Utf16String &, bool>(PICK_ADDRESS(0x0072AC80, 0), this, name, unk);
#endif
}

void LANAPI::Request_Game_Announce()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x0072A2D0, 0), this);
#endif
}

void LANAPI::Request_Set_Name(Utf16String name)
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, Utf16String>(PICK_ADDRESS(0x0072B0F0, 0), this, name);
#endif
}

void LANAPI::Request_Lobby_Leave()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x0072B3C0, 0), this);
#endif
}

/**
 * Resets the countdown timer that counts to game start.
 *
 * 0x0072AA10
 */
void LANAPI::Reset_Game_Start_Timer()
{
    m_nextStartTimerTime = 0;
    m_startTimerTicksRemaining = 0;
}

void LANAPI::On_Game_List(LANGameInfo *game)
{
    // TODO Requires LANGameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, LANGameInfo *>(PICK_ADDRESS(0x005F96C0, 0), this, game);
#endif
}

void LANAPI::On_Player_List(LANPlayer *player)
{
    // TODO Requires GadgetList functions
#ifdef GAME_DLL
    Call_Method<void, LANAPI, LANPlayer *>(PICK_ADDRESS(0x005F97D0, 0), this, player);
#endif
}

void LANAPI::On_Game_Join(ReturnType ret, LANGameInfo *game)
{
    // TODO Requires Shell, LANPreferences
#ifdef GAME_DLL
    Call_Method<void, LANAPI, ReturnType, LANGameInfo *>(PICK_ADDRESS(0x005F8EC0, 0), this, ret, game);
#endif
}

void LANAPI::On_Player_Join(uint32_t slot, Utf16String player)
{
    // TODO Requires LANGameSlot, generateGameOptionsString, lanUpdatesSlotList
#ifdef GAME_DLL
    Call_Method<void, LANAPI, uint32_t, Utf16String>(PICK_ADDRESS(0x005F8E40, 0), this, slot, player);
#endif
}

void LANAPI::On_Host_Leave()
{
    // TODO Requires GameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x005F9160, 0), this);
#endif
}

void LANAPI::On_Player_Leave(Utf16String player)
{
    // TODO Requires Shell, generateGameOptionsString, LANPreferences, lanSlotUpdate, ShaderClass
#ifdef GAME_DLL
    Call_Method<void, LANAPI, Utf16String>(PICK_ADDRESS(0x005F9190, 0), this, player);
#endif
}

void LANAPI::On_Accept(uint32_t player_ip, bool status)
{
    // TODO Requires LANGameSlot, generateGameOptionsString, LANGameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, uint32_t, bool>(PICK_ADDRESS(0x005F7960, 0), this, player_ip, status);
#endif
}

void LANAPI::On_Has_Map(uint32_t player_ip, bool status)
{
    // TODO Requires LANGameSlot, LANGameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI, uint32_t, bool>(PICK_ADDRESS(0x005F7AC0, 0), this, player_ip, status);
#endif
}

void LANAPI::On_Chat(Utf16String player, uint32_t ip, Utf16String message, ChatType format)
{
    // TODO Requires GameWindow, LanguageFilter, LANGameInfo, MultiplayerSettings, GadgetListBox*
#ifdef GAME_DLL
    Call_Method<void, LANAPI, Utf16String, uint32_t, Utf16String, ChatType>(
        PICK_ADDRESS(0x005F9910, 0), this, player, ip, message, format);
#endif
}

void LANAPI::On_Game_Start()
{
    // TODO Requires LANPreferences, LANGameInfo
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x005F7E50, 0), this);
#endif
}

/**
 * Action to take on game start timer message.
 *
 * 0x005F7D70
 */
void LANAPI::On_Game_Start_Timer(int time)
{
    Utf16String format;
    Utf16String msg;

    if (time == 1) {
        format = g_theGameText->Fetch("LAN:GameStartTimerSingular");
    } else {
        format = g_theGameText->Fetch("LAN:GameStartTimerPlural");
    }

    msg.Format(format, time);
    On_Chat(U_CHAR("SYSTEM"), m_localIP, msg, LANCHAT_SYSTEM);
}

void LANAPI::On_Game_Options(uint32_t player_addr, int player_slot, Utf8String options)
{
    // TODO Requires LANGameInfo, LANGameSlot, GameInfoToAsciiString
#ifdef GAME_DLL
    Call_Method<void, LANAPI, uint32_t, int, Utf8String>(
        PICK_ADDRESS(0x005F84D0, 0), this, player_addr, player_slot, options);
#endif
}

void LANAPI::On_Game_Create(ReturnType type)
{
    // TODO Requires Shell, GadgetList*
#ifdef GAME_DLL
    Call_Method<void, LANAPI, ReturnType>(PICK_ADDRESS(0x005F96E0, 0), this, type);
#endif
}

/**
 * Action to take on name change.
 *
 * 0x005F98C0
 */
void LANAPI::On_Name_Change(uint32_t ip, Utf16String name)
{
    On_Player_List(m_lobbyPlayers);
}

LANGameInfo *LANAPI::Lookup_Game(Utf16String name)
{
    // TODO Requires LANGameInfo
#ifdef GAME_DLL
    return Call_Method<LANGameInfo *, LANAPI, Utf16String>(PICK_ADDRESS(0x0072B410, 0), this, name);
#else
    return nullptr;
#endif
}

LANGameInfo *LANAPI::Lookup_Game_By_Offset(int offset)
{
    // TODO Requires LANGameInfo
#ifdef GAME_DLL
    return Call_Method<LANGameInfo *, LANAPI, int>(PICK_ADDRESS(0x0072B4C0, 0), this, offset);
#else
    return nullptr;
#endif
}

/**
 * Set the local IP address from a 32bit integer.
 *
 * 0x0072B880
 */
void LANAPI::Set_Local_IP(uint32_t address)
{
    m_localIP = address;
    m_transport->Reset();
    m_transport->Init(address, 8086);
    m_transport->Allow_Broadcast(true);
}

/**
 * Set the local IP address from a string.
 *
 * 0x0072B8D0
 */
void LANAPI::Set_Local_IP(Utf8String address)
{
    Set_Local_IP(Resolve_IP(address));
}

bool LANAPI::Am_I_Host()
{
    // TODO Requires LANGameInfo
#ifdef GAME_DLL
    return Call_Method<bool, LANAPI>(PICK_ADDRESS(0x0072B960, 0), this);
#else
    return false;
#endif
}

/**
 * Fill in the parts of a message that don't vary by message type.
 *
 * 0x0072B330
 */
void LANAPI::Fill_In_Message(LANMessage *msg)
{
    u_strlcpy_tpl(msg->name, m_name.Str());
    strlcpy_tpl(msg->user_name, m_userName.Str());
    strlcpy_tpl(msg->host_name, m_hostName.Str());
}

LANPlayer *LANAPI::Lookup_Player(uint32_t ip)
{
    // TODO Requires LANPlayer
#ifdef GAME_DLL
    return Call_Method<LANPlayer *, LANAPI, uint32_t>(PICK_ADDRESS(0x0072B540, 0), this, ip);
#else
    return nullptr;
#endif
}

/**
 * Sends a message to the specified recipient.
 *
 * 0x00729530
 */
void LANAPI::Send_Message(LANMessage *msg, uint32_t addr)
{
    // Needs LANGameInfo, LANPlayer
#ifdef GAME_DLL
    Call_Method<void, LANAPI>(PICK_ADDRESS(0x00729530, 0), this);
#endif
}

/**
 * Resolves a hostname or IP address string to a 32bit integer.
 * Original has this in networkutil.cpp, but its only used in lanapi.
 *
 * 0x005E1D20
 */
uint32_t LANAPI::Resolve_IP(const Utf8String &addr)
{
    // No string, no IP.
    if (addr.Is_Empty()) {
        return 0;
    }

    uint32_t ip = 0;

    // If we have a number assume we need to process it as an IP address string.
    if (isdigit(addr.Get_Char(0))) {
        ip = inet_addr(addr.Str());
    } else { // Otherwise assume its a host name.
        struct hostent *host = gethostbyname(addr.Str());
        ip = *reinterpret_cast<uint32_t *>(host->h_addr_list[0]);
    }

    return be32toh(ip);
}
