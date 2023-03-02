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
#include "money.h"
#include "audiomanager.h"
#include "ini.h"
#include "player.h"
#include "playerlist.h"

void Money::Xfer_Snapshot(Xfer *xfer) {}

void Money::Deposit(unsigned int amount, bool play_sound)
{
    if (amount == 0) {
        return;
    }

    AudioEventRTS deposit_sound = g_theAudio->Get_Misc_Audio()->m_moneyDeposit;
    deposit_sound.Set_Player_Index(m_playerIndex);

    if (play_sound) {
        g_theAudio->Add_Audio_Event(&deposit_sound);
    }

    m_money += amount;
    Player *player = g_thePlayerList->Get_Nth_Player(m_playerIndex);

    if (player != nullptr) {
        player->Get_Academy_Stats()->Record_Income();
    }
}

void Money::Parse_Money_Amount(INI *ini, void *formal, void *store, void const *user_data)
{
    INI::Parse_Unsigned_Int(ini, formal, &(static_cast<Money *>(store)->m_money), user_data);
}
