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
    void Add_To_Build_List(BuildListInfo *list, int pos);
    int Remove_From_Build_List(BuildListInfo *list);
    void Reorder_In_Build_List(BuildListInfo *list, int pos);
    void Set_Script_List(ScriptList *list) { m_scripts = list; }
    ScriptList *Get_Script_List() const { return m_scripts; }
    Dict &Get_Dict() { return m_dict; }
    BuildListInfo *Get_Build_List() { return m_buildList; }
    void Clear_Build_List() { m_buildList = nullptr; }
    void Reset() { Init(nullptr); }

    SidesInfo &operator=(const SidesInfo &that);

private:
    BuildListInfo *m_buildList;
    Dict m_dict;
    ScriptList *m_scripts;
};
