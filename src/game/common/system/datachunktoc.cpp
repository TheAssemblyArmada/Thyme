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
#include "datachunktoc.h"
#include "endiantype.h"
#include "rtsutils.h"
#include <algorithm>
#include <captainslog.h>

using rts::FourCC;

DataChunkTableOfContents::~DataChunkTableOfContents()
{
    Mapping *map = m_list;

    while (map != nullptr) {
        Mapping *next = map->m_next;
        map->Delete_Instance();
        map = next;
    }
}

/**
 * @brief Get the ID of an entry with the given name.
 */
unsigned DataChunkTableOfContents::Get_ID(const Utf8String &name)
{
    Mapping *map = Find_Mapping(name);

    if (map != nullptr) {
        return map->m_id;
    }

    captainslog_error("Name '%s' not found trying to get ID from DataChunk TOC.", name.Str());

    return 0;
}

/**
 * @brief Get the name of an entry with the given ID.
 */
Utf8String DataChunkTableOfContents::Get_Name(unsigned id)
{
    Mapping *map = m_list;

    while (map != nullptr) {
        if (id == map->m_id) {
            return map->m_name;
        }

        map = map->m_next;
    }

    captainslog_error("ID '%u' not found trying to get name from DataChunk TOC.", id);

    return Utf8String::s_emptyString;
}

/**
 * @brief Get the ID of an entry with the given name or create it if it doesn't exist.
 */
unsigned DataChunkTableOfContents::Allocate_ID(const Utf8String &name)
{
    Mapping *map = Find_Mapping(name);

    if (map != nullptr) {
        return map->m_id;
    }

    map = NEW_POOL_OBJ(Mapping);
    map->m_name = name;
    map->m_id = m_nextID++;
    map->m_next = m_list;
    m_list = map;
    ++m_listLength;

    return map->m_id;
}

/**
 * @brief Read the data chunk TOC from the given data stream.
 */
void DataChunkTableOfContents::Read(ChunkInputStream &stream)
{
    uint32_t fourcc = FourCC<'x', 'x', 'x', 'x'>::value;
    stream.Read(&fourcc, sizeof(fourcc));

    // Check we are reading a valid TOC chunk.
    if (fourcc == FourCC<'C', 'k', 'M', 'p'>::value) {
        unsigned max_id = 0;
        int32_t count;
        stream.Read(&count, sizeof(count));
        count = le32toh(count);

        // Process the entries in the TOC.
        for (int i = 0; i < count; ++i) {
            Mapping *map = NEW_POOL_OBJ(Mapping);
            uint8_t name_len;
            stream.Read(&name_len, sizeof(name_len));

            // If we have a name length, read it and null terminate it.
            if (name_len > 0) {
                char *buf = map->m_name.Get_Buffer_For_Read(name_len);
                stream.Read(buf, name_len);
                buf[name_len] = '\0';
            }

            stream.Read(&map->m_id, sizeof(map->m_id));
            map->m_id = le32toh(map->m_id);
            map->m_next = m_list;
            m_list = map;
            ++m_listLength;
            max_id = std::max(max_id, map->m_id);
        }

        m_headerOpened = !(count <= 0 || stream.Eof());
        m_nextID = std::max(m_nextID, max_id + 1);
    }
}

/**
 * @brief Write the data chunk TOC to the given data stream.
 */
void DataChunkTableOfContents::Write(OutputStream &stream)
{
    // Write the fourcc id tag and the number of entries.
    uint32_t fourcc = FourCC<'C', 'k', 'M', 'p'>::value;
    stream.Write(&fourcc, sizeof(fourcc));
    int32_t count = htole32(m_listLength);
    stream.Write(&count, sizeof(count));

    Mapping *map = m_list;

    // Write out the contents of the entries in the TOC.
    while (map != nullptr) {
        const char *string = map->m_name.Str();
        uint8_t slen = map->m_name.Get_Length();
        uint32_t id = htole32(map->m_id);

        stream.Write(&slen, sizeof(slen));
        stream.Write(string, slen);
        stream.Write(&id, sizeof(id));

        map = map->m_next;
    }
}

/**
 * @brief Find the Mapping entry corresponding to the given name.
 */
DataChunkTableOfContents::Mapping *DataChunkTableOfContents::Find_Mapping(const Utf8String &name)
{
    Mapping *map = m_list;

    while (map != nullptr) {
        if (name == map->m_name) {
            return map;
        }

        map = map->m_next;
    }

    return nullptr;
}
