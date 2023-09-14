/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for handling road and bridge properties.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "terrainroads.h"
#include <captainslog.h>
#include <cstddef>

#ifndef GAME_DLL
TerrainRoadCollection *g_theTerrainRoads = nullptr;
#endif

const FieldParse TerrainRoadCollection::s_terrainRoadFieldParseTable[] = {
    { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_texture) },
    { "RoadWidth", &INI::Parse_Real, nullptr, offsetof(TerrainRoadType, m_roadWidth) },
    { "RoadWidthInTexture", &INI::Parse_Real, nullptr, offsetof(TerrainRoadType, m_roadWidthInTexture) },
    { nullptr, nullptr, nullptr, 0 }
};

const FieldParse TerrainRoadCollection::s_terrainBridgeFieldParseTable[] = {
    { "BridgeScale", &INI::Parse_Real, nullptr, offsetof(TerrainRoadType, m_bridgeScale) },
    { "ScaffoldObjectName", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_scaffoldObjectName) },
    { "ScaffoldSupportObjectName",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(TerrainRoadType, m_scaffoldSupportObjectName) },
    { "RadarColor", &INI::Parse_RGB_Color, nullptr, offsetof(TerrainRoadType, m_radarColor) },
    { "TransitionEffectsHeight", &INI::Parse_Real, nullptr, offsetof(TerrainRoadType, m_transitionEffectsHeight) },
    { "NumFXPerType", &INI::Parse_Int, nullptr, offsetof(TerrainRoadType, m_numFXPerType) },
    { "BridgeModelName", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_bridgeModelName) },
    { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_texture) },
    { "BridgeModelNameDamaged", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_bridgeModelNameDamaged) },
    { "TextureDamaged", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_textureDamaged) },
    { "BridgeModelNameReallyDamaged",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(TerrainRoadType, m_bridgeModelNameReallyDamaged) },
    { "TextureReallyDamaged", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_textureReallyDamaged) },
    { "BridgeModelNameBroken", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_bridgeModelNameBroken) },
    { "TextureBroken", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_textureBroken) },
    { "TowerObjectNameFromLeft",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(TerrainRoadType, m_towerObjectName[BRIDGE_TOWER_FROM_LEFT]) },
    { "TowerObjectNameFromRight",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(TerrainRoadType, m_towerObjectName[BRIDGE_TOWER_FROM_RIGHT]) },
    { "TowerObjectNameToLeft",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(TerrainRoadType, m_towerObjectName[BRIDGE_TOWER_TO_LEFT]) },
    { "TowerObjectNameToRight",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(TerrainRoadType, m_towerObjectName[BRIDGE_TOWER_TO_RIGHT]) },
    { "DamagedToSound", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_damagedToSounds[BODY_DAMAGED]) },
    { "RepairedToSound", &INI::Parse_AsciiString, nullptr, offsetof(TerrainRoadType, m_repairedToSounds[BODY_DAMAGED]) },
    { "TransitionToOCL", &TerrainRoadType::Parse_Transition_To_OCL, nullptr, 0 },
    { "TransitionToFX", &TerrainRoadType::Parse_Transition_To_FX, nullptr, 0 },
    { nullptr, nullptr, nullptr, 0 }
};

int TerrainRoadCollection::s_idCounter;

TerrainRoadType::TerrainRoadType() :
    m_name(),
    m_isBridge(false),
    m_id(0),
    m_next(nullptr),
    m_roadWidth(0.0f),
    m_roadWidthInTexture(0.0f),
    m_bridgeScale(1.0f),
    m_scaffoldObjectName(),
    m_scaffoldSupportObjectName(),
    m_radarColor{ 0.0f, 0.0f, 0.0f },
    m_bridgeModelName(),
    m_texture(),
    m_bridgeModelNameDamaged(),
    m_textureDamaged(),
    m_bridgeModelNameReallyDamaged(),
    m_textureReallyDamaged(),
    m_bridgeModelNameBroken(),
    m_textureBroken(),
    m_transitionEffectsHeight(0.0f),
    m_numFXPerType(0)
{
}

void TerrainRoadType::Parse_Transition_To_OCL(INI *ini, void *formal, void *store, void const *user_data)
{
    const char *transition = ini->Get_Next_Sub_Token("Transition");
    bool is_damage;

    if (strcasecmp(transition, "Damage") == 0) {
        is_damage = true;
    } else {
        captainslog_relassert(strcasecmp(transition, "Repair") == 0,
            0xDEAD0006,
            "'%s' at line %d has an invalid 'Transition:' type, must be 'Repair' or 'Damage'.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        is_damage = false;
    }

    int dmg = INI::Scan_IndexList(ini->Get_Next_Sub_Token("ToState"), g_bodyDamageNames);
    int effect = INI::Scan_Int(ini->Get_Next_Sub_Token("EffectNum")) - 1; // Effect is 1 based, we need 0 based.

    captainslog_relassert(effect >= 0 && effect < 3,
        0xDEAD0006,
        "'%s' at line %d has an invalid 'EffectNum:' type, must be between 1 and 3 inclusive.",
        ini->Get_Filename().Str(),
        ini->Get_Line_Number());

    const char *name = ini->Get_Next_Sub_Token("OCL");

    if (is_damage) {
        static_cast<TerrainRoadType *>(formal)->Set_Damaged_OCL(dmg, effect, name);
    } else {
        static_cast<TerrainRoadType *>(formal)->Set_Repaired_OCL(dmg, effect, name);
    }
}

void TerrainRoadType::Parse_Transition_To_FX(INI *ini, void *formal, void *store, void const *user_data)
{
    const char *transition = ini->Get_Next_Sub_Token("Transition");
    bool is_damage;

    if (strcasecmp(transition, "Damage") == 0) {
        is_damage = true;
    } else {
        captainslog_relassert(strcasecmp(transition, "Repair") == 0,
            0xDEAD0006,
            "'%s' at line %d has an invalid 'Transition:' type, must be 'Repair' or 'Damage'.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        is_damage = false;
    }

    int dmg = INI::Scan_IndexList(ini->Get_Next_Sub_Token("ToState"), g_bodyDamageNames);
    int effect = INI::Scan_Int(ini->Get_Next_Sub_Token("EffectNum")) - 1; // Effect is 1 based, we need 0 based.

    captainslog_relassert(effect >= 0 && effect < 3,
        0xDEAD0006,
        "'%s' at line %d has an invalid 'EffectNum:' type, must be between 1 and 3 inclusive.",
        ini->Get_Filename().Str(),
        ini->Get_Line_Number());

    const char *name = ini->Get_Next_Sub_Token("FX");

    if (is_damage) {
        static_cast<TerrainRoadType *>(formal)->Set_Damaged_FX(dmg, effect, name);
    } else {
        static_cast<TerrainRoadType *>(formal)->Set_Repaired_FX(dmg, effect, name);
    }
}

TerrainRoadCollection::~TerrainRoadCollection()
{
    while (m_roadList != nullptr) {
        TerrainRoadType *next = m_roadList->m_next;
        m_roadList->Delete_Instance();
        m_roadList = next;
    }

    while (m_bridgeList != nullptr) {
        TerrainRoadType *next = m_bridgeList->m_next;
        m_bridgeList->Delete_Instance();
        m_bridgeList = next;
    }
}

TerrainRoadType *TerrainRoadCollection::New_Road(Utf8String name)
{
    TerrainRoadType *retval = NEW_POOL_OBJ(TerrainRoadType);
    TerrainRoadType *def = Find_Road("DefaultRoad");

    // Assign next id.
    retval->m_id = s_idCounter++;
    retval->m_isBridge = false;
    retval->m_name = name;

    // Copy the road relevant attributes.
    if (def != nullptr) {
        retval->m_texture = def->m_texture;
        retval->m_roadWidth = def->m_roadWidth;
        retval->m_roadWidthInTexture = def->m_roadWidthInTexture;
    }

    // Add to the road list.
    retval->m_next = m_roadList;
    m_roadList = retval;

    return retval;
}

TerrainRoadType *TerrainRoadCollection::New_Bridge(Utf8String name)
{
    TerrainRoadType *retval = NEW_POOL_OBJ(TerrainRoadType);
    TerrainRoadType *def = Find_Road("DefaultBridge");

    // Assign next id.
    retval->m_id = s_idCounter++;
    retval->m_isBridge = true;
    retval->m_name = name;

    // Copy the bridge relevant attributes.
    if (def != nullptr) {
        retval->m_bridgeScale = def->m_bridgeScale;
        retval->m_scaffoldObjectName = def->m_scaffoldObjectName;
        retval->m_scaffoldSupportObjectName = def->m_scaffoldSupportObjectName;
        retval->m_bridgeModelName = def->m_bridgeModelName;
        retval->m_texture = def->m_texture;
        retval->m_bridgeModelNameDamaged = def->m_bridgeModelNameDamaged;
        retval->m_textureDamaged = def->m_textureDamaged;
        retval->m_bridgeModelNameReallyDamaged = def->m_bridgeModelNameReallyDamaged;
        retval->m_textureReallyDamaged = def->m_textureReallyDamaged;
        retval->m_bridgeModelNameBroken = def->m_bridgeModelNameBroken;
        retval->m_textureBroken = def->m_textureBroken;

        for (int i = 0; i < BRIDGE_MAX_TOWERS; ++i) {
            retval->m_towerObjectName[i] = def->m_towerObjectName[i];
        }

        for (int i = 0; i < BODY_COUNT; ++i) {
            retval->m_damagedToSounds[i] = def->m_damagedToSounds[i];
            retval->m_repairedToSounds[i] = def->m_repairedToSounds[i];

            for (int j = 0; j < 3; ++j) {
                retval->m_damagedTransitionOCL[i][j] = def->m_damagedTransitionOCL[i][j];
                retval->m_damagedTransitionFX[i][j] = def->m_damagedTransitionFX[i][j];
                retval->m_repairedTransitionOCL[i][j] = def->m_repairedTransitionOCL[i][j];
                retval->m_repairedTransitionFX[i][j] = def->m_repairedTransitionFX[i][j];
            }
        }

        retval->m_transitionEffectsHeight = def->m_transitionEffectsHeight;
        retval->m_numFXPerType = def->m_numFXPerType;
    }

    // Add to the bridge list.
    retval->m_next = m_bridgeList;
    m_bridgeList = retval;

    return retval;
}

TerrainRoadType *TerrainRoadCollection::Find_Road(Utf8String name)
{
    TerrainRoadType *retval = m_roadList;

    while (retval != nullptr) {
        if (strcmp(retval->m_name.Str(), name.Str()) == 0) {
            break;
        }

        retval = retval->m_next;
    }

    return retval;
}

TerrainRoadType *TerrainRoadCollection::Find_Bridge(Utf8String name)
{
    TerrainRoadType *retval = m_bridgeList;

    while (retval != nullptr) {
        if (strcmp(retval->m_name.Str(), name.Str()) == 0) {
            break;
        }

        retval = retval->m_next;
    }

    return retval;
}

// Was originally INI::parseTerrainRoadDefinition
void TerrainRoadCollection::Parse_Terrain_Road_Definition(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();

    captainslog_relassert(g_theTerrainRoads->Find_Road(token) == nullptr,
        0xDEAD0006,
        "'%s' at line %d has a duplicated Road that has been loaded elsewhere already.",
        ini->Get_Filename().Str(),
        ini->Get_Line_Number());

    ini->Init_From_INI(g_theTerrainRoads->New_Road(token), s_terrainRoadFieldParseTable);
}

// Was originally INI::parseTerrainBridgeDefinition
void TerrainRoadCollection::Parse_Terrain_Bridge_Definition(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();

    captainslog_relassert(g_theTerrainRoads->Find_Bridge(token) == nullptr,
        0xDEAD0006,
        "'%s' at line %d has a duplicated Bridge that has been loaded elsewhere already.",
        ini->Get_Filename().Str(),
        ini->Get_Line_Number());

    ini->Init_From_INI(g_theTerrainRoads->New_Bridge(token), s_terrainBridgeFieldParseTable);
}
