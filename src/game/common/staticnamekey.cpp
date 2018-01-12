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
#include "staticnamekey.h"

#ifndef THYME_STANDALONE
StaticNameKey &g_theTeamNameKey = Make_Global<StaticNameKey>(0x00A3A830);
StaticNameKey &g_theInitialCameraPositionKey = Make_Global<StaticNameKey>(0x00A3A880);
StaticNameKey &g_thePlayerNameKey = Make_Global<StaticNameKey>(0x00A32580);
#else
StaticNameKey g_theTeamNameKey("teamName");
StaticNameKey g_theInitialCameraPositionKey("InitialCameraPosition");
StaticNameKey g_thePlayerNameKey("playerName");
#endif

NameKeyType StaticNameKey::Key()
{
    if (m_key == NAMEKEY_INVALID && g_theNameKeyGenerator != nullptr) {
        m_key = g_theNameKeyGenerator->Name_To_Key(m_name);
    }

    return m_key;
}
