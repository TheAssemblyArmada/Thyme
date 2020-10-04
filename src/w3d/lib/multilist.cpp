/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Handles lists of objects that can be in multiple lists at the same time.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "multilist.h"

/**
 * Checks if an object is contained within the multilist.
 *
 * 0x008A03F0
 */
bool GenericMultiListClass::Contains(MultiListObjectClass *obj) const
{
    captainslog_assert(obj != nullptr);
    for (MultiListNodeClass *node = obj->Get_List_Node(); node; node = node->m_nextList) {
        if (node->m_list == this) {
            return true;
        }
    };
    return false;
}

/**
 * Adds an object to the head of the multilist.
 *
 * 0x008A0420
 */
bool GenericMultiListClass::Internal_Add(MultiListObjectClass *obj, bool onlyonce)
{
    captainslog_assert(obj != nullptr);

    if (onlyonce && Is_In_List(obj)) {
        return false;
    }

    MultiListNodeClass *node = new MultiListNodeClass;
    node->m_object = obj;

    node->m_nextList = obj->Get_List_Node();
    obj->Set_List_Node(node);

    node->m_prev = &(m_head);
    node->m_next = m_head.m_next;
    node->m_next->m_prev = node;
    node->m_prev->m_next = node;

    node->m_list = this;

    return true;
};

/**
 * Adds an object to the tail of the multilist.
 */
bool GenericMultiListClass::Internal_Add_Tail(MultiListObjectClass *obj, bool onlyonce)
{
    captainslog_assert(obj != nullptr);

    if (onlyonce && Is_In_List(obj)) {
        return false;
    }

    MultiListNodeClass *node = new MultiListNodeClass;
    node->m_object = obj;

    node->m_nextList = obj->Get_List_Node();
    obj->Set_List_Node(node);

    node->m_prev = m_head.m_prev;
    node->m_next = &(m_head);
    node->m_next->m_prev = node;
    node->m_prev->m_next = node;

    node->m_list = this;

    return true;
};

/**
 * Adds an object after the existing member of the multilist.
 */
bool GenericMultiListClass::Internal_Add_After(
    MultiListObjectClass *obj, const MultiListObjectClass *existing_list_member, bool onlyonce)
{
    captainslog_assert(obj != nullptr);
    captainslog_assert(existing_list_member);

    if (onlyonce && Is_In_List(obj)) {
        return false;
    }

    MultiListNodeClass *existing_node = existing_list_member->Get_List_Node();
    while (existing_node && existing_node->m_list != this) {
        existing_node = existing_node->m_nextList;
    }

    if (existing_node == nullptr) {
        return false;
    }

    MultiListNodeClass *node = new MultiListNodeClass;
    node->m_object = obj;

    node->m_nextList = obj->Get_List_Node();
    obj->Set_List_Node(node);

    node->m_prev = existing_node;
    node->m_next = existing_node->m_next;
    node->m_next->m_prev = node;
    node->m_prev->m_next = node;
    node->m_list = this;

    return true;
};

/**
 * Removes an object from the multilist.
 *
 * 0x008A0800
 */
bool GenericMultiListClass::Internal_Remove(MultiListObjectClass *obj)
{
    captainslog_assert(obj != nullptr);

    MultiListNodeClass *node = obj->Get_List_Node();
    MultiListNodeClass *prevnode = nullptr;

    while (node != nullptr && node->m_list != this) {
        prevnode = node;
        node = node->m_nextList;
    }

    if (node == nullptr) {
        return false;
    }

    node->m_prev->m_next = node->m_next;
    node->m_next->m_prev = node->m_prev;

    if (prevnode != nullptr) {
        prevnode->m_nextList = node->m_nextList;
    } else {
        captainslog_assert(obj->Get_List_Node() == node);
        obj->Set_List_Node(node->m_nextList);
    }

    delete node;

    return true;
};

/**
 * Removes an object at the head of the multilist.
 *
 * 0x008A0880
 */
MultiListObjectClass *GenericMultiListClass::Internal_Remove_List_Head()
{
    if (Is_Empty()) {
        return nullptr;
    }

    MultiListObjectClass *obj = m_head.m_next->m_object;
    Internal_Remove(obj);

    return obj;
};

/**
 * Removes an object from the multilist.
 *
 * 0x00824720
 */
MultiListObjectClass *GenericMultiListClass::Internal_Get_List_Head() const
{
    if (Is_Empty()) {
        return 0;
    }

    captainslog_assert(m_head.m_next->m_object != nullptr);
    return m_head.m_next->m_object;
};
