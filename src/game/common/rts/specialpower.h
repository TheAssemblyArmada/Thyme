/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Power
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
#include "academystats.h"
#include "audioeventrts.h"
#include "object.h"
#include "subsysteminterface.h"

class SpecialPowerTemplate : public Overridable
{
    IMPLEMENT_POOL(SpecialPowerTemplate)

public:
    SpecialPowerTemplate();
    virtual ~SpecialPowerTemplate() override {}

    const SpecialPowerTemplate *Get_FO() const;

    Utf8String Get_Name() const { return Get_FO()->m_name; }
    SpecialPowerType Get_Type() const { return Get_FO()->m_type; }
    bool Get_Shortcut_Power() const { return Get_FO()->m_shortcutPower; }
    bool Get_Shared_Synced_Timer() const { return Get_FO()->m_sharedSyncedTimer; }
    unsigned int Get_ID() const { return Get_FO()->m_id; }
    unsigned int Get_Reload_Time() const { return Get_FO()->m_reloadTime; }
    ScienceType Get_Required_Science() const { return m_requiredScience; }
    AcademyClassificationType Get_Academy_Classify() const { return m_academyClassify; }
    float Get_Radius_Cursor_Radius() const { return Get_FO()->m_radiusCursorRadius; }

    void Set_Name_ID(Utf8String const &name, unsigned int id)
    {
        m_name = name;
        m_id = id;
    }

    static const FieldParse *Get_Field_Parse() { return s_specialPowerFieldParse; }

private:
    Utf8String m_name;
    unsigned int m_id;
    SpecialPowerType m_type;
    unsigned int m_reloadTime;
    ScienceType m_requiredScience;
    AudioEventRTS m_initiateSound;
    AudioEventRTS m_initiateAtLocationSound;
    AcademyClassificationType m_academyClassify;
    unsigned int m_detectionTime;
    unsigned int m_viewObjectDuration;
    float m_viewObjectRange;
    float m_radiusCursorRadius;
    bool m_publicTimer;
    bool m_sharedSyncedTimer;
    bool m_shortcutPower;

    static const FieldParse s_specialPowerFieldParse[];
};

class SpecialPowerStore : public SubsystemInterface
{
public:
    SpecialPowerStore() : m_nextSpecialPowerID(0) {}
    virtual ~SpecialPowerStore() override;
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

    bool Can_Use_Special_Power(Object *obj, const SpecialPowerTemplate *special_power_template);
    SpecialPowerTemplate *Find_Special_Power_Template_Private(Utf8String name);
    const SpecialPowerTemplate *Find_Special_Power_Template(Utf8String name);
    const SpecialPowerTemplate *Get_Special_Power_Template_By_Index(unsigned int index);
    int Get_Num_Special_Powers();

    const SpecialPowerTemplate *Find_Special_Power_Template_By_ID(unsigned int id);

    static void Parse_Special_Power_Definition(INI *ini);

private:
    std::vector<SpecialPowerTemplate *> m_specialPowerTemplates;
    unsigned int m_nextSpecialPowerID;
};

#ifdef GAME_DLL
extern SpecialPowerStore *&g_theSpecialPowerStore;
#else
extern SpecialPowerStore *g_theSpecialPowerStore;
#endif
