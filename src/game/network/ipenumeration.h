/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief IP Enumeration
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
#include "mempoolobj.h"

class EnumeratedIP : public MemoryPoolObject
{
    IMPLEMENT_POOL(EnumeratedIP)

public:
    EnumeratedIP() : m_IP(0), m_next(nullptr) {}
    virtual ~EnumeratedIP() override {}

    Utf8String Get_IP_String() { return m_IPstring; }
    unsigned int Get_IP() { return m_IP; }
    EnumeratedIP *Get_Next() { return m_next; }
    void Set_Next(EnumeratedIP *next) { m_next = next; }
    void Set_IP(unsigned int ip) { m_IP = ip; }
    void Set_IP_String(Utf8String name) { m_IPstring = name; }

private:
    Utf8String m_IPstring;
    unsigned int m_IP;
    EnumeratedIP *m_next;
};

class IPEnumeration
{
public:
    IPEnumeration() : m_IPlist(nullptr), m_isWinsockInitialized(false) {}
    ~IPEnumeration();
    EnumeratedIP *Get_Addresses();
    Utf8String Get_Machine_Name();

private:
    EnumeratedIP *m_IPlist;
    bool m_isWinsockInitialized;
};