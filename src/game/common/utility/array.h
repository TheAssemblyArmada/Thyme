/**
 * @file
 *
 * @author xezon
 *
 * @brief Simple array class with static storage size. (Thyme Feature)
 *        Can be removed and replaced with std::array if STL Port is abandoned.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>

namespace rts
{

// #TODO Add a constexpr reverse_iterator.
// #TODO Remove Array class?

template<typename T, std::size_t Size> class array
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    // Aggregate type. Constructors are implicitly declared.

    constexpr reference at(size_type pos)
    {
        assert(pos < size());
        return m_array[pos];
    }
    constexpr const_reference at(size_type pos) const
    {
        assert(pos < size());
        return m_array[pos];
    }
    constexpr reference operator[](size_type pos)
    {
        assert(pos < size());
        return m_array[pos];
    }
    constexpr const_reference operator[](size_type pos) const
    {
        assert(pos < size());
        return m_array[pos];
    }

    constexpr reference front() noexcept { return m_array[0]; }
    constexpr const_reference front() const noexcept { return m_array[0]; }
    constexpr reference back() noexcept { return m_array[size() - 1]; }
    constexpr const_reference back() const noexcept { return m_array[size() - 1]; }

    constexpr pointer data() noexcept { return m_array; }
    constexpr const_pointer data() const noexcept { return m_array; }

    constexpr iterator begin() noexcept { return iterator(data()); }
    constexpr const_iterator begin() const noexcept { return const_iterator(data()); }
    constexpr const_iterator cbegin() const noexcept { return const_iterator(data()); }

    constexpr iterator end() noexcept { return iterator(data() + size()); }
    constexpr const_iterator end() const noexcept { return const_iterator(data() + size()); }
    constexpr const_iterator cend() const noexcept { return const_iterator(data() + size()); }

    reverse_iterator rbegin() noexcept { return std::reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return std::reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return std::reverse_iterator(cbegin()); }

    reverse_iterator rend() noexcept { return std::reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return std::reverse_iterator(end()); }
    const_reverse_iterator crend() const noexcept { return std::reverse_iterator(cend()); }

    constexpr bool empty() const noexcept { return size() == 0; }
    constexpr size_type size() const noexcept { return sizeof(m_array) / sizeof(value_type); }
    constexpr size_type max_size() const noexcept { return size(); }

    void fill(const T &value) { std::fill_n(begin(), size(), value); }

    void swap(array &other) noexcept
    {
        for (std::size_t i = 0; i < size(); ++i) {
            std::swap(m_array[i], other.m_array[i]);
        }
    }

    value_type m_array[Size ? Size : 1];
};

} // namespace rts
