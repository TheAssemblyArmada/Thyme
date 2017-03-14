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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _NAMEKEYGENERATOR_H_
#define _NAMEKEYGENERATOR_H_

#include "asciistring.h"
#include "bittype.h"
#include "hooker.h"
#include "ini.h"
#include "macros.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"

#define TheNameKeyGenerator (Make_Global<NameKeyGenerator*>(0x00A2B928))

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
    Bucket() : NextInSocket(nullptr), Key(NAMEKEY_INVALID), NameString() {}
    virtual ~Bucket() {}

public:
    Bucket *NextInSocket;
    NameKeyType Key;
    AsciiString NameString;
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
    NameKeyType Name_To_Lower_Case_Key(char const *name);
    NameKeyType Name_To_Key(char const *name);

    static void Parse_String_As_NameKeyType(INI *ini, void *formal, void *store, void const *userdata);

private:
    void Free_Sockets();

private:
    Bucket *Sockets[SOCKET_COUNT];
    int32_t NextID;
};

#endif
