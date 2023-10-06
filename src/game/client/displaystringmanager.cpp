/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Virtual base class for managing DisplayString based classes.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "displaystringmanager.h"

#ifndef GAME_DLL
DisplayStringManager *g_theDisplayStringManager = nullptr;
#endif

DisplayStringManager::DisplayStringManager() : m_stringList(nullptr), m_currentCheckpoint(nullptr) {}

DisplayStringManager::~DisplayStringManager() {}

/**
 * @brief Links a DisplayString into the managers internal list.
 *
 * 0x0041FE10
 */
void DisplayStringManager::Link(DisplayString *string)
{
    string->m_next = m_stringList;

    if (m_stringList != nullptr) {
        m_stringList->m_prev = string;
    }

    m_stringList = string;
}

/**
 * @brief Unlinks a DisplayString from the managers internal list.
 *
 * 0x0041FE30
 */
void DisplayStringManager::Unlink(DisplayString *string)
{
    DisplayString *next = string->m_next;

    if (next != nullptr) {
        next->m_prev = string->m_prev;
    }

    DisplayString *prev = string->m_prev;

    if (prev != nullptr) {
        prev->m_next = string->m_next;
    } else {
        m_stringList = string->m_next;
    }
}
