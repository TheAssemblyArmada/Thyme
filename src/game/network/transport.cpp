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
#include "transport.h"
#include "crc.h"
#include "endiantype.h"
#include "globaldata.h"
#include "rtsutils.h"
#include "sockets.h"

/**
 * Initialises the packet transport system.
 *
 * @info Mac port of ZH does not take an address and passes 0 to Bind.
 *
 * 0x00733A30
 */
bool Transport::Init(uint32_t address, uint16_t port)
{
    // Perform any needed initialisation of the sockets api (basically just windows)
    if (!m_winsockInit) {
#ifdef PLATFORM_WINDOWS
        WSADATA wsadata;

        // Request version 2.2
        if (WSAStartup(MAKEWORD(2, 2) /*0x202u*/, &wsadata) != 0) {
            return false;
        }

        // Check we actually got 2.2
        if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
            WSACleanup();

            return false;
        }
#endif
        m_winsockInit = true;
    }

    if (m_udpsock != nullptr) {
        delete m_udpsock;
    }

    m_udpsock = new UDP;

    if (m_udpsock == nullptr) {
        return false;
    }

    uint32_t start_time = rts::Get_Time();
    int socket_result = SOCKET_ERROR;

    while (rts::Get_Time() - start_time < 1000) {
        socket_result = m_udpsock->Bind(address, port);
        if (socket_result == UDP::OK) {
            break;
        }
    }

    if (socket_result != UDP::OK) {
        delete m_udpsock;
        m_udpsock = nullptr;

        return false;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        m_inBuffer[i].length = 0;
        m_outBuffer[i].length = 0;
    }

    for (int i = 0; i < STATS_COUNT; ++i) {
        m_incomingBytes[i] = 0;
        m_unknownBytes[i] = 0;
        m_outgoingBytes[i] = 0;
        m_incomingPackets[i] = 0;
        m_unknownPackets[i] = 0;
        m_outgoingPackets[i] = 0;
    }

    m_statisticsSlot = 0;
    m_lastSecond = rts::Get_Time();
    m_port = port;

    return true;
}

/**
 * Sends any queued packets and receives any data waiting on the socket.
 *
 * 0x00716CD0
 */
bool Transport::Update()
{
    bool result = true;

    if (!Do_Recv() && m_udpsock != nullptr && m_udpsock->Get_Status() == UDP::ADDRNOTAVAIL) {
        result = false;
    }

    if (!Do_Send() && m_udpsock != nullptr && m_udpsock->Get_Status() == UDP::ADDRNOTAVAIL) {
        result = false;
    }

    return result;
}

/**
 * Deletes the udp socket wrapper and shuts down the winsock API if applicable.
 */
void Transport::Reset()
{
    if (m_udpsock != nullptr) {
        delete m_udpsock;
        m_udpsock = nullptr;
    }

    if (m_winsockInit) {
        m_winsockInit = false;
#ifdef PLATFORM_WINDOWS
        WSACleanup();
#endif
    }
}

/**
 * Sends any queued packets.
 *
 * 0x00716D20
 */
bool Transport::Do_Send()
{
    if (m_udpsock == nullptr) {
        return false;
    }

    bool all_sent = true;
    uint32_t now = rts::Get_Time();

    // If more than 1 second has elapsed, open up a new statistics slot.
    if (m_lastSecond + 1000 < now) {
        m_lastSecond = now;
        m_statisticsSlot = (m_statisticsSlot + 1) % STATS_COUNT;
        m_outgoingPackets[m_statisticsSlot] = 0;
        m_outgoingBytes[m_statisticsSlot] = 0;
        m_incomingPackets[m_statisticsSlot] = 0;
        m_incomingBytes[m_statisticsSlot] = 0;
        m_unknownPackets[m_statisticsSlot] = 0;
        m_unknownBytes[m_statisticsSlot] = 0;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        if (m_outBuffer[i].length != 0) {
            // Send the packet and free up the buffer if it succeeds, otherwise return that not all packets sent.
            if (m_udpsock->Write(reinterpret_cast<uint8_t *>(&m_outBuffer[i]),
                    m_outBuffer[i].length + sizeof(TransportMessageHeader),
                    m_outBuffer[i].addr,
                    m_outBuffer[i].port)
                <= 0) {
                all_sent = false;
            } else {
                ++m_outgoingPackets[m_statisticsSlot];
                m_outgoingBytes[m_statisticsSlot] += m_outBuffer[i].length + sizeof(TransportMessageHeader);
                m_outBuffer[i].length = 0;
            }
        }
    }

    return all_sent;
}

/**
 * Receives any data from the socket into waiting buffers.
 *
 * 0x00716E30
 */
bool Transport::Do_Recv()
{
    sockaddr_in from;
    TransportMessage incoming;
    int now = rts::Get_Time();
    int len;

    for (len = m_udpsock->Read(reinterpret_cast<uint8_t *>(&incoming), 1024, &from); len > 0;
         len = m_udpsock->Read(reinterpret_cast<uint8_t *>(&incoming), sizeof(incoming), &from)) {
        Reveal(&incoming, len);
        incoming.length = len - sizeof(TransportMessageHeader);

        // If we don't have a packet that looks like it was meant for us, ignore it and log it for stats.
        if (len <= 6 || !Is_Thyme_Packet(&incoming)) {
            ++m_unknownPackets[m_statisticsSlot];
            m_unknownBytes[m_statisticsSlot] += len;

            continue;
        }

        ++m_incomingPackets[m_statisticsSlot];
        m_incomingBytes[m_statisticsSlot] += len;
        int free_slot = 0;

        // Find a free slot
        while (m_outBuffer[free_slot++].length != 0) {
            if (free_slot == BUFFER_COUNT) {
                continue;
            }
        }

        // Copy the data into it.
        m_inBuffer[free_slot].length = incoming.length;
        m_inBuffer[free_slot].addr = be32toh(from.sin_addr.s_addr);
        m_inBuffer[free_slot].port = be16toh(from.sin_port);
        memcpy(&m_inBuffer[free_slot], &incoming, len);
    }

    if (len == SOCKET_ERROR) {
        return false;
    }

    return true;
}

/**
 * Queues data to be sent in the next update.
 *
 * 0x00717060
 */
bool Transport::Queue_Send(uint32_t addr, uint16_t port, const char *buf, int len)
{
    if (len < 1 || len > 476) {
        return false;
    }

    int free_slot = 0;

    while (m_outBuffer[free_slot++].length != 0) {
        if (free_slot == BUFFER_COUNT) {
            return false;
        }
    }

    // Prepare our chosen buffer slot with the data to send and where to send it.
    m_outBuffer[free_slot].length = len;
    memcpy(m_outBuffer[free_slot].data, buf, len);
    m_outBuffer[free_slot].addr = addr;
    m_outBuffer[free_slot].port = port;
    m_outBuffer[free_slot].header.magic = MAGIC_NUM;

    // Compute a CRC for the network header.
    CRC crc;
    crc.Compute_CRC(&m_outBuffer[free_slot].header.magic, m_outBuffer[free_slot].length + sizeof(uint16_t));
    m_outBuffer[free_slot].header.crc = crc.Get_CRC();

    /*// This appears to obfuscate the packets message, presumably to frustrate attempts to sniff it or interfere with it.
    int count = len + sizeof(TransportMessageHeader) / 4;
    int mix_magic = 0xFADE;

    // Treat the entire buffer including the header as a block of integers.
    uint32_a *block = reinterpret_cast<uint32_a *>(&m_outBuffer[free_slot].header);

    // Perform obfuscation maths.
    while (count--) {
        *block = htobe32(mix_magic ^ *block);
        ++block;
        mix_magic += 801;
    }*/

    Obfuscate(&m_outBuffer[free_slot].header, len + sizeof(TransportMessageHeader));

    return true;
}

/**
 * Obfuscates data to be sent.
 */
void Transport::Obfuscate(void *data, int len)
{
    int count = len / 4;
    int mix_magic = OBFUSCATE_NUM;
    uint32_a *block = static_cast<uint32_a *>(data); // Using aliasing type to keep GCC/Clang from doing the wrong thing.

    while (count--) {
        *block = htobe32(mix_magic ^ *block);
        ++block;
        mix_magic += 801;
    }
}

/**
 * Deobfuscates data that has been received.
 */
void Transport::Reveal(void *data, int len)
{
    int count = len / 4;
    int mix_magic = OBFUSCATE_NUM;
    uint32_a *block = static_cast<uint32_a *>(data); // Using aliasing type to keep GCC/Clang from doing the wrong thing.

    while (count--) {
        *block = mix_magic ^ htobe32(*block);
        ++block;
        mix_magic += 801;
    }
}

/**
 * Checks if the packet appears to be generated by the Thyme engine.
 */
bool Transport::Is_Thyme_Packet(const TransportMessage *msg)
{
    if (msg == nullptr || msg->length <= 0 || msg->length > 1024) {
        return false;
    }

    CRC crc;
    crc.Compute_CRC(&msg->header.magic, msg->length + 2);

    if (msg->header.crc != crc.Get_CRC()) {
        return false;
    }

    if (msg->header.magic != MAGIC_NUM) {
        return false;
    }

    return true;
}
