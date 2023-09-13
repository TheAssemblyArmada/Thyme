/**
 * @file
 *
 * @author Tiberian Technologies
 *
 * @brief Double linked list class.
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
#include "w3dmpo.h"

template<typename T> class DLNodeClass;

template<typename T> class DLListClass
{
    friend DLNodeClass<T>;

public:
    DLListClass() : m_head(nullptr), m_tail(nullptr) {}
    virtual ~DLListClass() {}

    void Add_Head(DLNodeClass<T> *node);
    void Remove_Head();
    void Add_Tail(DLNodeClass<T> *node);
    void Remove_Tail();

    T *Head() { return static_cast<T *>(m_head); }
    T *Tail() { return static_cast<T *>(m_tail); }
    const T *Const_Head() const { return static_cast<const T *>(m_head); }
    const T *Const_Tail() const { return static_cast<const T *>(m_tail); }

private:
    DLNodeClass<T> *m_head;
    DLNodeClass<T> *m_tail;
};

template<typename T> class DLDestroyListClass : public DLListClass<T>
{
    using DLListClass<T>::Head;

public:
    virtual ~DLDestroyListClass()
    {
        for (T *t = Head(); t != nullptr; t = Head()) {
            delete t;
        }
    }
};

template<typename T> class DLNodeClass : public W3DMPO
{
    friend DLListClass<T>;

public:
    DLNodeClass() : m_succ(0), m_pred(0), m_list(0) {}
    virtual ~DLNodeClass() { Remove(); }

    void Insert_Before(DLNodeClass<T> *n);
    void Insert_After(DLNodeClass<T> *n);
    void Remove();

    T *Succ() { return static_cast<T *>(m_succ); }
    T *Pred() { return static_cast<T *>(m_pred); }
    const T *Const_Succ() const { return static_cast<const T *>(m_succ); }
    const T *Const_Pred() const { return static_cast<const T *>(m_pred); }
    DLListClass<T> *List() { return m_list; }

private:
    DLNodeClass<T> *m_succ;
    DLNodeClass<T> *m_pred;
    DLListClass<T> *m_list;
};

// Start of DLListClass<T> methods.

template<typename T> void DLListClass<T>::Add_Head(DLNodeClass<T> *n)
{
    n->m_list = this;

    if (m_head != nullptr) {
        n->Insert_Before(m_head);
        m_head = n;
    } else {
        m_tail = n;
        m_head = n;
        n->m_succ = nullptr;
        n->m_pred = nullptr;
    }
}

template<typename T> void DLListClass<T>::Add_Tail(DLNodeClass<T> *n)
{
    n->m_list = this;

    if (m_tail) {
        n->Insert_After(m_tail);
        m_tail = n;
    } else {
        m_tail = n;
        m_head = n;
        n->m_succ = nullptr;
        n->m_pred = nullptr;
    }
}

template<typename T> void DLListClass<T>::Remove_Head()
{
    if (m_head == nullptr) {
        return;
    }

    DLNodeClass<T> *n = m_head;
    m_head = m_head->Succ();

    if (m_head == nullptr) {
        m_tail = m_head;
    } else {
        m_head->m_pred = nullptr;
    }

    n->Remove();
}

template<typename T> void DLListClass<T>::Remove_Tail()
{
    if (m_tail == nullptr) {
        return;
    }

    DLNodeClass<T> *n = m_tail;
    m_tail = m_tail->Pred();

    if (m_tail == nullptr) {
        m_head = m_tail;
    } else {
        m_tail->m_succ = nullptr;
    }

    n->Remove();
}

// Start of DLNodeClass<T> methods.

template<typename T> void DLNodeClass<T>::Insert_Before(DLNodeClass<T> *n)
{
    m_list = n->m_list;
    m_succ = n;
    m_pred = n->m_pred;

    if (n->m_pred != nullptr) {
        n->m_pred->m_succ = this;
    }

    n->m_pred = this;

    if (m_list->m_head == n) {
        m_list->m_head = this;
    }
}

template<typename T> void DLNodeClass<T>::Insert_After(DLNodeClass<T> *n)
{
    m_list = n->m_list;
    m_pred = n;
    m_succ = n->m_succ;

    if (n->m_succ != nullptr) {
        n->m_succ->m_pred = this;
    }

    n->m_succ = this;

    if (m_list->m_tail == n) {
        m_list->m_tail = this;
    }
}

template<typename T> void DLNodeClass<T>::Remove()
{
    if (m_list == nullptr) {
        return;
    }

    if (m_list->Head() == this) {
        DLListClass<T> *tmp_list = m_list;
        m_list = nullptr;
        tmp_list->Remove_Head();

        return;
    }

    if (m_list->Tail() == this) {
        DLListClass<T> *tmp_list = m_list;
        m_list = nullptr;
        tmp_list->Remove_Tail();

        return;
    }

    if (m_succ != nullptr) {
        m_succ->m_pred = m_pred;
    }

    if (m_pred != nullptr) {
        m_pred->m_succ = m_succ;
    }

    m_list = nullptr;
}
