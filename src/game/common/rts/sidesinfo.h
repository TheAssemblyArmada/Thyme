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
    void Add_To_BuildList(BuildListInfo *list, int32_t pos);
    int32_t Remove_From_BuildList(BuildListInfo *list);
    void Reorder_In_BuildList(BuildListInfo *list, int32_t pos);
    void Set_ScriptList(ScriptList *list) { m_scripts = list; }
    ScriptList *Get_ScriptList() const { return m_scripts; }
    Dict &Get_Dict() { return m_dict; }

    SidesInfo &operator=(const SidesInfo &that);

private:
    BuildListInfo *m_buildList;
    Dict m_dict;
    ScriptList *m_scripts;
};
