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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "namekeygenerator.h"
#include "unicodestring.h"

class Dict
{
    ALLOW_HOOKING
    enum
    {
        MAX_LEN = 0x7FFF,
    };

    enum DictPairKeyType : int32_t
    {
        DICTPAIRKEY_ILLEGAL,
    };

public:
    enum DataType : int32_t
    {
        DICT_NONE = -1,
        DICT_BOOL,
        DICT_INT,
        DICT_REAL,
        DICT_ASCIISTRING,
        DICT_UNICODESTRING,
        DICT_TYPEMAX,
        DICT_KEY_SHIFT = 8,
        DICT_TYPE_MASK = 0xFF,
    };

private:
    union DictPairValue
    {
        DictPairValue() {}
        ~DictPairValue() {}
        bool boolean;
        float real;
        int integer;
        Utf8String ascii;
        Utf16String unicode;
    };

    class DictPair
    {
    public:
        DictPair() {}
        ~DictPair() { Clear(); }

        DictPair &operator=(const DictPair &that)
        {
            if (this != &that) {
                Copy_From(that);
            }
            return *this;
        }

        void Copy_From(const DictPair &that);
        void Clear();
        void Set_Name_And_Type(NameKeyType key, DataType type);

        void Set_Value(int val) { m_value.integer = val; }
        void Set_Value(float val) { m_value.real = val; }
        void Set_Value(bool val) { m_value.boolean = val; }
        void Set_Value(const Utf8String &val) { m_value.ascii = val; }
        void Set_Value(const Utf16String &val) { m_value.unicode = val; }

        const DictPairValue &Get_Value() { return m_value; }

        DataType Get_Type() const { return DataType(m_key & DICT_TYPE_MASK); }
        NameKeyType Get_Key() const { return NameKeyType(m_key >> DICT_KEY_SHIFT); }

    private:
        void Set_Key(NameKeyType key, DataType type) { m_key = DictPairKeyType(type | key << DICT_KEY_SHIFT); }
        DictPairKeyType m_key;
        DictPairValue m_value;
    };

    class DictPairData
    {
        friend class Dict;

        DictPair *Get_Pairs() { return reinterpret_cast<DictPair *>(&this[1]); }

        uint16_t m_refCount;
        uint16_t m_numPairsAllocated;
        uint16_t m_numPairsUsed;
    };

public:
    Dict(const Dict &src);
    Dict(int pair_pre_alloc = 0);
    ~Dict();

    Dict &operator=(const Dict &src);

    void Init(const Dict *dict = nullptr);
    void Clear()
    {
        Release_Data();
        m_data = nullptr;
    }
    bool Known(NameKeyType key, DataType type) const { return type == Get_Type(key); }
    int Get_Pair_Count() const { return m_data->m_numPairsUsed; }
    NameKeyType Get_Nth_Key(int n) const;
    DataType Get_Type(NameKeyType key) const;
    DataType Get_Nth_Type(int n) const;
    bool Get_Bool(NameKeyType key, bool *exists = nullptr) const;
    int Get_Int(NameKeyType key, bool *exists = nullptr) const;
    float Get_Real(NameKeyType key, bool *exists = nullptr) const;
    Utf8String Get_AsciiString(NameKeyType key, bool *exists = nullptr) const;
    Utf16String Get_UnicodeString(NameKeyType key, bool *exists = nullptr) const;
    bool Get_Nth_Bool(int n) const;
    int Get_Nth_Int(int n) const;
    float Get_Nth_Real(int n) const;
    Utf8String Get_Nth_AsciiString(int n) const;
    Utf16String Get_Nth_UnicodeString(int n) const;
    void Set_Bool(NameKeyType key, bool value);
    void Set_Int(NameKeyType key, int value);
    void Set_Real(NameKeyType key, float value);
    void Set_AsciiString(NameKeyType key, const Utf8String &value);
    void Set_UnicodeString(NameKeyType key, const Utf16String &value);
    bool Remove(NameKeyType key);
    void Copy_Pair_From(Dict &that, NameKeyType key);
    void Release_Data();

private:
    DictPair *Ensure_Unique(int pairs_needed, bool preserve_data = false, DictPair *to_translate = nullptr);
    DictPair *Set_Prep(NameKeyType key, DataType type);
    DictPair *Find_Pair_By_Key(NameKeyType key) const;
    void Sort_Pairs();
    static int Pair_Compare(const void *l, const void *r);

private:
    DictPairData *m_data;
};

inline void Dict::Init(const Dict *that)
{
    Clear();

    if (that != nullptr) {
        *this = *that;
    }
}
