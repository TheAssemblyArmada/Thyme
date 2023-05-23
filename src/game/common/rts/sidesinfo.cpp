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
#include "sidesinfo.h"

SidesInfo::SidesInfo(const SidesInfo &that) : m_buildList(nullptr), m_dict(0), m_scripts(nullptr)
{
    *this = that;
}

SidesInfo::~SidesInfo()
{
    Init();
    m_dict.Release_Data();
}

/**
 * @brief Initializes this side info.
 *
 * 0x004D5C00
 */
void SidesInfo::Init(const Dict *dict)
{
    m_buildList->Delete_Instance();
    m_buildList = nullptr;
    m_dict.Init(dict);
    m_scripts->Delete_Instance();
    m_scripts = nullptr;
}

/**
 * @brief Inserts a build list info object into the list at the specified position.
 */
void SidesInfo::Add_To_Build_List(BuildListInfo *list, int pos)
{
    BuildListInfo *position = nullptr;
    BuildListInfo *build_list = m_buildList;

    for (int i = pos; i > 0; --i) {
        if (build_list == nullptr) {
            break;
        }

        position = build_list;
        build_list = build_list->Get_Next();
    }

    if (position != nullptr) {
        list->Set_Next_Build_List(position->Get_Next());
        position->Set_Next_Build_List(list);
    } else {
        list->Set_Next_Build_List(m_buildList);
        m_buildList = list;
    }
}

/**
 * @brief Removes a build list info object from the list.
 */
int SidesInfo::Remove_From_Build_List(BuildListInfo *list)
{
    if (list == nullptr) {
        return 0;
    }

    if (list == m_buildList) {
        m_buildList = list->Get_Next();
        list->Set_Next_Build_List(nullptr);

        return 0;
    }

    int pos = 1;

    for (BuildListInfo *next = m_buildList; next != nullptr; next = next->Get_Next()) {
        if (list == next->Get_Next()) {
            next->Set_Next_Build_List(list->Get_Next());

            break;
        }

        ++pos;
    }

    list->Set_Next_Build_List(nullptr);

    return pos;
}

/**
 * @brief Moves a build list info object in the list to the specified position.
 */
void SidesInfo::Reorder_In_Build_List(BuildListInfo *list, int pos)
{
    Remove_From_Build_List(list);
    Add_To_Build_List(list, pos);
}

/**
 * 0x004D5C80
 */
SidesInfo &SidesInfo::operator=(const SidesInfo &that)
{
    if (this != &that) {
        Init();
        m_dict = that.m_dict;

        // Copy the build list.
        for (BuildListInfo *next = that.m_buildList, *last = nullptr; next != nullptr; next = next->Get_Next()) {
            BuildListInfo *new_list = NEW_POOL_OBJ(BuildListInfo);
            *new_list = *next;

            if (last == nullptr) {
                m_buildList = new_list;
            } else {
                last->Set_Next_Build_List(new_list);
            }

            last = new_list;
        }

        if (that.m_scripts != nullptr) {
            m_scripts = that.m_scripts->Duplicate();
        }
    }

    return *this;
}
