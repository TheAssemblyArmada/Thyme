/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose intrusive smart pointer to wrap any type with. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "refcounter.h"
#include <algorithm>

namespace rts
{

// Intrusive reference counted smart pointer.
// Works similar to std::shared_ptr<>, but does not hold reference counter by itself.
// Can be assigned a raw pointer at any time without worrying about colliding with another reference counter.
// Adapter type needs to be set according to the Counter type used.
// Default adapter is set for Counter types using rts::atomic_intrusive_counter<> or rts::intrusive_counter<>.
template<class Counter, class Adapter = intrusive_counter_adapter<Counter>> class intrusive_ptr
{
public:
    using element_type = Counter;
    using value_type = Counter;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using integer_type = typename Adapter::integer_type;
    using adapter_type = typename Adapter;

private:
    value_type *m_ptr;

public:
    intrusive_ptr() noexcept : m_ptr(nullptr) {}

    intrusive_ptr(value_type *ptr)
    {
        if (ptr != nullptr)
            adapter_type::Add_Ref(ptr);
        m_ptr = ptr;
    }

    intrusive_ptr(const intrusive_ptr &other)
    {
        if (other.m_ptr != nullptr)
            adapter_type::Add_Ref(other.m_ptr);
        m_ptr = other.m_ptr;
    }

    intrusive_ptr(intrusive_ptr &&other) noexcept
    {
        m_ptr = other.m_ptr;
        other.m_ptr = nullptr;
    }

    template<typename RelatedType> intrusive_ptr(const intrusive_ptr<RelatedType> &other)
    {
        if (other.m_ptr != nullptr)
            adapter_type::Add_Ref(other.m_ptr);
        m_ptr = other.m_ptr;
    }

    ~intrusive_ptr()
    {
        if (m_ptr != nullptr)
            adapter_type::Remove_Ref(m_ptr);
    }

    intrusive_ptr &operator=(const intrusive_ptr &other)
    {
        reset(other.m_ptr);
        return *this;
    }

    intrusive_ptr &operator=(intrusive_ptr &&other)
    {
        if (this != &other) {
            if (m_ptr != nullptr)
                adapter_type::Remove_Ref(m_ptr);
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    template<typename RelatedType> intrusive_ptr &operator=(const intrusive_ptr<RelatedType> &other)
    {
        reset(other.m_ptr);
        return *this;
    }

    intrusive_ptr &operator=(value_type *ptr)
    {
        reset(ptr);
        return *this;
    }

    reference operator*() noexcept { return *m_ptr; }
    pointer operator->() noexcept { return m_ptr; }
    pointer get() noexcept { return m_ptr; }

    const_reference operator*() const noexcept { return *m_ptr; }
    const_pointer operator->() const noexcept { return m_ptr; }
    const_pointer get() const noexcept { return m_ptr; }

    operator bool() const noexcept { return m_ptr != nullptr; }

    // Reset smart pointer to null.
    void reset()
    {
        if (m_ptr != nullptr)
            adapter_type::Remove_Ref(m_ptr);
        m_ptr = nullptr;
    }

    // Reset smart pointer with new pointer.
    void reset(value_type *ptr)
    {
        if (ptr != m_ptr) {
            if (ptr != nullptr)
                adapter_type::Add_Ref(ptr);
            if (m_ptr != nullptr)
                adapter_type::Remove_Ref(m_ptr);
            m_ptr = ptr;
        }
    }

    // Swap smart pointer with other smart pointer.
    void swap(intrusive_ptr &other) noexcept { std::swap(m_ptr, other.m_ptr); }

    // Assign pointer without raising its reference count.
    // Used for example if the reference target already has a reference count on creation.
    void assign_without_add_ref(value_type *ptr)
    {
        if (m_ptr != nullptr)
            adapter_type::Remove_Ref(m_ptr);
        m_ptr = ptr;
    }

    // Release ownership of the pointer and do not change reference count.
    value_type *release() noexcept
    {
        value_type *ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    // Get the count of how many times the pointer is referenced.
    integer_type use_count() const { return adapter_type::Use_Count(m_ptr); }
};

// Adapter for DirectX and COM reference counted objects.
template<class Counter> class dx_intrusive_ptr_adapter
{
public:
    using integer_type = ULONG;

    static integer_type Add_Ref(Counter *counter) { return counter->AddRef(); }
    static integer_type Remove_Ref(Counter *counter) { return counter->Release(); }
    static integer_type Use_Count(const Counter *counter) { return integer_type{ ~0u }; }
};

template<class Counter> using dx_intrusive_ptr = intrusive_ptr<Counter, dx_intrusive_ptr_adapter<Counter>>;

} // namespace rts
