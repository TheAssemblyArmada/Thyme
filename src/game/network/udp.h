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
#pragma once

#include "always.h"
#include "sockets.h"

class UDP
{
public:
    enum SockStatus
    {
        OK = 0,
        UNKNOWN = -1,
        ISCONN = -2,
        INPROGRESS = -3,
        ALREADY = -4,
        AGAIN = -5,
        ADDRINUSE = -6,
        ADDRNOTAVAIL = -7,
        BADF = -8,
        CONNREFUSED = -9,
        INTR = -10,
        NOTSOCK = -11,
        WOULDBLOCK = -12,
        INVAL = -13,
        TIMEDOUT = -14,
    };

public:
    UDP() : m_fd(0)
    {
        // #BUGFIX Initialize all members
        m_myIP = 0;
        m_myPort = 0;
        m_addr = sockaddr_in{};
        m_status = 0;
    }
    ~UDP();

    int Bind(uint32_t address, uint16_t port);
    int Write(const uint8_t *buffer, int length, uint32_t address, uint16_t port);
    int Read(const uint8_t *buffer, int length, sockaddr_in *from);
    int Get_Status();
    void Clear_Status() { m_status = 0; }
    bool Allow_Broadcasts(bool allow);
    int Set_Blocking(bool block);
    bool Set_Input_Buffer(uint32_t size);
    bool Set_Output_Buffer(uint32_t size);
    uint32_t Get_Input_Buffer();
    uint32_t Get_Output_Buffer();
    int Get_Local_Addr(uint32_t &address, uint16_t &port);

private:
    SOCKET m_fd;
    uint32_t m_myIP;
    uint16_t m_myPort;
    sockaddr_in m_addr;
    int m_status;
};
