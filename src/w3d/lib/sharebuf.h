/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Share Buffer
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
#include "refcount.h"
#include "w3dmpo.h"
#include <cstring>

using std::memset;

template<class T> class ShareBufferClass : public W3DMPO, public RefCountClass
{
    IMPLEMENT_W3D_POOL(ShareBufferClass);

public:
#ifdef BUILD_EDITOR
    ShareBufferClass(int count, const char *name) : m_name(name), m_count(count)
#else
    ShareBufferClass(int count) : m_count(count)
#endif
    {
        m_array = new T[m_count];
    }

    ShareBufferClass(const ShareBufferClass &that) : m_count(that.m_count)
    {
#ifdef BUILD_EDITOR
        m_name = that.m_name;
#endif
        m_array = new T[m_count];

        for (int i = 0; i < m_count; i++) {
            m_array[i] = that.m_array[i];
        }
    }

    ~ShareBufferClass()
    {
        if (m_array) {
            delete[] m_array;
            m_array = NULL;
        }
    }

    T *Get_Array() { return m_array; }
    int Get_Count() { return m_count; }
    const T &Get_Element(int index) const { return m_array[index]; }
    T &Get_Element(int index) { return m_array[index]; }

    void Set_Element(int index, const T &thing) { m_array[index] = thing; }
    void Clear() { memset(m_array, 0, m_count * sizeof(T)); }

protected:
#ifdef BUILD_EDITOR
    const char *m_name;
#endif
    T *m_array;
    int m_count;
};

template<class T> inline ShareBufferClass<T> *New_Share_Buffer(int count, const char *name = nullptr)
{
#ifdef BUILD_EDITOR
    return new ShareBufferClass<T>(count, name);
#else
    return new ShareBufferClass<T>(count);
#endif
}
