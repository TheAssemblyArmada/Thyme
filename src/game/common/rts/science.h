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
#include "gametype.h"
#include "ini.h"
#include "mempoolobj.h"
#include "overridable.h"
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <vector>

class Player;
class INI;
struct FieldParse;

class ScienceInfo;

class ScienceStore : public SubsystemInterface
{
public:
    ScienceStore() {}
    virtual ~ScienceStore() {}

    virtual void Init() override { m_infoVec.clear(); }
    virtual void Reset() override;
    virtual void Update() override {}

    ScienceType Lookup_Science(const char *name);
    ScienceType Get_Science_From_Internal_Name(const Utf8String &name) const;
    Utf8String Get_Internal_Name_From_Science(ScienceType science) const;
    const ScienceInfo *Get_Science_Info(ScienceType science) const;
    std::vector<Utf8String> Get_All_Sciences() const;
    int32_t Get_Science_Purchase_Cost(ScienceType science) const;
    bool Is_Science_Grantable(ScienceType science) const;
    bool Get_Name_And_Description(ScienceType science, Utf16String &name, Utf16String &description);
    bool Player_Has_Prereqs_For_Science(const Player *player, ScienceType science);
    bool Player_Has_Root_Prereqs_For_Science(const Player *player, ScienceType science);

    static void Parse_Science_Definition(INI *ini);

    static void Parse_Science_Vector(INI *ini, void *, void *store, const void *);
    static void Parse_Science(INI *ini, void *, void *store, const void *);

private:
    bool Has_Science(ScienceType science) const { return Get_Science_Info(science) != nullptr; }
    std::vector<ScienceInfo *> m_infoVec;
};

class ScienceInfo : public Overridable
{
    IMPLEMENT_POOL(ScienceInfo);

protected:
    virtual ~ScienceInfo() override {}

public:
    ScienceInfo();

    ScienceType Get_Science_Type() const { return m_scienceType; }
    void Set_Science_Type(ScienceType science) { m_scienceType = science; }
    bool Check_Science_Type(ScienceType science) const { return science == m_scienceType; }
    int32_t Get_Purchase_Cost() const { return m_purchaseCost; }
    bool Is_Grantable() const { return m_isGrantable; }
    const Utf16String &Get_Name() const { return m_displayName; }
    const Utf16String &Get_Description() const { return m_description; }
    std::vector<ScienceType> &Get_Root_Sciences() { return m_rootSciences; }
    void Add_Root_Sciences(std::vector<ScienceType> &rootSciences) const;

    static const FieldParse *Get_Field_Parse() { return s_scienceFieldParseTable; }

private:
    ScienceType m_scienceType;
    Utf16String m_displayName;
    Utf16String m_description;
    std::vector<ScienceType> m_rootSciences;
    std::vector<ScienceType> m_prerequisites;
    int m_purchaseCost;
    bool m_isGrantable;

    static const FieldParse s_scienceFieldParseTable[];
};

#ifdef GAME_DLL
extern ScienceStore *&g_theScienceStore;
#else
extern ScienceStore *g_theScienceStore;
#endif