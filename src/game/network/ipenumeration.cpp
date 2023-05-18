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
#include "ipenumeration.h"
#include "sockets.h"

IPEnumeration::~IPEnumeration()
{
    if (m_isWinsockInitialized) {
#ifdef PLATFORM_WINDOWS
        WSACleanup();
        m_isWinsockInitialized = false;
#endif
    }

    for (EnumeratedIP *i = m_IPlist; i != nullptr; m_IPlist = i) {
        i = i->Get_Next();
        m_IPlist->Delete_Instance();
    }
}

EnumeratedIP *IPEnumeration::Get_Addresses()
{
    if (m_IPlist != nullptr) {
        return m_IPlist;
    }

    if (!m_isWinsockInitialized) {
#ifdef PLATFORM_WINDOWS
        WSADATA wsadata;

        // Request version 2.2
        if (WSAStartup(MAKEWORD(2, 2) /*0x202u*/, &wsadata) != 0) {
            return nullptr;
        }

        // Check we actually got 2.2
        if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
            WSACleanup();

            return nullptr;
        }

#endif
        m_isWinsockInitialized = true;
    }

    char name[256];

    if (gethostname(name, 256) != 0) {
        captainslog_debug("Failed call to gethostname; LastSocketError returned %d", LastSocketError);
        return 0;
    } else {
        captainslog_debug("Hostname is '%s'", name);
        hostent *entry = gethostbyname(name);

        if (entry == nullptr) {
            captainslog_debug("Failed call to gethostnyname; LastSocketError returned %d", LastSocketError);
            return nullptr;
        } else if (entry->h_length != 4) {
            captainslog_debug("gethostbyname returns oddly-sized IP addresses!");
            return nullptr;
        } else {
            for (int i = 0; entry->h_addr_list[i] != nullptr; i++) {
                char *addr = entry->h_addr_list[i];
                EnumeratedIP *enumip = new EnumeratedIP;
                Utf8String str;
                str.Format("%d.%d.%d.%d",
                    (unsigned char)addr[0],
                    (unsigned char)addr[1],
                    (unsigned char)addr[2],
                    (unsigned char)addr[3]);
                u_long ip = *reinterpret_cast<u_long *>(addr);
                u_long ip2 = ntohl(ip);
                enumip->Set_IP_String(str);
                enumip->Set_IP(ip2);
                captainslog_debug("IP: 0x%8.8X / 0x%8.8X (%s)", ip, ip2, str.Str());

                if (m_IPlist == nullptr) {
                    m_IPlist = enumip;
                    enumip->Set_Next(nullptr);
                } else {
                    if (enumip->Get_IP() < m_IPlist->Get_IP()) {
                        enumip->Set_Next(m_IPlist);
                        m_IPlist = enumip;
                    } else {
                        EnumeratedIP *enum_ip;

                        for (enum_ip = m_IPlist; enum_ip->Get_Next() != nullptr; enum_ip = enum_ip->Get_Next()) {
                            if (enum_ip->Get_Next()->Get_IP() >= enumip->Get_IP()) {
                                break;
                            }
                        }

                        enumip->Set_Next(enum_ip->Get_Next());
                        enum_ip->Set_Next(enumip);
                    }
                }
            }

            return m_IPlist;
        }
    }
}

Utf8String IPEnumeration::Get_Machine_Name()
{
    if (!m_isWinsockInitialized) {
#ifdef PLATFORM_WINDOWS
        WSADATA wsadata;

        // Request version 2.2
        if (WSAStartup(MAKEWORD(2, 2) /*0x202u*/, &wsadata) != 0) {
            return "";
        }

        // Check we actually got 2.2
        if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
            WSACleanup();

            return "";
        }

#endif
        m_isWinsockInitialized = true;
    }

    char name[256];

    if (gethostname(name, 256) != 0) {
        captainslog_debug("Failed call to gethostname; LastSocketError returned %d", LastSocketError);
        return "";
    } else {
        return name;
    }
}
