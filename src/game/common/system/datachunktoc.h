/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling the binary chunk format table of contents used by SAGE engine maps.
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
#include "mempoolobj.h"
#include "outputstream.h"

class DataChunkTableOfContents
{
    class Mapping : public MemoryPoolObject
    {
        IMPLEMENT_POOL(Mapping);
        friend class DataChunkTableOfContents;

    protected:
        virtual ~Mapping() override {}

    private:
        Mapping() : m_next(nullptr), m_name(), m_id(0) {}

    private:
        Mapping *m_next;
        Utf8String m_name;
        unsigned m_id;
    };

public:
    DataChunkTableOfContents() : m_list(nullptr), m_listLength(0), m_nextID(1), m_headerOpened(false) {}
    ~DataChunkTableOfContents();

    unsigned Get_ID(const Utf8String &name);
    Utf8String Get_Name(unsigned id);
    unsigned Allocate_ID(const Utf8String &name);
    void Read(ChunkInputStream &stream);
    void Write(OutputStream &stream);
    bool Header_Opened() { return m_headerOpened; }

private:
    Mapping *Find_Mapping(const Utf8String &name);

private:
    Mapping *m_list;
    int m_listLength;
    unsigned m_nextID;
    bool m_headerOpened;
};