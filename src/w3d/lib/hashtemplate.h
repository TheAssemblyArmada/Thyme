/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Hashmap like container.
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
#include <algorithm>
#include <captainslog.h>

using std::max;

template<typename Key, typename Value> class HashTemplateIterator;

template<typename Key> class HashTemplateKeyClass
{
public:
    static unsigned Get_Hash_Value(const Key &key) { return key.Get_Hash(); }
};

template<typename Key, typename Value> class HashTemplateClass
{

    friend HashTemplateIterator<Key, Value>;

    struct Entry
    {
        int32_t next;
        Key key;
        Value value;
    };

public:
    HashTemplateClass() : m_hash(nullptr), m_table(nullptr), m_first(-1), m_size(0) {}

    ~HashTemplateClass()
    {
        delete[] m_hash;
        delete[] m_table;
    }

    Value Get(const Key &key) const
    {
        if (m_hash != nullptr) {
            for (unsigned index = m_hash[Get_Hash_Val(key)]; index != -1; index = m_table[index].next) {
                if (m_table[index].key == key) {
                    return m_table[index].value;
                }
            }
        }

        return 0;
    }

    bool Exists(const Key &key) const { return Get(key) != 0; }

    void Insert(const Key &key, const Value &value)
    {
        int32_t index = Alloc_Entry();

        uint32_t hash = Get_Hash_Val(key);
        m_table[index].key = key;
        m_table[index].value = value;
        m_table[index].next = m_hash[hash];
        m_hash[hash] = index;
    }

    void Remove(const Key &key)
    {
        uint32_t hash = Get_Hash_Val(key);
        uint32_t index2 = -1;

        if (m_hash != nullptr) {
            for (uint32_t index = m_hash[hash]; index != -1; index = m_table[index].next) {

                if (m_table[index].key == key) {
                    if (index2 == -1) {
                        m_hash[hash] = m_table[index].next;
                    } else {
                        m_table[index2].next = m_table[index].next;
                    }

                    m_table[index].next = m_first;
                    m_first = index;
                    break;
                }
                index2 = index;
            }
        }
    }

    void Remove_All()
    {
        for (uint32_t hash = 0; hash < m_size; hash++) {
            uint32_t index = m_hash[hash];

            if (index != -1) {
                int32_t new_index = m_hash[hash];

                for (int32_t j = m_table[index].next; j != -1; j = m_table[j].next) {
                    new_index = j;
                }

                m_table[new_index].next = m_first;
                m_first = index;
                m_hash[hash] = -1;
            }
        }
    }

private:
    static uint32_t Get_Hash_Val(const Key &key, uint32_t max_size)
    {
        // Make sure max_size is a power of two, or the fast modulo code below will not work
        captainslog_assert((max_size % 2) == 0);
        return HashTemplateKeyClass<Key>::Get_Hash_Value(key) & (max_size - 1);
    }

    uint32_t Get_Hash_Val(const Key &key) const { return Get_Hash_Val(key, m_size); }

    void Re_Hash()
    {
        unsigned new_size = max(m_size * 2, 4u); // Increase the size and make sure there are at least 4 entries.

        Entry *new_table = new Entry[new_size];
        int32_t *new_index = new int32_t[new_size];
        uint32_t new_first = 0;

        for (unsigned index = 0; index < new_size; ++index) {
            new_table[index].next = -1;
            new_index[index] = -1;
        }

        for (uint32_t hash = 0; hash < m_size; ++hash) {
            for (int32_t index = m_hash[hash]; index != -1; index = m_table[index].next) {
                uint32_t hash2 = Get_Hash_Val(m_table[index].key, new_size);

                new_table[new_first].next = new_index[hash2];
                new_table[new_first].key = m_table[index].key;
                new_table[new_first].value = m_table[index].value;

                new_index[hash2] = new_first;
                new_first++;
            }
        }

        delete[] m_hash;
        delete[] m_table;

        for (uint32_t i = new_first; i < new_size - 1; ++i) {
            new_table[i].next = i + 1;
        }

        new_table[new_size - 1].next = -1;

        m_size = new_size;
        m_first = new_first;
        m_hash = new_index;
        m_table = new_table;
    }

    int32_t Alloc_Entry()
    {
        if (m_first == -1) {
            Re_Hash();
        }

        int32_t ret = m_first;
        m_first = m_table[ret].next;
        return ret;
    }

    int32_t *m_hash;
    Entry *m_table;
    int32_t m_first;
    uint32_t m_size;
};

template<typename Key, typename Value> class HashTemplateIterator
{
public:
    HashTemplateIterator(HashTemplateClass<Key, Value> &_table)
    {
        m_hashTable = &_table;
        First();
    }

    void First()
    {
        m_handle = -1;
        int32_t size = m_hashTable->m_size;

        for (m_hashIndex = 0; m_hashIndex < size; m_hashIndex++) {
            m_handle = m_hashTable->m_hash[m_hashIndex];

            if (m_handle != -1) {
                break;
            }
        }
    }

    void Next()
    {
        m_handle = m_hashTable->m_table[m_handle].next;
        int32_t size = m_hashTable->m_size;

        for (m_hashIndex++; m_hashIndex < size; m_hashIndex++) {
            m_handle = m_hashTable->m_hash[m_hashIndex];

            if (m_handle != -1) {
                break;
            }
        }
    }

    bool Is_Done() { return m_hashIndex == m_hashTable->m_size; }

    Value &Peek_Value() { return m_hashTable->m_table[m_handle].value; }

private:
    int32_t m_hashIndex;
    int32_t m_handle;
    HashTemplateClass<Key, Value> *m_hashTable;
};
