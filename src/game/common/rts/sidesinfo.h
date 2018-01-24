/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for holding side information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef SIDESINFO_H
#define SIDESINFO_H

#include "always.h"
#include "buildinfo.h"
#include "dict.h"
#include "scriptlist.h"

class SidesInfo
{
public:
    SidesInfo() : m_buildList(nullptr), m_dict(0), m_scripts(nullptr) {}
    SidesInfo(const SidesInfo &that);
    ~SidesInfo();

    void Init(const Dict *dict = nullptr);
    void Add_To_BuildList(BuildListInfo *list, int pos);
    int Remove_From_BuildList(BuildListInfo *list);
    void Reorder_In_BuildList(BuildListInfo *list, int pos);
    void Set_ScriptList(ScriptList *list) { m_scripts = list; }
    ScriptList *Get_ScriptList() const { return m_scripts; }
    Dict &Get_Dict() { return m_dict; }

    SidesInfo &operator=(const SidesInfo &that);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    BuildListInfo *m_buildList;
    Dict m_dict;
    ScriptList *m_scripts;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void SidesInfo::Hook_Me()
{
    Hook_Method(0x004D5C00, &Init);
    Hook_Method(0x004D5C80, &operator=);
}
#endif

#endif // SIDESINFO_H
