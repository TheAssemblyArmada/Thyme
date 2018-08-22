/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Sockets api wrapper around UDP protocol transfers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "udp.h"
#include "endiantype.h"
#include "gamedebug.h"

/**
 * 0x00733A20 
 */
UDP::~UDP()
{
    if (m_fd != 0) {
        closesocket(m_fd);
    }
}

/**
 * Opens a socket for this UDP object and binds the provided address to it.
 *
 * 0x00733A30
 */
int UDP::Bind(uint32_t address, uint16_t port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htobe16(port);
    m_addr.sin_addr.s_addr = htobe32(address);
    m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (m_fd != INVALID_SOCKET) {
        if (bind(m_fd, (sockaddr *)&m_addr, sizeof(m_addr)) == SOCKET_ERROR) {
            m_status = LastSocketError;

            return Get_Status();
        }

        int len = sizeof(m_addr);
        getsockname(m_fd, (sockaddr *)&m_addr, &len);
        m_myIP = be32toh(m_addr.sin_addr.s_addr);
        m_myPort = be16toh(m_addr.sin_port);

        if (Set_Blocking(false) == -1) {
            ASSERT_PRINT(false, "Couldn't set nonblocking mode!\n");
        }

        return OK;
    }

    return UNKNOWN;
}

/**
 * Sends data to the destination address and port. Returns -1 on error, call Get_Status for reason.
 *
 * 0x00733A30
 */
int UDP::Write(const uint8_t *buffer, int length, uint32_t address, uint16_t port)
{
    sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_port = htobe16(port);
    to.sin_addr.s_addr = htobe32(address);
    Clear_Status();
    int result = sendto(m_fd, (char *)buffer, length, 0, (sockaddr *)&to, sizeof(to));

    if (result == SOCKET_ERROR) {
        m_status = LastSocketError;
    }

    return result;
}

/**
 * Retrives data sent to the open socket. Will return 0 if the operation would have blocked, -1 on all other errors.
 *
 * 0x00733BA0
 */
int UDP::Read(const uint8_t *buffer, int length, sockaddr_in *from)
{
    int result = SOCKET_ERROR;

    if (from != nullptr) {
        int addr_len = sizeof(*from);
        result = recvfrom(m_fd, (char *)buffer, length, 0, (sockaddr *)from, &addr_len);

        if (result != SOCKET_ERROR) {
            return result;
        }

        if (LastSocketError != SOCKEWOULDBLOCK && LastSocketError != SOCKEAGAIN) {
            m_status = LastSocketError;

            return SOCKET_ERROR;
        }
    } else {
        result = recvfrom(m_fd, (char *)buffer, length, 0, nullptr, nullptr);

        if (result != SOCKET_ERROR) {
            return result;
        }

        if (LastSocketError != SOCKEWOULDBLOCK && LastSocketError != SOCKEAGAIN) {
            m_status = LastSocketError;

            return SOCKET_ERROR;
        }
    }

    return 0;
}

/**
 * Retrieves the last error set by UDP operations. See UDP::SockStatus enum for possible values.
 *
 * 0x00733C30
 */
int UDP::Get_Status()
{
    switch (m_status) {
        case SOCKEISCONN:
            return ISCONN;
        case SOCKEINPROGRESS:
            return INPROGRESS;
        case SOCKEALREADY:
            return ALREADY;
        case SOCKEADDRINUSE:
            return ADDRINUSE;
        case SOCKEADDRNOTAVAIL:
            return ADDRNOTAVAIL;
        case SOCKEBADF:
            return BADF;
        case SOCKECONNREFUSED:
            return CONNREFUSED;
        case SOCKEINTR:
            return INTR;
        case SOCKENOTSOCK:
            return NOTSOCK;
        case SOCKEWOULDBLOCK:
            return WOULDBLOCK;
#ifndef PLATFORM_WINDOWS // EAGAIN is an alternative to EWOULDBLOCK, but not on windows.
        case SOCKEAGAIN:
            return AGAIN;
#endif
        case SOCKEINVAL:
            return INVAL;
        case SOCKETIMEDOUT:
            return TIMEDOUT;
        default:
            break;
    }

    return m_status;
}

/**
 * Set whether the underlying socket uses broadcast or not.
 *
 * 0x00733CC0
 */
bool UDP::Allow_Broadcasts(bool allow)
{
    int32_t mode = allow;

    return setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST, (char *)&mode, sizeof(mode)) == 0;
}

/**
 * Set whether the socket will block on IO operations or not.
 */
int UDP::Set_Blocking(bool block)
{
    unsigned long mode = block == false;

    return (ioctlsocket(m_fd, FIONBIO, &mode) != SOCKET_ERROR) - 1;
}
