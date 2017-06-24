////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: NAMEKEYGENERATOR.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Maps strings to 32bit integers.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef NAMEKEYGENERATOR_H
#define NAMEKEYGENERATOR_H

#include "asciistring.h"
#include "bittype.h"
#include "hooker.h"
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

public:
    Bucket() : m_nextInSocket(nullptr), m_key(NAMEKEY_INVALID), m_nameString() {}
    virtual ~Bucket() {}

public:
    Bucket *m_nextInSocket;
    NameKeyType m_key;
    AsciiString m_nameString;
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
    AsciiString Key_To_Name(NameKeyType key);
    NameKeyType Name_To_Lower_Case_Key(const char *name);
    NameKeyType Name_To_Key(const char *name);

    static void Parse_String_As_NameKeyType(INI *ini, void *formal, void *store, void const *userdata);

    static void Hook_Me();
private:
    void Free_Sockets();

private:
    Bucket *m_sockets[SOCKET_COUNT];
    NameKeyType m_nextID;
};

class StaticNameKey
{
public:
    StaticNameKey(const char *name) : m_key(NAMEKEY_INVALID), m_name(name) {}

    operator NameKeyType() { return Key(); }

    NameKeyType Key();
    const char *Name() { return m_name; }

private:
    NameKeyType m_key;
    const char *m_name;
};

#define g_theNameKeyGenerator (Make_Global<NameKeyGenerator*>(0x00A2B928))
//extern NameKeyGenerator *g_theNameKeyGenerator;

inline void NameKeyGenerator::Hook_Me()
{
    Hook_Method((Make_Method_Ptr<AsciiString, NameKeyGenerator, NameKeyType>(0x0047B2F0)), &NameKeyGenerator::Key_To_Name);
    Hook_Method((Make_Method_Ptr<NameKeyType, NameKeyGenerator, char const*>(0x0047B360)), &NameKeyGenerator::Name_To_Key);
    Hook_Method((Make_Method_Ptr<NameKeyType, NameKeyGenerator, char const*>(0x0047B500)), &NameKeyGenerator::Name_To_Lower_Case_Key);
}

#endif
