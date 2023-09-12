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
#include "hash.h"
#include "realcrc.h"
#include <captainslog.h>
#include <strings.h>

/**
 * 0x008A0900
 */
HashTableClass::HashTableClass(int size)
{
    m_hashTableSize = size;
    captainslog_assert((m_hashTableSize & (m_hashTableSize - 1)) == 0);
    m_hashTable = new HashableClass *[m_hashTableSize];
    Reset();
}

/**
 * 0x008A0940
 */
HashTableClass::~HashTableClass()
{
    if (m_hashTable) {
        delete[](m_hashTable);
    }
    m_hashTable = nullptr;
}

/**
 * Reset hash table.
 *
 * 0x008A0960
 */
void HashTableClass::Reset()
{
    for (int i = 0; i < m_hashTableSize; ++i) {
        m_hashTable[i] = nullptr;
    }
}

/**
 * Add entry to the hash table.
 *
 * 0x008A0980
 */
void HashTableClass::Add(HashableClass *entry)
{
    captainslog_assert(entry != nullptr);
    unsigned int hash = Hash(entry->Get_Key());
    entry->m_nextHash = m_hashTable[hash];
    m_hashTable[hash] = entry;
}

/**
 * Remove entry from hash table.
 *
 * 0x008A09C0
 */
bool HashTableClass::Remove(HashableClass *entry)
{
    captainslog_assert(entry != nullptr);
    int hash = Hash(entry->Get_Key());

    if (!m_hashTable[hash]) {
        return false;
    }
    if (m_hashTable[hash] == entry) {
        m_hashTable[hash] = entry->m_nextHash;
        return true;
    }
    for (HashableClass *i = m_hashTable[hash]; i->m_nextHash; i = i->m_nextHash) {
        if (i->m_nextHash == entry) {
            i->m_nextHash = entry->m_nextHash;
            return true;
        }
    }
    return false;
}

/**
 * Try finding the hash that matches the specified string.
 *
 * 0x008A0A30
 */
HashableClass *HashTableClass::Find(const char *key)
{
    for (HashableClass *i = m_hashTable[Hash(key)]; i; i = i->m_nextHash) {
        if (!strcasecmp(i->Get_Key(), key)) {
            return i;
        }
    }
    return nullptr;
}

/**
 * Generate a checksum from the specified string.
 */
int HashTableClass::Hash(const char *key)
{
    return (m_hashTableSize - 1) & CRC_Stringi(key, 0);
}

/**
 *
 *
 * 0x008A0A90
 */
void HashTableIteratorClass::First()
{
    m_index = 0;
    m_nextEntry = m_table.m_hashTable[m_index];
    Advance_Next();
    Next();
}

/**
 *
 *
 * 0x008A0AF0
 */
void HashTableIteratorClass::Next()
{
    m_currentEntry = m_nextEntry;
    if (m_nextEntry) {
        m_nextEntry = m_nextEntry->m_nextHash;
        Advance_Next();
    }
}

/**
 *
 */
void HashTableIteratorClass::Advance_Next()
{
    while (!m_nextEntry) {
        if (++m_index >= m_table.m_hashTableSize) {
            break;
        }
        m_nextEntry = m_table.m_hashTable[m_index];
    }
}
