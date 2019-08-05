/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for reading and writing the binary chunk format used in WW3D and SAGE games.
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
    ChunkHeader(uint32_t type = 0, uint32_t size = 0) : m_chunkType(type), m_chunkSize(size) {}

    uint32_t Get_Type() { return m_chunkType; }
    uint32_t Get_Size() { return m_chunkSize & (~SUB_CHUNK_FLAG); }
    uint32_t Get_Sub_Chunk_Flag() { return m_chunkSize & SUB_CHUNK_FLAG; }
    void Set_Type(uint32_t type) { m_chunkType = type; }
    void Set_Size(uint32_t size) { m_chunkSize = (size & (~SUB_CHUNK_FLAG)) | (m_chunkSize & SUB_CHUNK_FLAG); }

    void Add_Size(uint32_t size)
    {
        m_chunkSize = ((Get_Size() + size) & (~SUB_CHUNK_FLAG)) | (m_chunkSize & SUB_CHUNK_FLAG);
    }

    void Set_Sub_Chunk_Flag(uint8_t a1) { m_chunkSize |= SUB_CHUNK_FLAG; }

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

/**
 * @brief Westwood Chunk format writer.
 *
 * This class handles writing to a binary chunk format as used by various
 * W3D binary file formats.
 */
class ChunkSaveClass
{
private:
    enum
    {
        MAX_STACK_DEPTH = 256
    };

public:
    ChunkSaveClass(FileClass *file);

    bool Begin_Chunk(unsigned id);
    bool End_Chunk();

    bool Begin_Micro_Chunk(unsigned id);
    bool End_Micro_Chunk();

    unsigned Write(const void *buf, unsigned bytes);
    unsigned Write(const IOVector2Struct &vect);
    unsigned Write(const IOVector3Struct &vect);
    unsigned Write(const IOVector4Struct &vect);
    unsigned Write(const IOQuaternionStruct &quat);

    int Cur_Chunk_Depth() { return m_stackIndex; }

private:
    FileClass *m_file;
    int m_stackIndex;
    int m_positionStack[MAX_STACK_DEPTH];
    ChunkHeader m_headerStack[MAX_STACK_DEPTH];
    bool m_inMicroChunk;
    int m_microChunkPos;
    MicroChunkHeader m_microChunkHeader;
};

/**
 * @brief Westwood Chunk format reader.
 *
 * This class handles reading from a binary chunk format as used by various
 * W3D binary file formats.
 */
class ChunkLoadClass
{
private:
    enum
    {
        MAX_STACK_DEPTH = 256
    };

public:
    // TODO check return types
    ChunkLoadClass(FileClass *file);

    bool Open_Chunk();
    bool Close_Chunk();

    unsigned Cur_Chunk_ID();
    unsigned Cur_Chunk_Length();

    int Contains_Chunks();

    bool Open_Micro_Chunk();
    bool Close_Micro_Chunk();

    unsigned Cur_Micro_Chunk_ID();
    unsigned Cur_Micro_Chunk_Length();

    unsigned Seek(unsigned bytes);

    unsigned Read(void *buf, unsigned bytes);
    unsigned Read(IOVector2Struct *vect);
    unsigned Read(IOVector3Struct *vect);
    unsigned Read(IOVector4Struct *vect);
    unsigned Read(IOQuaternionStruct *quat);

    int Cur_Chunk_Depth() { return m_stackIndex; }

private:
    FileClass *m_file;
    int m_stackIndex;
    int m_positionStack[MAX_STACK_DEPTH];
    ChunkHeader m_headerStack[MAX_STACK_DEPTH];
    bool m_inMicroChunk;
    int m_microChunkPos;
    MicroChunkHeader m_microChunkHeader;
};
