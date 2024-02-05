/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Crate System
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
#include "bitflags.h"
#include "overridable.h"
#include "subsysteminterface.h"
#include <list>

struct CrateCreationEntry
{
    Utf8String crate_name;
    float crate_chance;
};

class CrateTemplate : public Overridable
{
    IMPLEMENT_POOL(CrateTemplate);

public:
    CrateTemplate();
    virtual ~CrateTemplate() override;

    static void Parse_Crate_Creation_Entry(INI *ini, void *formal, void *store, const void *user_data);
    static const FieldParse *Build_Field_Parse() { return s_theCrateTemplateFieldParseTable; }
    static const FieldParse s_theCrateTemplateFieldParseTable[];

    void Set_Name(Utf8String name) { m_name = name; }

    Utf8String Get_Name() const { return m_name; }

private:
    Utf8String m_name;
    float m_creationChance;
    VeterancyLevel m_veterancyLevel;
    BitFlags<KINDOF_COUNT> m_killedByTypeKindof;
    ScienceType m_killerScience;
    std::list<CrateCreationEntry> m_possibleCrates;
    bool m_ownedByMaker;
};

class CrateSystem : public SubsystemInterface
{
public:
    CrateSystem();
    virtual ~CrateSystem() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    CrateTemplate *New_Crate_Template(Utf8String name);
    CrateTemplate *New_Crate_Template_Override(CrateTemplate *crate);
    const CrateTemplate *Find_Crate_Template(Utf8String name);
    CrateTemplate *Friend_Find_Crate_Template(Utf8String name);

    static void Parse_Crate_Template_Definition(INI *ini);

private:
    std::vector<CrateTemplate *> m_crateTemplateVector;
};

#ifdef GAME_DLL
extern CrateSystem *&g_theCrateSystem;
#else
extern CrateSystem *g_theCrateSystem;
#endif
