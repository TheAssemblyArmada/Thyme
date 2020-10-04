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
#pragma once

#ifndef THINGTEMPLATE_H
#define THINGTEMPLATE_H

#include "always.h"
#include "armortemplateset.h"
#include "asciistring.h"
#include "audioeventrts.h"
#include "geometry.h"
#include "kindof.h"
#include "moduleinfo.h"
#include "overridable.h"
#include "productionprerequisite.h"
#include "sparsematchfinder.h"
#include "unicodestring.h"
#include "weapontemplateset.h"
#include <map>
#include <vector>

class FXList;

class ThingTemplate : public Overridable
{
    IMPLEMENT_POOL(ThingTemplate); // Implements standard functions for memory pool objects.

    enum
    {
        THING_VOICESELECT,
        THING_VOICEGROUPSELECT,
        THING_VOICESELECTELITE,
        THING_VOICEMOVE,
        THING_VOICEATTACK,
        THING_VOICEENTER,
        THING_VOICEFEAR,
        THING_VOICECREATED,
        THING_UNK,
        THING_VOICETASKUNABLE,
        THING_VOICETASKCOMPLETE,
        THING_VOICEMEETENEMY,
        THING_SOUNDMOVESTART,
        THING_SOUNDMOVESTARTDAMAGED,
        THING_SOUNDMOVELOOP,
        THING_SOUNDMOVELOOPDAMAGED,
        THING_SOUNDAMBIENT,
        THING_SOUNDAMBIENTDAMAGED,
        THING_SOUNDAMBIENTREALLYDAMAGED,
        THING_SOUNDAMBIENTRUBBLE,
        THING_SOUNDSTEALTHON,
        THING_SOUNDSTEALTHOFF,
        THING_SOUNDCREATED,
        THING_SOUNDONDAMAGED,
        THING_SOUNDONREALLYDAMAGED,
        THING_SOUNDENTER,
        THING_SOUNDEXIT,
        THING_SOUNDPROMOTEDVETERAN,
        THING_SOUNDPROMOTEDELITE,
        THING_SOUNDPROMOTEDHERO,
        THING_VOICEGARRISON,
        THING_SOUNDFALLINGFROMPLANE,
        THING_VOICEDEFECT,
        THING_VOICEATTACKSPECIAL,
        THING_VOICEATTACKGROUND,
        THING_VOICEGUARD,
        THING_SOUNDCOUNT,
    };

public:
    ThingTemplate();
    virtual ~ThingTemplate() {}

    bool Is_KindOf(KindOfType type) const { return m_kindOf.Get(type); }

private:
    Utf16String m_displayName;
    Utf8String m_unkAsciiString1;
    Utf8String m_side;
    Utf8String m_commandSet;
    Utf8String m_selectPortrait;
    Utf8String m_buttonImage;
    Utf8String m_upgradeCameo[5];
    Utf8String m_shadowTexture;
    Utf8String m_unkAsciiString2;
    Utf8String m_unkAsciiString3;
    Utf8String m_unkAsciiString4;
    GeometryInfo m_geometryInfo;
    // int m_kindOf[4];
    BitFlags<KINDOF_COUNT> m_kindOf;
    AudioEventRTS *m_standardSounds[THING_SOUNDCOUNT];
    ModuleInfo m_unkModule; // TODO
    ModuleInfo m_draw;
    ModuleInfo m_clientUpdate;
    int m_skillPointValue[4];
    int m_experienceValue[4];
    int m_experienceRequired[4];
    std::vector<ProductionPrerequisite> m_prerequisites;
    std::vector<Utf8String> m_buildVariations;
    std::vector<WeaponTemplateSet> m_weaponTemplateSets;
    SparseMatchFinder<BitFlags<WEAPONSET_COUNT>, WeaponTemplateSet> m_weaponTemplateSetFinder;
    std::vector<ArmorTemplateSet> m_armorTemplateSets;
    SparseMatchFinder<BitFlags<ARMORSET_COUNT>, ArmorTemplateSet> m_armorTemplateSetFinder;
    std::map<Utf8String, AudioEventRTS> m_perUnitSounds;
    std::map<Utf8String, FXList *> m_perUnitEffects;
    int m_unkInt1;
    int m_unkInt2;
    int m_unkInt3;
    int m_unkInt4;
    int m_fenceWidth;
    int m_fenceXOffset;
    int m_visionRange;
    int m_shroudClearingRange;
    int m_shroudRevealToAllRange;
    int m_placementViewAngle;
    int m_factoryExitWidth;
    int m_factoryExtraBibWidth;
    int m_buildTime;
    int m_assetScale;
    int m_instanceScaleFuzziness;
    int m_shadowSizeX;
    int m_shadowSizeY;
    int m_shadowOffsetX;
    int m_shadowOffsetY;
    int m_energyProduction;
    int m_energyBonus;
    int m_displayColor;
    int m_occlusionDelay;
    int m_axSimultaneousLinkKey;
    int16_t m_unkShort1;
    int16_t m_buildCost;
    int16_t m_refundValue;
    int16_t m_threatValue;
    int16_t m_maxSimultaneousOfType;
    uint8_t m_getMaxSimultaneousOfType256;
    bool m_isPrerequisite;
    bool m_isBridge;
    bool m_unkBool1;
    bool m_isTrainable;
    bool m_enterGuard;
    bool m_hijackGuard;
    bool m_isForbidden;
    bool m_armorSetLoaded;
    bool m_weaponSetLoaded;
    bool m_radarPriority;
    bool m_transportSlotCount;
    bool m_buildable;
    bool m_buildCompletion;
    bool m_editorSorting;
    bool m_structureRubbleHeight;
    bool m_shadow;
    bool m_unkBool4;
    bool m_crusherLevel;
    bool m_crushableLevel;
};

#endif // THINGTEMPLATE_H
