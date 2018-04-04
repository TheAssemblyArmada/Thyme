/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Handles abilities granted by gaining experience points.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "ini.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "overridable.h"
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <vector>

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

enum ScienceType : int32_t
{
    SCIENCE_INVALID = -1,
    SCIENCE_BOGUS = 0,
};

class ScienceInfo : public Overridable
{
    IMPLEMENT_POOL(ScienceInfo);

public:
    ScienceInfo();
    virtual ~ScienceInfo() {}

    NameKeyType Get_Name_Key() { return m_nameKey; }

private:
    NameKeyType m_nameKey;
    UnicodeString m_displayName;
    UnicodeString m_description;
    std::vector<ScienceType> m_unkVec1;
    std::vector<ScienceType> m_prerequisites;
    int m_purchaseCost;
    bool m_isGrantable;

    static FieldParse s_scienceFieldParseTable[];
};

class ScienceStore : public SubsystemInterface
{
public:
    virtual ~ScienceStore() {}

    virtual void Init() override { m_infoVec.clear(); }
    virtual void Reset() override;
    virtual void Update() override {}

private:
    std::vector<ScienceInfo *> m_infoVec;
};

#ifndef THYME_STANDALONE
extern ScienceStore *&g_theScienceStore;
#else
extern ScienceStore *g_theScienceStore;
#endif