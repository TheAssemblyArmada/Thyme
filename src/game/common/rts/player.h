/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object for tracking player information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "handicap.h"
#include "snapshot.h"
#include "unicodestring.h"

// TODO this is just a skeleton to support other objects at the moment.
class Player : public SnapShot
{
public:
    int Get_Player_Index() { return m_playerIndex; }
    int Get_Color() { return m_playerColor; }

public:
    int unk1;
    int unk2;
    Handicap m_handicap;
    int unk3;
    int unk4;
    int m_playerIndex;
    char unk[0xFC];
    int m_playerColor;
};