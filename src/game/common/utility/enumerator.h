/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose enumerator utility class. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "type_traits.h"
#include <cstddef>
#include <typesize.h>

namespace rts
{

template<typename ValueType, typename UnderlyingType = underlying_type_t<ValueType>> class enumerator
{
public:
    using Value = ValueType;
    using value_type = ValueType;
    using underlying_type = UnderlyingType;

public:
    constexpr enumerator() noexcept : m_value(0) {}
    constexpr enumerator(value_type value) noexcept : m_value(static_cast<underlying_type>(value)) {}
    constexpr enumerator(const enumerator &other) noexcept : m_value(other.m_value) {}

    constexpr enumerator &operator=(enumerator other) noexcept
    {
        m_value = other.m_value;
        return *this;
    }
    constexpr enumerator &operator+=(enumerator other) noexcept
    {
        m_value += other.m_value;
        return *this;
    }
    constexpr enumerator &operator-=(enumerator other) noexcept
    {
        m_value -= other.m_value;
        return *this;
    }
    constexpr enumerator &operator*=(enumerator other) noexcept
    {
        m_value *= other.m_value;
        return *this;
    }
    constexpr enumerator &operator/=(enumerator other) noexcept
    {
        m_value /= other.m_value;
        return *this;
    }
    constexpr enumerator &operator%=(enumerator other) noexcept
    {
        m_value %= other.m_value;
        return *this;
    }
    constexpr enumerator &operator++() noexcept
    {
        ++m_value;
        return *this;
    }
    constexpr enumerator &operator--() noexcept
    {
        --m_value;
        return *this;
    }

    constexpr enumerator operator+(enumerator other) noexcept
    {
        return enumerator(static_cast<value_type>(m_value + other.m_value));
    }
    constexpr enumerator operator-(enumerator other) noexcept
    {
        return enumerator(static_cast<value_type>(m_value - other.m_value));
    }
    constexpr enumerator operator*(enumerator other) noexcept
    {
        return enumerator(static_cast<value_type>(m_value * other.m_value));
    }
    constexpr enumerator operator/(enumerator other) noexcept
    {
        return enumerator(static_cast<value_type>(m_value / other.m_value));
    }
    constexpr enumerator operator%(enumerator other) noexcept
    {
        return enumerator(static_cast<value_type>(m_value % other.m_value));
    }
    constexpr enumerator operator++(int) noexcept { return enumerator(static_cast<value_type>(m_value++)); }
    constexpr enumerator operator--(int) noexcept { return enumerator(static_cast<value_type>(m_value--)); }

    constexpr bool operator==(enumerator other) const noexcept { return m_value == other.m_value; }
    constexpr bool operator!=(enumerator other) const noexcept { return m_value != other.m_value; }
    constexpr bool operator<(enumerator other) const noexcept { return m_value < other.m_value; }
    constexpr bool operator>(enumerator other) const noexcept { return m_value > other.m_value; }
    constexpr bool operator<=(enumerator other) const noexcept { return m_value <= other.m_value; }
    constexpr bool operator>=(enumerator other) const noexcept { return m_value >= other.m_value; }

    constexpr operator bool() const noexcept { return m_value != underlying_type(0); }

    constexpr void reset() noexcept { m_value = underlying_type(0); }

    constexpr value_type value() const noexcept { return static_cast<value_type>(m_value); }
    constexpr underlying_type underlying() const noexcept { return m_value; }

private:
    underlying_type m_value;
};

} // namespace rts
