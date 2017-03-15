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
#include "namekeygenerator.h"
#include "gamedebug.h"

// Initialise object pool resources for Bucket class
NameKeyGenerator::NameKeyGenerator() :
    NextID(NAMEKEY_INVALID)
{
    memset(Sockets, 0, sizeof(Sockets));
}

NameKeyGenerator::~NameKeyGenerator()
{
    Free_Sockets();
}

void NameKeyGenerator::Init()
{
    Free_Sockets();

    NextID = (NameKeyType)1;
}

void NameKeyGenerator::Reset()
{
    Free_Sockets();

    NextID = (NameKeyType)1;
}

AsciiString NameKeyGenerator::Key_To_Name(NameKeyType key)
{
    // Find the bucket that matches the provided key if it exists.
    Bucket *bucket;

    for ( int i = 0; i < SOCKET_COUNT; ++i ) {
        bucket = Sockets[i];

        while ( bucket != nullptr ) {
            if ( bucket->Key == key ) {
                return bucket->NameString;
            }

            bucket = bucket->NextInSocket;
        }
    }

    return AsciiString::EmptyString;
}

NameKeyType NameKeyGenerator::Name_To_Lower_Case_Key(char const *name)
{
    // Calculate a simple hash of the name
    unsigned int socket_hash = 0;

    for ( char const *c = name; *c != '\0'; ++c ) {
        socket_hash = (33 * socket_hash) + tolower(*c);
    }

    // Make sure the hash falls within range of sockets
    socket_hash %= SOCKET_COUNT;

    Bucket *bucket;

    for ( bucket = Sockets[socket_hash]; bucket != nullptr; bucket = bucket->NextInSocket ) {
        if ( strcasecmp(bucket->NameString.Str(), name) == 0 ) {
            return bucket->Key;
        }
    }

    bucket = new Bucket;
    bucket->Key = (NameKeyType)NextID++;
    bucket->NameString = name;
    bucket->NextInSocket = Sockets[socket_hash];
    Sockets[socket_hash] = bucket;

    // Debug info suggests there is some kind of count here to check the longest
    // linked list of buckets and log if its too large and the socket count might
    // need increasing.

    return bucket->Key;
}

NameKeyType NameKeyGenerator::Name_To_Key(char const *name)
{
    // Calculate a simple hash of the name
    unsigned int socket_hash = 0;

    for ( char const *c = name; *c != '\0'; ++c ) {
        socket_hash = (33 * socket_hash) + *c;
    }

    // Make sure the hash falls within range of sockets
    socket_hash %= SOCKET_COUNT;

    Bucket *bucket;

    for ( bucket = Sockets[socket_hash]; bucket != nullptr; bucket = bucket->NextInSocket ) {
        if ( strcmp(bucket->NameString.Str(), name) == 0 ) {
            return bucket->Key;
        }
    }

    bucket = new Bucket;
    bucket->Key = (NameKeyType)NextID++;
    bucket->NameString = name;
    bucket->NextInSocket = Sockets[socket_hash];
    Sockets[socket_hash] = bucket;

    // Debug info suggests there is some kind of count here to check the longest
    // linked list of buckets and log if its too large and the socket count might
    // need increasing.

    return bucket->Key;
}

void NameKeyGenerator::Parse_String_As_NameKeyType(INI *ini, void *formal, void *store, void const *userdata)
{
    *static_cast<NameKeyType*>(store) = TheNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
}

void NameKeyGenerator::Free_Sockets()
{
    // Go over sockets and free them.
    for ( int i = 0; i < SOCKET_COUNT; ++i ) {
        // Delete linked list of entries under given key.
        if ( Sockets[i] != nullptr ) {
            Bucket *bucket = Sockets[i];
            Bucket *next;

            do {
                next = bucket->NextInSocket;
                Delete_Instance(bucket);
                bucket = next;
            } while ( next != nullptr );
        }

        Sockets[i] = nullptr;
    }
}
