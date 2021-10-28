/**
 * @file
 *
 * @author Jonathan Wilson
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
#pragma once

#include "always.h"
#include "hashtemplate.h"
#include "htree.h"
#include "wwstring.h"

class ChunkLoadClass;
class W3DExclusionListClass;

class HTreeManagerClass
{
public:
    HTreeManagerClass();
    ~HTreeManagerClass();
    int Load_Tree(ChunkLoadClass &cload);
    void Free_All_Trees();
    HTreeClass *Get_Tree(int id);
    HTreeClass *Get_Tree(const char *name);
    int Get_Tree_ID(const char *name);
    void Free_All_Trees_With_Exclusion_List(const W3DExclusionListClass &list);

private:
    int m_numTrees;
    HTreeClass *m_treePtr[16000];
    HashTemplateClass<StringClass, HTreeClass *> m_hashTable;
};
