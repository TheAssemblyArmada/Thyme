/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Provides an interface for networking implementations.
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
#include "subsysteminterface.h"
#include "unicodestring.h"

class LANGameInfo;
class LANPlayer;

// Message is sent on wire and must be packed.
#pragma pack(push, 1)
struct LANMessage
{
    enum
    {
        MSG_REQUEST_LOCATIONS,
        MSG_GAME_ANNOUNCE,
        MSG_LOBBY_ANNOUNCE,
        MSG_REQUEST_JOIN,
        MSG_JOIN_ACCEPT,
        MSG_JOIN_DENY,
        MSG_REQUEST_GAME_LEAVE,
        MSG_REQUEST_LOBBY_LEAVE,
        MSG_SET_ACCEPT,
        MSG_MAP_AVAILABILITY,
        MSG_CHAT,
        MSG_GAME_START,
        MSG_GAME_TIMER,
        MSG_GAME_OPTIONS,
        MSG_SET_ACTIVE,
        MSG_REQUEST_GAME_INFO,
        MSG_MAX
    } message_type;
    unichar_t name[13];
    char user_name[2];
    char host_name[2];

    union
    {
        struct
        {
            uint32_t game_slot;
            uint32_t game_crc;
            uint32_t ini_crc;
            char cd_key[24];
        } join;

        // Its not clear from the original how large the name array actually is.
        // Pratically the actual space is 216 based on overall size of the LANMessage union.
        struct
        {
            uint32_t addr;
            unichar_t name[17];
        } direct_join;

        struct
        {
            unichar_t name[17];
        } leave;

        struct
        {
            unichar_t name[17];
            bool unk_bool;
        } accept;

        struct
        {
            unichar_t game_name[17];
            char unk_byte;
            char info_string[401];
            char unk_byte2;
        } announce;

        struct
        {
            unichar_t game_name[17];
            int map_name_crc;
            bool has_map;
        } map;

        struct
        {
            unichar_t game_name[17];
            int32_t chat_type;
            unichar_t chat_buff[101];
        } chat;

        struct
        {
            uint32_t tick;
        } timer;

        struct
        {
            char info_string[401];
        } options;
    };
};
#pragma pack(pop)

class LANAPIInterface : public SubsystemInterface
{
public:
    enum ChatType
    {
        LANCHAT_NORMAL,
        LANCHAT_EMOTE,
        LANCHAT_SYSTEM,
        LANCHAT_MAX,
    };

    enum ReturnType
    {
        RET_OK,
        RET_TIMEOUT,
        RET_GAME_FULL,
        RET_DUPLICATE_NAME,
        RET_CRC_MISMATCH,
        RET_DUPLICATE_SERIAL,
        RET_GAME_STARTED,
        RET_GAME_EXISTS,
        RET_GAME_GONE,
        RET_BUSY,
        RET_UNKNOWN,
        RET_MAX,
    };

public:
    virtual ~LANAPIInterface() { static_assert(sizeof(LANMessage) == 471, "LANMessage not expected size"); }

    virtual void Set_Is_Active(bool active) = 0;
    virtual void Request_Locations() = 0;
    virtual void Request_Game_Join(LANGameInfo *game, uint32_t addr) = 0;
    virtual void Request_Game_Join_Direct(uint32_t addr) = 0;
    virtual void Request_Game_Leave() = 0;
    virtual void Request_Accept() = 0;
    virtual void Request_Has_Map() = 0;
    virtual void Request_Chat(const Utf16String &msg, ChatType format) = 0;
    virtual void Request_Game_Start() = 0;
    virtual void Request_Game_Start_Timer() = 0;
    virtual void Request_Game_Options(Utf8String options, bool is_public, uint32_t addr) = 0;
    virtual void Request_Game_Create(const Utf16String &name, bool unk) = 0;
    virtual void Request_Game_Announce() = 0;
    virtual void Request_Set_Name(Utf16String name) = 0;
    virtual void Request_Lobby_Leave() = 0;
    virtual void Reset_Game_Start_Timer() = 0;
    virtual void On_Game_List(LANGameInfo *game) = 0;
    virtual void On_Player_List(LANPlayer *player) = 0;
    virtual void On_Game_Join(ReturnType ret, LANGameInfo *game) = 0;
    virtual void On_Player_Join(uint32_t slot, Utf16String player) = 0;
    virtual void On_Host_Leave() = 0;
    virtual void On_Player_Leave(Utf16String player) = 0;
    virtual void On_Accept(uint32_t player_ip, bool status) = 0;
    virtual void On_Has_Map(uint32_t player_ip, bool status) = 0;
    virtual void On_Chat(Utf16String player, uint32_t ip, Utf16String message, ChatType format) = 0;
    virtual void On_Game_Start() = 0;
    virtual void On_Game_Start_Timer(int time) = 0;
    virtual void On_Game_Options(uint32_t player_addr, int player_slot, Utf8String options) = 0;
    virtual void On_Game_Create(ReturnType type) = 0;
    virtual void On_Name_Change(uint32_t ip, Utf16String name) = 0;
    virtual LANGameInfo *Lookup_Game(Utf16String name) = 0;
    virtual LANGameInfo *Lookup_Game_By_Offset(int offset) = 0;
    virtual void Set_Local_IP(uint32_t address) = 0;
    virtual void Set_Local_IP(Utf8String address) = 0;
    virtual bool Am_I_Host() = 0;
    virtual Utf16String Get_My_Name() = 0;
    virtual LANGameInfo *Get_My_Game() = 0;
    virtual void Fill_In_Message(LANMessage *msg) = 0;
    virtual void Check_MOTD() = 0; // Both these two virtuals are empty in implementation.
    virtual void On_In_Active() = 0; // Order they occur is unclear.
    virtual LANPlayer *Lookup_Player(uint32_t ip) = 0;
    virtual uint32_t Get_Local_IP() = 0;
};

#ifdef GAME_DLL
extern LANAPIInterface *&g_theLAN;
#else
extern LANAPIInterface *g_theLAN;
#endif
