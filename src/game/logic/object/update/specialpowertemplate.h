/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Template
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "audioeventrts.h"
#include "overridable.h"
#include "ini.h"

class SpecialPowerStore;
enum SpecialPowerType;
enum ScienceType;

enum AcademyClassType : int32_t // #TODO move somewhere else
{
    ACT_NONE,
    ACT_UPGRADE_RADAR,
    ACT_SUPERPOWER,

    ACT_COUNT
};
extern const char* const TheAcademyClassificationTypeNames[ACT_COUNT + 1]; // #TODO move somewhere else

class SpecialPowerTemplate : public Overridable
{
    friend class SpecialPowerStore;

    IMPLEMENT_NAMED_POOL(SpecialPowerTemplate, SpecialPowerTemplate);

public:
    SpecialPowerTemplate();

    void Init(const Utf8String &name, unsigned int id);

    const Utf8String& Get_Name() const;
    unsigned int Get_ID() const;
    ScienceType Get_Required_Science() const;

    const SpecialPowerTemplate *Friend_Get_Final_Override() const;
    SpecialPowerTemplate *Friend_Get_Final_Override();

protected:
    virtual ~SpecialPowerTemplate() override;

private:
    static const FieldParse *Get_Parse_Table();

protected:
    Utf8String m_name;
    unsigned int m_id;
    SpecialPowerType m_type;
    uint32_t m_reloadTime;
    ScienceType m_requiredScience;
    AudioEventRTS m_initiateSound;
    AudioEventRTS m_initiateAtLocationSound;
    AcademyClassType m_academyClassType;
    uint32_t m_detectionTime;
    uint32_t m_viewObjectDuration;
    float m_viewObjectRange;
    float m_cursorRadius;
    bool m_hasPublicTimer;
    bool m_hasSharedSyncedTimer;
    bool m_hasShortcutPower;

private:
    static const FieldParse s_parseTable[];
};
