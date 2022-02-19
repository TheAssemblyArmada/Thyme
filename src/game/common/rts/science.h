/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Handles abilities granted by gaining experience points.
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
#include "ini.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "overridable.h"
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <vector>

enum ScienceType : int32_t
{
    SCIENCE_INVALID = -1,
    SCIENCE_BOGUS = 0,
};

class ScienceInfo;

class ScienceStore : public SubsystemInterface
{
public:
    virtual ~ScienceStore() {}

    virtual void Init() override { m_infoVec.clear(); }
    virtual void Reset() override;
    virtual void Update() override {}

    ScienceType Lookup_Science(const char *name);

    static void Parse_Science_Vector(INI *ini, void *, void *store, const void *);

private:
    std::vector<ScienceInfo *> m_infoVec;
};

class ScienceInfo : public Overridable
{
    IMPLEMENT_POOL(ScienceInfo);

protected:
    virtual ~ScienceInfo() override {}

public:
    ScienceInfo();

    NameKeyType Get_Name_Key() { return m_nameKey; }
    bool Check_Name_Key(NameKeyType key) { return key == m_nameKey; }

private:
    NameKeyType m_nameKey;
    Utf16String m_displayName;
    Utf16String m_description;
    std::vector<ScienceType> m_unkVec1;
    std::vector<ScienceType> m_prerequisites;
    int32_t m_purchaseCost;
    bool m_isGrantable;

    static FieldParse s_scienceFieldParseTable[];
};

#ifdef GAME_DLL
extern ScienceStore *&g_theScienceStore;
#else
extern ScienceStore *g_theScienceStore;
#endif