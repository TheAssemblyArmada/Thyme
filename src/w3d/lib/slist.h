/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief SList class
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

template<typename T> class SList;

class GenericSLNode : public AutoPoolClass<GenericSLNode, 256>
{
protected:
    void *Internal_Get_Next() { return m_nodeNext; };
    void Internal_Set_Next(void *n) { m_nodeNext = n; };
    void *Internal_Get_Data() { return m_nodeData; };
    void Internal_Get_Data(void *d) { m_nodeData = d; };

    GenericSLNode(void *obj)
    {
        m_nodeData = obj;
        m_nodeNext = nullptr;
    };

private:
    GenericSLNode(){};

    void *m_nodeNext;
    void *m_nodeData;
};

template<typename T> class SLNode : public GenericSLNode
{
public:
    friend class SList<T>;

    SLNode<T> *Next() { return reinterpret_cast<SLNode<T> *>(Internal_Get_Next()); }
    T *Data() { return reinterpret_cast<T *>(Internal_Get_Data()); }
    void Set_Next(SLNode<T> *n) { Internal_Set_Next(reinterpret_cast<void *>(n)); }

protected:
    SLNode(T *obj) : GenericSLNode(obj) {}

private:
    SLNode(){};
};

template<class T> class SList
{
public:
    SList()
    {
        m_headNode = nullptr;
        m_tailNode = nullptr;
    };
    virtual ~SList() { Remove_All(); };
    SLNode<T> *Head() const;
    SLNode<T> *Tail() const;
    SLNode<T> *Find_Node(T *data) const;
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
    virtual bool Is_Empty() const;
    virtual long Get_Count() const;

private:
    SLNode<T> *m_headNode;
    SLNode<T> *m_tailNode;
};
