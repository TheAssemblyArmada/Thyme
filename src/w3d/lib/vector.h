/**
 * @file
 *
 * @author OmniBlade
 * @author CCHyper
 *
 * @brief Classes for resizable arrays ("vectors").
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
#include <captainslog.h>
#include <cstring>
#include <new>

using std::memmove;

class NoInitClass;

template<typename T> class VectorClass
{
public:
    VectorClass(int size = 0, const T *array = nullptr);
    VectorClass(const NoInitClass &noinit) {}
    VectorClass(const VectorClass<T> &that);
    virtual ~VectorClass();

    T &operator[](int index);
    const T &operator[](int index) const;

    VectorClass<T> &operator=(VectorClass<T> const &that);
    virtual bool operator==(const VectorClass<T> &that) const;

    virtual bool Resize(int newsize, const T *array = nullptr);
    virtual void Clear();
    virtual int ID(const T *ptr);
    virtual int ID(const T &ptr);

    int Length() const { return m_vectorMax; }

protected:
    T *m_vector;
    int m_vectorMax;
    bool m_isValid;
    bool m_isAllocated;
};

template<typename T> T &VectorClass<T>::operator[](int index)
{
    captainslog_assert(unsigned(index) < unsigned(m_vectorMax));
    return m_vector[index];
}

template<typename T> const T &VectorClass<T>::operator[](int index) const
{
    captainslog_assert(unsigned(index) < unsigned(m_vectorMax));
    return m_vector[index];
}

template<typename T>
VectorClass<T>::VectorClass(int size, const T *array) :
    m_vector(nullptr), m_vectorMax(size), m_isValid(true), m_isAllocated(false)
{
    //    Allocate the vector. The default constructor will be called for every
    //    object in this vector.
    if (size > 0) {
        if (array != nullptr) {
            m_vector = new ((void *)array) T[size];
        } else {
            m_vector = new T[size];
            m_isAllocated = true;
        }
    }
}

template<typename T> VectorClass<T>::~VectorClass()
{
    VectorClass<T>::Clear();
}

template<typename T>
VectorClass<T>::VectorClass(VectorClass<T> const &vector) : m_vector(nullptr), m_vectorMax(0), m_isAllocated(false)
{
    *this = vector;
}

template<typename T> VectorClass<T> &VectorClass<T>::operator=(VectorClass<T> const &vector)
{
    if (this != &vector) {
        Clear();
        m_vectorMax = vector.Length();

        if (m_vectorMax) {
            m_vector = new T[m_vectorMax];

            if (m_vector) {
                m_isAllocated = true;

                for (int index = 0; index < m_vectorMax; index++) {
                    m_vector[index] = vector[index];
                }
            }
        } else {
            m_vector = nullptr;
            m_isAllocated = false;
        }
    }

    return *this;
}

template<typename T> bool VectorClass<T>::operator==(VectorClass<T> const &vector) const
{
    if (m_vectorMax == vector.Length()) {
        for (int index = 0; index < m_vectorMax; index++) {
            if (m_vector[index] != vector[index]) {
                return false;
            }
        }

        return true;
    }

    return false;
}

template<typename T> inline int VectorClass<T>::ID(const T *ptr)
{
    if (!m_isValid) {
        return 0;
    }

    return ((uintptr_t)ptr - (uintptr_t)m_vector) / sizeof(T);
}

template<typename T> int VectorClass<T>::ID(const T &object)
{
    if (!m_isValid) {
        return 0;
    }

    for (int index = 0; index < m_vectorMax; index++) {
        if ((*this)[index] == object) {
            return index;
        }
    }

    return -1;
}

template<typename T> void VectorClass<T>::Clear()
{
    if (m_vector && m_isAllocated) {
        delete[] m_vector;
        m_vector = nullptr;
    }

    m_isAllocated = false;
    m_vectorMax = 0;
}

template<typename T> bool VectorClass<T>::Resize(int newsize, const T *array)
{
    if (newsize != 0) {
        T *newptr;
        m_isValid = false;

        if (array == nullptr) {
            newptr = new T[newsize];
        } else {
            newptr = new ((void *)array) T[newsize];
        }

        m_isValid = true;

        if (newptr == nullptr) {
            return false;
        }

        if (m_vector != nullptr) {
            int copy_count = (newsize < m_vectorMax) ? newsize : m_vectorMax;

            for (int i = 0; i < copy_count; ++i) {
                newptr[i] = m_vector[i];
            }

            if (m_isAllocated) {
                delete[] m_vector;
                m_vector = nullptr;
            }
        }

        m_vector = newptr;
        m_vectorMax = newsize;
        m_isAllocated = (m_vector != nullptr && array == nullptr);

    } else {
        Clear();
    }

    return true;
}

template<typename T> class DynamicVectorClass : public VectorClass<T>
{
    using VectorClass<T>::m_vector;
    using VectorClass<T>::m_vectorMax;
    using VectorClass<T>::m_isAllocated;

public:
    DynamicVectorClass(unsigned size = 0, const T *array = nullptr);

    bool operator==(const DynamicVectorClass &src) { return false; }
    bool operator!=(const DynamicVectorClass &src) { return true; }
    DynamicVectorClass<T> &operator=(DynamicVectorClass<T> const &rvalue);

    virtual bool Resize(int newsize, const T *array = nullptr) override;
    virtual void Clear() override;
    virtual int ID(const T *ptr) override { return VectorClass<T>::ID(ptr); };
    virtual int ID(const T &ptr) override;

    void Reset_Active() { m_activeCount = 0; }
    void Set_Active(int count) { m_activeCount = count; }
    int Count() const { return (m_activeCount); }
    bool Add(const T &object);
    bool Add_Head(const T &object);
    const T &Fetch_Head() const { return (*this)[0]; }
    bool Insert(int index, const T &object);
    bool Delete(const T &object);
    bool Delete(int index);
    void Delete_All();
    int Set_Growth_Step(int step) { return m_growthStep = step; }
    int Growth_Step() { return m_growthStep; }
    T *Uninitialized_Add();

    T *begin() { return &(*this)[0]; }
    const T *begin() const { return cbegin(); }
    const T *cbegin() const { return &(*this)[0]; }
    T *end() { return &(*this)[Count()]; }
    const T *end() const { return cend(); }
    const T *cend() const { return &(*this)[Count()]; }

protected:
    int m_activeCount;
    int m_growthStep;
};

template<typename T> void DynamicVectorClass<T>::Clear()
{
    m_activeCount = 0;
    VectorClass<T>::Clear();
};

template<typename T> DynamicVectorClass<T> &DynamicVectorClass<T>::operator=(DynamicVectorClass<T> const &rvalue)
{
    VectorClass<T>::operator=(rvalue);
    m_activeCount = rvalue.m_activeCount;
    m_growthStep = rvalue.m_growthStep;

    return *this;
}

template<typename T> DynamicVectorClass<T>::DynamicVectorClass(unsigned size, const T *array) : VectorClass<T>(size, array)
{
    m_growthStep = 10;
    m_activeCount = 0;
}

template<typename T> bool DynamicVectorClass<T>::Resize(int newsize, const T *array)
{
    if (VectorClass<T>::Resize(newsize, array)) {
        if (m_vectorMax < m_activeCount) {
            m_activeCount = m_vectorMax;
        }

        return true;
    }

    return false;
}

template<typename T> int DynamicVectorClass<T>::ID(const T &object)
{
    for (int index = 0; index < Count(); ++index) {
        if ((*this)[index] == object)
            return index;
    }

    return -1;
}

template<typename T> bool DynamicVectorClass<T>::Add(const T &object)
{
    if (m_activeCount >= m_vectorMax) {
        if ((m_isAllocated || !m_vectorMax) && m_growthStep > 0) {
            if (!Resize(m_vectorMax + m_growthStep)) {
                return false;
            }
        } else {
            return false;
        }
    }

    (*this)[m_activeCount++] = object;

    return true;
}

template<typename T> bool DynamicVectorClass<T>::Add_Head(const T &object)
{
    if (m_activeCount >= m_vectorMax) {
        if ((m_isAllocated || !m_vectorMax) && m_growthStep > 0) {
            if (!Resize(m_vectorMax + m_growthStep)) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (m_activeCount) {
        memmove(&(*this)[1], &(*this)[0], m_activeCount * sizeof(T));
    }

    (*this)[0] = object;
    m_activeCount++;

    return true;
}

template<typename T> bool DynamicVectorClass<T>::Insert(int index, const T &object)
{
    if (index < 0 || index > m_activeCount) {
        return false;
    }

    if (m_activeCount >= m_vectorMax) {
        if ((m_isAllocated || !m_vectorMax) && m_growthStep > 0) {
            if (!Resize(m_vectorMax + m_growthStep)) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (index < m_activeCount) {
        memmove(&(*this)[index + 1], &(*this)[index], (m_activeCount - index) * sizeof(T));
    }

    (*this)[index] = object;
    m_activeCount++;

    return true;
}

template<typename T> bool DynamicVectorClass<T>::Delete(const T &object)
{
    int id = ID(object);

    if (id != -1) {
        return Delete(id);
    }

    return false;
}

template<typename T> bool DynamicVectorClass<T>::Delete(int index)
{
    if (index < m_activeCount) {
        m_activeCount--;

        for (int i = index; i < m_activeCount; ++i) {
            (*this)[i] = (*this)[i + 1];
        }

        return true;
    }

    return false;
}

template<typename T> void DynamicVectorClass<T>::Delete_All()
{
    int len = m_vectorMax;
    Clear();
    Resize(len);
}

template<typename T> T *DynamicVectorClass<T>::Uninitialized_Add()
{
    if (m_activeCount >= m_vectorMax) {
        if (m_growthStep > 0) {
            if (!Resize(m_vectorMax + m_growthStep)) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    return &((*this)[m_activeCount++]);
}
