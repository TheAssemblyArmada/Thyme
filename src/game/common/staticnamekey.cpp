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

#ifndef GAME_DLL
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
StaticNameKey g_objectInitialHealthKey("objectInitialHealth");
StaticNameKey g_objectEnabledKey("objectEnabled");
StaticNameKey g_objectIndestructibleKey("objectIndestructible");
StaticNameKey g_objectUnsellableKey("objectUnsellable");
StaticNameKey g_objectPoweredKey("objectPowered");
StaticNameKey g_objectRecruitableAIKey("objectRecruitableAI");
StaticNameKey g_objectTargetableKey("objectTargetable");
StaticNameKey g_originalOwnerKey("originalOwner");
StaticNameKey g_uniqueIDKey("uniqueID");
StaticNameKey g_waypointIDKey("waypointID");
StaticNameKey g_waypointNameKey("waypointName");
#endif

NameKeyType StaticNameKey::Key()
{
    if (m_key == NAMEKEY_INVALID && g_theNameKeyGenerator != nullptr) {
        m_key = g_theNameKeyGenerator->Name_To_Key(m_name);
    }

    return m_key;
}
