/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for overridable objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "mempoolobj.h"

class Overridable : public MemoryPoolObject
{
    IMPLEMENT_POOL(Overridable);

protected:
    virtual ~Overridable() override
    {
        if (m_next != nullptr) {
            delete m_next;
        }
    }

public:
    Overridable() : m_next(nullptr), m_isAllocated(false) {}

    Overridable *Get_Final_Override();
    Overridable *Delete_Overrides();
    void Add_Override(Overridable *obj);

    bool Is_Allocated() { return m_isAllocated; }

protected:
    Overridable *m_next;
    bool m_isAllocated;
};

inline Overridable *Overridable::Get_Final_Override()
{
    Overridable *retval = this;

    while (retval->m_next != nullptr) {
        retval = retval->m_next;
    }

    return retval;
}

inline Overridable *Overridable::Delete_Overrides()
{
    if (m_isAllocated) {
        this->Delete_Instance();

        return nullptr;
    }

    if (m_next != nullptr) {
        m_next = m_next->Delete_Overrides();
    }

    return this;
}

inline void Overridable::Add_Override(Overridable *obj)
{
    if (m_next != nullptr) {
        Get_Final_Override()->m_next = obj;
    } else {
        m_next = obj;
    }
}
