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
class DynamicAudioEventRTS;
class Image;

class AIUpdateModuleData;
class Player;

enum NameKeyType : int32_t;

enum RadarPriorityType
{
    RADAR_PRIORITY_NOT_ON_RADAR,
    RADAR_PRIORITY_STRUCTURE,
    RADAR_PRIORITY_UNIT,
    RADAR_PRIORITY_BOMB,
    RADAR_PRIORITY_NUM_PRIORITIES,
};

enum EditorSortingType
{
    ES_FIRST = 0,
    ES_NONE = 0,
    ES_STRUCTURE = 1,
    ES_INFANTRY = 2,
    ES_VEHICLE = 3,
    ES_SHRUBBERY = 4,
    ES_MISC_MAN_MADE = 5,
    ES_MISC_NATURAL = 6,
    ES_DEBRIS = 7,
    ES_SYSTEM = 8,
    ES_AUDIO = 9,
    ES_TEST = 10,
    ES_FOR_REVIEW = 11,
    ES_ROAD = 12,
    ES_WAYPOINT = 13,
    ES_NUM_SORTING_TYPES = 14,
};

enum BuildCompletionType
{
    BC_INVALID = 0,
    BC_APPEARS_AT_RALLY_POINT = 1,
    BC_PLACED_BY_PLAYER = 2,
    BC_NUM_TYPES = 3,
};

enum ThingTemplateAudioType
{
    THING_VOICESELECT,
    THING_VOICEGROUPSELECT,
    THING_VOICESELECTELITE,
    THING_VOICEMOVE,
    THING_VOICEATTACK,
    THING_VOICEENTER,
    THING_VOICEFEAR,
    THING_VOICECREATED,
    THING_VOICENEARENEMY,
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
    THING_VOICEATTACKAIR,
    THING_VOICEGUARD,
    THING_SOUNDCOUNT,
};

struct AudioArray
{
    DynamicAudioEventRTS *sound[THING_SOUNDCOUNT];
    AudioArray();
    ~AudioArray();
    AudioArray &operator=(const AudioArray &that);
};

class ThingTemplate : public Overridable
{
    IMPLEMENT_NAMED_POOL(ThingTemplate, ThingTemplatePool);

    enum
    {
        RANK_LEVEL_COUNT = 4
    };

protected:
    virtual ~ThingTemplate() override {}

public:
    // Not 100% clear on these values
    enum ModuleParseState
    {
        MODULEPARSE_NORMAL,
        MODULEPARSE_MODIFY_ON_COPY,
        MODULEPARSE_KEEP_ON_COPY,
        MODULEPARSE_REPLACE_BY_LIKE_KIND,
    };

    ThingTemplate();
    ThingTemplate &operator=(const ThingTemplate &that);

    static void Parse_Add_Module(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Armor_Template_Set(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Inheritable_Module(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Int_List(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Max_Simultaneous(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Module_Name(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Per_Unit_FX(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Arbitrary_FX_Into_Map(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Per_Unit_Sounds(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Arbitrary_Sounds_Into_Map(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Prerequisites(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Prerequisite_Unit(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Prerequisite_Science(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Remove_Module(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Replace_Module(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Weapon_Template_Set(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Overrideable_By_Like_Kind(INI *ini, void *instance, void *store, const void *user_data);

    int Calc_Cost_To_Build(const Player *player) const;
    int Calc_Time_To_Build(const Player *player) const;
    bool Can_Possibly_Have_Any_Weapon() const;
    void Copy_From(const ThingTemplate *that);
    const ArmorTemplateSet *Find_Armor_Template_Set(const BitFlags<ARMORSET_COUNT> &t) const;
    const WeaponTemplateSet *Find_Weapon_Template_Set(const BitFlags<WEAPONSET_COUNT> &t) const;
    AIUpdateModuleData *Friend_Get_AI_Module_Info() const;
    ThingTemplate *Get_Build_Facility_Template(const Player *player) const;
    BuildableStatus Get_Buildable() const;
    Utf8String Get_LTA_Name() const { return m_LTAName; }
    int Get_Max_Simultaneous_Of_Type() const;
    const FXList *Get_Per_Unit_FX(const Utf8String &fxname) const;
    const AudioEventRTS *Get_Per_Unit_Sound(const Utf8String &soundname) const;
    int Get_Skill_Point_Value(int level) const;
    void Init_For_LTA(const Utf8String &name);
    bool Is_Equivalent_To(const ThingTemplate *reskin) const;
    bool Remove_Module_Info(const Utf8String &tag, Utf8String &name);
    void Resolve_Names();
    void Set_Copied_From_Default();

    void Validate();
    void Validate_Audio();
    void Validate_Sound(const AudioEventRTS *event, const char *name);

    static const FieldParse *Get_Field_Parse();
    static const FieldParse *Get_Reskin_Field_Parse();

    bool Is_Any_KindOf(const BitFlags<KINDOF_COUNT> &any_kind_of) const
    {
        return m_kindOf.Any_Intersection_With(any_kind_of);
    }

    bool Is_KindOf_Multi(const BitFlags<KINDOF_COUNT> &must_be_set, const BitFlags<KINDOF_COUNT> &must_be_clear) const
    {
        return m_kindOf.Test_Set_And_Clear(must_be_set, must_be_clear);
    }

    const AudioEventRTS *Get_Voice_Select() const { return Get_Audio(THING_VOICESELECT); }
    const AudioEventRTS *Get_Voice_Group_Select() const { return Get_Audio(THING_VOICEGROUPSELECT); }
    const AudioEventRTS *Get_Voice_Select_Elite() const { return Get_Audio(THING_VOICESELECTELITE); }
    const AudioEventRTS *Get_Voice_Move() const { return Get_Audio(THING_VOICEMOVE); }
    const AudioEventRTS *Get_Voice_Attack() const { return Get_Audio(THING_VOICEATTACK); }
    const AudioEventRTS *Get_Voice_Enter() const { return Get_Audio(THING_VOICEENTER); }
    const AudioEventRTS *Get_Voice_Fear() const { return Get_Audio(THING_VOICEFEAR); }
    const AudioEventRTS *Get_Voice_Created() const { return Get_Audio(THING_VOICECREATED); }
    const AudioEventRTS *Get_Voice_Near_Enemy() const { return Get_Audio(THING_VOICENEARENEMY); }
    const AudioEventRTS *Get_Voice_Task_Unable() const { return Get_Audio(THING_VOICETASKUNABLE); }
    const AudioEventRTS *Get_Voice_Task_Complete() const { return Get_Audio(THING_VOICETASKCOMPLETE); }
    const AudioEventRTS *Get_Voice_Meet_Enemy() const { return Get_Audio(THING_VOICEMEETENEMY); }
    const AudioEventRTS *Get_Sound_Move_Start() const { return Get_Audio(THING_SOUNDMOVESTART); }
    const AudioEventRTS *Get_Sound_Move_Start_Damaged() const { return Get_Audio(THING_SOUNDMOVESTARTDAMAGED); }
    const AudioEventRTS *Get_Sound_Move_Loop() const { return Get_Audio(THING_SOUNDMOVELOOP); }
    const AudioEventRTS *Get_Sound_Move_Loop_Damaged() const { return Get_Audio(THING_SOUNDMOVELOOPDAMAGED); }
    const AudioEventRTS *Get_Sound_Ambient() const { return Get_Audio(THING_SOUNDAMBIENT); }
    const AudioEventRTS *Get_Sound_Ambient_Damaged() const { return Get_Audio(THING_SOUNDAMBIENTDAMAGED); }
    const AudioEventRTS *Get_Sound_Ambient_Really_Damaged() const { return Get_Audio(THING_SOUNDAMBIENTREALLYDAMAGED); }
    const AudioEventRTS *Get_Sound_Ambient_Rubble() const { return Get_Audio(THING_SOUNDAMBIENTRUBBLE); }
    const AudioEventRTS *Get_Sound_Stealth_On() const { return Get_Audio(THING_SOUNDSTEALTHON); }
    const AudioEventRTS *Get_Sound_Stealth_Off() const { return Get_Audio(THING_SOUNDSTEALTHOFF); }
    const AudioEventRTS *Get_Sound_Created() const { return Get_Audio(THING_SOUNDCREATED); }
    const AudioEventRTS *Get_Sound_On_Damaged() const { return Get_Audio(THING_SOUNDONDAMAGED); }
    const AudioEventRTS *Get_Sound_On_Really_Damaged() const { return Get_Audio(THING_SOUNDONREALLYDAMAGED); }
    const AudioEventRTS *Get_Sound_Enter() const { return Get_Audio(THING_SOUNDENTER); }
    const AudioEventRTS *Get_Sound_Exit() const { return Get_Audio(THING_SOUNDEXIT); }
    const AudioEventRTS *Get_Sound_Promoted_Veteran() const { return Get_Audio(THING_SOUNDPROMOTEDVETERAN); }
    const AudioEventRTS *Get_Sound_Promoted_Elite() const { return Get_Audio(THING_SOUNDPROMOTEDELITE); }
    const AudioEventRTS *Get_Sound_Promoted_Hero() const { return Get_Audio(THING_SOUNDPROMOTEDHERO); }
    const AudioEventRTS *Get_Voice_Garrison() const { return Get_Audio(THING_VOICEGARRISON); }
    const AudioEventRTS *Get_Sound_Falling_From_Plane() const { return Get_Audio(THING_SOUNDFALLINGFROMPLANE); }
    const AudioEventRTS *Get_Voice_Defect() const { return Get_Audio(THING_VOICEDEFECT); }
    const AudioEventRTS *Get_Voice_Attack_Special() const { return Get_Audio(THING_VOICEATTACKSPECIAL); }
    const AudioEventRTS *Get_Voice_Attack_Air() const { return Get_Audio(THING_VOICEATTACKAIR); }
    const AudioEventRTS *Get_Voice_Guard() const { return Get_Audio(THING_VOICEGUARD); }

    const AudioEventRTS *Get_Audio(ThingTemplateAudioType type) const
    {
        if (m_audio.sound[type]) {
            return &m_audio.sound[type]->m_event;
        } else {
            return &s_audioEventNoSound;
        }
    }

    const std::
        map<Utf8String, AudioEventRTS, std::less<Utf8String>, std::allocator<std::pair<Utf8String const, AudioEventRTS>>> *
        Get_All_Per_Unit_Sounds() const
    {
        return &m_perUnitSounds;
    }

    bool Has_Sound_Ambient() const { return Has_Audio(THING_SOUNDAMBIENT); }
    bool Has_Audio(ThingTemplateAudioType type) const { return m_audio.sound[type] != nullptr; }

    int Get_Energy_Production() const { return m_energyProduction; }
    EditorSortingType Get_Editor_Sorting() const { return (EditorSortingType)m_editorSorting; }
    const GeometryInfo &Get_Template_Geometry_Info() const { return m_geometryInfo; }
    float Get_Vision_Range() const { return m_visionRange; }
    const std::vector<WeaponTemplateSet> &Get_Weapon_Template_Sets() const { return m_weaponTemplateSets; }
    const Utf8String &Get_Name() const { return m_nameString; }
    const Utf8String &Get_Command_Set_String() const { return m_commandSetString; }
    float Get_Fence_Width() const { return m_fenceWidth; }
    float Get_Fence_X_Offset() { return m_fenceXOffset; }
    unsigned int Get_Display_Color() const { return m_displayColor; }
    const Utf16String &Get_Display_Name() const { return m_displayName; }
    const ModuleInfo *Get_Body_Modules() const { return &m_body; }
    float Get_Asset_Scale() const { return m_assetScale; }
    float Get_Placement_View_Angle() const { return m_placementViewAngle; }
    const ModuleInfo *Get_Draw_Modules() const { return &m_draws; }
    unsigned int Get_Shadow_Type() const { return m_shadowType; }
    float Get_Shadow_Size_X() const { return m_shadowSizeX; }
    float Get_Shadow_Size_Y() const { return m_shadowSizeY; }
    float Get_Shadow_Offset_X() const { return m_shadowOffsetX; }
    float Get_Shadow_Offset_Y() const { return m_shadowOffsetY; }
    const Utf8String &Get_Shadow_Texture_Name() const { return m_shadowTextureName; }
    unsigned short Get_Template_ID() const { return m_templateID; }
    const std::vector<Utf8String> &Get_Build_Variations() const { return m_buildVariations; }
    const ModuleInfo *Get_Client_Update_Modules() const { return &m_clientUpdates; }
    int Get_Prereq_Count() const { return m_prerequisites.size(); }
    const ProductionPrerequisite *Get_Nth_Prereq(int i) const { return &m_prerequisites[i]; }
    int Get_Build_Cost() const { return m_buildCost; }
    BuildCompletionType Get_Build_Completion() const { return (BuildCompletionType)m_buildCompletion; }
    float Get_Build_Time() const { return m_buildTime; }
    float Get_Shroud_Clearing_Range() const { return m_shroudClearingRange; }
    unsigned int Get_Occlusion_Delay() const { return m_occlusionDelay; }
    unsigned char Get_Crusher_Level() const { return m_crusherLevel; }
    int Get_Energy_Bonus() const { return m_energyBonus; }
    RadarPriorityType Get_Radar_Priority() const { return (RadarPriorityType)m_radarPriority; }
    int Get_Raw_Transport_Slot_Count() const { return m_transportSlotCount; }
    Image *Get_Selected_Portrait_Image() const { return m_selectedPortraitImage; }
    float Get_Instance_Scale_Fuzziness() const { return m_instanceScaleFuzziness; }
    float Get_Shroud_Reveal_To_All_Range() const { return m_shroudRevealToAllRange; }
    unsigned char Get_Crushable_Level() const { return m_crushableLevel; }
    int Get_Threat_Value() const { return m_threatValue; }
    const Utf8String &Get_Default_Owning_Side() const { return m_defaultOwningSide; }
    int Get_Experience_Value(int level) const { return m_experienceValues[level]; }
    NameKeyType Get_Max_Simultaneous_Link_Key() const { return m_maxSimultaneousLinkKey; }
    int Get_Experience_Required(int level) const { return m_experienceRequired[level]; }
    Image *Get_Button_Image() const { return m_buttonImage; }
    Utf8String Get_Upgrade_Cameo_Name(int i) const { return m_upgradeCameoNames[i]; }
    float Get_Structure_Rubble_Height() const { return m_structureRubbleHeight; }
    int Get_Refund_Value() const { return m_refundValue; }
    float Get_Factory_Exit_Width() const { return m_factoryExitWidth; }
    float Get_Factory_Extra_Bib_Width() const { return m_factoryExtraBibWidth; }
    Utf8String Get_Upgrade_Cameo_Name(int cameo) { return m_upgradeCameoNames[cameo]; }

    bool Is_Trainable() const { return m_isTrainable; }
    bool Is_Build_Facility() const { return m_isBuildFacility; }
    bool Is_Prerequisite() const { return m_isPrerequisite; }
    bool Is_Enter_Guard() const { return m_enterGuard; }
    bool Is_Hijack_Guard() const { return m_hijackGuard; }
    bool Is_KindOf(KindOfType t) const { return m_kindOf.Test(t); }
    bool Is_Bridge() const { return m_isBridge; }
    int Is_Buildable_Item() const { return m_buildCost != 0; }

    ThingTemplate *Friend_Get_Next_Template() const { return m_nextThingTemplate; }
    void Friend_Set_Next_Template(ThingTemplate *tmplate) { m_nextThingTemplate = tmplate; }
    void Friend_Set_Template_Name(Utf8String name) { m_nameString = name; }
    void Friend_Set_Template_ID(unsigned short id) { m_templateID = id; }
    void Friend_Set_Original_Skin_Template(ThingTemplate *tmplate)
    {
        captainslog_assert(!m_originalSkinTemplate);
        m_originalSkinTemplate = tmplate;
    }

private:
    Utf16String m_displayName;
    Utf8String m_nameString;
    Utf8String m_defaultOwningSide;
    Utf8String m_commandSetString;
    Utf8String m_selectedPortraitImageName;
    Utf8String m_buttonImageName;
    Utf8String m_upgradeCameoNames[5];
    Utf8String m_shadowTextureName;
    Utf8String m_replacementModuleName;
    Utf8String m_replacementModuleTag;
    Utf8String m_LTAName;
    GeometryInfo m_geometryInfo;
    BitFlags<KINDOF_COUNT> m_kindOf;
    AudioArray m_audio;
    ModuleInfo m_body;
    ModuleInfo m_draws;
    ModuleInfo m_clientUpdates;
    int m_skillPointValues[RANK_LEVEL_COUNT];
    int m_experienceValues[RANK_LEVEL_COUNT];
    int m_experienceRequired[RANK_LEVEL_COUNT];
    std::vector<ProductionPrerequisite> m_prerequisites;
    std::vector<Utf8String> m_buildVariations;
    std::vector<WeaponTemplateSet> m_weaponTemplateSets;
    SparseMatchFinder<WeaponTemplateSet, BitFlags<WEAPONSET_COUNT>> m_weaponTemplateSetFinder;
    std::vector<ArmorTemplateSet> m_armorTemplateSets;
    SparseMatchFinder<ArmorTemplateSet, BitFlags<ARMORSET_COUNT>> m_armorTemplateSetFinder;
    std::map<Utf8String, AudioEventRTS> m_perUnitSounds;
    std::map<Utf8String, FXList *> m_perUnitEffects;
    ThingTemplate *m_nextThingTemplate;
    ThingTemplate *m_originalSkinTemplate;
    Image *m_selectedPortraitImage;
    Image *m_buttonImage;
    float m_fenceWidth;
    float m_fenceXOffset;
    float m_visionRange;
    float m_shroudClearingRange;
    float m_shroudRevealToAllRange;
    float m_placementViewAngle;
    float m_factoryExitWidth;
    float m_factoryExtraBibWidth;
    float m_buildTime;
    float m_assetScale;
    float m_instanceScaleFuzziness;
    float m_shadowSizeX;
    float m_shadowSizeY;
    float m_shadowOffsetX;
    float m_shadowOffsetY;
    int m_energyProduction;
    int m_energyBonus;
    unsigned int m_displayColor;
    unsigned int m_occlusionDelay;
    NameKeyType m_maxSimultaneousLinkKey;
    unsigned short m_templateID;
    unsigned short m_buildCost;
    unsigned short m_refundValue;
    unsigned short m_threatValue;
    unsigned short m_maxSimultaneousOfType;
    bool m_determinedBySuperweaponRestriction; // if true GameLogic m_maxSimultaneousOfType is used instead of this
    bool m_isPrerequisite;
    bool m_isBridge;
    bool m_isBuildFacility;
    bool m_isTrainable;
    bool m_enterGuard;
    bool m_hijackGuard;
    bool m_isForbidden;
    bool m_armorCopiedFromDefault;
    bool m_weaponsCopiedFromDefault;
    unsigned char m_radarPriority;
    unsigned char m_transportSlotCount;
    unsigned char m_buildable;
    unsigned char m_buildCompletion;
    unsigned char m_editorSorting;
    unsigned char m_structureRubbleHeight;
    unsigned char m_shadowType;
    unsigned char m_moduleParseState;
    unsigned char m_crusherLevel;
    unsigned char m_crushableLevel;

#ifdef GAME_DLL
    static AudioEventRTS &s_audioEventNoSound;
#else
    static AudioEventRTS s_audioEventNoSound;
#endif
    static const FieldParse s_objectFieldParseTable[];
    static const FieldParse s_objectReskinFieldParseTable[];
};
