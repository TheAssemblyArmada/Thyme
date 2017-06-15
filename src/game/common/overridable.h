////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: OVERRIDABLE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Appears to provide a base class for objects that can be
//                 Overridden.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef OVERRIDABLE_H
#define OVERRIDABLE_H

#include "mempoolobj.h"

class Overridable : public MemoryPoolObject
{
    IMPLEMENT_POOL(Overridable);
public:
    Overridable() : m_next(nullptr), m_deleteFlag(false) {}
    virtual ~Overridable() { if ( m_next != nullptr ) { delete m_next; } }

    Overridable *Get_Final_Override();
    Overridable *Delete_Overrides();

protected:
    Overridable *m_next;
    bool m_deleteFlag;
};

inline Overridable *Overridable::Get_Final_Override()
{
    Overridable *retval = this;

    while ( retval->m_next != nullptr ) {
        retval = retval->m_next;
    }

    return retval;
}

inline Overridable *Overridable::Delete_Overrides()
{
    if ( m_deleteFlag ) {
        delete this;

        return nullptr;
    }

    if ( m_next != nullptr ) {
        m_next = m_next->Delete_Overrides();
    }

    return this;
}

#endif // OVERRIDABLE_H
