/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose number flags utility class. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "bittype.h"
#include <cassert>
#include <cstddef>

namespace rts
{

namespace detail
{

// clang-format off

template<typename IntegerType, std::size_t Bytes> struct BitBucketShiftStruct;
template<typename IntegerType, std::size_t Bytes> struct BitBucketMaskStruct;

template<typename IntegerType> struct BitBucketShiftStruct<IntegerType, 1>{ static constexpr IntegerType Get() { return IntegerType(3); } };
template<typename IntegerType> struct BitBucketShiftStruct<IntegerType, 2>{ static constexpr IntegerType Get() { return IntegerType(4); } };
template<typename IntegerType> struct BitBucketShiftStruct<IntegerType, 4>{ static constexpr IntegerType Get() { return IntegerType(5); } };
template<typename IntegerType> struct BitBucketShiftStruct<IntegerType, 8>{ static constexpr IntegerType Get() { return IntegerType(6); } };

template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 1>{ static constexpr IntegerType Get() { return IntegerType(0xfff); } };
template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 2>{ static constexpr IntegerType Get() { return IntegerType(0xffff); } };
template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 4>{ static constexpr IntegerType Get() { return IntegerType(0xfffff); } };
template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 8>{ static constexpr IntegerType Get() { return IntegerType(0xffffff); } };

template<typename IntegerType, std::size_t Bytes> constexpr IntegerType Bit_Bucket_Shift() { return BitBucketShiftStruct<IntegerType, Bytes>::Get(); }
template<typename IntegerType, std::size_t Bytes> constexpr IntegerType Bit_Bucket_Mask() { return BitBucketMaskStruct<IntegerType, Bytes>::Get(); }

// clang-format on

} // namespace detail

template<typename ValueType, std::size_t ValueCount> class enumflags
{
public:
    using Value = ValueType;
    using value_type = ValueType;
    using size_type = std::size_t;

private:
    template<typename, size_type> friend class enumflags;
    using storage_type = uint32_t;

public:
    constexpr enumflags() { reset(); }

    constexpr enumflags(value_type value)
    {
        reset();
        set(value);
    }

    constexpr enumflags(const enumflags &other) noexcept { copy(m_values, other.m_values, bucket_size()); }

    template<size_type OtherValueCount> constexpr enumflags(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        const size_type shared_bucket_size = min(bucket_size(), other.bucket_size());
        copy(m_values, other.m_values, shared_bucket_size);
        zero(m_values + shared_bucket_size, bucket_size() - shared_bucket_size);
    }

    constexpr enumflags &operator=(const enumflags &other) noexcept
    {
        copy(m_values, other.m_values, bucket_size());
        return *this;
    }

    template<size_type OtherValueCount>
    constexpr enumflags &operator=(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        const size_type shared_bucket_size = min(bucket_size(), other.bucket_size());
        copy(m_values, other.m_values, shared_bucket_size);
        return *this;
    }

    constexpr enumflags &operator|=(const enumflags &other) noexcept
    {
        for (size_type i = 0; i < bucket_size(); ++i) {
            m_values[i] |= other.m_values[i];
        }
        return *this;
    }

    template<size_type OtherValueCount>
    constexpr enumflags &operator|=(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        const size_type shared_bucket_size = min(bucket_size(), other.bucket_size());
        for (size_type i = 0; i < shared_bucket_size; ++i) {
            m_values[i] |= other.m_values[i];
        }
        return *this;
    }

    constexpr enumflags &operator&=(const enumflags &other) noexcept
    {
        for (size_type i = 0; i < bucket_size(); ++i) {
            m_values[i] &= other.m_values[i];
        }
        return *this;
    }

    template<size_type OtherValueCount>
    constexpr enumflags &operator&=(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        const size_type shared_bucket_size = min(bucket_size(), other.bucket_size());
        for (size_type i = 0; i < shared_bucket_size; ++i) {
            m_values[i] &= other.m_values[i];
        }
        return *this;
    }

    constexpr enumflags &operator^=(const enumflags &other) noexcept
    {
        for (size_type i = 0; i < bucket_size(); ++i) {
            m_values[i] ^= other.m_values[i];
        }
        return *this;
    }

    template<size_type OtherValueCount>
    constexpr enumflags &operator^=(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        const size_type shared_bucket_size = min(bucket_size(), other.bucket_size());
        for (size_type i = 0; i < shared_bucket_size; ++i) {
            m_values[i] ^= other.m_values[i];
        }
        return *this;
    }

    // #TODO implement
    constexpr enumflags &operator<<=(size_type pos);
    constexpr enumflags &operator>>=(size_type pos);

    constexpr bool operator==(const enumflags &other) const noexcept
    {
        return 0 == compare(m_values, other.m_values, sizeof(m_values));
    }

    template<size_type OtherValueCount>
    constexpr bool operator==(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        const size_type shared_bucket_bytes = min(bucket_bytes(), other.bucket_bytes());
        return 0 == compare(m_values, other.m_values, shared_bucket_bytes);
    }

    constexpr bool operator!=(const enumflags &other) const noexcept { return !operator==(other); }

    template<size_type OtherValueCount>
    constexpr bool operator!=(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        return !operator==(other);
    }

    constexpr operator bool() const noexcept { return any(); }

    constexpr enumflags operator~() const noexcept
    {
        enumflags inst;
        for (size_type i = 0; i < bucket_size(); ++i) {
            inst.m_values[i] = ~m_values[i];
        }
        return inst;
    }

    constexpr enumflags operator|(const enumflags &other) const noexcept
    {
        enumflags inst(*this);
        inst |= other;
        return inst;
    }

    template<size_type OtherValueCount>
    constexpr enumflags operator|(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        enumflags inst(*this);
        inst |= other;
        return inst;
    }

    constexpr enumflags operator&(const enumflags &other) const noexcept
    {
        enumflags inst(*this);
        inst &= other;
        return inst;
    }

    template<size_type OtherValueCount>
    constexpr enumflags operator&(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        enumflags inst(*this);
        inst &= other;
        return inst;
    }

    constexpr enumflags operator^(const enumflags &other) const noexcept
    {
        enumflags inst(*this);
        inst ^= other;
        return inst;
    }

    template<size_type OtherValueCount>
    constexpr enumflags operator^(const enumflags<value_type, OtherValueCount> &other) noexcept
    {
        enumflags inst(*this);
        inst ^= other;
        return inst;
    }

    // #TODO implement
    constexpr enumflags operator<<(size_type pos) const;
    constexpr enumflags operator>>(size_type pos) const;

    constexpr void set(value_type value) { access(value) |= bit(value); }

    constexpr void set(const enumflags &flags) noexcept
    {
        for (size_type i = 0; i < bucket_size(); ++i) {
            m_values[i] |= flags.m_values[i];
        }
    }
    constexpr void reset(value_type value) { access(value) &= ~bit(value); }

    constexpr void reset(const enumflags &flags) noexcept
    {
        for (size_type i = 0; i < bucket_size(); ++i) {
            m_values[i] &= ~flags.m_values[i];
        }
    }

    constexpr void reset() noexcept { zero(m_values, bucket_size()); }

    constexpr size_type size() const noexcept { return static_cast<size_type>(ValueCount); }

    constexpr size_type count() const noexcept
    {
        size_type count = 0;
        for (size_type i = 0; i < size(); ++i) {
            if (has(static_cast<value_type>(i))) {
                ++count;
            }
        }
        return count;
    }

    constexpr bool none() const noexcept
    {
        storage_type bits = 0;
        for (const storage_type value : m_values) {
            bits |= value;
        }
        return bits == storage_type(0);
    }

    constexpr bool any() const noexcept { return !none(); }

    constexpr bool all() const noexcept
    {
        for (size_type i = 0; i < size(); ++i) {
            if (!has(static_cast<value_type>(i))) {
                return false;
            }
        }
        return true;
    }

    constexpr bool has(value_type value) const { return ((access(value) & bit(value)) != storage_type(0)); }

    constexpr bool has_only(value_type value) const
    {
        const size_type value_bucket = bucket(value);

        if ((m_values[value_bucket] & bit(value)) != m_values[value_bucket]) {
            return false;
        }
        storage_type other_bits = 0;
        size_type i = 0;

        for (; i < value_bucket; ++i) {
            other_bits |= m_values[i];
        }
        for (++i; i < bucket_size(); ++i) {
            other_bits |= m_values[i];
        }
        return other_bits == storage_type(0);
    }

    constexpr bool has_any_of(enumflags flags) const noexcept
    {
        bool test = false;
        for (size_type i = 0; i < bucket_size(); ++i) {
            test |= ((m_values[i] & flags.m_values[i]) != storage_type(0));
        }
        return test;
    }

    constexpr bool has_all_of(enumflags flags) const noexcept
    {
        bool test = true;
        for (size_type i = 0; i < bucket_size(); ++i) {
            test &= ((m_values[i] & flags.m_values[i]) == flags.m_values[i]);
        }
        return test;
    }

    constexpr bool get(value_type &value, size_type occurence = 0) const noexcept
    {
        size_type num = 0;
        for (size_type i = 0; i < size(); ++i) {
            if (has(static_cast<value_type>(i))) {
                if (occurence == num++) {
                    value = static_cast<value_type>(i);
                    return true;
                }
            }
        }
        return false;
    }

private:
    static constexpr size_type min(const size_type a, const size_type b) { return a < b ? a : b; }

    static constexpr void copy(storage_type *dst, const storage_type *src, size_type count)
    {
        while (count-- > 0) {
            dst[count] = src[count];
        }
    }

    static constexpr void zero(storage_type *dst, size_type count)
    {
        while (count-- > 0) {
            dst[count] = storage_type(0);
        }
    }

    static constexpr int compare(const storage_type *left, const storage_type *right, size_type count)
    {
        while (count-- > 0) {
            if (*left++ != *right++)
                return left[-1] < right[-1] ? -1 : 1;
        }
        return 0;
    }

    static constexpr size_type bucket_size() noexcept { return sizeof(m_values) / sizeof(storage_type); }

    static constexpr size_type bucket_bytes() noexcept { return sizeof(m_values); }

    static constexpr storage_type bit(value_type value) noexcept
    {
        // Mask value to max storage bit count and shift to single storage bit. Example with 1 byte storage:
        // value   0101 1111 = 95
        // masked  0000 0111 = 7
        // shifted 1000 0000 = 128
        constexpr storage_type mask = detail::Bit_Bucket_Mask<storage_type, sizeof(storage_type)>();
        storage_type masked = static_cast<storage_type>(value) & mask;
        storage_type shifted = 1 << masked;
        return shifted;
    }

    static constexpr size_type bucket(value_type value)
    {
        assert(static_cast<size_type>(value) < ValueCount);

        // Get bucket index with given value. Example with 1 byte storage:
        // value 0101 1111 = 95
        // index 0000 1011 = 11
        constexpr size_type shift = detail::Bit_Bucket_Shift<size_type, sizeof(storage_type)>();
        size_type index = static_cast<size_type>(value) >> shift;
        return index;
    }

    constexpr storage_type &access(value_type value) { return m_values[bucket(value)]; }

    constexpr storage_type access(value_type value) const { return m_values[bucket(value)]; }

private:
    storage_type m_values[1 + (bucket(value_type(ValueCount - 1)))];
};

} // namespace rts
