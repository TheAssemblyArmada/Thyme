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

#ifdef GAME_DLL
StaticNameKey &g_teamNameKey = Make_Global<StaticNameKey>(0x00A3A830);
StaticNameKey &g_theInitialCameraPositionKey = Make_Global<StaticNameKey>(0x00A3A880);
StaticNameKey &g_playerNameKey = Make_Global<StaticNameKey>(0x00A32580);
StaticNameKey &g_playerIsHumanKey = Make_Global<StaticNameKey>(0x00A3A8F0);
StaticNameKey &g_playerDisplayNameKey = Make_Global<StaticNameKey>(0x00A32588);
StaticNameKey &g_playerFactionKey = Make_Global<StaticNameKey>(0x00A326B8);
StaticNameKey &g_playerAlliesKey = Make_Global<StaticNameKey>(0x00A36758);
StaticNameKey &g_playerEnemiesKey = Make_Global<StaticNameKey>(0x00A32668);
StaticNameKey &g_teamOwnerKey = Make_Global<StaticNameKey>(0x00A32568);
StaticNameKey &g_teamIsSingletonKey = Make_Global<StaticNameKey>(0x00A3A948);
#else
StaticNameKey g_teamNameKey("teamName");
StaticNameKey g_theInitialCameraPositionKey("InitialCameraPosition");
StaticNameKey g_playerNameKey("playerName");
StaticNameKey g_playerIsHumanKey("playerIsHuman");
StaticNameKey g_playerDisplayNameKey("playerDisplayName");
StaticNameKey g_playerFactionKey("playerFaction");
StaticNameKey g_playerAlliesKey("playerAllies");
StaticNameKey g_playerEnemiesKey("playerEnemies");
StaticNameKey g_teamOwnerKey("teamOwner");
StaticNameKey g_teamIsSingletonKey("teamIsSingleton");
#endif

NameKeyType StaticNameKey::Key()
{
    if (m_key == NAMEKEY_INVALID && g_theNameKeyGenerator != nullptr) {
        m_key = g_theNameKeyGenerator->Name_To_Key(m_name);
    }

    return m_key;
}
