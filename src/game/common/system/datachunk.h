/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for handling the binary chunk format used by SAGE engine maps.
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
#include "chunkinputstream.h"
#include "datachunktoc.h"
#include "dict.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "unicodestring.h"

// Mac ZH also includes DataChunkOutput class, presumably for writing maps though it doesn't appear in the windows binary.

class DataChunkOutput;

struct DataChunkInfo
{
    Utf8String label;
    Utf8String parent_label;
    uint16_t version;
    int data_size;
};

class DataChunkInput
{
    struct InputChunk : public MemoryPoolObject
    {
        IMPLEMENT_POOL(InputChunk);
        friend class DataChunkInput;

    protected:
        virtual ~InputChunk() override {}

    private:
        InputChunk *next;
        uint32_t id;
        uint16_t version;
        int32_t chunk_start;
        int32_t data_size;
        int32_t data_left;
    };

    struct UserParser : public MemoryPoolObject
    {
        IMPLEMENT_POOL(UserParser);
        friend class DataChunkInput;

    protected:
        virtual ~UserParser() override {}

    private:
        UserParser *next;
        bool (*parser)(DataChunkInput &, DataChunkInfo *, void *);
        Utf8String label;
        Utf8String parent_label;
        void *user_data;
    };

public:
    DataChunkInput(ChunkInputStream *stream);
    ~DataChunkInput();

    void Register_Parser(const Utf8String &label,
        const Utf8String &parent_label,
        bool (*parser)(DataChunkInput &, DataChunkInfo *, void *),
        void *user_data);
    bool Parse(void *user_data);
    bool Is_Valid_File() { return m_contents.Header_Opened(); }
    Utf8String Open_Data_Chunk(uint16_t *version);
    void Close_Data_Chunk();
    bool At_End_Of_File() { return m_file->Eof(); }
    bool At_End_Of_Chunk() { return m_chunkStack != nullptr ? m_chunkStack->data_left <= 0 : true; }
    void Reset();
    Utf8String Get_Chunk_Label();
    uint16_t Get_Chunk_Version() { return m_chunkStack != nullptr ? m_chunkStack->version : 0; }
    int Get_Chunk_Data_Size() { return m_chunkStack != nullptr ? m_chunkStack->data_size : 0; }
    int Get_Chunk_Data_Left() { return m_chunkStack != nullptr ? m_chunkStack->data_left : 0; }
    float Read_Real32();
    int32_t Read_Int32();
    uint8_t Read_Byte();
    Utf8String Read_AsciiString();
    Utf16String Read_UnicodeString();
    Dict Read_Dict();
    void Read_Byte_Array(uint8_t *ptr, int length);
    NameKeyType Read_Name_Key();

private:
    void Decrement_Data_Left(int size);
    void Clear_Chunk_Stack();

private:
    ChunkInputStream *m_file;
    DataChunkTableOfContents m_contents;
    int m_fileposOfFirstChunk;
    UserParser *m_parserList;
    InputChunk *m_chunkStack;
    void *m_currentObject;
    void *m_userData;
    friend class WorldHeightMap;
    friend bool Parse_Objects_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
};
