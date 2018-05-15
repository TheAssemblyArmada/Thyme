/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for handling in game money tracking.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "snapshot.h"

class INI;

class Money : public SnapShot
{
public:
    Money() : m_money(0), m_playerIndex(0) {}
    virtual ~Money() {}

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

    void Empty() { m_money = 0; }
    void Deposit(unsigned amount, bool play_sound);

    static void Parse_Money_Amount(INI *ini, void *formal, void *store, void const *user_data);
protected:
    unsigned m_money;
    int m_playerIndex;
};