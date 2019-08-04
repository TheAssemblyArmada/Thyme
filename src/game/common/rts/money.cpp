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

void Money::Xfer_Snapshot(Xfer *xfer)
{

}

void Money::Deposit(unsigned amount, bool play_sound)
{
#ifdef GAME_DLL
    Call_Method<void, Money, unsigned, bool>(0x005008D0, this, amount, play_sound);
#else
    if (amount == 0) {
        return;
    }

    AudioEventRTS deposit_sound = g_theAudio->Get_Misc_Audio()->Money_Deposit_Sound();
    deposit_sound.Set_Player_Index(m_playerIndex);

    if (play_sound) {
        g_theAudio->Add_Audio_Event(&deposit_sound);
    }

    m_money += amount;
    // TODO Requires PlayerList, Player, AcademyStats
#endif
}

void Money::Parse_Money_Amount(INI *ini, void *formal, void *store, void const *user_data)
{
    INI::Parse_Unsigned(ini, formal, &(static_cast<Money*>(store)->m_money), user_data);
}
