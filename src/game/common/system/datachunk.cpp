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
#include "datachunk.h"
#include "endiantype.h"

void DataChunkInput::Decrement_Data_Left(int size)
{
    for (InputChunk *chunk = m_chunkStack; chunk != nullptr; chunk = chunk->next) {
        chunk->data_left -= size;
    }
}

void DataChunkInput::Clear_Chunk_Stack()
{
    InputChunk *current = m_chunkStack;

    while (current != nullptr) {
        InputChunk *next = current->next;
        current->Delete_Instance();
        current = next;
    }

    m_chunkStack = nullptr;
}

DataChunkInput::DataChunkInput(ChunkInputStream *stream) :
    m_file(stream), m_contents(), m_parserList(nullptr), m_chunkStack(nullptr), m_currentObject(nullptr), m_userData(nullptr)
{
    m_contents.Read(*m_file);
    m_fileposOfFirstChunk = m_file->Tell();
}

// 0x00572370
DataChunkInput::~DataChunkInput()
{
    Clear_Chunk_Stack();

    auto *current = m_parserList;
    while (current != nullptr) {
        auto *next = current->next;
        current->Delete_Instance();
        current = next;
    }
    m_parserList = nullptr;
}

/**
 * @brief Registers a chunk parsing function to handle chunks with the given label and parent label.
 */
void DataChunkInput::Register_Parser(const Utf8String &label,
    const Utf8String &parent_label,
    bool (*parser)(DataChunkInput &, DataChunkInfo *, void *),
    void *user_data)
{
    UserParser *user_parser = NEW_POOL_OBJ(UserParser);
    user_parser->label = label;
    user_parser->parent_label = parent_label;
    user_parser->parser = parser;
    user_parser->user_data = user_data;
    user_parser->next = m_parserList;
    m_parserList = user_parser;
}

/**
 * @brief Parse the file using all currently registered chunk parsers.
 */
bool DataChunkInput::Parse(void *user_data)
{
    Utf8String label;
    Utf8String parent_label;

    // We can't parse if the header chunk hasn't been opened yet.
    if (!Is_Valid_File()) {
        captainslog_debug("Attempted to parse a chunk file before TOC has been read.");
        return false;
    }

    if (m_chunkStack != nullptr) {
        parent_label = m_contents.Get_Name(m_chunkStack->id);
    }

    while (!At_End_Of_File()) {
        if (m_chunkStack != nullptr) {
            if (m_chunkStack->data_left < 4) {
                break;
            }
        }

        uint16_t version;
        label = Open_Data_Chunk(&version);

        if (At_End_Of_File()) {
            break;
        }

        // Find suitable parser for the current chunk.
        UserParser *parser;
        for (parser = m_parserList; parser != nullptr; parser = parser->next) {
            if (label == parser->label && parent_label == parser->parent_label) {
                DataChunkInfo info;
                info.label = label;
                info.parent_label = parent_label;
                info.version = version;
                info.data_size = Get_Chunk_Data_Size();

                // If parsing failed, return false.
                if (!parser->parser(*this, &info, user_data)) {
                    return false;
                }

                break;
            }
        }

        // Close the current chunk and remove it from the stack.
        Close_Data_Chunk();
    }

    return true;
}

/**
 * @brief Opens a data chunk and returns the label associated with it. Optionally returns version in passed pointer.
 */
Utf8String DataChunkInput::Open_Data_Chunk(uint16_t *version)
{
    InputChunk *chunk = NEW_POOL_OBJ(InputChunk);
    chunk->id = 0;
    chunk->version = 0;
    chunk->data_size = 0;

    m_file->Read(&chunk->id, sizeof(chunk->id));
    chunk->id = le32toh(chunk->id);
    Decrement_Data_Left(sizeof(chunk->id));

    m_file->Read(&chunk->version, sizeof(chunk->version));
    chunk->version = le16toh(chunk->version);
    Decrement_Data_Left(sizeof(chunk->version));

    m_file->Read(&chunk->data_size, sizeof(chunk->data_size));
    chunk->data_size = le32toh(chunk->data_size);
    Decrement_Data_Left(sizeof(chunk->data_size));

    if (version != nullptr) {
        *version = chunk->version;
    }

    chunk->data_left = chunk->data_size;
    chunk->chunk_start = m_file->Tell();
    chunk->next = m_chunkStack;
    m_chunkStack = chunk;

    if (m_file->Eof()) {
        return Utf8String::s_emptyString;
    }

    return m_contents.Get_Name(chunk->id);
}

/**
 * @brief Closes the current chunk and pops its entry off the stack.
 */
void DataChunkInput::Close_Data_Chunk()
{
    if (m_chunkStack == nullptr) {
        return;
    }

    if (m_chunkStack->data_left > 0) {
        m_file->Absolute_Seek(m_chunkStack->data_left + m_file->Tell());
        Decrement_Data_Left(m_chunkStack->data_left);
    }

    InputChunk *done = m_chunkStack;
    m_chunkStack = m_chunkStack->next;
    done->Delete_Instance();
}

/**
 * @brief Empties the chunk stack and positions the file stream at the start of the first chunk.
 */
void DataChunkInput::Reset()
{
    Clear_Chunk_Stack();
    m_file->Absolute_Seek(m_fileposOfFirstChunk);
}

Utf8String DataChunkInput::Get_Chunk_Label()
{
    if (m_chunkStack != nullptr) {
        return m_contents.Get_Name(m_chunkStack->id);
    }

    return Utf8String::s_emptyString;
}

/**
 * @brief Reads a 32bit floating point value from the chunk.
 */
float DataChunkInput::Read_Real32()
{
    // Read as 32bit int so we can byteswap easily and then type pun for the return.
    uint32_t tmp;

    captainslog_dbgassert(m_chunkStack->data_left >= sizeof(tmp), "Read past end of chunk reading a float.");

    m_file->Read(&tmp, sizeof(tmp));
    Decrement_Data_Left(sizeof(tmp));
    tmp = le32toh(tmp);

    // Use aliasing type for the cast as GCC and Clang enforce aliasing rules which might break this otherwise.
    return *reinterpret_cast<float_a *>(&tmp);
}

/**
 * @brief Reads a 32bit integer value from the chunk.
 */
int32_t DataChunkInput::Read_Int32()
{
    int32_t tmp;

    captainslog_dbgassert(m_chunkStack->data_left >= sizeof(tmp), "Read past end of chunk reading an int.");

    m_file->Read(&tmp, sizeof(tmp));
    Decrement_Data_Left(sizeof(tmp));
    tmp = le32toh(tmp);

    return tmp;
}

/**
 * @brief Reads a byte from the chunk.
 */
uint8_t DataChunkInput::Read_Byte()
{
    uint8_t tmp;

    captainslog_dbgassert(m_chunkStack->data_left >= sizeof(tmp), "Read past end of chunk reading a byte.");

    m_file->Read(&tmp, sizeof(tmp));
    Decrement_Data_Left(sizeof(tmp));

    return tmp;
}

/**
 * @brief Reads an ascii string from the chunk.
 */
Utf8String DataChunkInput::Read_AsciiString()
{
    uint16_t size;
    Utf8String string;

    captainslog_dbgassert(m_chunkStack->data_left >= sizeof(size), "Read past end of chunk reading Utf8String length.");

    m_file->Read(&size, sizeof(size));
    Decrement_Data_Left(sizeof(size));
    size = le16toh(size);

    captainslog_dbgassert(m_chunkStack->data_left >= size, "Read past end of chunk reading Utf8String string.");

    char *buf = string.Get_Buffer_For_Read(size);
    m_file->Read(buf, size);
    Decrement_Data_Left(size);
    buf[size] = '\0';

    return string;
}

/**
 * @brief Reads a UCS-16 string from the chunk.
 */
Utf16String DataChunkInput::Read_UnicodeString()
{
    uint16_t size;
    char16_t ch;
    Utf16String string;

    captainslog_dbgassert(m_chunkStack->data_left >= sizeof(size), "Read past end of chunk reading Utf8String length.");

    m_file->Read(&size, sizeof(size));
    Decrement_Data_Left(sizeof(size));
    size = le16toh(size);

    captainslog_dbgassert(
        m_chunkStack->data_left >= int(sizeof(ch) * size), "Read past end of chunk reading Utf16String string.");

    // Data is stored as LE UCS-16, so only BMP unicode chars can be stored.
    // TODO revamp unicode handling for none windows systems using libicu or libiconv.
    for (unsigned i = 0; i < size; ++i) {
        m_file->Read(&ch, sizeof(ch));
        ch = le16toh(ch);
        string += ch;
    }

    Decrement_Data_Left(sizeof(ch) * size);

    return string;
}

/**
 * @brief Reads a Dict from the chunk.
 */
Dict DataChunkInput::Read_Dict()
{
    uint16_t size = 0;

    captainslog_dbgassert(m_chunkStack->data_left >= sizeof(size), "Read past end of chunk reading Utf8String length.");

    m_file->Read(&size, sizeof(size));
    Decrement_Data_Left(sizeof(size));
    size = le16toh(size);

    captainslog_dbgassert(m_chunkStack->data_left >= size, "Read past end of chunk reading Dict.");

    Dict dict(size);

    for (int i = 0; i < size; ++i) {
        uint32_t key = Read_Int32();
        // Generate a new name key based on the one that was stored in the file.
        NameKeyType nk = g_theNameKeyGenerator->Name_To_Key(m_contents.Get_Name(key >> Dict::DICT_KEY_SHIFT).Str());

        switch (key & Dict::DICT_TYPE_MASK) {
            case Dict::DICT_BOOL:
                dict.Set_Bool(nk, bool(Read_Byte()));
                break;
            case Dict::DICT_INT:
                dict.Set_Int(nk, Read_Int32());
                break;
            case Dict::DICT_REAL:
                dict.Set_Real(nk, Read_Real32());
                break;
            case Dict::DICT_ASCIISTRING:
                dict.Set_AsciiString(nk, Read_AsciiString());
                break;
            case Dict::DICT_UNICODESTRING:
                dict.Set_UnicodeString(nk, Read_UnicodeString());
                break;
            default:
                captainslog_relassert(
                    false, 0xDEAD0005, "Attempting to handle unknown Dict type data, code shouldn't reach here.");
        }
    }

    return dict;
}

/**
 * @brief Reads an array of bytes from the chunk.
 */
void DataChunkInput::Read_Byte_Array(uint8_t *ptr, int length)
{
    captainslog_dbgassert(m_chunkStack->data_left >= length, "Read past end of chunk reading Utf8String string.");

    m_file->Read(ptr, length);
    Decrement_Data_Left(length);
}

/**
 * @brief Reads a NameKeyType from the chunk.
 */
NameKeyType DataChunkInput::Read_Name_Key()
{
    uint32_t key = Read_Int32();

    return g_theNameKeyGenerator->Name_To_Key(m_contents.Get_Name(key >> Dict::DICT_KEY_SHIFT).Str());
}
