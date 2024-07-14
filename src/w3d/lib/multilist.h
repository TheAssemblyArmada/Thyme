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
#pragma once

#include "always.h"
#include "autopool.h"
#include "refcount.h"
#include <captainslog.h>

// #TODO investigate C casts in this file.

class GenericMultiListClass;
class MultiListObjectClass;
class GenericMultiListIterator;

class MultiListNodeClass : public AutoPoolClass<MultiListNodeClass, 256>
{
public:
    MultiListNodeClass() : m_prev(nullptr), m_next(nullptr), m_nextList(nullptr), m_object(nullptr), m_list(nullptr) {}

    MultiListNodeClass *m_prev; // Previous object in list
    MultiListNodeClass *m_next; // Next object in list
    MultiListNodeClass *m_nextList; // Next list for object
    MultiListObjectClass *m_object; // Pointer to object
    GenericMultiListClass *m_list; // List for this node
};

class MultiListObjectClass
{
public:
    MultiListObjectClass() : m_listNode(nullptr){};
    virtual ~MultiListObjectClass();

    MultiListNodeClass *Get_List_Node() const { return m_listNode; };
    void Set_List_Node(MultiListNodeClass *node) { m_listNode = node; };

private:
    MultiListNodeClass *m_listNode;
};

class GenericMultiListClass
{
    ALLOW_HOOKING
    friend class GenericMultiListIterator;
    friend class MultiListObjectClass;

public:
    GenericMultiListClass()
    {
        m_head.m_next = m_head.m_prev = &m_head;
        m_head.m_object = 0;
        m_head.m_nextList = 0;
    };

    virtual ~GenericMultiListClass() { captainslog_assert(Is_Empty()); };

    bool Is_In_List(MultiListObjectClass *obj) const { return Contains(obj); };
    bool Is_Empty() const { return (m_head.m_next == &m_head); };
    bool Contains(MultiListObjectClass *obj) const;
    int Count() const;

protected:
    bool Internal_Add(MultiListObjectClass *obj, bool onlyonce = true);
    bool Internal_Add_Tail(MultiListObjectClass *obj, bool onlyonce = true);
    bool Internal_Add_After(
        MultiListObjectClass *obj, const MultiListObjectClass *existing_list_member, bool onlyonce = true);
    bool Internal_Remove(MultiListObjectClass *obj);
    MultiListObjectClass *Internal_Remove_List_Head();
    MultiListObjectClass *Internal_Get_List_Head() const;

private:
    MultiListNodeClass m_head;

    // not implemented
    GenericMultiListClass(const GenericMultiListClass &);
    GenericMultiListClass &operator=(const GenericMultiListClass &);
};

class GenericMultiListIterator
{
public:
    GenericMultiListIterator(const GenericMultiListClass *list)
    {
        captainslog_assert(list != nullptr);
        First(list);
    };

    void First(const GenericMultiListClass *list)
    {
        m_list = list;
        m_curNode = m_list->m_head.m_next;
    };

    void First() { m_curNode = m_list->m_head.m_next; };
    void Next() { m_curNode = m_curNode->m_next; };
    void Prev() { m_curNode = m_curNode->m_prev; };
    bool Is_Done() const { return (m_curNode == &(m_list->m_head)); };

protected:
    MultiListObjectClass *Current_Object() const { return m_curNode->m_object; };

    const GenericMultiListClass *m_list; // List we're working in.
    const MultiListNodeClass *m_curNode; // Node we're currently at.
};

template<typename T> class MultiListIterator;
template<typename T> class MultiListClass : public GenericMultiListClass
{
public:
    MultiListClass() : GenericMultiListClass(){};

    virtual ~MultiListClass()
    {
        while (!Is_Empty()) {
            Remove_Head();
        }
    };

    bool Add(T *obj, bool onlyonce = true) { return Internal_Add(obj, onlyonce); };
    bool Add_Tail(T *obj, bool onlyonce = true) { return Internal_Add_Tail(obj, onlyonce); };

    bool Add_After(T *obj, const T *existing_list_member, bool onlyonce = true)
    {
        return Internal_Add_After(obj, existing_list_member, onlyonce);
    };

    bool Remove(T *obj) { return Internal_Remove(obj); };
    T *Get_Head() const { return (T *)Internal_Get_List_Head(); };
    T *Peek_Head() const { return (T *)Internal_Get_List_Head(); };
    T *Remove_Head() { return (T *)Internal_Remove_List_Head(); };

    void Reset_List()
    {
        while (Peek_Head() != nullptr) {
            Remove_Head();
        };
    };

    MultiListIterator<T> Iterator() const;

private:
    // Not implemented
    MultiListClass(const MultiListClass &that);
    MultiListClass &operator=(const MultiListClass &that);
};

template<typename T> class MultiListIterator : public GenericMultiListIterator
{
public:
    MultiListIterator(const MultiListClass<T> *list) : GenericMultiListIterator(list){};

    T *Get_Obj(void) const { return (T *)Current_Object(); };
    T *Peek_Obj(void) const { return (T *)Current_Object(); };

    void Remove_Current_Object()
    {
        T *obj = Peek_Obj();
        if (obj != nullptr) {
            Next();
            ((MultiListClass<T> *)m_list)->Remove(obj);
        }
    };

    void operator++() { Next(); }
    operator bool() const { return !Is_Done(); }
    T *operator->() { return Get_Obj(); }
    T *operator*() { return Get_Obj(); }
};

template<typename T> MultiListIterator<T> MultiListClass<T>::Iterator() const
{
    return MultiListIterator<T>(this);
};

template<typename T> class RefMultiListIterator;
template<typename T> class RefMultiListClass : public GenericMultiListClass
{
public:
    RefMultiListClass() : GenericMultiListClass(){};

    virtual ~RefMultiListClass()
    {
        while (!Is_Empty()) {
            Release_Head();
        }
    };

    bool Add(T *obj, bool onlyonce = true)
    {
        bool res = Internal_Add(obj, onlyonce);

        if (res == true) {
            obj->Add_Ref();
        }

        return res;
    };

    bool Add_Tail(T *obj, bool onlyonce = true)
    {
        bool res = Internal_Add_Tail(obj, onlyonce);

        if (res == true) {
            obj->Add_Ref();
        }

        return res;
    };

    bool Add_After(T *obj, const T *existing_list_member, bool onlyonce = true)
    {
        bool res = Internal_Add_After(obj, existing_list_member, onlyonce);

        if (res == true) {
            obj->Add_Ref();
        }

        return res;
    };

    bool Remove(T *obj)
    {
        bool res = Internal_Remove(obj);

        if (res == true) {
            obj->Release_Ref();
        }

        return res;
    };

    bool Release_Head()
    {
        T *obj = (T *)Internal_Remove_List_Head();

        if (obj != nullptr) {
            obj->Release_Ref();
            return true;
        }

        return false;
    };

    T *Get_Head() const
    {
        T *obj = (T *)Internal_Get_List_Head();

        if (obj != nullptr) {
            obj->Add_Ref();
        }

        return obj;
    };

    T *Peek_Head() const { return (T *)Internal_Get_List_Head(); };
    T *Remove_Head() { return (T *)Internal_Remove_List_Head(); };

    void Reset_List()
    {
        while (Peek_Head() != nullptr) {
            Release_Head();
        };
    };

    RefMultiListIterator<T> Iterator() const;

private:
    // Not implemented
    RefMultiListClass(const RefMultiListClass &);
    RefMultiListClass &operator=(const RefMultiListClass &);
};

template<typename T> class RefMultiListIterator : public GenericMultiListIterator
{
public:
    RefMultiListIterator(const RefMultiListClass<T> *list) : GenericMultiListIterator(list){};

    T *Get_Obj() const
    {
        T *obj = (T *)Current_Object();

        if (obj != nullptr) {
            obj->Add_Ref();
        }

        return obj;
    };

    T *Peek_Obj() const { return (T *)Current_Object(); };

    void Remove_Current_Object()
    {
        T *obj = Peek_Obj();

        if (obj != nullptr) {
            Next();
            ((RefMultiListClass<T> *)m_list)->Remove(obj);
        };
    };

    void operator++() { Next(); }
    operator bool() const { return !Is_Done(); }
    T *operator->() const { return Peek_Obj(); }
};

template<typename T> RefMultiListIterator<T> RefMultiListClass<T>::Iterator() const
{
    return RefMultiListIterator<T>(this);
};

inline MultiListObjectClass::~MultiListObjectClass()
{
    while (m_listNode != nullptr) {
        m_listNode->m_list->Internal_Remove(this);
    };
}

inline int GenericMultiListClass::Count() const
{
    int counter = 0;
    GenericMultiListIterator it(this);

    for (it.First(); !it.Is_Done(); it.Next()) {
        ++counter;
    }

    return counter;
}

template<class ObjectType> class PriorityMultiListIterator : public MultiListIterator<ObjectType>
{
    using MultiListIterator<ObjectType>::First;
    using MultiListIterator<ObjectType>::Remove_Current_Object;
    using MultiListIterator<ObjectType>::m_curNode;

public:
    PriorityMultiListIterator(MultiListClass<ObjectType> *list) :
        m_originalHead(nullptr), MultiListIterator<ObjectType>(list)
    {
        First();
    }

    bool Process_Head(ObjectType **object)
    {
        bool result = false;

        // Check to ensure we don't wrap around the list (stop after iterating the list once).
        if (m_curNode != nullptr && m_curNode->m_object != nullptr && m_originalHead != m_curNode) {
            if (m_originalHead == nullptr) {
                m_originalHead = m_curNode;
            }

            *object = (ObjectType *)m_curNode->m_object;

            // Remove the node from the head of the list and add it to the tail of the list
            Remove_Current_Object();
            ((MultiListClass<ObjectType> *)PriorityMultiListIterator::m_list)->Add_Tail((*object));

            result = true;
        }

        return result;
    }

protected:
    const MultiListNodeClass *m_originalHead;
};
