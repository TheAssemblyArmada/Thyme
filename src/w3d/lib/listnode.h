/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Linked list implementation templates.
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
#include <captainslog.h>

class GenericList;

class GenericNode
{
public:
    GenericNode() : m_nextNode(nullptr), m_prevNode(nullptr) {}
    GenericNode(GenericNode &node) { node.Link(this); }
    virtual ~GenericNode() { Unlink(); }

    GenericNode &operator=(GenericNode &node);
    void Unlink();
    GenericList *Main_List() const;
    void Link(GenericNode *node);
    GenericNode *Next() const { return m_nextNode; }
    GenericNode *Next_Valid() const { return ((m_nextNode && m_nextNode->m_nextNode) ? m_nextNode : nullptr); }
    GenericNode *Prev() const { return m_prevNode; }
    GenericNode *Prev_Valid() const { return ((m_prevNode && m_prevNode->m_prevNode) ? m_prevNode : nullptr); }
    bool Is_Valid() const { return (m_nextNode != nullptr && m_prevNode != nullptr); }

protected:
    GenericNode *m_nextNode;
    GenericNode *m_prevNode;
};

inline GenericNode &GenericNode::operator=(GenericNode &node)
{
    if (this != &node) {
        node.Link(this);
    }

    return *this;
}

/**
 * @brief Removes this element from the list.
 *
 * @info This function is safe to call even if the element is not linked.
 */
inline void GenericNode::Unlink()
{
    // note that this means that the special generic node at the head
    // and tail of the list can not be unlinked.  This is done because
    // the user should never unlink them -- it will destroy the list in
    // an evil way.
    if (Is_Valid()) {
        m_prevNode->m_nextNode = m_nextNode;
        m_nextNode->m_prevNode = m_prevNode;

        // Make it safe to call Unlink again.
        m_prevNode = nullptr;
        m_nextNode = nullptr;
    }
}

inline GenericList *GenericNode::Main_List() const
{
    GenericNode const *node = this;

    while (node->m_prevNode) {
        node = m_prevNode;
    }

    return (GenericList *)node;
}

/**
 * @brief Adds this element to a list, before the given element.
 */
inline void GenericNode::Link(GenericNode *node)
{
    captainslog_assert(node != nullptr);

    node->Unlink();
    node->m_nextNode = m_nextNode;
    node->m_prevNode = this;

    if (m_nextNode) {
        m_nextNode->m_prevNode = node;
    }

    m_nextNode = node;
}

class GenericList
{
public:
    GenericList() { m_firstNode.Link(&m_lastNode); }
    virtual ~GenericList();

    GenericNode *First() const { return m_firstNode.Next(); }
    GenericNode *First_Valid() const;
    GenericNode *Last() const { return m_lastNode.Prev(); }
    GenericNode *Last_Valid() const;
    bool Is_Empty() const { return !m_firstNode.Next()->Is_Valid(); }
    void Add_Head(GenericNode *node) { m_firstNode.Link(node); }
    void Add_Tail(GenericNode *node) { m_lastNode.Prev()->Link(node); }
    int Get_Valid_Count() const;
    // Removes all nodes from the list.
    // void Delete() { while ( m_firstNode.Next()->Is_Valid() ) delete m_firstNode.Next(); }

private:
    GenericList(GenericList &list);
    GenericList &operator=(GenericList const &that);

protected:
    GenericNode m_firstNode;
    GenericNode m_lastNode;
};

inline GenericList::~GenericList()
{
    while (m_firstNode.Next()->Is_Valid()) {
        m_firstNode.Next()->Unlink();
    }
}

inline GenericNode *GenericList::Last_Valid() const
{
    GenericNode *node = m_lastNode.Prev();

    return node->Prev() ? node : nullptr;
}

inline GenericNode *GenericList::First_Valid() const
{
    GenericNode *node = m_firstNode.Next();

    return (node->Next() != nullptr ? node : nullptr);
}

inline int GenericList::Get_Valid_Count() const
{
    int counter = 0;
    GenericNode *node = First_Valid();

    while (node) {
        counter++;
        node = node->Next_Valid();
    }

    return counter;
}

template<class T> class List;

template<class T> class Node : public GenericNode
{
public:
    List<T> *Main_List() const { return (List<T> *)GenericNode::Main_List(); }
    T Next() const { return (T)GenericNode::Next(); }
    T Next_Valid() const { return (T)GenericNode::Next_Valid(); }
    T Prev() const { return (T)GenericNode::Prev(); }
    T Prev_Valid() const { return (T)GenericNode::Prev_Valid(); }
    bool Is_Valid() const { return GenericNode::Is_Valid(); }
};

template<class T> class List : public GenericList
{
public:
    List() {}
    T First() const { return (T)GenericList::First(); }
    T First_Valid() const { return (T)GenericList::First_Valid(); }
    T Last() const { return (T)GenericList::Last(); }
    T Last_Valid() const { return (T)GenericList::Last_Valid(); }
    void Delete()
    {
        while (First()->Is_Valid())
            delete First();
    }

private:
    List(List<T> const &rvalue);
    List<T> operator=(List<T> const &rvalue);
};
