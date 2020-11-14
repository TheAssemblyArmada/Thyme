/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Single linked list node.
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

template<class T> class SList;

class GenericSLNode : public AutoPoolClass<GenericSLNode, 256>
{
protected:
    GenericSLNode(void *obj)
    {
        m_data = obj;
        m_next = nullptr;
    }

    GenericSLNode(void *data, GenericSLNode *next)
    {
        m_data = data;
        m_next = next;
    }

protected:
    void *m_next;
    void *m_data;
};

template<class T> class SLNode : public GenericSLNode
{
    friend class SList<T>;

public:
    void Set_Next(SLNode<T> *n) { m_next = n; }
    SLNode<T> *Next() const { return reinterpret_cast<SLNode<T> *>(m_next); }
    T *Data() const { return reinterpret_cast<T *>(m_data); }

protected:
    // Only a list object can make nodes.
    SLNode(T *obj) : GenericSLNode(obj) {}
    SLNode(T *data, SLNode<T> *next) : GenericSLNode(data, next) {}
};