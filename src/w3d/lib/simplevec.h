/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Simple vector class
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
#include <algorithm>
#include <captainslog.h>
#include <cstring>

template<typename T> class SimpleVecClass
{
public:
    SimpleVecClass(int size = 0) : m_vector(nullptr), m_vectorMax(0)
    {
        if (size > 0) {
            Resize(size);
        }
    }
    virtual ~SimpleVecClass()
    {
        if (m_vector != nullptr) {
            delete[] m_vector;
            m_vector = nullptr;
            m_vectorMax = 0;
        }
    }
    T &operator[](int index)
    {
        captainslog_assert(index < m_vectorMax);
        return m_vector[index];
    }
    T const &operator[](int index) const
    {
        captainslog_assert(index < m_vectorMax);
        return m_vector[index];
    }
    int Length() const { return m_vectorMax; }
    virtual bool Resize(int newsize)
    {
        if (newsize == m_vectorMax) {
            return true;
        }
        if (newsize > 0) {
            T *newptr = new T[newsize];
            if (m_vector != nullptr) {
                int copycount = (newsize < m_vectorMax) ? newsize : m_vectorMax;
                std::memcpy(newptr, m_vector, copycount * sizeof(T));
                delete[] m_vector;
                m_vector = nullptr;
            }
            m_vector = newptr;
            m_vectorMax = newsize;
        } else {
            m_vectorMax = 0;
            if (m_vector != nullptr) {
                delete[] m_vector;
                m_vector = nullptr;
            }
        }
        return true;
    }
    virtual bool Uninitialised_Grow(int newsize)
    {
        if (newsize <= m_vectorMax) {
            return true;
        }
        if (newsize > 0) {
            delete[] m_vector;
            m_vector = new T[newsize];
            m_vectorMax = newsize;
        }
        return true;
    }
    void Zero_Memory()
    {
        if (m_vector != nullptr) {
            memset(m_vector, 0, m_vectorMax * sizeof(T));
        }
    }

protected:
    T *m_vector;
    int m_vectorMax;
};

template<typename T> class SimpleDynVecClass : public SimpleVecClass<T>
{
    using SimpleVecClass<T>::m_vector;
    using SimpleVecClass<T>::m_vectorMax;

public:
    SimpleDynVecClass(int size = 0) : SimpleVecClass<T>(size), m_activeCount(0) {}
    virtual ~SimpleDynVecClass(void)
    {
        if (m_vector != nullptr) {
            delete[] m_vector;
            m_vector = nullptr;
        }
    }
    int Count(void) const { return m_activeCount; }
    T &operator[](int index)
    {
        captainslog_assert(index < m_activeCount);
        return m_vector[index];
    }
    T const &operator[](int index) const
    {
        captainslog_assert(index < m_activeCount);
        return m_vector[index];
    }
    virtual bool Resize(int newsize)
    {
        if (SimpleVecClass<T>::Resize(newsize)) {
            if (m_vectorMax < m_activeCount)
                m_activeCount = m_vectorMax;
            return true;
        }
        return false;
    }
    bool Add(T const &object, int new_size_hint = 0)
    {
        if (m_activeCount >= m_vectorMax) {
            if (!Grow(new_size_hint)) {
                return false;
            }
        }
        (*this)[m_activeCount++] = object;
        return true;
    }
    T *Add_Multiple(int number_to_add)
    {
        int index = m_activeCount;
        m_activeCount += number_to_add;
        if (m_activeCount >= m_vectorMax) {
            Grow(m_activeCount);
        }
        return &m_vector[index];
    }
    bool Delete(int index, bool allow_shrink = true)
    {
        captainslog_assert(index < m_activeCount);
        if (index < m_activeCount - 1) {
            memmove(&(m_vector[index]), &(m_vector[index + 1]), (m_activeCount - index - 1) * sizeof(T));
        }
        m_activeCount--;
        if (allow_shrink) {
            Shrink();
        }
        return true;
    }
    bool Delete(T const &object, bool allow_shrink = true)
    {
        int id = Find_Index(object);
        if (id != -1) {
            return Delete(id, allow_shrink);
        }
        return false;
    }
    bool Delete_Range(int start, int count, bool allow_shrink = true)
    {
        captainslog_assert(start >= 0);
        captainslog_assert(start <= m_activeCount - count);
        if (start < m_activeCount - count) {
            memmove(&(m_vector[start]), &(m_vector[start + count]), (m_activeCount - start - count) * sizeof(T));
        }
        m_activeCount -= count;
        if (allow_shrink) {
            Shrink();
        }
        return true;
    }
    void Delete_All(bool allow_shrink = true)
    {
        m_activeCount = 0;
        if (allow_shrink) {
            Shrink();
        }
    }

    T *begin() { return &(*this)[0]; }
    const T *begin() const { return cbegin(); }
    const T *cbegin() const { return &(*this)[0]; }
    T *end() { return &(*this)[Count()]; }
    const T *end() const { return cend(); }
    const T *cend() const { return &(*this)[Count()]; }

protected:
    bool Grow(int new_size_hint)
    {
        int new_size = std::max(m_vectorMax + m_vectorMax / 4, m_vectorMax + 4);
        new_size = std::max(new_size, new_size_hint);
        return Resize(new_size);
    }
    bool Shrink(void)
    {
        if (m_activeCount < m_vectorMax / 4) {
            return Resize(m_activeCount);
        }
        return true;
    }
    int Find_Index(T const &object)
    {
        for (int index = 0; index < Count(); index++) {
            if ((*this)[index] == object)
                return index;
        }
        return -1;
    }
    int m_activeCount;
};
