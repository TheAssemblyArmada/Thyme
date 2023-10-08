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
#include "academystats.h"
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

enum UpgradeStatusType
{
    UPGRADE_STATUS_INVALID,
    UPGRADE_STATUS_IN_PRODUCTION,
    UPGRADE_STATUS_COMPLETE,
};

class UpgradeTemplate : public MemoryPoolObject
{
    IMPLEMENT_POOL(UpgradeTemplate);

public:
    UpgradeTemplate();
    virtual ~UpgradeTemplate() override {}
    UpgradeTemplate *Friend_Get_Next() { return m_next; }
    UpgradeTemplate *Friend_Get_Prev() { return m_prev; }
    const UpgradeTemplate *Friend_Get_Next() const { return m_next; }
    BitFlags<128> Get_Upgrade_Mask() const { return m_upgradeMask; }
    Utf8String Get_Name() const { return m_name; }
    NameKeyType Get_Name_Key() const { return m_nameKey; }
    UpgradeType Get_Type() const { return m_type; }
    AcademyClassificationType Get_Academy_Classify() const { return m_academyClassify; }
    const Image *Get_Button_Image() const { return m_buttonImage; }

    void Set_Name(const Utf8String &name) { m_name = name; }
    void Set_Name_Key(NameKeyType key) { m_nameKey = key; }
    void Friend_Set_Next(UpgradeTemplate *next) { m_next = next; }
    void Friend_Set_Prev(UpgradeTemplate *prev) { m_prev = prev; }
    void Set_Upgrade_Mask(BitFlags<128> mask) { m_upgradeMask = mask; }

    static const FieldParse *Get_Field_Parse() { return s_upgradeFieldParseTable; }

    int Calc_Cost_To_Build(Player *player) const { return m_cost; }
    int Calc_Time_To_Build(Player *player) const;
    void Friend_Make_Veterancy_Upgrade(VeterancyLevel level);
    void Cache_Button_Image();

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

    static const FieldParse s_upgradeFieldParseTable[];
};

class UpgradeCenter : public SubsystemInterface
{
public:
    UpgradeCenter();
    virtual ~UpgradeCenter() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    const UpgradeTemplate *Find_Veterancy_Upgrade(VeterancyLevel level);

    UpgradeTemplate *Get_Upgrade_List() { return m_upgradeList; }
    const UpgradeTemplate *Find_Upgrade(const Utf8String &name);
    const UpgradeTemplate *Find_Upgrade_By_Key(NameKeyType key);
    UpgradeTemplate *Find_Non_Const_Upgrade_By_Key(NameKeyType key);
    bool Can_Afford_Upgrade(Player *player, const UpgradeTemplate *upgrade, bool show_message);
    UpgradeTemplate *New_Upgrade(const Utf8String &name);
    void Link_Upgrade(UpgradeTemplate *upgrade);
    void Unlink_Upgrade(UpgradeTemplate *upgrade);
    std::vector<Utf8String> Get_Upgrade_Names();
    static void Parse_Upgrade_Definition(INI *ini);

private:
    UpgradeTemplate *m_upgradeList;
    int m_upgradeCount;
    bool m_buttonImagesCached;
};

class Upgrade : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Upgrade);

public:
    Upgrade(const UpgradeTemplate *upgrade_template);

    virtual ~Upgrade() override {}
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    Upgrade *Friend_Get_Next() const { return m_next; }
    Upgrade *Friend_Get_Prev() const { return m_prev; }
    void Friend_Set_Next(Upgrade *upgrade) { m_next = upgrade; }
    void Friend_Set_Prev(Upgrade *upgrade) { m_prev = upgrade; }
    UpgradeStatusType Get_Status() const { return m_status; }
    const UpgradeTemplate *Get_Template() const { return m_template; }
    void Set_Status(UpgradeStatusType status) { m_status = status; }

private:
    const UpgradeTemplate *m_template;
    UpgradeStatusType m_status;
    Upgrade *m_next;
    Upgrade *m_prev;
};

#ifdef GAME_DLL
extern UpgradeCenter *&g_theUpgradeCenter;
#else
extern UpgradeCenter *g_theUpgradeCenter;
#endif
