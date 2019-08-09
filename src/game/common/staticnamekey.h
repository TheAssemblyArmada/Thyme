/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Holds static mappings of names to a name key.
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
#include "namekeygenerator.h"

class StaticNameKey
{
public:
    StaticNameKey(const char *name) : m_key(NAMEKEY_INVALID), m_name(name) {}

    operator NameKeyType() { return Key(); }

    NameKeyType Key();
    const char *Name() { return m_name; }

private:
    NameKeyType m_key;
    const char *m_name;
};

#ifdef GAME_DLL
extern StaticNameKey &g_teamNameKey;
extern StaticNameKey &g_theInitialCameraPositionKey;
extern StaticNameKey &g_playerNameKey;
extern StaticNameKey &g_playerIsHumanKey;
extern StaticNameKey &g_playerDisplayNameKey;
extern StaticNameKey &g_playerFactionKey;
extern StaticNameKey &g_playerAlliesKey;
extern StaticNameKey &g_playerEnemiesKey;
extern StaticNameKey &g_teamOwnerKey;
extern StaticNameKey &g_teamIsSingletonKey;
#else
extern StaticNameKey g_teamNameKey;
extern StaticNameKey g_theInitialCameraPositionKey;
extern StaticNameKey g_playerNameKey;
extern StaticNameKey g_playerIsHumanKey;
extern StaticNameKey g_playerDisplayNameKey;
extern StaticNameKey g_playerFactionKey;
extern StaticNameKey g_playerAlliesKey;
extern StaticNameKey g_playerEnemiesKey;
extern StaticNameKey g_teamOwnerKey;
extern StaticNameKey g_teamIsSingletonKey;
#endif