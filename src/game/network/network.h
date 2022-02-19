/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Network
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
#include "subsysteminterface.h"
#include <list>

class GameInfo;
class Transport;

class NetworkInterface : public SubsystemInterface
{
    virtual ~NetworkInterface() override;
    virtual void Lite_Update() = 0;
    virtual void Set_Local_Address(uint32_t ip, uint32_t port) = 0;
    virtual bool Is_Frame_Data_Ready() = 0;
    virtual void Parse_User_List(GameInfo const *game) = 0;
    virtual void Start_Game() = 0;
    virtual uint32_t Get_Run_Ahead() = 0;
    virtual uint32_t Get_Frame_Rate() = 0;
    virtual uint32_t Get_Packet_Arrival_Cushion() = 0;
    virtual void Send_Chat(Utf16String text, int32_t player_mask) = 0;
    virtual void Send_Disconnect_Chat(Utf16String text) = 0;
    virtual void Send_File(Utf8String name, unsigned char c, unsigned short s) = 0;
    virtual unsigned short Send_File_Announce(Utf8String name, unsigned char c) = 0;
    virtual int32_t Get_File_Transfer_Progress(int32_t i, Utf8String name) = 0;
    virtual bool Are_All_Queues_Empty() = 0;
    virtual void Quit_Game() = 0;
    virtual void Self_Destruct_Player(int32_t slot) = 0;
    virtual void Vote_For_Player_Disconnect(int32_t slot) = 0;
    virtual bool Is_Packet_Router() = 0;
    virtual float Get_Incoming_Bytes_Per_Second() = 0;
    virtual float Get_Incoming_Packets_Per_Second() = 0;
    virtual float Get_Outgoing_Bytes_Per_Second() = 0;
    virtual float Get_Outgoing_Packets_Per_Second() = 0;
    virtual float Get_Unknown_Bytes_Per_Second() = 0;
    virtual float Get_Unknown_Packets_Per_Second() = 0;
    virtual void Update_Load_Progress(int32_t percent) = 0;
    virtual void Load_Progress_Complete() = 0;
    virtual void Send_Time_Out_Game_Start() = 0;
    virtual uint32_t Get_Local_Player_ID() = 0;
    virtual Utf16String Get_Player_Name(int32_t slot) = 0;
    virtual int32_t Get_Num_Players() = 0;
    virtual int32_t Get_Average_FPS() = 0;
    virtual int32_t Get_Slot_Average_FPS(int32_t slot) = 0;
    virtual void Attach_Transport(Transport *transport) = 0;
    virtual void Init_Transport() = 0;
    virtual bool Saw_CRC_Mismatch() = 0;
    virtual void Set_Saw_CRC_Mismatch() = 0;
    virtual bool Is_Player_Connected(int32_t slot) = 0;
    virtual void Notify_Others_Of_Current_Frame() = 0;
    virtual void Notify_Others_Of_New_Frame(uint32_t frame) = 0;
    virtual int32_t Get_Execution_Frame() = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Toggle_Enabled() = 0;
#endif
    virtual uint32_t Get_Ping_Frame() = 0;
    virtual int32_t Get_Pings_Sent() = 0;
    virtual int32_t Get_Pings_Recieved() = 0;
};

class ConnectionManager;
class GameWindow;

enum NetLocalStatus
{
    NETLOCALSTATUS_UNK,
};

class Network : public NetworkInterface
{

public:
    virtual ~Network() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Lite_Update() override;
    virtual void Set_Local_Address(uint32_t ip, uint32_t port) override;
    virtual bool Is_Frame_Data_Ready() override;
    virtual void Parse_User_List(GameInfo const *game) override;
    virtual void Start_Game() override;
    virtual uint32_t Get_Run_Ahead() override;
    virtual uint32_t Get_Frame_Rate() override;
    virtual uint32_t Get_Packet_Arrival_Cushion() override;
    virtual void Send_Chat(Utf16String text, int32_t player_mask) override;
    virtual void Send_Disconnect_Chat(Utf16String text) override;
    virtual void Send_File(Utf8String name, unsigned char c, unsigned short s) override;
    virtual unsigned short Send_File_Announce(Utf8String name, unsigned char c) override;
    virtual int32_t Get_File_Transfer_Progress(int32_t i, Utf8String name) override;
    virtual bool Are_All_Queues_Empty() override;
    virtual void Quit_Game() override;
    virtual void Self_Destruct_Player(int32_t slot) override;
    virtual void Vote_For_Player_Disconnect(int32_t slot) override;
    virtual bool Is_Packet_Router() override;
    virtual float Get_Incoming_Bytes_Per_Second() override;
    virtual float Get_Incoming_Packets_Per_Second() override;
    virtual float Get_Outgoing_Bytes_Per_Second() override;
    virtual float Get_Outgoing_Packets_Per_Second() override;
    virtual float Get_Unknown_Bytes_Per_Second() override;
    virtual float Get_Unknown_Packets_Per_Second() override;
    virtual void Update_Load_Progress(int32_t percent) override;
    virtual void Load_Progress_Complete() override;
    virtual void Send_Time_Out_Game_Start() override;
    virtual uint32_t Get_Local_Player_ID() override;
    virtual Utf16String Get_Player_Name(int32_t slot) override;
    virtual int32_t Get_Num_Players() override;
    virtual int32_t Get_Average_FPS() override;
    virtual int32_t Get_Slot_Average_FPS(int32_t slot) override;
    virtual void Attach_Transport(Transport *transport) override;
    virtual void Init_Transport() override;
    virtual bool Saw_CRC_Mismatch() override;
    virtual void Set_Saw_CRC_Mismatch() override;
    virtual bool Is_Player_Connected(int32_t slot) override;
    virtual void Notify_Others_Of_Current_Frame() override;
    virtual void Notify_Others_Of_New_Frame(uint32_t frame) override;
    virtual int32_t Get_Execution_Frame() override;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Toggle_Enabled() override; // not 100% identified yet
#endif
    virtual uint32_t Get_Ping_Frame() override;
    virtual int32_t Get_Pings_Sent() override;
    virtual int32_t Get_Pings_Recieved() override;

private:
    ConnectionManager *m_conMgr;
    NetLocalStatus m_lastFrame;
    int32_t m_localStatus;
    int32_t m_runAhead;
    int32_t m_frameRate;
    int32_t m_lastExecutionFrame;
    int32_t m_lastFrameCompleted;
    bool m_didSelfSlug;
    int64_t m_perfCountFreq;
    int64_t m_nextFrameTime;
    bool m_frameDataReady;
    bool m_unk;
    bool m_sawCRCMismatch;
    std::vector<uint32_t> m_uintVector[8]; // not 100% identified yet
    std::list<int32_t> m_intList; // not 100% identified yet
    GameWindow *m_crcMismatchWindow;
    bool m_enabled; // not 100% identified yet
};

#ifdef GAME_DLL
extern Network *&g_theNetwork;
#else
extern Network *g_theNetwork;
#endif
