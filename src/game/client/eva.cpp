/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief EVA
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "eva.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
Eva *g_theEva;
#endif

Eva::Eva() : m_player(nullptr), m_unk1(0), m_unk2(0), m_currentMessageType(NUM_EVA_MESSAGES), m_evaEnabled(true)
{
    for (int i = 0; i < NUM_EVA_MESSAGES; i++) {
        m_shouldPlay[i] = false;
    }
}

Eva::~Eva()
{
    for (auto it = m_checkInfo.begin(); it != m_checkInfo.end(); it++) {
        if (*it != nullptr) {
            (*it)->Delete_Instance();
        }
    }
}

void Eva::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Eva.ini", INI_LOAD_OVERWRITE, nullptr);
}

void Eva::Reset()
{
    m_unk2 = 0;
    m_unk1 = 0;

    for (auto it = m_check.begin(); it != m_check.end(); it = m_check.erase(it)) {
    }

    for (int i = 0; i < NUM_EVA_MESSAGES; i++) {
        m_shouldPlay[i] = false;
    }

    m_evaEnabled = true;
}

void Eva::Update()
{
#ifdef GAME_DLL
    Call_Method<void, Eva>(PICK_ADDRESS(0x00513340, 0x009BB4DA), this);
#endif
}

void Eva::Set_Should_Play(EvaMessage message)
{
    m_shouldPlay[message] = true;
}
