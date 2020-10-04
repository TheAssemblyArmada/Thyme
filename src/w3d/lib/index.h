/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Class for creating a searchable index of objects on a key/value pair basis
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
#include <cstdlib>

template<typename KeyType, typename ValueType> class IndexClass
{
public:
    // a NodeElement is a entry in the indexer, it contains uniqiue m_id and a pointer to the data to index.
    class NodeElement
    {
    public:
        NodeElement(const KeyType &id, const ValueType &data) : m_id(id), m_data(data) {}
        NodeElement() : m_id((KeyType)0), m_data((ValueType) nullptr) {}

        bool operator==(NodeElement const &node) const { return m_id == node.m_id; }
        bool operator!=(NodeElement const &node) const { return m_id != node.m_id; }
        bool operator<(NodeElement const &node) const { return m_id < node.m_id; }
        bool operator>(NodeElement const &node) const { return m_id > node.m_id; }
        bool operator<=(NodeElement const &node) const { return m_id <= node.m_id; }
        bool operator>=(NodeElement const &node) const { return m_id >= node.m_id; }

        KeyType Get_ID() { return m_id; }
        ValueType Get_Data() { return m_data; }

    public:
        KeyType m_id;
        ValueType m_data;
    };

public:
    IndexClass() : m_indexTable(nullptr), m_indexCount(0), m_indexSize(0), m_isSorted(false), m_archive(nullptr) {}
    ~IndexClass() { Clear(); }

    const ValueType &operator[](const KeyType &id) const;
    ValueType Fetch_Index(const KeyType &id) const;
    ValueType &Fetch_Data_By_Index(int index) { return m_indexTable[index].m_data; }
    KeyType Fetch_ID_By_Index(int index) { return m_indexTable[index].m_id; }
    bool Is_Present(const KeyType &id) const;
    bool Add_Index(const KeyType &id, const ValueType &data);
    bool Remove_Index(const KeyType &id);
    void Clear();
    int Count() const { return m_indexCount; }
    bool Increase_Table_Size(int amount);
    void Set_Archive(NodeElement *archive) const { m_archive = archive; }
    bool Is_Archive_Same(const KeyType &id) const { return m_archive != nullptr && m_archive->m_id == id; }
    void Invalidate_Archive() const { m_archive = nullptr; }
    NodeElement *Search_For_Node(const KeyType &id) const;

private:
    // Comparator function for qsort to sort an array and bsearch to search it.
    static int Search_Compare(void const *a, void const *b);

private:
    NodeElement *m_indexTable;
    int m_indexCount;
    int m_indexSize;
    mutable bool m_isSorted;
    mutable NodeElement *m_archive;
};

template<typename KeyType, class ValueType>
const ValueType &IndexClass<KeyType, ValueType>::operator[](const KeyType &id) const
{
    static const ValueType x;

    if (Is_Present(id)) {
        return m_archive->m_data;
    }

    return x;
}

template<typename KeyType, class ValueType> bool IndexClass<KeyType, ValueType>::Is_Present(const KeyType &id) const
{
    if (Count() > 0) {
        if (Is_Archive_Same(id)) {
            return true;
        }

        NodeElement *node = Search_For_Node(id);
        if (node != nullptr) {
            Set_Archive(node);

            return true;
        }
    }

    return false;
}

template<typename KeyType, class ValueType> ValueType IndexClass<KeyType, ValueType>::Fetch_Index(const KeyType &id) const
{
    if (Is_Present(id)) {
        return m_archive->m_data;
    }

    return nullptr;
}

template<typename KeyType, class ValueType>
bool IndexClass<KeyType, ValueType>::Add_Index(const KeyType &id, const ValueType &data)
{
    if (Count() + 1 <= m_indexSize) {
        m_indexTable[m_indexCount].m_id = id;
        m_indexTable[m_indexCount].m_data = data;
        ++m_indexCount;
        m_isSorted = false;

        return true;
    }

    int size = m_indexSize;

    if (size <= 0) {
        size = 10;
    }

    if (Increase_Table_Size(size)) {
        m_indexTable[m_indexCount].m_id = id;
        m_indexTable[m_indexCount].m_data = data;
        ++m_indexCount;
        m_isSorted = false;

        return true;
    }

    return false;
}

template<typename KeyType, class ValueType> bool IndexClass<KeyType, ValueType>::Remove_Index(const KeyType &id)
{
    int pos = -1;
    for (int i = 0; i < m_indexCount; ++i) {
        if (m_indexTable[i].m_id == id) {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        return false;
    }

    for (int i = pos; i < m_indexCount - 1; ++i) {
        m_indexTable[i] = m_indexTable[i + 1];
    }

    m_indexCount--;
    m_indexTable[m_indexCount].m_id = (KeyType)0;
    m_indexTable[m_indexCount].m_data = (ValueType)0;

    Invalidate_Archive();

    return true;
}

template<typename KeyType, class ValueType> void IndexClass<KeyType, ValueType>::Clear()
{
    if (m_indexTable != nullptr) {
        delete[] m_indexTable;
    }

    m_indexTable = nullptr;
    m_indexCount = 0;
    m_indexSize = 0;
    m_isSorted = false;

    Invalidate_Archive();
}

template<typename KeyType, class ValueType> bool IndexClass<KeyType, ValueType>::Increase_Table_Size(int amount)
{
    captainslog_assert(amount > 0);

    if (amount >= 0) {
        int newsize = m_indexSize + amount;
        NodeElement *newindex = new NodeElement[newsize];

        if (newindex != nullptr) {
            captainslog_assert(m_indexCount < newsize);

            for (int i = 0; i < m_indexCount; ++i) {
                newindex[i].m_id = m_indexTable[i].m_id;
                newindex[i].m_data = m_indexTable[i].m_data;
            }

            if (m_indexTable != nullptr) {
                delete[] m_indexTable;
            }

            m_indexTable = nullptr;
            m_indexTable = newindex;
            m_indexSize += amount;
            Invalidate_Archive();

            return true;
        }
    }

    return false;
}

template<typename KeyType, class ValueType>
typename IndexClass<KeyType, ValueType>::NodeElement *IndexClass<KeyType, ValueType>::Search_For_Node(
    const KeyType &id) const
{
    if (Count() > 0) {
        if (!m_isSorted) {
            std::qsort(m_indexTable, m_indexCount, sizeof(NodeElement), Search_Compare);
            Invalidate_Archive();
            m_isSorted = true;
        }

        NodeElement elem;
        elem.m_id = id;

        return (NodeElement *)bsearch(&elem, m_indexTable, m_indexCount, sizeof(NodeElement), Search_Compare);
    }

    return nullptr;
}

template<typename KeyType, class ValueType>
int IndexClass<KeyType, ValueType>::Search_Compare(void const *ptr1, void const *ptr2)
{
    if (static_cast<const NodeElement *>(ptr1)->m_id < static_cast<const NodeElement *>(ptr2)->m_id) {
        return -1;
    }

    if (static_cast<const NodeElement *>(ptr1)->m_id > static_cast<const NodeElement *>(ptr2)->m_id) {
        return 1;
    }

    return 0;
}
