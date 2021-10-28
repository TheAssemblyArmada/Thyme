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
#pragma once
#include "always.h"
#include "hashtemplate.h"
#include "vector.h"
#include "wwstring.h"

class HAnimClass;
class HTreeClass;
class PrototypeClass;

class W3DExclusionListClass
{
public:
    W3DExclusionListClass(DynamicVectorClass<StringClass> const &list);
    ~W3DExclusionListClass();
    bool Is_Excluded(HAnimClass *anim) const;
    bool Is_Excluded(HTreeClass *tree) const;
    bool Is_Excluded(PrototypeClass *proto) const;
    bool Is_Excluded(char const *name) const;

private:
    const DynamicVectorClass<StringClass> *m_list;
    HashTemplateClass<StringClass, int> m_hash;
};
