/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Classes for storing hashes in tables.
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

class HashableClass
{
    friend class HashTableClass;
    friend class HashTableIteratorClass;

public:
    HashableClass() : m_nextHash(nullptr) {}
    virtual ~HashableClass() {}
    virtual const char *Get_Key() = 0;

private:
    HashableClass *m_nextHash;
};

class HashTableClass
{
    friend class HashTableIteratorClass;

public:
    HashTableClass(int32_t size);
    ~HashTableClass();
    void Reset();
    void Add(HashableClass *entry);
    bool Remove(HashableClass *entry);
    HashableClass *Find(char const *key);

private:
    int32_t Hash(const char *key);

private:
    int32_t m_hashTableSize;
    HashableClass **m_hashTable;
};

class HashTableIteratorClass
{
public:
    HashTableIteratorClass(const HashTableIteratorClass &that) :
        m_table(that.m_table), m_index(that.m_index), m_currentEntry(that.m_currentEntry), m_nextEntry(that.m_nextEntry)
    {
    }
    HashTableIteratorClass(HashTableClass &table) : m_table(table) {}
    virtual ~HashTableIteratorClass() {}
    void First();
    void Next();
    bool Is_Done() { return m_currentEntry == nullptr; } // i think, based on enb..
    HashableClass *Get_Current() { return m_currentEntry; }

private:
    void Advance_Next();

private:
    HashTableClass &m_table;
    int32_t m_index;
    HashableClass *m_currentEntry;
    HashableClass *m_nextEntry;
};
