/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Maps strings to 32bit integers.
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
#include "bittype.h"
#include "ini.h"
#include "macros.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"

enum NameKeyType : int32_t
{
    NAMEKEY_INVALID = 0,
    NAMEKEY_MAX = 0x80000,
};

DEFINE_ENUMERATION_OPERATORS(NameKeyType);

class Bucket : public MemoryPoolObject
{
    IMPLEMENT_NAMED_POOL(Bucket, NameKeyBucketPool);

protected:
    virtual ~Bucket() override {}

public:
    Bucket() : m_nextInSocket(nullptr), m_key(NAMEKEY_INVALID), m_nameString() {}

public:
    Bucket *m_nextInSocket;
    NameKeyType m_key;
    Utf8String m_nameString;
};

class NameKeyGenerator : public SubsystemInterface
{
    enum
    {
        SOCKET_COUNT = 0xAFCF,
    };

public:
    NameKeyGenerator();
    virtual ~NameKeyGenerator();

    // SubsystemInterface implementation
    virtual void Init();
    virtual void Reset();
    virtual void Update() {}

    // Key to name functions
    Utf8String Key_To_Name(NameKeyType key);
    NameKeyType Name_To_Lower_Case_Key(const char *name);
    NameKeyType Name_To_Key(const char *name);

    static void Parse_String_As_NameKeyType(INI *ini, void *formal, void *store, void const *userdata);

private:
    void Free_Sockets();

private:
    Bucket *m_sockets[SOCKET_COUNT];
    NameKeyType m_nextID;
};

NameKeyType Name_To_Key(const char *name);

#ifdef GAME_DLL
extern NameKeyGenerator *&g_theNameKeyGenerator;
#else
extern NameKeyGenerator *g_theNameKeyGenerator;
#endif