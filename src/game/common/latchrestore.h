/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Latch Restore
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

template<class T> class LatchRestore
{
public:
    LatchRestore(T *pointer, T *new_pointer) : m_latchPointer(pointer)
    {
        m_latchValue = *pointer;
        *pointer = *new_pointer;
    }

    virtual ~LatchRestore() { *m_latchPointer = m_latchValue; }

private:
    T m_latchValue;
    T *m_latchPointer;
};
