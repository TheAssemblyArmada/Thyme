/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Gamespy stuff
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
#include "stagingroomgameinfo.h"
#include <list>
#include <map>
#include <set>

class GameSpyGroupRoom;
class PlayerInfo;
class BuddyInfo;
class BuddyMessage;
class PSPlayerStats;
class GameWindow;

class GameSpyInfoInterface
{
public:
    virtual ~GameSpyInfoInterface();
    virtual void Reset();
    virtual void Clear_Group_Room_List() = 0;
    virtual std::map<int, GameSpyGroupRoom> *Get_Group_Room_List() = 0;
    virtual void Add_Group_Room(GameSpyGroupRoom) = 0;
    virtual bool Got_Group_Room_List() = 0;
    virtual void Join_Group_Room(int) = 0;
    virtual void Leave_Group_Room() = 0;
    virtual void Join_Best_Group_Room() = 0;
    virtual void Set_Current_Group_Room(int) = 0;
    virtual int Get_Current_Group_Room() = 0;
    virtual void Update_Player_Info(PlayerInfo, Utf8String) = 0;
    virtual void Player_Left_Group_Room(Utf8String) = 0;
    virtual std::map<Utf8String, PlayerInfo> *Get_Player_Info_Map() = 0;
    virtual std::map<int, BuddyInfo> *Get_Buddy_Map() = 0;
    virtual std::map<int, BuddyInfo> *Get_Buddy_Request_Map() = 0;
    virtual std::list<BuddyMessage> *Get_Buddy_Messages() = 0;
    virtual bool Is_Buddy(int) = 0;
    virtual void Set_Local_Name(Utf8String) = 0;
    virtual Utf8String Get_Local_Name() = 0;
    virtual void Set_Local_Profile_ID(int) = 0;
    virtual int Get_Local_Profile_ID() = 0;
    virtual Utf8String Get_Local_Email() = 0;
    virtual void Set_Local_Email(Utf8String) = 0;
    virtual Utf8String Get_Local_Password() = 0;
    virtual void Set_Local_Password(Utf8String) = 0;
    virtual void Set_Local_Base_Name(Utf8String) = 0;
    virtual Utf8String Get_Local_Base_Name() = 0;
    virtual void Set_Cached_Local_Player_Stats(PSPlayerStats) = 0;
    virtual PSPlayerStats Get_Cached_Local_Player_Stats() = 0;
    virtual void Clear_Staging_Room_List() = 0;
    virtual std::map<int, GameSpyStagingRoom *> *Get_Staging_Room_List() = 0;
    virtual GameSpyStagingRoom *Find_Staging_Room_By_ID(int) = 0;
    virtual void Add_Staging_Room(GameSpyStagingRoom) = 0;
    virtual void Update_Staging_Room(GameSpyStagingRoom) = 0;
    virtual void Remove_Staging_Room(GameSpyStagingRoom) = 0;
    virtual bool Has_Staging_Room_List_Changed() = 0;
    virtual void Leave_Staging_Room() = 0;
    virtual void Mark_As_Staging_Room_Host() = 0;
    virtual void Mark_As_Staging_Room_Joiner(int) = 0;
    virtual void Saw_Full_Game_List() = 0;
    virtual bool Am_I_Host() const = 0;
    virtual GameSpyStagingRoom *Get_Current_Staging_Room() = 0;
    virtual void Set_Game_Options() = 0;
    virtual int Get_Current_Staging_Room_ID() = 0;
    virtual void Set_Disallow_Asian_Text(bool) = 0;
    virtual void Set_Disallow_Non_Asian_Text(bool) = 0;
    virtual bool Get_Disallow_Asian_Text() = 0;
    virtual bool Get_Disallow_Non_Asian_Text() = 0;
    virtual void Register_Text_Window(GameWindow *) = 0;
    virtual void Unregister_Text_Window(GameWindow *) = 0;
    virtual int Add_Text(Utf16String, int, GameWindow *) = 0;
    virtual void Add_Chat(Utf8String, int, Utf16String, bool, bool, GameWindow *) = 0;
    virtual void Add_Chat(PlayerInfo, Utf16String, bool, bool, GameWindow *) = 0;
    virtual bool Send_Chat(Utf16String, bool, GameWindow *) = 0;
    virtual void Set_MOTD(Utf8String const &) = 0;
    virtual const Utf8String &Get_MOTD() = 0;
    virtual void Set_Config(Utf8String const &) = 0;
    virtual const Utf8String &Get_Config() = 0;
    virtual void Set_Ping_String(Utf8String const &) = 0;
    virtual const Utf8String &Get_Ping_String() = 0;
    virtual int Get_Ping_Value(Utf8String const &) = 0;
    virtual void Add_To_Saved_Ignore_List(int, Utf8String) = 0;
    virtual void Remove_From_Saved_Ignore_List(int) = 0;
    virtual bool Is_Saved_Ignored(int) = 0;
    virtual std::map<int, Utf8String> Return_Saved_Ignore_List() = 0;
    virtual void Load_Saved_Ignore_List() = 0;
    virtual std::set<Utf8String> Return_Ignore_List() = 0;
    virtual void Add_To_Ignore_List(Utf8String) = 0;
    virtual void Remove_From_Ignore_List(Utf8String) = 0;
    virtual void Is_Ignored(Utf8String) = 0;
    virtual void Set_Local_IPs(unsigned int, unsigned int) = 0;
    virtual unsigned int Get_Internal_IP() = 0;
    virtual unsigned int Get_External_IP() = 0;
    virtual bool Is_Disconnected_After_Game_Start(int *) const = 0;
    virtual void Mark_As_Disconnected_After_Game_Start(int) = 0;
    virtual bool Did_Player_Preorder(int) const = 0;
    virtual void Mark_Player_As_Preorder(int) = 0;
    virtual void Set_Max_Messages_Per_Update(int) = 0;
    virtual int Get_Max_Messages_Per_Update() = 0;
    virtual int Get_Additional_Disconnects() const = 0;
    virtual void Clear_Disconnects() = 0;
    virtual void Read_Additional_Disconnects() = 0;
    virtual void Update_Additional_Game_Spy_Disconnections(int) = 0;
};

#ifdef GAME_DLL
extern GameSpyInfoInterface *&g_theGameSpyInfo;
extern GameSpyStagingRoom *&g_theGameSpyGame;
#else
extern GameSpyInfoInterface *g_theGameSpyInfo;
extern GameSpyStagingRoom *g_theGameSpyGame;
#endif
