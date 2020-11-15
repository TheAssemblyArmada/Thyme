/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Single linked list.
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
#include "slnode.h"

template<class T> class SList
{
public:
    SList() : m_headNode(nullptr), m_tailNode(nullptr) {}
    virtual ~SList() { Remove_All(); }

    SLNode<T> *Head() const { return m_headNode; }
    SLNode<T> *Tail() const { return m_tailNode; }

    SLNode<T> *Find_Node(T *data) const;
    SLNode<T> *Find_Prior_Node(T *data) const;

    virtual bool Add_Head(T *data);
    virtual bool Add_Head(SList<T> &list);
    virtual bool Add_Tail(T *data);
    virtual bool Add_Tail(SList<T> &list);

    virtual T *Remove_Head();
    virtual T *Remove_Tail();
    virtual bool Remove(T *element);
    virtual void Remove_All();

    virtual bool Insert_Before(T *newnode, T *oldnode = nullptr);
    virtual bool Insert_After(T *newnode, T *oldnode = nullptr);

    virtual bool Is_Empty() const { return m_headNode == nullptr; }

    virtual uint32_t Get_Count() const;

private:
    SLNode<T> *m_headNode;
    SLNode<T> *m_tailNode;
};

template<class T> inline SLNode<T> *SList<T>::Find_Node(T *data) const
{
    SLNode<T> *cur = nullptr;

    for (cur = m_headNode; cur != nullptr && cur->Data() != data; cur = cur->Next()) {
    }

    return cur;
}

template<class T> inline SLNode<T> *SList<T>::Find_Prior_Node(T *data) const
{
    SLNode<T> *cur = nullptr;

    for (cur = m_headNode; cur->Next() && cur->Next()->Data() != data; cur = cur->Next()) {
    }

    return cur;
}

template<class T> inline bool SList<T>::Add_Head(T *data)
{
    if (data == nullptr) {
        return false;
    }

    SLNode<T> *temp = new SLNode<T>(data);
    temp->Set_Next(m_headNode);
    m_headNode = temp;

    if (m_tailNode == nullptr) {
        m_tailNode = temp;
    }

    return true;
}

template<class T> inline bool SList<T>::Add_Head(SList<T> &list)
{
    if (list.Is_Empty()) {
        return false;
    }

    SLNode<T> *addpoint = nullptr;

    for (SLNode<T> *cur = list.m_headNode; cur != nullptr; cur = cur->Next()) {
        if (addpoint != nullptr) {
            SLNode<T> *temp = new SLNode<T>(cur->Data());
            temp->Set_Next(addpoint->Next());
            addpoint->Set_Next(temp);
            addpoint = temp;
        } else {
            Add_Head(cur->Data());
            addpoint = m_headNode;
        }
    }

    return true;
}

template<class T> inline bool SList<T>::Add_Tail(T *data)
{
    if (data == nullptr) {
        return false;
    }

    SLNode<T> *temp = new SLNode<T>(data);

    if (Is_Empty()) {
        m_headNode = temp;
        m_tailNode = temp;
    } else {
        m_tailNode->Set_Next(temp);
        m_tailNode = temp;
    }

    return true;
}

template<class T> inline bool SList<T>::Add_Tail(SList<T> &list)
{
    if (list.Is_Empty()) {
        return false;
    }

    for (SLNode<T> *cur = list.m_headNode; cur != nullptr; cur = cur->Next()) {
        Add_Tail(cur->Data());
    }

    return true;
}

template<class T> inline T *SList<T>::Remove_Head()
{
    if (Is_Empty()) {
        return nullptr;
    }

    SLNode<T> *temp = m_headNode;
    m_headNode = m_headNode->Next();

    if (Is_Empty()) {
        m_tailNode = nullptr;
    }

    T *data = temp->Data();
    delete temp;

    return data;
}

template<class T> inline T *SList<T>::Remove_Tail()
{
    if (Is_Empty()) {
        return nullptr;
    }

    T *data = m_tailNode->Data();
    return Remove(data) ? data : nullptr;
}

template<class T> inline bool SList<T>::Remove(T *element)
{
    if (element == nullptr || Is_Empty()) {
        return false;
    }

    if (m_headNode->Data() == element) {
        return Remove_Head() != nullptr ? true : false;
    }

    SLNode<T> *cur = Find_Prior_Node(oldnode);

    if (cur->Next() != nullptr && cur->Next()->Data() == element) {
        SLNode<T> *temp = cur->Next();
        cur->Set_Next(temp->Next());

        if (temp == m_tailNode) {
            m_tailNode = cur;
        }

        delete temp;

        return true;
    }

    return false;
}

template<class T> inline void SList<T>::Remove_All()
{
    SLNode<T> *next;

    for (SLNode<T> *cur = m_headNode; cur != nullptr; cur = next) {
        next = cur->Next();
        delete cur;
    }

    m_headNode = nullptr;
    m_tailNode = nullptr;
}

template<class T> inline bool SList<T>::Insert_Before(T *newnode, T *oldnode)
{
    if (newnode == nullptr) {
        return false;
    }

    if (oldnode == nullptr || Is_Empty() || m_headNode->Data() == oldnode) {
        return Add_Head(newnode);
    }

    SLNode<T> *cur = Find_Prior_Node(oldnode);

    if (cur->Next() != nullptr && cur->Next()->Data() == oldnode) {
        SLNode<T> *temp = new SLNode<T>(newnode);
        temp->Set_Next(cur->Next());
        cur->Set_Next(temp);

        return true;
    }

    return false;
}

template<class T> inline bool SList<T>::Insert_After(T *newnode, T *oldnode)
{
    if (newnode == nullptr) {
        return false;
    }

    if (oldnode == nullptr || Is_Empty()) {
        return Add_Head(newnode);
    }

    SLNode<T> *cur = Find_Node(oldnode);

    if (cur != nullptr && cur->Data() == oldnode) {
        if (cur == m_tailNode) {
            return (Add_Tail(newnode));
        }

        SLNode<T> *temp = new SLNode<T>(newnode);
        temp->Set_Next(cur->Next());
        cur->Set_Next(temp);

        return true;
    }

    return false;
}

template<class T> inline uint32_t SList<T>::Get_Count() const
{
    uint32_t count = 0;

    for (SLNode<T> *node = m_headNode; node != nullptr; node = node->Next()) {
        ++count;
    }

    return count;
}
