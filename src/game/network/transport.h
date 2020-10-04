/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class handling network transport of data.
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
#include "udp.h"

#pragma pack(push, 1)
struct TransportMessageHeader
{
    uint32_t crc;
    uint16_t magic;
};

struct TransportMessage
{
    TransportMessageHeader header;
    char data[1024];
    int32_t length;
    uint32_t addr;
    uint16_t port;
};
#pragma pack(pop)

class Transport
{
    enum
    {
        BUFFER_COUNT = 128,
        STATS_COUNT = 30,
        MAGIC_NUM = 0xF00D,
        OBFUSCATE_NUM = 0xFADE,
    };

public:
    Transport() : m_winsockInit(false), m_udpsock(nullptr) {}
    ~Transport() { Reset(); }

    bool Init(uint32_t address, uint16_t port);
    bool Update();
    void Reset();
    bool Do_Send();
    bool Do_Recv();
    bool Queue_Send(uint32_t addr, uint16_t port, const char *buf, int len);
    void Allow_Broadcast(bool allow)
    {
        if (m_udpsock != nullptr)
            m_udpsock->Allow_Broadcasts(allow);
    }

private:
    static void Obfuscate(void *data, int len);
    static void Reveal(void *data, int len);
    static bool Is_Thyme_Packet(const TransportMessage *msg);

private:
    TransportMessage m_outBuffer[BUFFER_COUNT];
    TransportMessage m_inBuffer[BUFFER_COUNT];
    uint16_t m_port;
    bool m_winsockInit;
    UDP *m_udpsock;
    bool m_useLatency;
    bool m_usePacketLoss;
    uint32_t m_incomingBytes[STATS_COUNT];
    uint32_t m_unknownBytes[STATS_COUNT];
    uint32_t m_outgoingBytes[STATS_COUNT];
    uint32_t m_incomingPackets[STATS_COUNT];
    uint32_t m_unknownPackets[STATS_COUNT];
    uint32_t m_outgoingPackets[STATS_COUNT];
    int32_t m_statisticsSlot;
    uint32_t m_lastSecond;
};
