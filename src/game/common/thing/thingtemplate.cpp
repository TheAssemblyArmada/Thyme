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
#include "audioeventrts.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "productionprerequisite.h"

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
                sound[i] = that.sound[i];
            } else {
                sound[i] = NEW_POOL_OBJ(DynamicAudioEventRTS, *that.sound[i]);
            }
        }
    }

    return *this;
}

ThingTemplate::ThingTemplate() :
    m_geometryInfo(GEOMETRY_SPHERE, false, 1.0f, 1.0f, 1.0f),
    m_moduleParseState(0),
    m_originalSkinTemplate(nullptr),
    m_radarPriority(RADAR_PRIORITY_NOT_ON_RADAR),
    m_nextThingTemplate(nullptr),
    m_transportSlotCount(0),
    m_fenceWidth(0.0f),
    m_fenceXOffset(0.0f),
    m_visionRange(0.0f),
    m_shroudClearingRange(-1.0f),
    m_shroudRevealToAllRange(-1.0f),
    m_buildCost(0),
    m_buildTime(1.0f),
    m_refundValue(0),
    m_energyProduction(0),
    m_energyBonus(0),
    m_buildCompletion(BC_APPEARS_AT_RALLY_POINT),
    m_experienceValues{ 0, 0, 0, 0 },
    m_experienceRequired{ 0, 0, 0, 0 },
    m_skillPointValues{ -999, -999, -999, -999 },
    m_isTrainable(false),
    m_enterGuard(false),
    m_hijackGuard(false),
    m_templateID(0),
    m_kindOf(KINDOFMASK_NONE),
    m_isBuildFacility(false),
    m_isPrerequisite(false),
    m_placementViewAngle(0.0f),
    m_factoryExitWidth(0.0f),
    m_factoryExtraBibWidth(0.0f),
    m_selectedPortraitImage(nullptr),
    m_buttonImage(nullptr),
    m_shadowType(0),
    m_shadowSizeX(0.0f),
    m_shadowSizeY(0.0f),
    m_shadowOffsetX(0.0f),
    m_shadowOffsetY(0.0f),
    m_occlusionDelay(g_theWriteableGlobalData->m_defaultOcclusionDelay),
    m_structureRubbleHeight(0),
    m_instanceScaleFuzziness(0.0f),
    m_threatValue(0),
    m_maxSimultaneousOfType(0),
    m_maxSimultaneousLinkKey(NAMEKEY_INVALID),
    m_determinedBySuperweaponRestriction(false),
    m_crusherLevel(0),
    m_crushableLevel(255)
{
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
    m_moduleInfo[0] = that.m_moduleInfo[0];
    m_moduleInfo[1] = that.m_moduleInfo[1];
    m_moduleInfo[2] = that.m_moduleInfo[2];

    for (int i = 0; i < 4; i++) {
        m_skillPointValues[i] = that.m_skillPointValues[i];
    }

    for (int i = 0; i < 4; i++) {
        m_experienceValues[i] = that.m_experienceValues[i];
    }

    for (int i = 0; i < 4; i++) {
        m_experienceRequired[i] = that.m_experienceRequired[i];
    }

    m_buildVariations = that.m_buildVariations;
    m_weaponTemplateSetFinder = that.m_weaponTemplateSetFinder;
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

unsigned short ThingTemplate::Get_Max_Simultaneous_Of_Type() const
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

    if (this->Get_Final_Override() == reskin->Get_Final_Override()) {
        return true;
    }

    if (this->m_originalSkinTemplate == reskin) {
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
    m_moduleInfo[0].Set_Copied_From_Default(true);
    m_moduleInfo[1].Set_Copied_From_Default(true);
    m_moduleInfo[2].Set_Copied_From_Default(true);
}

void ThingTemplate::Resolve_Names()
{
    // todo needs more of ProductionPrerequisite and ImageCollection
#ifdef GAME_DLL
    Call_Method<void, ThingTemplate>(PICK_ADDRESS(0x0058B870, 0x006ECF80), this);
#endif
}

void ThingTemplate::Init_For_LTA(const Utf8String &name)
{
    // todo needs more of ModuleFactory and Module
#ifdef GAME_DLL
    Call_Method<void, ThingTemplate>(PICK_ADDRESS(0x0058BA00, 0x006ED235), this);
#endif
}
