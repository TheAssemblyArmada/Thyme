/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Hashed animation manager.
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
#include "chunkio.h"
#include "hash.h"
#include "missing.h"
#include "vector.h"
#include "wwstring.h"

class W3DExclusionListClass;
class HAnimClass;

class HAnimManagerClass
{
public:
    HAnimManagerClass();
    ~HAnimManagerClass();

    int Load_Anim(ChunkLoadClass &cload);
    int Load_Morph_Anim(ChunkLoadClass &cload);
    int Load_Raw_Anim(ChunkLoadClass &cload);
    int Load_Compressed_Anim(ChunkLoadClass &cload);
    HAnimClass *Get_Anim(const char *name);
    void Free_All_Anims();
    void Free_All_Anims_With_Exclusion_List(const W3DExclusionListClass &exclude_list);
    void Create_Asset_List(DynamicVectorClass<StringClass> &list);
    unsigned char Add_Anim(HAnimClass *new_anim);
    void Register_Missing(const char *name);
    unsigned char Is_Missing(const char *name);
    void Reset_Missing();
    HAnimClass *Peek_Anim(const char *name);

private:
    HashTableClass *m_animPtrTable;
    HashTableClass *m_missingAnimTable;
};

class HAnimManagerIterator : public HashTableIteratorClass
{
public:
    HAnimManagerIterator(HAnimManagerIterator &iterator) : HashTableIteratorClass(iterator) {}
    virtual ~HAnimManagerIterator() {}

    HAnimClass *Get_Current_Anim()
    {
        if (Get_Current() != nullptr) {
            return (HAnimClass *)Get_Current();
        }

        return nullptr;
    }
};
