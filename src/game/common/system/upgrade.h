/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Upgrade
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
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "subsysteminterface.h"

class Image;

enum UpgradeType
{
    UPGRADE_TYPE_PLAYER,
    UPGRADE_TYPE_OBJECT,
};

enum AcademyClassificationType
{
    ACT_NONE,
    ACT_UPGRADE_RADAR,
    ACT_SUPERPOWER,
};

class UpgradeTemplate : public MemoryPoolObject
{
    IMPLEMENT_POOL(UpgradeTemplate);

public:
    virtual ~UpgradeTemplate() override;
    UpgradeTemplate *Friend_Get_Next() { return m_next; }
    UpgradeTemplate *Friend_Get_Prev() { return m_prev; }
    const UpgradeTemplate *Friend_Get_Next() const { return m_next; }

private:
    UpgradeType m_type;
    Utf8String m_name;
    NameKeyType m_nameKey;
    Utf8String m_displayNameLabel;
    float m_buildTime;
    int m_cost;
    BitFlags<128> m_upgradeMask;
    AudioEventRTS m_researchSound;
    AudioEventRTS m_unitSpecificSound;
    AcademyClassificationType m_academyClassify;
    UpgradeTemplate *m_next;
    UpgradeTemplate *m_prev;
    Utf8String m_buttonImageName;
    Image *m_buttonImage;
};

class UpgradeCenter : public SubsystemInterface
{
public:
    UpgradeCenter();
    virtual ~UpgradeCenter() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

private:
    UpgradeTemplate *m_upgradeList;
    int m_upgradeCount;
    bool m_buttonImagesCached;
};

#ifdef GAME_DLL
extern UpgradeCenter *&g_theUpgradeCenter;
#else
extern UpgradeCenter *g_theUpgradeCenter;
#endif
