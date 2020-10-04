/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Dictionary class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dict.h"
#include "memdynalloc.h"
#include <captainslog.h>

void Dict::DictPair::Copy_From(const DictPair &that)
{
    if (Get_Type() != that.Get_Type()) {
        Clear();
    }

    m_key = that.m_key;

    switch (Get_Type()) {
        case DICT_BOOL:
            m_value.boolean = that.m_value.boolean;
            break;
        case DICT_INT:
            m_value.integer = that.m_value.integer;
            break;
        case DICT_REAL:
            m_value.real = that.m_value.real;
            break;
        case DICT_ASCIISTRING:
            m_value.ascii = that.m_value.ascii;
            break;
        case DICT_UNICODESTRING:
            m_value.unicode = that.m_value.unicode;
            break;
        default:
            return;
    }
}

void Dict::DictPair::Clear()
{
    switch (Get_Type()) {
        case DICT_BOOL: // Fallthrough
        case DICT_INT:
            m_value.integer = 0;
            break;
        case DICT_REAL:
            m_value.real = 0.0f;
            break;
        case DICT_ASCIISTRING:
            m_value.ascii.Clear();
            break;
        case DICT_UNICODESTRING:
            m_value.unicode.Clear();
            break;
        default:
            return;
    }
}

void Dict::DictPair::Set_Name_And_Type(NameKeyType key, DataType type)
{
    if (Get_Type() != type) {
        Clear();
    }

    Set_Key(key, type);
}

Dict::Dict(const Dict &src) : m_data(src.m_data)
{
    if (src.m_data != nullptr) {
        ++src.m_data->m_refCount;
    }
}

Dict::Dict(int pair_pre_alloc) : m_data(nullptr)
{
    if (pair_pre_alloc > 0) {
        Ensure_Unique(pair_pre_alloc);
    }
}

Dict::~Dict()
{
    Release_Data();
}

Dict &Dict::operator=(const Dict &src)
{
    if (&src != this) {
        Release_Data();
        m_data = src.m_data;

        if (src.m_data != nullptr) {
            ++src.m_data->m_refCount;
        }
    }

    return *this;
}

NameKeyType Dict::Get_Nth_Key(int n) const
{
    return m_data->Get_Pairs()[n - 1].Get_Key();
}

Dict::DataType Dict::Get_Type(NameKeyType key) const
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair == nullptr) {
        return DICT_NONE;
    }

    return pair->Get_Type();
}

Dict::DataType Dict::Get_Nth_Type(int n) const
{
    return m_data->Get_Pairs()[n - 1].Get_Type();
}

bool Dict::Get_Bool(NameKeyType key, bool *exists) const
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair != nullptr && pair->Get_Type() == DICT_BOOL) {
        if (exists != nullptr) {
            *exists = true;
        }

        return pair->Get_Value().boolean;
    }

    if (exists != nullptr) {
        *exists = false;
    } else {
        captainslog_dbgassert(false, "Dict key missing or of wrong type.");
    }

    return false;
}

int Dict::Get_Int(NameKeyType key, bool *exists) const
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair != nullptr && pair->Get_Type() == DICT_INT) {
        if (exists != nullptr) {
            *exists = true;
        }

        return pair->Get_Value().integer;
    }

    if (exists != nullptr) {
        *exists = false;
    } else {
        captainslog_dbgassert(false, "Dict key missing or of wrong type.");
    }

    return 0;
}

float Dict::Get_Real(NameKeyType key, bool *exists) const
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair != nullptr && pair->Get_Type() == DICT_REAL) {
        if (exists != nullptr) {
            *exists = true;
        }

        return pair->Get_Value().real;
    }

    if (exists != nullptr) {
        *exists = false;
    } else {
        captainslog_dbgassert(false, "Dict key missing or of wrong type.");
    }

    return 0.0f;
}

Utf8String Dict::Get_AsciiString(NameKeyType key, bool *exists) const
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair != nullptr && pair->Get_Type() == DICT_ASCIISTRING) {
        if (exists != nullptr) {
            *exists = true;
        }

        return pair->Get_Value().ascii;
    }

    if (exists != nullptr) {
        *exists = false;
    } else {
        captainslog_dbgassert(false, "Dict key missing or of wrong type.");
    }

    return Utf8String();
}

Utf16String Dict::Get_UnicodeString(NameKeyType key, bool *exists) const
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair != nullptr && pair->Get_Type() == DICT_UNICODESTRING) {
        if (exists != nullptr) {
            *exists = true;
        }

        return pair->Get_Value().unicode;
    }

    if (exists != nullptr) {
        *exists = false;
    } else {
        captainslog_dbgassert(false, "Dict key missing or of wrong type.");
    }

    return Utf16String();
}

bool Dict::Get_Nth_Bool(int n) const
{
    captainslog_dbgassert(n > 0 && m_data != nullptr && n < m_data->m_numPairsUsed, "n out of range.");

    if (m_data != nullptr && Get_Nth_Key(n) != (NameKeyType)(~DICT_TYPEMAX) && Get_Nth_Type(n) == DICT_BOOL) {
        return m_data->Get_Pairs()[n - 1].Get_Value().boolean;
    }

    captainslog_dbgassert(false, "Dict key missing or of wrong type.");

    return false;
}

int Dict::Get_Nth_Int(int n) const
{
    captainslog_dbgassert(n > 0 && m_data != nullptr && n < m_data->m_numPairsUsed, "n out of range.");

    if (m_data != nullptr && Get_Nth_Key(n) != (NameKeyType)(~DICT_TYPEMAX) && Get_Nth_Type(n) == DICT_INT) {
        return m_data->Get_Pairs()[n - 1].Get_Value().integer;
    }

    captainslog_dbgassert(false, "Dict key missing or of wrong type.");

    return 0;
}

float Dict::Get_Nth_Real(int n) const
{
    captainslog_dbgassert(n > 0 && m_data != nullptr && n < m_data->m_numPairsUsed, "n out of range.");

    if (m_data != nullptr && Get_Nth_Key(n) != (NameKeyType)(~DICT_TYPEMAX) && Get_Nth_Type(n) == DICT_REAL) {
        return m_data->Get_Pairs()[n - 1].Get_Value().real;
    }

    captainslog_dbgassert(false, "Dict key missing or of wrong type.");

    return 0.0f;
}

Utf8String Dict::Get_Nth_AsciiString(int n) const
{
    captainslog_dbgassert(n > 0 && m_data != nullptr && n < m_data->m_numPairsUsed, "n out of range.");

    if (m_data != nullptr && Get_Nth_Key(n) != (NameKeyType)(~DICT_TYPEMAX) && Get_Nth_Type(n) == DICT_ASCIISTRING) {
        return m_data->Get_Pairs()[n - 1].Get_Value().ascii;
    }

    captainslog_dbgassert(false, "Dict key missing or of wrong type.");

    return Utf8String();
}

Utf16String Dict::Get_Nth_UnicodeString(int n) const
{
    captainslog_dbgassert(n > 0 && m_data != nullptr && n < m_data->m_numPairsUsed, "n out of range.");

    if (m_data != nullptr && Get_Nth_Key(n) != (NameKeyType)(~DICT_TYPEMAX) && Get_Nth_Type(n) == DICT_UNICODESTRING) {
        return m_data->Get_Pairs()[n - 1].Get_Value().unicode;
    }

    captainslog_dbgassert(false, "Dict key missing or of wrong type.");

    return Utf16String();
}

void Dict::Set_Bool(NameKeyType key, bool value)
{
    DictPair *pair = Set_Prep(key, DICT_BOOL);
    pair->Set_Value(value);
    Sort_Pairs();
}

void Dict::Set_Int(NameKeyType key, int value)
{
    DictPair *pair = Set_Prep(key, DICT_INT);
    pair->Set_Value(value);
    Sort_Pairs();
}

void Dict::Set_Real(NameKeyType key, float value)
{
    DictPair *pair = Set_Prep(key, DICT_REAL);
    pair->Set_Value(value);
    Sort_Pairs();
}

void Dict::Set_AsciiString(NameKeyType key, const Utf8String &value)
{
    DictPair *pair = Set_Prep(key, DICT_ASCIISTRING);
    pair->Set_Value(value);
    Sort_Pairs();
}

void Dict::Set_UnicodeString(NameKeyType key, const Utf16String &value)
{
    DictPair *pair = Set_Prep(key, DICT_UNICODESTRING);
    pair->Set_Value(value);
    Sort_Pairs();
}

bool Dict::Remove(NameKeyType key)
{
    DictPair *pair = Find_Pair_By_Key(key);

    if (pair == nullptr) {
        captainslog_dbgassert(false, "Dict key missing in Remove");

        return false;
    }

    pair = Ensure_Unique(m_data->m_numPairsUsed, true, pair);
    pair->Set_Name_And_Type(NAMEKEY_MAX, DICT_BOOL);
    Sort_Pairs();
    --m_data->m_numPairsUsed;

    return true;
}

void Dict::Copy_Pair_From(Dict &that, NameKeyType key)
{
    DictPair *pair = that.Find_Pair_By_Key(key);

    if (pair != nullptr) {
        DictPair *prep_pair = Set_Prep(key, pair->Get_Type());
        prep_pair->Copy_From(*pair);
        Sort_Pairs();
    } else if (Find_Pair_By_Key(key) != nullptr) {
        Remove(key);
    }
}

void Dict::Sort_Pairs()
{
    if (m_data == nullptr) {
        return;
    }

#if 0
    for (unsigned j = m_data->m_numPairsUsed / 2; j > 0; j /= 2) {
        for (unsigned i = j; i < m_data->m_numPairsUsed; ++i) {
            for (int k = i - j; k >= 0; k -= j) {
                if (m_data->Get_Pairs()[k].Get_Key() <= m_data->Get_Pairs()[k + j].Get_Key()) {
                    break;
                }

                DictPair tmp;
                memcpy(&tmp, &m_data->Get_Pairs()[k], sizeof(DictPair));
                memcpy(&m_data->Get_Pairs()[k], &m_data->Get_Pairs()[k + j], sizeof(DictPair));
                memcpy(&m_data->Get_Pairs()[k + j], &tmp, sizeof(DictPair));
            }
        }
    }
#endif

    qsort(m_data->Get_Pairs(), m_data->m_numPairsUsed, sizeof(DictPair), Pair_Compare);
}

int Dict::Pair_Compare(const void *l, const void *r)
{
    if (static_cast<const DictPair *>(l)->Get_Key() > static_cast<const DictPair *>(r)->Get_Key()) {
        return 1;
    }

    if (static_cast<const DictPair *>(l)->Get_Key() < static_cast<const DictPair *>(r)->Get_Key()) {
        return -1;
    }

    return 0;
}

Dict::DictPair *Dict::Set_Prep(NameKeyType key, DataType type)
{
    DictPair *pair = Find_Pair_By_Key(key);
    int needed = m_data != nullptr ? m_data->m_numPairsUsed : 0;

    if (pair == nullptr) {
        ++needed;
    }

    pair = Ensure_Unique(needed, true, pair);

    if (pair == nullptr) {
        pair = &m_data->Get_Pairs()[m_data->m_numPairsUsed++];
    }

    pair->Set_Name_And_Type(key, type);

    return pair;
}

Dict::DictPair *Dict::Find_Pair_By_Key(NameKeyType key) const
{
    if (m_data == nullptr || m_data->m_numPairsUsed <= 0) {
        return nullptr;
    }

    unsigned lower = 0;
    unsigned upper = m_data->m_numPairsUsed;

    while (lower < upper) {
        unsigned index = (upper + lower - 1) / 2;
        NameKeyType pair_key = m_data->Get_Pairs()[index].Get_Key();

        if (pair_key >= key) {
            if (pair_key == key) {
                return &m_data->Get_Pairs()[index];
            }

            upper = (upper + lower - 1) / 2;
        } else {
            lower = index + 1;
        }
    }

    return nullptr;
}

void Dict::Release_Data()
{
    if (m_data == nullptr) {
        return;
    }

    --m_data->m_refCount;

    if (m_data->m_refCount == 0) {
        for (int i = 0; i < m_data->m_numPairsUsed; ++i) {
            m_data->Get_Pairs()[i].Clear();
        }

        g_dynamicMemoryAllocator->Free_Bytes(m_data);
    }

    m_data = nullptr;
}

Dict::DictPair *Dict::Ensure_Unique(int pairs_needed, bool preserve_data, DictPair *to_translate)
{
    if (m_data != nullptr && m_data->m_refCount == 1 && m_data->m_numPairsAllocated >= pairs_needed) {
        return to_translate;
    }

    captainslog_relassert(
        pairs_needed <= INT16_MAX, 0xDEAD0002, "pairs_needed exceeds max permittable with data types currently used.");

    DictPairData *new_data = nullptr;

    if (pairs_needed > 0) {
        // captainslog_trace("Allocating for %d Dict pairs.", pairs_needed);
        int size =
            g_dynamicMemoryAllocator->Get_Actual_Allocation_Size(sizeof(DictPair) * pairs_needed + sizeof(DictPairData));
        new_data = reinterpret_cast<DictPairData *>(g_dynamicMemoryAllocator->Allocate_Bytes(size));

        new_data->m_refCount = 1;
        new_data->m_numPairsAllocated = (size - sizeof(DictPairData)) / sizeof(DictPair);
        new_data->m_numPairsUsed = 0;
        // captainslog_trace("  Allocated for %d Dict pairs.", (int)new_data->m_numPairsAllocated);

        if (preserve_data && m_data != nullptr) {
            // captainslog_trace("  Preserving %d Dict pairs.", (int)m_data->m_numPairsUsed);
            DictPair *newpair = new_data->Get_Pairs();
            DictPair *oldpair = m_data->Get_Pairs();

            for (int i = 0; i < m_data->m_numPairsUsed; ++i) {
                newpair[i].Copy_From(oldpair[i]);
            }

            new_data->m_numPairsUsed = m_data->m_numPairsUsed;
        }
    }

    intptr_t translate_val;

    if (to_translate != nullptr && m_data != nullptr) {
        translate_val = ((uintptr_t)to_translate - (uintptr_t)m_data - sizeof(DictPairData)) / sizeof(DictPair);
        captainslog_trace("  Dict translate position is %d.", translate_val);
    } else {
        translate_val = 0;
        captainslog_trace("  Have no current data to translate from, position is %d.", translate_val);
    }

    Release_Data();
    m_data = new_data;

    if (to_translate != nullptr && new_data != nullptr) {
        captainslog_trace("  Getting translated pair at position %d.", translate_val);
        to_translate = &new_data->Get_Pairs()[translate_val];
    }

    return to_translate;
}
