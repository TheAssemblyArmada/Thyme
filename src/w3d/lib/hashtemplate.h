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
#include "gamedebug.h"
#include <algorithm>

using std::max;

template<typename Key>
class HashTemplateKeyClass
{
public:
    static unsigned Get_Hash_Value(const Key &key) { return key.Get_Hash(); }
};

template<typename Key, typename Value>
class HashTemplateClass
{
    friend class HashTemplateIterator<Key, Value>;

    struct Entry
    {
        int next;
        Key key;
        Value value;
    };

public:
    HashTemplateClass() : m_index(nullptr), m_table(nullptr), m_nextFree(-1), m_size(0) {}

    ~HashTemplateClass()
    {
        delete[] m_index;
        delete[] m_table;
    }

    Value *Get(const Key &key) const
    {
        if (m_index != nullptr) {
            for (unsigned index = m_index[Get_Hash_Val(key)]; index != -1; index = m_table[index].next) {
                if (m_table[index].key == key) {
                    return &m_table[index].value;
                }
            }
        }

        return nullptr;
    }

    Value Get(const Key &key, Value default) const
    {
        Value *result = Get(key);
        return result ? *result : default;
    }

    bool Exists(const Key &key) const { return Get(key) != nullptr; }

    void Insert(const Key &key, const Value &value)
    {
        // If all entries are in use, enlarge the table
        if (m_nextFree == -1) {
            Re_Hash();
        }

        unsigned index = m_nextFree;
        m_nextFree = m_table[index].next;

        unsigned hash = Get_Hash_Val(key);
        m_table[index].key = key;
        m_table[index].value = value;
        m_table[index].next = m_index[hash];
        m_index[hash] = index;
    }

    void Remove(const Key &key)
    {
        if (m_index != nullptr) {
            unsigned hash = Get_Hash_Val(key);
            int *last_next_entry = &m_index[hash];

            for (unsigned index = m_index[hash]; index != -1; index = m_table[index].next) {
                Entry &entry = m_table[index];

                if (entry.key == key) {
                    *last_next_entry = entry.next;
                    m_table[index].next = m_nextFree;
                    m_nextFree = index;

                    break;
                }

                last_next_entry = &m_table[index].next;
            }
        }
    }

    // Remove if keys and values match
    void Remove(const Key &key, const Value &value)
    {
        if (m_index != nullptr) {
            unsigned hash = Get_Hash_Val(key);
            int *lastEntryNext = &m_index[hash];

            for (unsigned index = m_index[hash]; index != -1; index = m_table[index].next) {
                Entry &entry = m_table[index];

                if (entry.key == key && entry.value == value) {
                    *lastEntryNext = entry.next;
                    m_table[index].next = m_nextFree;
                    m_nextFree = index;

                    break;
                }

                lastEntryNext = &m_table[index].next;
            }
        }
    }

    void Remove_All()
    {
        for (unsigned hash = 0; hash < m_size; hash++) {
            unsigned index = m_index[hash];

            if (index != -1) {
                unsigned lastHash = index;

                while (m_table[lastHash].next != -1) {
                    lastHash = m_table[lastHash].next;
                }

                m_table[lastHash].next = m_nextFree;
                m_nextFree = index;
                m_index[hash] = -1;
            }
        }
    }

    unsigned Get_Size() const
    {
        unsigned result = 0;

        for (unsigned hash = 0; hash < m_size; hash++) {
            unsigned index = m_index[hash];

            while (index != -1) {
                ++result;
                index = m_table[index].next;
            }
        }

        return result;
    }

    const Value Get(int index) const
    {
        Value value = m_table[index].value;

        return value;
    }

private:
    static unsigned Get_Hash_Val(const Key &key, unsigned max_size)
    {
        // Make sure max_size is a power of two, or the fast modulo code below will not work
        GAME_ASSERT((max_size % 2) == 0);
        return HashTemplateKeyClass<Key>::Get_Hash_Value(key) & (max_size - 1);
    }

    unsigned Get_Hash_Val(const Key &key) const { return Get_Hash_Val(key, m_size); }

    void Re_Hash()
    {
        unsigned new_size = max(m_size * 2, 4); // Increase the size and make sure there are at least 4 entries.

        Entry *new_table = new Entry[new_size];
        int *new_index = new int[new_size];
        m_nextFree = 0;

        for (unsigned index = 0; index < new_size; ++index) {
            new_index[index] = -1;
        }

        for (unsigned hash = 0; hash < m_size; ++hash) {
            for (int index = m_index[hash]; index != -1; index = m_table[index].next) {
                unsigned hash2 = Get_Hash_Val(m_table[index].key, new_size);

                new_table[m_nextFree].next = new_index[hash2];
                new_table[m_nextFree].key = m_table[index].key;
                new_table[m_nextFree].value = m_table[index].value;

                new_index[hash2] = m_nextFree;
                m_nextFree++;
            }
        }

        delete[] m_index;
        delete[] m_table;

        for (unsigned i = m_nextFree; i < new_size - 1; ++i) {
            new_table[i].next = i + 1;
        }

        new_table[new_size - 1].next = -1;

        m_index = new_index;
        m_table = new_table;
        m_size = new_size;
    }

private:
    int *m_index;
    Entry *m_table;
    int m_nextFree;
    unsigned m_size;
};
