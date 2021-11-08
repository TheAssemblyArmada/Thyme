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
#include "w3dexclusionlist.h"
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
int HTreeManagerClass::Load_Tree(ChunkLoadClass &cload)
{
    HTreeClass *tree = new HTreeClass();

    if (tree->Load_W3D(cload) || Get_Tree_ID(tree->Get_Name()) != -1) {
        delete tree;
        return 1;
    }

    m_treePtr[m_numTrees] = tree;
    m_numTrees++;
    StringClass str = tree->Get_Name();
    str.To_Lower();
    m_hashTable.Insert(str, tree);
    return 0;
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
    return m_hashTable.Get(key);
}

int HTreeManagerClass::Get_Tree_ID(const char *name)
{
    for (auto i = 0; i < m_numTrees; ++i) {
        auto &tree = m_treePtr[i];
        if (tree != nullptr && strcasecmp(tree->Get_Name(), name) == 0) {
            return i;
        }
    }
    return -1;
}

void HTreeManagerClass::Free_All_Trees_With_Exclusion_List(const W3DExclusionListClass &list)
{
    int count = 0;

    for (auto i = 0; i < m_numTrees; ++i) {
        auto &tree = m_treePtr[i];

        if (list.Is_Excluded(tree)) {
            count++;
        } else {
            delete m_treePtr[i];
            m_treePtr[i] = nullptr;
        }
    }

    m_numTrees = count;
    m_hashTable.Remove_All();

    for (auto &tree : m_treePtr) {
        if (tree != nullptr) {
            StringClass str = tree->Get_Name();
            str.To_Lower();
            m_hashTable.Insert(str, tree);
        }
    }
}
