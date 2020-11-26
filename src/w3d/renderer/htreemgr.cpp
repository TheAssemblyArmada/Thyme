/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Hierarchy Tree Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "htreemgr.h"
#include "chunkio.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

// 0x008517E0
HTreeManagerClass::HTreeManagerClass() : m_numTrees(0)
{
    for (auto &tree : m_treePtr) {
        tree = nullptr;
    }
}

// 0x00851820
HTreeManagerClass::~HTreeManagerClass()
{
    Free_All_Trees();
}

// 0x00851C30
bool HTreeManagerClass::Load_Tree(ChunkLoadClass &cload)
{
#ifdef GAME_DLL
    return Call_Method<bool, HTreeManagerClass, ChunkLoadClass &>(0x00851C30, this, cload);
#else
    return 0;
#endif
}

// 0x00851930
void HTreeManagerClass::Free_All_Trees()
{
    m_hashTable.Remove_All();
    for (auto &tree : m_treePtr) {
        delete tree;
        tree = nullptr;
    }
    m_numTrees = 0;
}

// 0x00852000
HTreeClass *HTreeManagerClass::Get_Tree(int id)
{
    // Not used
    if (id < 0) {
        return nullptr;
    }
    if (id >= m_numTrees) {
        return nullptr;
    }
    return m_hashTable.Get(id);
}

// 0x00851EB0
HTreeClass *HTreeManagerClass::Get_Tree(const char *name)
{
    StringClass key{ name };
    key.To_Lower();
    auto *value = m_hashTable.Get(key);
    return value == nullptr ? nullptr : *value;
}

int HTreeManagerClass::Get_Tree_ID(const char *name)
{
    for (auto i = 0; i < m_numTrees; ++i) {
        auto &tree = m_treePtr[i];
        if (tree == nullptr) {
            continue;
        }

        if (stricmp(tree->Get_Name(), name) == 0) {
            return i;
        }
    }
    return -1;
}
