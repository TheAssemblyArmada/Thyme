/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief w3d exclusion list
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dexclusionlist.h"
#include "hanim.h"
#include "htree.h"
#include "proto.h"

W3DExclusionListClass::W3DExclusionListClass(DynamicVectorClass<StringClass> const &list) : m_list(&list)
{
    for (int i = 0; i < list.Count(); i++) {
        m_hash.Insert(list[i], i);
    }
}

W3DExclusionListClass::~W3DExclusionListClass()
{
    m_hash.Remove_All();
}

bool W3DExclusionListClass::Is_Excluded(HAnimClass *anim) const
{
    StringClass str = anim->Get_Name();
    const char *c = strrchr(str, '.');

    if (c) {
        return Is_Excluded(c + 1);
    }

    return false;
}

bool W3DExclusionListClass::Is_Excluded(HTreeClass *tree) const
{
    return Is_Excluded(tree->Get_Name());
}

bool W3DExclusionListClass::Is_Excluded(PrototypeClass *proto) const
{
    StringClass str = proto->Get_Name();

    if (strchr(str, '#')) {
        return false;
    } else {
        char *c = const_cast<char *>(strrchr(str, '.'));

        if (c) {
            *c = 0;
        }

        return Is_Excluded(str);
    }
}

bool W3DExclusionListClass::Is_Excluded(char const *name) const
{
    StringClass str = name;
    return m_hash.Exists(str);
}
