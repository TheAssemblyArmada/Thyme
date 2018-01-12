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

#ifndef STATICNAMEKEY_H
#define STATICNAMEKEY_H

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

#ifndef THYME_STANDALONE
#include "hooker.h"

extern StaticNameKey &g_theTeamNameKey;
extern StaticNameKey &g_theInitialCameraPositionKey;
extern StaticNameKey &g_thePlayerNameKey;
#else
extern StaticNameKey g_theTeamNameKey;
extern StaticNameKey g_theInitialCameraPositionKey;
extern StaticNameKey g_thePlayerNameKey;
#endif

#endif // STATICNAMEKEY_H
