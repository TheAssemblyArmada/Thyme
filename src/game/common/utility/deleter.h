/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose template deleter. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

// Deleter for objects created by 'new'.
template<typename Type> struct NewDeleter
{
    void operator()(Type *instance) const { delete instance; }
};

// Deleter for objects created by 'new[]'.
template<typename Type> struct NewArrayDeleter
{
    void operator()(Type *instance) const { delete[] instance; }
};

// Deleter for objects created by 'malloc()'.
template<typename Type> struct AllocDeleter
{
    void operator()(Type *instance) const { free(instance); }
};

// Deleter for objects using a game memory pool.
template<typename Type> struct MemoryPoolObjectDeleter
{
    void operator()(Type *instance) const { instance->Delete_Instance(); }
};

template<typename Deleter, typename DeleteType> inline void Invoke_Deleter(DeleteType *instance)
{
    Deleter deleter;
    deleter(instance);
}
