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

#include "always.h"
#include "mempoolobj.h"

template<class T> class Override
{
public:
    Override(T *ptr = nullptr) : m_ptr(ptr) {}
    Override(const Override<T> &that) { m_ptr = that.m_ptr; }

    void operator=(T *ptr) { m_ptr = ptr; }
    operator T *() { return m_ptr; }

    const T *operator->() const
    {
        if (m_ptr) {
            return static_cast<const T *>(m_ptr->Get_Final_Override());
        } else {
            return nullptr;
        }
    }

    const T *Get_Final_Override() const
    {
        if (m_ptr) {
            return static_cast<const T *>(m_ptr->Get_Final_Override());
        } else {
            return nullptr;
        }
    }

    const T *operator*() const { return Get_Final_Override(); }

private:
    T *m_ptr;
};

class WaterTransparencySetting;
class WeatherSetting;
class ThingTemplate;
class Overridable : public MemoryPoolObject
{
    IMPLEMENT_POOL(Overridable);

protected:
    virtual ~Overridable() override
    {
        if (m_next != nullptr) {
            m_next->Delete_Instance();
        }
    }

public:
    Overridable() : m_next(nullptr), m_isAllocated(false) {}

    void Set_Next(Overridable *next) { m_next = next; }
    void Set_Is_Allocated() { m_isAllocated = true; }

    const Overridable *Friend_Get_Final_Override() const;
    Overridable *Friend_Get_Final_Override();
    const Overridable *Get_Final_Override() const;
    Overridable *Delete_Overrides();

private:
    Overridable *m_next;
    bool m_isAllocated;
};

inline const Overridable *Overridable::Get_Final_Override() const
{
    if (m_next) {
        return m_next->Friend_Get_Final_Override();
    } else {
        return this;
    }
}

inline const Overridable *Overridable::Friend_Get_Final_Override() const
{
    if (m_next) {
        return const_cast<Overridable *>(m_next)->Friend_Get_Final_Override();
    } else {
        return this;
    }
}

inline Overridable *Overridable::Friend_Get_Final_Override()
{
    if (m_next) {
        return m_next->Friend_Get_Final_Override();
    } else {
        return this;
    }
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
