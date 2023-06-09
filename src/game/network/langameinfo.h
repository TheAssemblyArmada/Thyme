/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief LAN Game Info
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
#include "gameinfo.h"

class LANPlayer
{
private:
    Utf16String m_name;
    Utf16String m_login;
    Utf16String m_host;
    unsigned int m_lastHeard;
    LANPlayer *m_next;
    unsigned int m_IP;
};

class LANGameSlot : public GameSlot
{
private:
    LANPlayer m_user;
    Utf8String m_serial;
    unsigned int m_lastHeard;
};

class LANGameInfo : public GameInfo
{
public:
    virtual int Get_Local_Slot_Num() const override;
    virtual void Reset_Accepted() override;

private:
    LANGameSlot m_LANSlot[8];
    LANGameInfo *m_next;
    unsigned int m_lastHeard;
    Utf16String m_gameName;
};
