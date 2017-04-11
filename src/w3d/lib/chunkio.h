////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: CHUNKIO.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Classes for reading and writing the binary chunk format used
//                 in WW3D and SAGE games.
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

#ifndef _CHUNKIO_H_
#define _CHUNKIO_H_

#include "always.h"
#include "hooker.h"

class FileClass;

struct IOVector2Struct
{
    float x;
    float y;
};

struct IOVector3Struct
{
    float x;
    float y;
    float z;
};

struct IOVector4Struct
{
    float x;
    float y;
    float z;
    float w;
};

struct IOQuaternionStruct
{
    float q[4];
};

class ChunkHeader
{
private:
    enum : uint32_t
    {
        SUB_CHUNK_FLAG = 0x80000000,
    };

public:
    ChunkHeader(uint32_t type, uint32_t size) : m_chunkType(type), m_chunkSize(size) {}
    ChunkHeader() : m_chunkType(), m_chunkSize() {}

    uint32_t Get_Type() { return m_chunkType; }
    uint32_t Get_Size() { return m_chunkSize & (~SUB_CHUNK_FLAG); }
    uint32_t Get_Sub_Chunk_Flag() { return m_chunkSize & SUB_CHUNK_FLAG; }

    void Set_Type(uint32_t type) { m_chunkType = type; }

    void Set_Size(uint32_t size)
    {
        m_chunkSize = (size & (~SUB_CHUNK_FLAG)) | (m_chunkSize & SUB_CHUNK_FLAG);
    }

    void Add_Size(uint32_t size)
    {
        m_chunkSize = ((Get_Size() + size) & (~SUB_CHUNK_FLAG)) | (m_chunkSize & SUB_CHUNK_FLAG);
    }

    void Set_Sub_Chunk_Flag(unsigned char a1) { m_chunkSize |= SUB_CHUNK_FLAG; }

private:
    uint32_t m_chunkType;
    uint32_t m_chunkSize;
};

class MicroChunkHeader
{
public:
    MicroChunkHeader(uint8_t type, uint8_t size) : m_chunkType(type), m_chunkSize(size) {}
    MicroChunkHeader() : m_chunkType(), m_chunkSize() {}

    void Set_Type(uint8_t type) { m_chunkType = type; }
    uint8_t Get_Type() { return m_chunkType; }

    void Set_Size(uint8_t size) { m_chunkSize = size; }
    void Add_Size(uint8_t size) { m_chunkSize += size; }
    uint8_t Get_Size() { return m_chunkSize; }

private:
    uint8_t m_chunkType;
    uint8_t m_chunkSize;
};

////////////////////////////////////////////////////////////////////////////////
/// \brief Westwood Chunk format writer.
/// 
/// This class handles writing to a binary chunk format as used by various 
/// W3D binary file formats.
////////////////////////////////////////////////////////////////////////////////
class ChunkSaveClass
{
private:
    enum
    {
        MAX_STACK_DEPTH = 256
    };

public:
    ChunkSaveClass(FileClass *file);

    bool Begin_Chunk(unsigned int id);
    bool End_Chunk();

    bool Begin_Micro_Chunk(unsigned int id);
    bool End_Micro_Chunk();

    unsigned int Write(void const *buf, unsigned int bytes);
    unsigned int Write(IOVector2Struct const &vect);
    unsigned int Write(IOVector3Struct const &vect);
    unsigned int Write(IOVector4Struct const &vect);
    unsigned int Write(IOQuaternionStruct const &quat);

    int Cur_Chunk_Depth() { return m_stackIndex; }

    static void Hook_Me();
private:
    FileClass *m_file;
    int m_stackIndex;
    int m_positionStack[MAX_STACK_DEPTH];
    ChunkHeader m_headerStack[MAX_STACK_DEPTH];
    bool m_inMicroChunk;
    int m_microChunkPos;
    MicroChunkHeader m_microChunkHeader;
};

////////////////////////////////////////////////////////////////////////////////
/// \brief Westwood Chunk format reader.
/// 
/// This class handles reading from a binary chunk format as used by various 
/// W3D binary file formats.
////////////////////////////////////////////////////////////////////////////////
class ChunkLoadClass
{
private:
    enum
    {
        MAX_STACK_DEPTH = 256
    };

public:
    //TODO check return types
    ChunkLoadClass(FileClass *file);

    bool Open_Chunk();
    bool Close_Chunk();

    unsigned int Cur_Chunk_ID();
    unsigned int Cur_Chunk_Length();

    int Contains_Chunks();

    bool Open_Micro_Chunk();
    bool Close_Micro_Chunk();

    unsigned int Cur_Micro_Chunk_ID();
    unsigned int Cur_Micro_Chunk_Length();

    unsigned int Seek(unsigned int bytes);

    unsigned int Read(void *buf, unsigned int bytes);
    unsigned int Read(IOVector2Struct *vect);
    unsigned int Read(IOVector3Struct *vect);
    unsigned int Read(IOVector4Struct *vect);
    unsigned int Read(IOQuaternionStruct *quat);

    int Cur_Chunk_Depth() { return m_stackIndex; }

    static void Hook_Me();
private:
    FileClass *m_file;
    int m_stackIndex;
    int m_positionStack[MAX_STACK_DEPTH];
    ChunkHeader m_headerStack[MAX_STACK_DEPTH];
    bool m_inMicroChunk;
    int m_microChunkPos;
    MicroChunkHeader m_microChunkHeader;
};

inline void ChunkSaveClass::Hook_Me()
{
    Hook_Method(Make_Method_Ptr<bool, ChunkSaveClass, unsigned int>(0x008A0F90), &Begin_Chunk);
    Hook_Method(Make_Method_Ptr<bool, ChunkSaveClass>(0x008A1020), &End_Chunk);
    Hook_Method(Make_Method_Ptr<bool, ChunkSaveClass, unsigned int>(0x008A10D0), &Begin_Micro_Chunk);
    Hook_Method(Make_Method_Ptr<bool, ChunkSaveClass>(0x008A1160), &End_Micro_Chunk);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkSaveClass, void const*, unsigned int>(0x008A11C0), &Write);
}

inline void ChunkLoadClass::Hook_Me()
{
    Hook_Method(Make_Method_Ptr<bool, ChunkLoadClass>(0x008A1290), &Open_Chunk);
    Hook_Method(Make_Method_Ptr<bool, ChunkLoadClass>(0x008A12E0), &Close_Chunk);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkLoadClass>(0x008A1330), &Cur_Chunk_ID);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkLoadClass>(0x008A1340), &Cur_Chunk_Length);
    Hook_Method(Make_Method_Ptr<bool, ChunkLoadClass>(0x008A1350), &Open_Micro_Chunk);
    Hook_Method(Make_Method_Ptr<bool, ChunkLoadClass>(0x008A1380), &Close_Micro_Chunk);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkLoadClass>(0x008A13D0), &Cur_Micro_Chunk_ID);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkLoadClass>(0x008A13E0), &Cur_Micro_Chunk_Length);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkLoadClass, unsigned int>(0x008A13F0), &Seek);
    Hook_Method(Make_Method_Ptr<unsigned int, ChunkLoadClass, void*, unsigned int>(0x008A1480), &Read);
}

#endif // _CHUNKIO_H_
