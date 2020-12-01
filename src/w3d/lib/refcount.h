/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 * @author tomsons26
 *
 * @brief Base class for objects that need to be reference counted.
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

class RefCountClass
{
public:
    RefCountClass() : m_numRefs(1)
    {
#ifdef GAME_DEBUG
        Inc_Total_Refs(this);
#endif
    }

    RefCountClass(const RefCountClass &) : m_numRefs(1)
    {
#ifdef GAME_DEBUG
        Inc_Total_Refs(this);
#endif
    }

    void Add_Ref() { m_numRefs++; }
    int Num_Refs() { return m_numRefs; }

    void Release_Ref()
    {
#ifdef GAME_DEBUG
        Dec_Total_Refs(this);
#endif
        --m_numRefs;

        if (m_numRefs == 0) {
            Delete_This();
        }
    }

    virtual void Delete_This() { delete this; }

protected:
    virtual ~RefCountClass() {}

private:
    static void Inc_Total_Refs(RefCountClass *obj) { m_totalRefs++; }
    static void Dec_Total_Refs(RefCountClass *obj) { m_totalRefs--; }
    int m_numRefs;
    static int m_totalRefs;
};

template<typename T> void Ref_Ptr_Set(T *&dst, T *&src)
{
    if (src != nullptr) {
        src->Add_Ref();
    }

    if (dst != nullptr) {
        dst->Release_Ref();
    }

    dst = src;
}

template<typename T> void Ref_Ptr_Release(T *&ptr)
{
    if (ptr != nullptr) {
        ptr->Release_Ref();
    }

    ptr = nullptr;
}
