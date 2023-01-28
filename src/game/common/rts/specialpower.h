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
#include "audioeventrts.h"
#include "subsysteminterface.h"

class SpecialPowerTemplate : public Overridable
{
    IMPLEMENT_POOL(SpecialPowerTemplate)

public:
    virtual ~SpecialPowerTemplate() override;

private:
    Utf8String m_name;
    int m_id;
    int m_enum;
    int m_reloadTime;
    int m_requiredScience;
    AudioEventRTS m_initiateSound;
    AudioEventRTS m_initiateAtLocationSound;
    int m_academyClassify;
    int m_detectionTime;
    int m_viewObjectDuration;
    int m_viewObjectRange;
    int m_radiusCursorRadius;
    bool m_publicTimer;
    bool m_sharedSyncedTimer;
    bool m_shortcutPower;
};

class SpecialPowerStore : public SubsystemInterface
{
public:
    SpecialPowerStore() : m_nextSpecialPowerID(0) {}
    virtual ~SpecialPowerStore() override;
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

private:
    std::vector<SpecialPowerTemplate *> m_specialPowerTemplates;
    unsigned int m_nextSpecialPowerID;
};

#ifdef GAME_DLL
extern SpecialPowerStore *&g_theSpecialPowerStore;
#else
extern SpecialPowerStore *g_theSpecialPowerStore;
#endif
