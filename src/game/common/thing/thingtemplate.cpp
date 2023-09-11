/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base template class for "things" static data.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "thingtemplate.h"
#include "aiupdate.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "fxlist.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "image.h"
#include "module.h"
#include "modulefactory.h"
#include "player.h"
#include "productionprerequisite.h"
#include "w3dshadow.h"
#include <cstring>
using std::strcpy;
using std::strncpy;

static const char *s_radarPriorityNames[] = { "INVALID", "NOT_ON_RADAR", "STRUCTURE", "UNIT", "LOCAL_UNIT_ONLY", nullptr };

static const char *s_buildableStatusNames[] = { "Yes", "Ignore_Prerequisites", "No", "Only_By_AI", nullptr };

static const char *s_buildCompletionNames[] = { "INVALID", "APPEARS_AT_RALLY_POINT", "PLACED_BY_PLAYER", nullptr };

static const char *s_editorSortingNames[] = { "NONE",
    "STRUCTURE",
    "INFANTRY",
    "VEHICLE",
    "SHRUBBERY",
    "MISC_MAN_MADE",
    "MISC_NATURAL",
    "DEBRIS",
    "SYSTEM",
    "AUDIO",
    "TEST",
    "FOR_REVIEW",
    "ROAD",
    "WAYPOINT",
    nullptr };

static const char *s_theShadowNames[] = { "SHADOW_DECAL",
    "SHADOW_VOLUME",
    "SHADOW_PROJECTION",
    "SHADOW_DYNAMIC_PROJECTION",
    "SHADOW_DIRECTIONAL_PROJECTION",
    "SHADOW_ALPHA_DECAL",
    "SHADOW_ADDITIVE_DECAL",
    nullptr };

#ifndef GAME_DLL
AudioEventRTS ThingTemplate::s_audioEventNoSound;
#endif

// clang-format off
const FieldParse ThingTemplate::s_objectFieldParseTable[] = {
    {"DisplayName", &INI::Parse_And_Translate_Label, nullptr, offsetof(ThingTemplate, m_displayName)},
    {"RadarPriority", &INI::Parse_Byte_Sized_Index_List, &s_radarPriorityNames, offsetof(ThingTemplate, m_radarPriority)},
    {"TransportSlotCount", &INI::Parse_Unsigned_Byte, nullptr, offsetof(ThingTemplate, m_transportSlotCount)},
    {"FenceWidth", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_fenceWidth)},
    {"FenceXOffset", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_fenceXOffset)},
    {"IsBridge", &INI::Parse_Bool, nullptr, offsetof(ThingTemplate, m_isBridge)},
    {"ArmorSet", &ThingTemplate::Parse_Armor_Template_Set, nullptr, 0},
    {"WeaponSet", &ThingTemplate::Parse_Weapon_Template_Set, nullptr, 0},
    {"VisionRange", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_visionRange)},
    {"ShroudClearingRange", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_shroudClearingRange)},
    {"ShroudRevealToAllRange", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_shroudRevealToAllRange)},
    {"PlacementViewAngle", &INI::Parse_Angle_Real, nullptr, offsetof(ThingTemplate, m_placementViewAngle)},
    {"FactoryExitWidth", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_factoryExitWidth)},
    {"FactoryExtraBibWidth", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_factoryExtraBibWidth)},
    {"SkillPointValue", &ThingTemplate::Parse_Int_List, reinterpret_cast<const void *>(4), offsetof(ThingTemplate, m_skillPointValues)},
    {"ExperienceValue", &ThingTemplate::Parse_Int_List, reinterpret_cast<const void *>(4), offsetof(ThingTemplate, m_experienceValues)},
    {"ExperienceRequired", &ThingTemplate::Parse_Int_List, reinterpret_cast<const void *>(4), offsetof(ThingTemplate, m_experienceRequired)},
    {"IsTrainable", &INI::Parse_Bool, nullptr, offsetof(ThingTemplate, m_isTrainable)},
    {"EnterGuard", &INI::Parse_Bool, nullptr, offsetof(ThingTemplate, m_enterGuard)},
    {"HijackGuard", &INI::Parse_Bool, nullptr, offsetof(ThingTemplate, m_hijackGuard)},
    {"Side", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_defaultOwningSide)},
    {"Prerequisites", &ThingTemplate::Parse_Prerequisites, nullptr, 0},
    {"Buildable", &INI::Parse_Byte_Sized_Index_List, s_buildableStatusNames, offsetof(ThingTemplate, m_buildable)},
    {"BuildCost", &INI::Parse_Unsigned_Short, nullptr, offsetof(ThingTemplate, m_buildCost)},
    {"BuildTime", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_buildTime)},
    {"RefundValue", &INI::Parse_Unsigned_Short, nullptr, offsetof(ThingTemplate, m_refundValue)},
    {"BuildCompletion", &INI::Parse_Byte_Sized_Index_List, s_buildCompletionNames, offsetof(ThingTemplate, m_buildCompletion)},
    {"EnergyProduction", &INI::Parse_Int, nullptr, offsetof(ThingTemplate, m_energyProduction)},
    {"EnergyBonus", &INI::Parse_Int, nullptr, offsetof(ThingTemplate, m_energyBonus)},
    {"IsForbidden", &INI::Parse_Bool, nullptr, offsetof(ThingTemplate, m_isForbidden)},
    {"IsPrerequisite", &INI::Parse_Bool, nullptr, offsetof(ThingTemplate, m_isPrerequisite)},
    {"DisplayColor", &INI::Parse_Color_Int, nullptr, offsetof(ThingTemplate, m_displayColor)},
    {"EditorSorting", &INI::Parse_Byte_Sized_Index_List, s_editorSortingNames, offsetof(ThingTemplate, m_editorSorting)},
    {"KindOf", &BitFlags<KINDOF_COUNT>::Parse_From_INI, nullptr, offsetof(ThingTemplate, m_kindOf)},
    {"CommandSet", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_commandSetString)},
    {"BuildVariations", &INI::Parse_AsciiString_Vector, nullptr, offsetof(ThingTemplate, m_buildVariations)},
    {"Behavior", &ThingTemplate::Parse_Module_Name, reinterpret_cast<const void *>(0), offsetof(ThingTemplate, m_body)},
    {"Body", &ThingTemplate::Parse_Module_Name, reinterpret_cast<const void *>(999), offsetof(ThingTemplate, m_body)},
    {"Draw", &ThingTemplate::Parse_Module_Name, reinterpret_cast<const void *>(1), offsetof(ThingTemplate, m_draws)},
    {"ClientUpdate", &ThingTemplate::Parse_Module_Name, reinterpret_cast<const void *>(2), offsetof(ThingTemplate, m_clientUpdates)},
    {"SelectPortrait", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_selectedPortraitImageName)},
    {"ButtonImage", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_buttonImageName)},
    {"UpgradeCameo1", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_upgradeCameoNames[0])},
    {"UpgradeCameo2", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_upgradeCameoNames[1])},
    {"UpgradeCameo3", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_upgradeCameoNames[2])},
    {"UpgradeCameo4", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_upgradeCameoNames[3])},
    {"UpgradeCameo5", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_upgradeCameoNames[4])},
    {"VoiceSelect", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[0])},
    {"VoiceGroupSelect", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[1])},
    {"VoiceMove", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[3])},
    {"VoiceAttack", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[4])},
    {"VoiceEnter", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[5])},
    {"VoiceFear", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[6])},
    {"VoiceSelectElite", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[2])},
    {"VoiceCreated", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[7])},
    {"VoiceTaskUnable", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[9])},
    {"VoiceTaskComplete", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[10])},
    {"VoiceMeetEnemy", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[11])},
    {"VoiceGarrison", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[30])},
    {"VoiceDefect", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[32])},
    {"VoiceAttackSpecial", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[33])},
    {"VoiceAttackAir", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[34])},
    {"VoiceGuard", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[35])},
    {"SoundMoveStart", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[12])},
    {"SoundMoveStartDamaged", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[13])},
    {"SoundMoveLoop", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[14])},
    {"SoundMoveLoopDamaged", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[15])},
    {"SoundAmbient", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[16])},
    {"SoundAmbientDamaged", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[17])},
    {"SoundAmbientReallyDamaged", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[18])},
    {"SoundAmbientRubble", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[19])},
    {"SoundStealthOn", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[20])},
    {"SoundStealthOff", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[21])},
    {"SoundCreated", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[22])},
    {"SoundOnDamaged", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[23])},
    {"SoundOnReallyDamaged", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[24])},
    {"SoundEnter", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[25])},
    {"SoundExit", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[26])},
    {"SoundPromotedVeteran", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[27])},
    {"SoundPromotedElite", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[28])},
    {"SoundPromotedHero", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[29])},
    {"SoundFallingFromPlane", &INI::Parse_Dynamic_Audio_Event_RTS, nullptr, offsetof(ThingTemplate, m_audio.sound[31])},
    {"UnitSpecificSounds", &ThingTemplate::Parse_Per_Unit_Sounds, nullptr, offsetof(ThingTemplate, m_perUnitSounds)},
    {"UnitSpecificFX", &ThingTemplate::Parse_Per_Unit_FX, nullptr, offsetof(ThingTemplate, m_perUnitEffects)},
    {"Scale", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_assetScale)},
    {"Geometry", &GeometryInfo::Parse_Geometry_Type, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryMajorRadius", &GeometryInfo::Parse_Geometry_MajorRadius, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryMinorRadius", &GeometryInfo::Parse_Geometry_MinorRadius, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryHeight", &GeometryInfo::Parse_Geometry_Height, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryIsSmall", &GeometryInfo::Parse_Geometry_IsSmall, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"Shadow", &INI::Parse_Bitstring8, s_theShadowNames, offsetof(ThingTemplate, m_shadowType)},
    {"ShadowSizeX", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_shadowSizeX)},
    {"ShadowSizeY", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_shadowSizeY)},
    {"ShadowOffsetX", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_shadowOffsetX)},
    {"ShadowOffsetY", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_shadowOffsetY)},
    {"ShadowTexture", &INI::Parse_AsciiString, nullptr, offsetof(ThingTemplate, m_shadowTextureName)},
    {"OcclusionDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(ThingTemplate, m_occlusionDelay)},
    {"AddModule", &ThingTemplate::Parse_Add_Module, nullptr, 0},
    {"RemoveModule", &ThingTemplate::Parse_Remove_Module, nullptr, 0},
    {"ReplaceModule", &ThingTemplate::Parse_Replace_Module, nullptr, 0},
    {"InheritableModule", &ThingTemplate::Parse_Inheritable_Module, nullptr, 0},
    {"OverrideableByLikeKind", &ThingTemplate::Parse_Overrideable_By_Like_Kind, nullptr, 0},
    {"Locomotor", &AIUpdateModuleData::Parse_Locomotor_Set, nullptr, 0},
    {"InstanceScaleFuzziness", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_instanceScaleFuzziness)},
    {"StructureRubbleHeight", &INI::Parse_Unsigned_Byte, nullptr, offsetof(ThingTemplate, m_structureRubbleHeight)},
    {"ThreatValue", &INI::Parse_Unsigned_Short, nullptr, offsetof(ThingTemplate, m_threatValue)},
    {"MaxSimultaneousOfType", &ThingTemplate::Parse_Max_Simultaneous, nullptr, offsetof(ThingTemplate, m_maxSimultaneousOfType)},
    {"MaxSimultaneousLinkKey", &NameKeyGenerator::Parse_String_As_NameKeyType, nullptr, offsetof(ThingTemplate, m_maxSimultaneousLinkKey)},
    {"CrusherLevel", &INI::Parse_Unsigned_Byte, nullptr, offsetof(ThingTemplate, m_crusherLevel)},
    {"CrushableLevel", &INI::Parse_Unsigned_Byte, nullptr, offsetof(ThingTemplate, m_crushableLevel)},
    {nullptr, nullptr, nullptr, 0}
};

const FieldParse ThingTemplate::s_objectReskinFieldParseTable[] = 
{
    {"Draw", &ThingTemplate::Parse_Module_Name, reinterpret_cast<const void *>(1), offsetof(ThingTemplate, m_draws)},
    {"Geometry", &GeometryInfo::Parse_Geometry_Type, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryMajorRadius", &GeometryInfo::Parse_Geometry_MajorRadius, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryMinorRadius", &GeometryInfo::Parse_Geometry_MinorRadius, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryHeight", &GeometryInfo::Parse_Geometry_Height, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"GeometryIsSmall", &GeometryInfo::Parse_Geometry_IsSmall, nullptr, offsetof(ThingTemplate, m_geometryInfo)},
    {"FenceWidth", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_fenceWidth)},
    {"FenceXOffset", &INI::Parse_Real, nullptr, offsetof(ThingTemplate, m_fenceXOffset)},
    {"MaxSimultaneousOfType", &ThingTemplate::Parse_Max_Simultaneous, nullptr, offsetof(ThingTemplate, m_maxSimultaneousOfType)},
    {"MaxSimultaneousLinkKey", &NameKeyGenerator::Parse_String_As_NameKeyType, nullptr, offsetof(ThingTemplate, m_maxSimultaneousLinkKey)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

AudioArray::AudioArray()
{
    for (int i = 0; i < THING_SOUNDCOUNT; i++) {
        sound[i] = nullptr;
    }
}

AudioArray::~AudioArray()
{
    for (int i = 0; i < THING_SOUNDCOUNT; i++) {
        sound[i]->Delete_Instance();
    }
}

AudioArray &AudioArray::operator=(const AudioArray &that)
{
    if (this != &that) {
        for (int i = 0; i < THING_SOUNDCOUNT; i++) {
            if (that.sound[i] == nullptr) {
                sound[i] = nullptr;
            } else if (sound[i] != nullptr) {
                *sound[i] = *that.sound[i];
            } else {
                sound[i] = NEW_POOL_OBJ(DynamicAudioEventRTS, *that.sound[i]);
            }
        }
    }

    return *this;
}

ThingTemplate::ThingTemplate() :
    m_geometryInfo(GEOMETRY_SPHERE, false, 1.0f, 1.0f, 1.0f),
    m_kindOf(),
    m_audio(),
    m_body(),
    m_draws(),
    m_clientUpdates(),
    m_prerequisites(),
    m_buildVariations(),
    m_weaponTemplateSets(),
    m_weaponTemplateSetFinder(),
    m_armorTemplateSets(),
    m_armorTemplateSetFinder(),
    m_perUnitSounds(),
    m_perUnitEffects()
{
    m_moduleParseState = 0;
    m_originalSkinTemplate = nullptr;
    m_radarPriority = RADAR_PRIORITY_NOT_ON_RADAR;
    m_nextThingTemplate = nullptr;
    m_transportSlotCount = 0;
    m_fenceWidth = 0.0f;
    m_fenceXOffset = 0.0f;
    m_visionRange = 0.0f;
    m_shroudClearingRange = -1.0f;
    m_shroudRevealToAllRange = -1.0f;
    m_buildCost = 0;
    m_buildTime = 1.0f;
    m_refundValue = 0;
    m_energyProduction = 0;
    m_energyBonus = 0;
    m_buildCompletion = BC_APPEARS_AT_RALLY_POINT;
    for (int i = 0; i < RANK_LEVEL_COUNT; ++i) {
        m_experienceValues[i] = 0;
        m_experienceRequired[i] = 0;
        m_skillPointValues[i] = -999;
    }
    m_isTrainable = false;
    m_enterGuard = false;
    m_hijackGuard = false;
    m_templateID = 0;
    m_kindOf = KINDOFMASK_NONE;
    m_isBuildFacility = false;
    m_isPrerequisite = false;
    m_placementViewAngle = 0.0f;
    m_factoryExitWidth = 0.0f;
    m_factoryExtraBibWidth = 0.0f;
    m_selectedPortraitImage = nullptr;
    m_buttonImage = nullptr;
    m_shadowType = 0;
    m_shadowSizeX = 0.0f;
    m_shadowSizeY = 0.0f;
    m_shadowOffsetX = 0.0f;
    m_shadowOffsetY = 0.0f;
    m_occlusionDelay = g_theWriteableGlobalData->m_defaultOcclusionDelay;
    m_structureRubbleHeight = 0;
    m_instanceScaleFuzziness = 0.0f;
    m_threatValue = 0;
    m_maxSimultaneousOfType = 0;
    m_maxSimultaneousLinkKey = NAMEKEY_INVALID;
    m_determinedBySuperweaponRestriction = false;
    m_crusherLevel = 0;
    m_crushableLevel = 255;

    // #BUGFIX Initialize all members.
    m_assetScale = 1.0f;
    m_displayColor = 0xFFFFFFFF;
    m_isBridge = false;
    m_isForbidden = false;
    m_armorCopiedFromDefault = false;
    m_weaponsCopiedFromDefault = false;
    m_buildable = 0;
    m_editorSorting = 0;
}

void ThingTemplate::Copy_From(const ThingTemplate *that)
{
    if (that) {
        ThingTemplate *t = m_nextThingTemplate;
        unsigned short id = m_templateID;
        Utf8String str(m_nameString);
        *this = *that;
        m_nextThingTemplate = t;
        m_templateID = id;
        m_nameString = str;
    }
}

ThingTemplate &ThingTemplate::operator=(const ThingTemplate &that)
{
    Overridable::operator=(that);
    m_displayName = that.m_displayName;
    m_nameString = that.m_nameString;
    m_defaultOwningSide = that.m_defaultOwningSide;
    m_commandSetString = that.m_commandSetString;
    m_selectedPortraitImageName = that.m_selectedPortraitImageName;
    m_buttonImageName = that.m_buttonImageName;

    for (int i = 0; i < 5; i++) {
        m_upgradeCameoNames[i] = that.m_upgradeCameoNames[i];
    }

    m_shadowTextureName = that.m_shadowTextureName;
    m_replacementModuleName = that.m_replacementModuleName;
    m_replacementModuleTag = that.m_replacementModuleTag;
    m_LTAName = that.m_LTAName;
    m_geometryInfo = that.m_geometryInfo;
    m_kindOf = that.m_kindOf;
    m_audio = that.m_audio;
    m_body = that.m_body;
    m_draws = that.m_draws;
    m_clientUpdates = that.m_clientUpdates;

    for (int i = 0; i < 4; i++) {
        m_skillPointValues[i] = that.m_skillPointValues[i];
    }

    for (int i = 0; i < 4; i++) {
        m_experienceValues[i] = that.m_experienceValues[i];
    }

    for (int i = 0; i < 4; i++) {
        m_experienceRequired[i] = that.m_experienceRequired[i];
    }

    m_prerequisites = that.m_prerequisites;
    m_buildVariations = that.m_buildVariations;
    m_weaponTemplateSets = that.m_weaponTemplateSets;
    m_weaponTemplateSetFinder = that.m_weaponTemplateSetFinder;
    m_armorTemplateSets = that.m_armorTemplateSets;
    m_armorTemplateSetFinder = that.m_armorTemplateSetFinder;
    m_perUnitSounds = that.m_perUnitSounds;
    m_perUnitEffects = that.m_perUnitEffects;
    m_nextThingTemplate = that.m_nextThingTemplate;
    m_originalSkinTemplate = that.m_originalSkinTemplate;
    m_selectedPortraitImage = that.m_selectedPortraitImage;
    m_buttonImage = that.m_buttonImage;
    m_fenceWidth = that.m_fenceWidth;
    m_fenceXOffset = that.m_fenceXOffset;
    m_visionRange = that.m_visionRange;
    m_shroudClearingRange = that.m_shroudClearingRange;
    m_shroudRevealToAllRange = that.m_shroudRevealToAllRange;
    m_placementViewAngle = that.m_placementViewAngle;
    m_factoryExitWidth = that.m_factoryExitWidth;
    m_factoryExtraBibWidth = that.m_factoryExtraBibWidth;
    m_buildTime = that.m_buildTime;
    m_assetScale = that.m_assetScale;
    m_instanceScaleFuzziness = that.m_instanceScaleFuzziness;
    m_shadowSizeX = that.m_shadowSizeX;
    m_shadowSizeY = that.m_shadowSizeY;
    m_shadowOffsetX = that.m_shadowOffsetX;
    m_shadowOffsetY = that.m_shadowOffsetY;
    m_energyProduction = that.m_energyProduction;
    m_energyBonus = that.m_energyBonus;
    m_displayColor = that.m_displayColor;
    m_occlusionDelay = that.m_occlusionDelay;
    m_maxSimultaneousLinkKey = that.m_maxSimultaneousLinkKey;
    m_templateID = that.m_templateID;
    m_buildCost = that.m_buildCost;
    m_refundValue = that.m_refundValue;
    m_threatValue = that.m_threatValue;
    m_maxSimultaneousOfType = that.m_maxSimultaneousOfType;
    m_determinedBySuperweaponRestriction = that.m_determinedBySuperweaponRestriction;
    m_isPrerequisite = that.m_isPrerequisite;
    m_isBridge = that.m_isBridge;
    m_isBuildFacility = that.m_isBuildFacility;
    m_isTrainable = that.m_isTrainable;
    m_enterGuard = that.m_enterGuard;
    m_hijackGuard = that.m_hijackGuard;
    m_isForbidden = that.m_isForbidden;
    m_armorCopiedFromDefault = that.m_armorCopiedFromDefault;
    m_weaponsCopiedFromDefault = that.m_weaponsCopiedFromDefault;
    m_radarPriority = that.m_radarPriority;
    m_transportSlotCount = that.m_transportSlotCount;
    m_buildable = that.m_buildable;
    m_buildCompletion = that.m_buildCompletion;
    m_editorSorting = that.m_editorSorting;
    m_structureRubbleHeight = that.m_structureRubbleHeight;
    m_shadowType = that.m_shadowType;
    m_moduleParseState = that.m_moduleParseState;
    m_crusherLevel = that.m_crusherLevel;
    m_crushableLevel = that.m_crushableLevel;
    return *this;
}

BuildableStatus ThingTemplate::Get_Buildable() const
{
    BuildableStatus status;

    if (g_theGameLogic != nullptr && g_theGameLogic->Find_Buildable_Status_Override(this, status)) {
        return status;
    } else {
        return (BuildableStatus)m_buildable;
    }
}

int ThingTemplate::Get_Max_Simultaneous_Of_Type() const
{
    if (m_determinedBySuperweaponRestriction && g_theGameLogic != nullptr) {
        return g_theGameLogic->Get_Max_Simultaneous_Of_Type();
    } else {
        return m_maxSimultaneousOfType;
    }
}

int ThingTemplate::Get_Skill_Point_Value(int level) const
{
    int points = m_skillPointValues[level];

    if (points == -999) {
        points = Get_Experience_Value(level);
    }

    return points;
}

bool ThingTemplate::Is_Equivalent_To(const ThingTemplate *reskin) const
{
    if (this == nullptr || reskin == nullptr) {
        return false;
    }

    if (this == reskin) {
        return true;
    }

    if (Get_Final_Override() == reskin->Get_Final_Override()) {
        return true;
    }

    if (m_originalSkinTemplate == reskin) {
        return true;
    }

    if (this == reskin->m_originalSkinTemplate) {
        return true;
    }

    if (m_originalSkinTemplate != nullptr && m_originalSkinTemplate == reskin->m_originalSkinTemplate) {
        return true;
    }

    for (unsigned int i = 0; i < m_buildVariations.size(); i++) {
        if (!m_buildVariations[i].Compare_No_Case(reskin->Get_Name())) {
            return true;
        }
    }

    for (unsigned int i = 0; i < reskin->m_buildVariations.size(); i++) {
        if (!reskin->m_buildVariations[i].Compare_No_Case(Get_Name())) {
            return true;
        }
    }

    return false;
}

void ThingTemplate::Set_Copied_From_Default()
{
    m_armorCopiedFromDefault = true;
    m_weaponsCopiedFromDefault = true;
    m_body.Set_Copied_From_Default(true);
    m_draws.Set_Copied_From_Default(true);
    m_clientUpdates.Set_Copied_From_Default(true);
}

void ThingTemplate::Resolve_Names()
{
    for (ProductionPrerequisite &prerequisite : m_prerequisites) {
        prerequisite.Resolve_Names();
    }

    for (ProductionPrerequisite &prerequisite : m_prerequisites) {
        ThingTemplate *tmpls[32];
        int count = prerequisite.Get_All_Possible_Build_Facility_Templates(tmpls, 32);

        for (int j = 0; j < count; j++) {
            if (tmpls[j] != nullptr) {
                tmpls[j]->m_isBuildFacility = true;
            }
        }
    }

    if (Is_KindOf(KINDOF_COMMANDCENTER)) {
        m_isBuildFacility = true;
    }

    if (g_theMappedImageCollection != nullptr) {
        if (m_selectedPortraitImageName.Is_Not_Empty()) {
            m_selectedPortraitImage = g_theMappedImageCollection->Find_Image_By_Name(m_selectedPortraitImageName);
            captainslog_dbgassert(m_selectedPortraitImage != nullptr,
                "%s is looking for Portrait %s but can't find it. Skipping...",
                Get_Name().Str(),
                m_selectedPortraitImageName.Str());
            m_selectedPortraitImageName.Clear();
        }

        if (m_buttonImageName.Is_Not_Empty()) {
            m_buttonImage = g_theMappedImageCollection->Find_Image_By_Name(m_buttonImageName);
            captainslog_dbgassert(m_selectedPortraitImage != nullptr,
                "%s is looking for Portrait %s but can't find it. Skipping...",
                Get_Name().Str(),
                m_buttonImageName.Str());
            m_buttonImageName.Clear();
        }
    }
}

void ThingTemplate::Init_For_LTA(const Utf8String &name)
{
    m_nameString = name;
    char buffer[1024];
    strncpy(buffer, name.Str(), ARRAY_SIZE(buffer));
    int i;

    for (i = 0; buffer[i]; ++i) {
        if (buffer[i] == '/') {
            ++i;
            break;
        }
    }

    m_LTAName = &buffer[i];
    m_body.Clear();
    m_draws.Clear();
    m_clientUpdates.Clear();

    Utf8String tag;
    tag.Format("LTA_%sDestroyDie", m_LTAName.Str());
    m_body.Add_Module_Info(this,
        "DestroyDie",
        tag,
        g_theModuleFactory->New_Module_Data_From_INI(nullptr, "DestroyDie", MODULE_DEFAULT, tag),
        MODULEINTERFACE_DIE,
        false,
        false);

    tag.Format("LTA_%sInactiveBody", m_LTAName.Str());
    m_body.Add_Module_Info(this,
        "InactiveBody",
        tag,
        g_theModuleFactory->New_Module_Data_From_INI(nullptr, "InactiveBody", MODULE_DEFAULT, tag),
        MODULEINTERFACE_BODY,
        false,
        false);

    tag.Format("LTA_%sW3DDefaultDraw", m_LTAName.Str());
    m_body.Add_Module_Info(this,
        "W3DDefaultDraw",
        tag,
        g_theModuleFactory->New_Module_Data_From_INI(nullptr, "W3DDefaultDraw", MODULE_DRAW, tag),
        MODULEINTERFACE_DRAW,
        false,
        false);

    m_armorCopiedFromDefault = false;
    m_weaponsCopiedFromDefault = false;
    m_kindOf = KINDOFMASK_NONE;
    m_assetScale = 1.0f;
    m_instanceScaleFuzziness = 0.0f;
    m_structureRubbleHeight = 0;
    m_displayName.Translate(name);
    m_shadowType = SHADOW_VOLUME;
    m_geometryInfo.Set(GEOMETRY_SPHERE, false, 10.0f, 10.0f, 10.0f);
}

const FieldParse *ThingTemplate::Get_Field_Parse()
{
    return s_objectFieldParseTable;
}

const FieldParse *ThingTemplate::Get_Reskin_Field_Parse()
{
    return s_objectReskinFieldParseTable;
}

void ThingTemplate::Parse_Add_Module(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);
    unsigned char oldState = tmplate->m_moduleParseState;

    if (oldState != MODULEPARSE_NORMAL) {
        throw CODE_06;
    }

    tmplate->m_moduleParseState = MODULEPARSE_MODIFY_ON_COPY;
    ini->Init_From_INI(tmplate, Get_Field_Parse());
    tmplate->m_moduleParseState = oldState;
}

void ThingTemplate::Parse_Armor_Template_Set(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *t = reinterpret_cast<ThingTemplate *>(instance);

    if (t->m_armorCopiedFromDefault) {
        t->m_armorCopiedFromDefault = false;
        t->m_armorTemplateSets.clear();
    }

    ArmorTemplateSet set;
    set.Parse_Armor_Template_Set(ini);

    if (ini->Get_Load_Type() != INI_LOAD_CREATE_OVERRIDES) {
        for (ArmorTemplateSet &i : t->m_armorTemplateSets) {
            captainslog_dbgassert(!(i.Get_Conditions_Yes(0) == set.Get_Conditions_Yes(0)),
                "dup armorset condition in %s",
                t->Get_Name().Str());
        }
    }

    t->m_armorTemplateSets.push_back(set);
    t->m_armorTemplateSetFinder.Clear();
}

void ThingTemplate::Parse_Inheritable_Module(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);
    unsigned char oldState = tmplate->m_moduleParseState;

    if (oldState != MODULEPARSE_NORMAL) {
        throw CODE_06;
    }

    tmplate->m_moduleParseState = MODULEPARSE_KEEP_ON_COPY;
    ini->Init_From_INI(tmplate, Get_Field_Parse());
    tmplate->m_moduleParseState = oldState;
}

void ThingTemplate::Parse_Int_List(INI *ini, void *formal, void *store, const void *user_data)
{
    for (int i = 0; i < reinterpret_cast<intptr_t>(user_data); i++) {
        static_cast<int *>(store)[i] = INI::Scan_Int(ini->Get_Next_Token());
    }
}

void ThingTemplate::Parse_Max_Simultaneous(INI *ini, void *instance, void *store, const void *user_data)
{
    char str[36];
    strcpy(str, "DeterminedBySuperweaponRestriction");
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);

    captainslog_dbgassert(&tmplate->m_maxSimultaneousOfType == store, "Bad store passed to parseMaxSimultaneous");

    const char *str2 = ini->Get_Next_Token();

    if (!strcasecmp(str2, str)) {
        tmplate->m_determinedBySuperweaponRestriction = true;
        *static_cast<uint16_t *>(store) = 0;
    } else {
        int tmp = ini->Scan_Int(str2);

        captainslog_relassert(tmp >= 0 && tmp < 65535, 0xDEAD0001, "Value parsed outside range of a short.");

        *static_cast<uint16_t *>(store) = tmp;
        tmplate->m_determinedBySuperweaponRestriction = false;
    }
}

bool ThingTemplate::Remove_Module_Info(const Utf8String &tag, Utf8String &name)
{
    bool ret = false;

    if (m_body.Clear_Module_Data_With_Tag(tag, name)) {
        ret = true;
    }

    if (m_draws.Clear_Module_Data_With_Tag(tag, name)) {
        captainslog_dbgassert(!ret, "Hmm, multiple removed in ThingTemplate::removeModuleInfo, should this be possible?");
        ret = true;
    }

    if (m_clientUpdates.Clear_Module_Data_With_Tag(tag, name)) {
        captainslog_dbgassert(!ret, "Hmm, multiple removed in ThingTemplate::removeModuleInfo, should this be possible?");
        ret = true;
    }

    return ret;
}

void ThingTemplate::Parse_Module_Name(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);
    ModuleInfo *info = static_cast<ModuleInfo *>(store);
    int data = reinterpret_cast<intptr_t>(user_data);
    Utf8String name = ini->Get_Next_Token();
    Utf8String tag_name = ini->Get_Next_Token();

    captainslog_relassert(tag_name.Is_Not_Empty(),
        CODE_06,
        "[LINE: %d - FILE: '%s'] Module tag not found for module '%s' on thing template '%s'.  Module tags are required and "
        "must be unique for all modules within an object definition",
        ini->Get_Line_Number(),
        ini->Get_Filename().Str(),
        name.Str(),
        tmplate->Get_Name().Str());

    int mask;

    if (data == 999) {
        data = 0;
        mask = g_theModuleFactory->Find_Module_Interface_Mask(name, MODULE_DEFAULT);
        captainslog_relassert((mask & MODULEINTERFACE_BODY) != 0, CODE_06, "Only Body allowed here");
    } else {
        mask = g_theModuleFactory->Find_Module_Interface_Mask(name, (ModuleType)data);
        captainslog_relassert((mask & MODULEINTERFACE_BODY) == 0, CODE_06, "No Body allowed here");
    }

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        captainslog_relassert(tmplate->m_moduleParseState == MODULEPARSE_MODIFY_ON_COPY,
            CODE_06,
            "[LINE: %d - FILE: '%s'] You must use AddModule to add modules in override INI files.",
            ini->Get_Line_Number(),
            ini->Get_Filename().Str());
    } else {
        tmplate->m_body.Clear_Copied_From_Default_Entries(mask, name, tmplate);
        tmplate->m_draws.Clear_Copied_From_Default_Entries(mask, name, tmplate);
        tmplate->m_clientUpdates.Clear_Copied_From_Default_Entries(mask, name, tmplate);
    }

    captainslog_relassert(tmplate->m_moduleParseState != MODULEPARSE_MODIFY_ON_COPY
            || !tmplate->m_replacementModuleName.Is_Not_Empty() || tmplate->m_replacementModuleName == name,
        CODE_06,
        "[LINE: %d - FILE: '%s'] ReplaceModule must replace modules with another module of the same type, but you are "
        "attempting to replace a %s with a %s for Object %s.",
        ini->Get_Line_Number(),
        ini->Get_Filename().Str(),
        tmplate->m_replacementModuleName.Str(),
        name.Str(),
        tmplate->Get_Name().Str());

    captainslog_relassert(tmplate->m_moduleParseState != MODULEPARSE_MODIFY_ON_COPY
            || !tmplate->m_replacementModuleTag.Is_Not_Empty() || tmplate->m_replacementModuleTag != tag_name,
        CODE_06,
        "[LINE: %d - FILE: '%s'] ReplaceModule must specify a new, unique tag for the replaced module, but you are "
        "attempting to replace a %s with a %s for Object %s.",
        ini->Get_Line_Number(),
        ini->Get_Filename().Str(),
        tmplate->m_replacementModuleName.Str(),
        name.Str(),
        tmplate->Get_Name().Str());

    ModuleData *m = g_theModuleFactory->New_Module_Data_From_INI(ini, name, (ModuleType)data, tag_name);

    if (m->Is_AI_Module_Data()) {
        info->Clear_Ai_Module_Info();
    }

    info->Add_Module_Info(tmplate,
        name,
        tag_name,
        m,
        mask,
        tmplate->m_moduleParseState == MODULEPARSE_KEEP_ON_COPY,
        tmplate->m_moduleParseState == MODULEPARSE_REPLACE_BY_LIKE_KIND);
}

void ThingTemplate::Parse_Per_Unit_FX(INI *ini, void *instance, void *store, const void *user_data)
{
    std::map<Utf8String, FXList *> *map = static_cast<std::map<Utf8String, FXList *> *>(store);
    map->clear();

    // clang-format off
    static const FieldParse myFieldParse[] = {
        { nullptr, &Parse_Arbitrary_FX_Into_Map, nullptr, 0},
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ini->Init_From_INI(map, myFieldParse);
}

void ThingTemplate::Parse_Arbitrary_FX_Into_Map(INI *ini, void *instance, void *store, const void *user_data)
{
    std::map<Utf8String, FXList *> *map = static_cast<std::map<Utf8String, FXList *> *>(instance);
    const char *str = ini->Get_Next_Token();
    FXList *list = g_theFXListStore->Find_FXList(str);
    captainslog_dbgassert(list != nullptr || strcasecmp(str, "None") != 0, "FXList %s not found!", str);
    Utf8String s(static_cast<const char *>(user_data));
    std::pair<Utf8String, FXList *> pair(s, list);
    map->insert(pair);
}

void ThingTemplate::Parse_Per_Unit_Sounds(INI *ini, void *instance, void *store, const void *user_data)
{
    std::map<Utf8String, AudioEventRTS> *map = static_cast<std::map<Utf8String, AudioEventRTS> *>(store);
    map->clear();

    // clang-format off
    static const FieldParse myFieldParse[] = {
        { nullptr, &Parse_Arbitrary_Sounds_Into_Map, nullptr, 0},
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ini->Init_From_INI(map, myFieldParse);
}

void ThingTemplate::Parse_Arbitrary_Sounds_Into_Map(INI *ini, void *instance, void *store, const void *user_data)
{
    std::map<Utf8String, AudioEventRTS> *map = static_cast<std::map<Utf8String, AudioEventRTS> *>(instance);
    AudioEventRTS sound;
    const char *str = ini->Get_Next_Token();

    if (str != nullptr) {
        sound.Set_Event_Name(str);
    }

    Utf8String s(static_cast<const char *>(user_data));
    std::pair<Utf8String, AudioEventRTS> pair(s, sound);
    map->insert(pair);
}

void ThingTemplate::Parse_Prerequisites(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *thing = static_cast<ThingTemplate *>(instance);

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        thing->m_prerequisites.clear();
    }

    // clang-format off
    static const FieldParse myFieldParse[] = {
        { "Object", &Parse_Prerequisite_Unit, nullptr, 0},
        { "Science", &Parse_Prerequisite_Science, nullptr, 0},
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ini->Init_From_INI(&thing->m_prerequisites, myFieldParse);
}

void ThingTemplate::Parse_Prerequisite_Unit(INI *ini, void *instance, void *store, const void *user_data)
{
    std::vector<ProductionPrerequisite> *vec = static_cast<std::vector<ProductionPrerequisite> *>(instance);
    ProductionPrerequisite prereq;
    bool or_with_previous = false;

    for (const char *str = ini->Get_Next_Token(); str != nullptr; str = ini->Get_Next_Token_Or_Null()) {
        prereq.Add_Unit_Prereq(str, or_with_previous);
        or_with_previous = true;
    }
    vec->push_back(prereq);
}

void ThingTemplate::Parse_Prerequisite_Science(INI *ini, void *instance, void *store, const void *user_data)
{
    std::vector<ProductionPrerequisite> *vec = static_cast<std::vector<ProductionPrerequisite> *>(instance);
    ProductionPrerequisite prereq;
    ScienceType science = ini->Scan_Science(ini->Get_Next_Token());
    prereq.Add_Science_Prereq(science);
    vec->push_back(prereq);
}

void ThingTemplate::Parse_Remove_Module(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);
    unsigned char oldState = tmplate->m_moduleParseState;

    if (oldState != MODULEPARSE_NORMAL) {
        throw CODE_06;
    }

    tmplate->m_moduleParseState = MODULEPARSE_MODIFY_ON_COPY;
    const char *tag = ini->Get_Next_Token();
    Utf8String name;
    bool remove = tmplate->Remove_Module_Info(tag, name);

    captainslog_relassert(
        remove, CODE_06, "RemoveModule %s was not found for %s. The game will crash now!", tag, tmplate->Get_Name().Str());

    tmplate->m_moduleParseState = oldState;
}

void ThingTemplate::Parse_Replace_Module(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);
    unsigned char oldState = tmplate->m_moduleParseState;

    if (oldState != MODULEPARSE_NORMAL) {
        throw CODE_06;
    }

    tmplate->m_moduleParseState = MODULEPARSE_MODIFY_ON_COPY;
    const char *tag = ini->Get_Next_Token();
    Utf8String name;
    bool remove = tmplate->Remove_Module_Info(tag, name);

    captainslog_relassert(remove,
        CODE_06,
        "[LINE: %d - FILE: '%s'] ReplaceModule %s was not found for %s; cannot continue.",
        ini->Get_Line_Number(),
        ini->Get_Filename().Str(),
        tag,
        tmplate->Get_Name().Str());

    tmplate->m_replacementModuleName = name;
    tmplate->m_replacementModuleTag = Utf8String(tag);
    ini->Init_From_INI(tmplate, Get_Field_Parse());
    tmplate->m_replacementModuleName.Clear();
    tmplate->m_replacementModuleTag.Clear();
    tmplate->m_moduleParseState = oldState;
}

void ThingTemplate::Parse_Weapon_Template_Set(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *t = reinterpret_cast<ThingTemplate *>(instance);

    if (t->m_weaponsCopiedFromDefault) {
        t->m_weaponsCopiedFromDefault = false;
        t->m_weaponTemplateSets.clear();
    }

    WeaponTemplateSet set;
    set.Parse_Weapon_Template_Set(ini, t);

    if (ini->Get_Load_Type() != INI_LOAD_CREATE_OVERRIDES) {
        for (WeaponTemplateSet &i : t->m_weaponTemplateSets) {
            captainslog_dbgassert(!(i.Get_Conditions_Yes(0) == set.Get_Conditions_Yes(0)),
                "dup weaponset condition in %s",
                t->Get_Name().Str());
        }
    }

    t->m_weaponTemplateSets.push_back(set);
    t->m_weaponTemplateSetFinder.Clear();
}

void ThingTemplate::Parse_Overrideable_By_Like_Kind(INI *ini, void *instance, void *store, const void *user_data)
{
    ThingTemplate *tmplate = static_cast<ThingTemplate *>(instance);
    unsigned char oldState = tmplate->m_moduleParseState;

    if (oldState != MODULEPARSE_NORMAL) {
        throw CODE_06;
    }

    tmplate->m_moduleParseState = MODULEPARSE_REPLACE_BY_LIKE_KIND;
    ini->Init_From_INI(tmplate, Get_Field_Parse());
    tmplate->m_moduleParseState = oldState;
}

bool ThingTemplate::Can_Possibly_Have_Any_Weapon() const
{
    for (auto &set : m_weaponTemplateSets) {
        if (set.Has_Any_Weapons()) {
            return true;
        }
    }

    return false;
}

AIUpdateModuleData *ThingTemplate::Friend_Get_AI_Module_Info() const
{
    int count = m_body.Get_Count();

    for (int i = 0; i < count; i++) {
        if (m_body.Get_Nth_Data(i)) {
            const ModuleData *data = m_body.Get_Nth_Data(i);
            if (data->Is_AI_Module_Data()) {
                return static_cast<AIUpdateModuleData *>(m_body.Friend_Get_Nth_Data(i));
            }
        }
    }

    return nullptr;
}

void ThingTemplate::Validate()
{
    if (m_shadowTextureName.Is_Empty()) {
        GeometryType type = Get_Template_Geometry_Info().Get_Type();

        if (type >= GEOMETRY_SPHERE) {
            if (type <= GEOMETRY_CYLINDER) {
                m_shadowTextureName = "shadow";
            } else if (type == GEOMETRY_BOX) {
                m_shadowTextureName = "shadows";
            }
        }
    }

    Validate_Audio();

    if (Get_Name() != "DefaultThingTemplate") {
        if (Get_Name() != "NeutronMissile") {
            if (Get_Name() != "FlamethrowerProjectileStream" && !Is_KindOf(KINDOF_DRAWABLE_ONLY)
                && m_buildVariations.empty()) {
                bool immobile = Is_KindOf(KINDOF_IMMOBILE);
                captainslog_dbgassert(
                    !Is_KindOf(KINDOF_SHRUBBERY) || immobile, "SHRUBBERY %s must be marked IMMOBILE!", Get_Name().Str());
                captainslog_dbgassert(!Is_KindOf(KINDOF_STRUCTURE) || immobile,
                    "Structure %s is not marked immobile, but probably should be -- please fix it. (If we ever add mobile "
                    "structures, this debug sniffer will need to be revised.)",
                    Get_Name().Str());
                captainslog_dbgassert(!Is_KindOf(KINDOF_STICK_TO_TERRAIN_SLOPE) || immobile,
                    "item %s is marked STICK_TO_TERRAIN_SLOPE but not IMMOBILE -- please fix it.",
                    Get_Name().Str());

                if (Is_KindOf(KINDOF_STRUCTURE)) {
                    if (m_armorTemplateSets.empty()
                        || (m_armorTemplateSets.size() == 1 && m_armorTemplateSets[0].Get_Armor_Template() == nullptr)) {
                        captainslog_debug(
                            "Structure %s has no armor, but probably should (StructureArmor) -- please fix it.)",
                            Get_Name().Str());
                    }

                    for (auto set : m_armorTemplateSets) {
                        captainslog_dbgassert(set.Get_Damage_FX() != nullptr,
                            "Structure %s has no ArmorDamageFX, and really should.",
                            Get_Name().Str());
                    }
                }
            }
        }
    }
}

const ArmorTemplateSet *ThingTemplate::Find_Armor_Template_Set(const BitFlags<ARMORSET_COUNT> &t) const
{
    return m_armorTemplateSetFinder.Find_Best_Info(m_armorTemplateSets, t);
}

const WeaponTemplateSet *ThingTemplate::Find_Weapon_Template_Set(const BitFlags<WEAPONSET_COUNT> &t) const
{
    return m_weaponTemplateSetFinder.Find_Best_Info(m_weaponTemplateSets, t);
}

const AudioEventRTS *ThingTemplate::Get_Per_Unit_Sound(const Utf8String &soundname) const
{
    if (soundname.Is_Empty()) {
        return &s_audioEventNoSound;
    }

    auto iter = m_perUnitSounds.find(soundname);

    if (iter != m_perUnitSounds.end()) {
        return &iter->second;
    }

    captainslog_debug("Unknown Audio name (%s) asked for in ThingTemplate (%s).", soundname.Str(), m_nameString.Str());
    return &s_audioEventNoSound;
}

const FXList *ThingTemplate::Get_Per_Unit_FX(const Utf8String &fxname) const
{
    if (fxname.Is_Empty()) {
        return nullptr;
    }

    auto iter = m_perUnitEffects.find(fxname);

    if (iter != m_perUnitEffects.end()) {
        return iter->second;
    }

    captainslog_debug("Unknown FX name (%s) asked for in ThingTemplate (%s). ", m_nameString.Str(), fxname.Str());
    return nullptr;
}

void ThingTemplate::Validate_Sound(const AudioEventRTS *event, const char *name)
{
    if (!event->Get_Event_Name().Is_Empty()) {
        if (event->Get_Event_Name().Compare_No_Case("NoSound")) {
            if (!g_theAudio->Is_Valid_Audio_Event(event)) {
                captainslog_debug(
                    "Invalid Sound '%s' in Object '%s'. (%s?)", name, Get_Name().Str(), event->Get_Event_Name().Str());
            }
        }
    }
}

void ThingTemplate::Validate_Audio()
{
    Validate_Sound(Get_Voice_Select(), "VoiceSelect");
    Validate_Sound(Get_Voice_Group_Select(), "VoiceGroupSelect");
    Validate_Sound(Get_Voice_Move(), "VoiceMove");
    Validate_Sound(Get_Voice_Attack(), "VoiceAttack");
    Validate_Sound(Get_Voice_Enter(), "VoiceEnter");
    Validate_Sound(Get_Voice_Fear(), "VoiceFear");
    Validate_Sound(Get_Voice_Select_Elite(), "VoiceSelectElite");
    Validate_Sound(Get_Voice_Created(), "VoiceCreated");
    Validate_Sound(Get_Voice_Near_Enemy(), "VoiceNearEnemy");
    Validate_Sound(Get_Voice_Task_Unable(), "VoiceTaskUnable");
    Validate_Sound(Get_Voice_Task_Complete(), "VoiceTaskComplete");
    Validate_Sound(Get_Voice_Meet_Enemy(), "VoiceMeetEnemy");
    Validate_Sound(Get_Voice_Garrison(), "VoiceGarrison");
    Validate_Sound(Get_Voice_Defect(), "VoiceDefect");
    Validate_Sound(Get_Voice_Attack_Special(), "VoiceAttackSpecial");
    Validate_Sound(Get_Voice_Attack_Air(), "VoiceAttackAir");
    Validate_Sound(Get_Sound_Move_Start(), "SoundMoveStart");
    Validate_Sound(Get_Sound_Move_Start_Damaged(), "SoundMoveStartDamaged");
    Validate_Sound(Get_Sound_Move_Loop(), "SoundMoveLoop");
    Validate_Sound(Get_Sound_Ambient(), "SoundAmbient");
    Validate_Sound(Get_Sound_Ambient_Damaged(), "SoundAmbientDamaged");
    Validate_Sound(Get_Sound_Ambient_Really_Damaged(), "SoundAmbientReallyDamaged");
    Validate_Sound(Get_Sound_Ambient_Rubble(), "SoundAmbientRubble");
    Validate_Sound(Get_Sound_Stealth_On(), "SoundStealthOn");
    Validate_Sound(Get_Sound_Stealth_Off(), "SoundStealthOff");
    Validate_Sound(Get_Sound_Created(), "SoundCreated");
    Validate_Sound(Get_Sound_On_Damaged(), "SoundOnDamaged");
    Validate_Sound(Get_Sound_On_Really_Damaged(), "SoundOnReallyDamaged");
    Validate_Sound(Get_Sound_Enter(), "SoundEnter");
    Validate_Sound(Get_Sound_Exit(), "SoundExit");
    Validate_Sound(Get_Sound_Promoted_Veteran(), "SoundPromotedVeteran");
    Validate_Sound(Get_Sound_Promoted_Elite(), "SoundPromotedElite");
    Validate_Sound(Get_Sound_Promoted_Hero(), "SoundPromotedHero");

    const std::
        map<Utf8String, AudioEventRTS, std::less<Utf8String>, std::allocator<std::pair<Utf8String const, AudioEventRTS>>>
            *sounds = Get_All_Per_Unit_Sounds();

    if (sounds != nullptr) {
        for (auto sound : *sounds) {
            Validate_Sound(&sound.second, sound.first.Str());
        }
    }
}

int ThingTemplate::Calc_Cost_To_Build(const Player *player) const
{
    if (player != nullptr) {
        float cost = player->Get_Production_Cost_Change_Percent(Get_Name()) + 1.0f;
        cost *= player->Get_Production_Cost_Change_Based_On_Kind_Of(m_kindOf);
        cost *= Get_Build_Cost();
        return player->Get_Handicap()->Get_Handicap(Handicap::BUILDCOST, this) * cost;
    } else {
        return 0;
    }
}

int ThingTemplate::Calc_Time_To_Build(const Player *player) const
{
    int frame_build_time = Get_Build_Time() * 30.0f;
    int time = player->Get_Handicap()->Get_Handicap(Handicap::BUILDTIME, this) * frame_build_time;
    time *= player->Get_Production_Time_Change_Percent(Get_Name()) + 1.0f;

#ifdef GAME_DEBUG_STRUCTS
    if (player->Is_Instant_Build()) {
        time = 1;
    }
#endif

    float ratio = player->Get_Energy()->Get_Energy_Supply_Ratio();

    if (ratio > 1.0f) {
        ratio = 1.0f;
    }

    float speed = 1.0f - ratio;
    speed *= g_theWriteableGlobalData->m_lowEnergyPenaltyModifier;
    speed = 1.0f - speed;
    speed = std::max(speed, g_theWriteableGlobalData->m_minLowEnergyProductionSpeed);

    if (ratio < 1.0f) {
        speed = std::min(speed, g_theWriteableGlobalData->m_maxLowEnergyProductionSpeed);
    }

    if (speed <= 0.0f) {
        speed = 0.01f;
    }

    time /= speed;

    if (Get_Build_Completion() == BC_APPEARS_AT_RALLY_POINT) {
        ThingTemplate *facility = Get_Build_Facility_Template(player);
        int count = 0;

        if (facility != nullptr) {
            player->Count_Objects_By_Thing_Template(1, &facility, false, &count, true);
            float mult = g_theWriteableGlobalData->m_multipleFactory;

            if (mult > 0.0f) {
                for (int i = 0; i < count - 1; i++) {
                    time *= mult;
                }
            }
        }
    }

    return time;
}

ThingTemplate *ThingTemplate::Get_Build_Facility_Template(const Player *player) const
{
    if (Get_Prereq_Count() <= 0) {
        return nullptr;
    }

    return m_prerequisites[0].Get_Existing_Build_Facility_Template(player);
}
