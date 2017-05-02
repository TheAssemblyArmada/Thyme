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
    m_nextID(NAMEKEY_INVALID)
{
    memset(m_sockets, 0, sizeof(m_sockets));
}

NameKeyGenerator::~NameKeyGenerator()
{
    Free_Sockets();
}

void NameKeyGenerator::Init()
{
    Free_Sockets();

    m_nextID = (NameKeyType)1;
}

void NameKeyGenerator::Reset()
{
    Free_Sockets();

    m_nextID = (NameKeyType)1;
}

AsciiString NameKeyGenerator::Key_To_Name(NameKeyType key)
{
    // Find the bucket that matches the provided key if it exists.
    Bucket *bucket;

    for ( int i = 0; i < SOCKET_COUNT; ++i ) {
        bucket = m_sockets[i];

        while ( bucket != nullptr ) {
            if ( bucket->m_key == key ) {
                return bucket->m_nameString;
            }

            bucket = bucket->m_nextInSocket;
        }
    }

    return AsciiString::s_emptyString;
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

    for ( bucket = m_sockets[socket_hash]; bucket != nullptr; bucket = bucket->m_nextInSocket ) {
        if ( strcasecmp(bucket->m_nameString.Str(), name) == 0 ) {
            return bucket->m_key;
        }
    }

    bucket = new Bucket;
    bucket->m_key = (NameKeyType)m_nextID++;
    bucket->m_nameString = name;
    bucket->m_nextInSocket = m_sockets[socket_hash];
    m_sockets[socket_hash] = bucket;

    // Debug info suggests there is some kind of count here to check the longest
    // linked list of buckets and log if its too large and the socket count might
    // need increasing.

    return bucket->m_key;
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

    for ( bucket = m_sockets[socket_hash]; bucket != nullptr; bucket = bucket->m_nextInSocket ) {
        if ( strcmp(bucket->m_nameString.Str(), name) == 0 ) {
            return bucket->m_key;
        }
    }

    bucket = new Bucket;
    bucket->m_key = (NameKeyType)m_nextID++;
    bucket->m_nameString = name;
    bucket->m_nextInSocket = m_sockets[socket_hash];
    m_sockets[socket_hash] = bucket;

    // Debug info suggests there is some kind of count here to check the longest
    // linked list of buckets and log if its too large and the socket count might
    // need increasing.

    return bucket->m_key;
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
        if ( m_sockets[i] != nullptr ) {
            Bucket *bucket = m_sockets[i];
            Bucket *next;

            do {
                next = bucket->m_nextInSocket;
                Delete_Instance(bucket);
                bucket = next;
            } while ( next != nullptr );
        }

        m_sockets[i] = nullptr;
    }
}

NameKeyType StaticNameKey::Key()
{
    if ( m_key == NAMEKEY_INVALID && TheNameKeyGenerator != nullptr ) {
        m_key = TheNameKeyGenerator->Name_To_Key(m_name);
    }

    return m_key;
}
