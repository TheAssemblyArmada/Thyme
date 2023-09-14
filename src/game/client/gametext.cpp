/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief String file handler.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gametext.h"
#include "filesystem.h"
#include "main.h" // For g_applicationHWnd
#include "registryget.h"
#include "rtsutils.h"
#include <algorithm>
#include <captainslog.h>
#include <cctype>

#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#endif

#ifdef BUILD_WITH_SDL2
#include <SDL.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef GAME_DLL
#include "hookcrt.h"
#endif

using rts::FourCC;

#ifndef GAME_DLL
GameTextInterface *g_theGameText = nullptr;
#endif

// Comparison function used for sorting and searching StringLookUp arrays.
int GameTextManager::Compare_LUT(void const *a, void const *b)
{
    const char *ac = static_cast<StringLookUp const *>(a)->label->Str();
    const char *bc = static_cast<StringLookUp const *>(b)->label->Str();

    return strcasecmp(ac, bc);
}

GameTextInterface *GameTextManager::Create_Game_Text_Interface()
{
    return new GameTextManager;
}

// Get a char from a file.
char GameTextManager::Read_Char(File *file)
{
    char tmp;

    if (file->Read(&tmp, sizeof(tmp)) == sizeof(tmp)) {
        return tmp;
    }

    return '\0';
}

// Read a quoted string and also any extra data.
void GameTextManager::Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len)
{
    bool escape = false;
    int i;

    for (i = 0; i < buff_len; ++i) {
        char current;

        // if in pointer is valid, read data from that, otherwise read from file.
        if (in != nullptr) {
            current = *in++;

            if (current == '\0') {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // -1 and we are done?
        if (current == '\xFF') {
            return;
        }

        // Handle some special characters.
        if (current == '\n') {
            escape = false;
            current = ' ';
        } else if (current == '\\') {
            escape = !escape;
        } else if (current == '"' && !escape) {
            break;
        } else {
            escape = false;
        }

        // Treat any white space as a space char.
        if (isspace(uint8_t(current))) {
            current = ' ';
        }

        // Copy to output buffer.
        out[i] = current;
    }

    out[i] = '\0';

    int wave_pos = 0;
    int state = 0;

    while (true) {
        char current;

        // if in pointer is valid, read data from that, otherwise read from file.
        if (in != nullptr) {
            current = *in++;

            if (current == '\0') {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // Stop reading on line break or -1 char.
        if (current == '\n' || current == '\xFF') {
            break;
        }

        // state 0 ignores initial whitespace and '='
        if (state == 0 && !isspace(uint8_t(current)) && current != '=') {
            state = 1;
        }

        // state 1 read so long as its alphanumeric characters or underscore
        if (state == 1) {
            if ((current < 'a' || current > 'z') && (current < 'A' || current > 'Z') && (current < '0' || current > '9')
                && current != '_') {
                state = 2;
            } else {
                wave[wave_pos++] = current;
            }
        }

        // state 2 ignore everything and keep reading until a breaking condition is encountered
    }

    if (wave_pos > 0) {
        if (wave[wave_pos - 1] >= '0' && wave[wave_pos - 1] <= '9') {
            wave[wave_pos++] = 'e';
        }
    }

    wave[wave_pos] = '\0';
}

// Read a line from a str file into provided buffer.
bool GameTextManager::Read_Line(char *buffer, int length, File *file)
{
    bool ret = false;
    char *putp = buffer;

    for (int i = 0; i < length; ++i) {
        if (file->Read(putp, sizeof(*putp)) != sizeof(*putp)) {
            break;
        }

        ret = true;

        if (*putp == '\n') {
            break;
        }

        ++putp;
    }

    *putp = '\0';

    return ret;
}

// Converts an ASCII string into a usc2/utf16 string.
void GameTextManager::Translate_Copy(unichar_t *out, char *in)
{
    // Unsigned allows handling Latin extended code page.
    unsigned char *in_unsigned = reinterpret_cast<unsigned char *>(in);
    unsigned char current = *in_unsigned++;
    bool escape = false;

    while (current != '\0') {
        // Last char was a '\', handle for escape sequence.
        if (escape) {
            escape = false;

            switch (current) {
                case '\\':
                    *out++ = U_CHAR('\\');
                    break;
                case '\'':
                    *out++ = U_CHAR('\'');
                    break;
                case '"':
                    *out++ = U_CHAR('"');
                    break;
                case '?':
                    *out++ = U_CHAR('?');
                    break;
                case 't':
                    *out++ = U_CHAR('\t');
                    break;
                case 'n':
                    *out++ = U_CHAR('\n');
                    break;
                case '\0':
                    return;
                default:
                    *out++ = current;
                    break;
            }
        } else {
            if (current == '\\') {
                // Set to handle escape sequence.
                escape = true;
            } else {
                // Otherwise its a naive copy, assumes input is pure ascii.
                *out++ = current;
            }
        }

        current = *in_unsigned++;
    }

    // Null terminate.
    *out = U_CHAR('\0');
}

// Remove whitespace from the start and end of a ascii/utf8 string.
void GameTextManager::Remove_Leading_And_Trailing(char *buffer)
{
    int first = 0;

    // Find first none whitespace char.
    while (buffer[first] != '\0' && isspace(uint8_t(buffer[first]))) {
        ++first;
    }

    int pos = 0;

    // Move data down to start of buffer.
    while (buffer[first] != '\0') {
        buffer[pos++] = buffer[first++];
    }

    // Move pos back to last none whitespace.
    while (--pos >= 0 && isspace(buffer[pos])) {
        // Empty loop.
    }

    // Null terminate after last none whitespace
    buffer[pos + 1] = '\0';
}

// Strip spaces from a ucs2/utf16 string.
void GameTextManager::Strip_Spaces(unichar_t *buffer)
{
    unichar_t *getp = buffer;
    unichar_t *putp = buffer;

    unichar_t current = *getp++;
    unichar_t last = U_CHAR('\0');

    bool prev_whitepsace = true;

    while (current != U_CHAR('\0')) {
        if (current == U_CHAR(' ')) {
            if (last == U_CHAR(' ') || prev_whitepsace) {
                current = *getp++;

                continue;
            }
        } else if (current == U_CHAR('\n') || current == U_CHAR('\t')) {
            if (last == U_CHAR(' ')) {
                --putp;
            }

            *putp++ = current;
            prev_whitepsace = true;
            last = current;
            current = *getp++;

            continue;
        }

        *putp++ = current;
        prev_whitepsace = false;
        last = current;
        current = *getp++;
    }

    if (last == U_CHAR(' ')) {
        --putp;
    }

    // Ensure we null terminate after the last shuffled character.
    *putp = U_CHAR('\0');
}

// Reverses a word, appears to be unused in release, involved in jabberwocky setting
// in old dev builds its seems
void GameTextManager::Reverse_Word(char *start, char *end)
{
    bool first_char = true;

    while (start < end) {
        char s = *start;
        char e = *end;

        // Swap the capitalization for first char if its within standard alphabet.
        if (first_char) {
            if (s >= 'A' && s <= 'Z' && e >= 'a' && e <= 'z') {
                s += 32;
                e -= 32;
            }

            first_char = false;
        }

        *start++ = e;
        *end-- = s;
    }
}

// Get the count of strings in a str file.
bool GameTextManager::Get_String_Count(const char *filename, int &count)
{
    File *file = g_theFileSystem->Open_File(filename, File::TEXT | File::READ);
    count = 0;

    if (file == nullptr) {
        return false;
    }

    while (Read_Line(m_bufferIn, sizeof(m_bufferIn) - 1, file)) {
        Remove_Leading_And_Trailing(m_bufferIn);

        if (m_bufferIn[0] == '"') {
            size_t len = strlen(m_bufferIn);
            m_bufferIn[len] = '\n';
            m_bufferIn[len + 1] = '\0';
            Read_To_End_Of_Quote(file, &m_bufferIn[1], m_bufferOut, m_bufferEx, sizeof(m_bufferOut));
        } else if (strcasecmp(m_bufferIn, "END") == 0) {
            ++count;
        }
    }

    count += 500;

    file->Close();

    return true;
}

// Read info from a CSF file header.
bool GameTextManager::Get_CSF_Info(const char *filename)
{
    static_assert(sizeof(CSFHeader) == 24, "CSFHeader struct not expected size.");
    CSFHeader header;
    File *file = g_theFileSystem->Open_File(filename, File::BINARY | File::READ);

    if (file == nullptr || file->Read(&header, sizeof(header)) != sizeof(header)
        || header.id != FourCC<' ', 'F', 'S', 'C'>::value) {
        return false;
    }

    m_textCount = le32toh(header.num_labels);

    if (le32toh(header.version) <= 1) {
        m_language = LanguageID::US;
    } else {
        m_language = letoh<LanguageID>(header.langid);
    }

    file->Close();

    return true;
}

// Parses older format string files which only support ascii.
bool GameTextManager::Parse_String_File(const char *filename)
{
    captainslog_info("Parsing string file '%s'.", filename);
    File *file = g_theFileSystem->Open_File(filename, File::TEXT | File::READ);

    if (file == nullptr) {
        return false;
    }

    int index = 0;
    bool end = false;

    while (Read_Line(m_bufferIn, sizeof(m_bufferIn), file)) {
        Remove_Leading_And_Trailing(m_bufferIn);
        captainslog_trace("We have '%s' buffered.", m_bufferIn);
        // If we got a "//" (which is 2F2F in hex) comment line or no string at all,
        // read next line
        if (*reinterpret_cast<uint16_t *>(m_bufferIn) == 0x2F2F || m_bufferIn[0] == '\0') {
            captainslog_trace("Line started with // or empty line.");
            continue;
        }

        end = false;

#if ASSERT_LEVEL >= ASSERTS_DEBUG
        if (index > 0) {
            for (int i = 0; i < index; ++i) {
                captainslog_dbgassert(strcasecmp(m_stringInfo[i].label.Str(), m_bufferIn) != 0,
                    "String label '%s' is defined multiple times!",
                    m_bufferIn);
            }
        }
#endif

        m_stringInfo[index].label = m_bufferIn;
        m_maxLabelLen = std::max<int>(strlen(m_bufferIn), m_maxLabelLen);

        bool read_string = false;

        while (Read_Line(m_bufferIn, sizeof(m_bufferIn) - 1, file)) {
            Remove_Leading_And_Trailing(m_bufferIn);
            captainslog_trace("We have '%s' buffered.", m_bufferIn);

            if (m_bufferIn[0] == '"') {
                size_t len = strlen(m_bufferIn);
                m_bufferIn[len] = '\n';
                m_bufferIn[len + 1] = '\0';
                Read_To_End_Of_Quote(file, m_bufferIn + 1, m_bufferOut, m_bufferEx, sizeof(m_bufferOut));

                if (read_string) {
                    captainslog_trace("String label '%s' has more than one string defined!", m_bufferIn);

                    continue;
                }

                Translate_Copy(m_translateBuffer, m_bufferOut);
                Strip_Spaces(m_translateBuffer);

                // TODO maybe Windows build does something extra here not done in mac version.

                m_stringInfo[index].text = m_translateBuffer;
                m_stringInfo[index].speech = m_bufferEx;

                read_string = true;
            } else if (strcasecmp(m_bufferIn, "END") == 0) {
                ++index;
                end = true;

                break;
            }
        }
    }

    file->Close();

    if (!end) {
        captainslog_error("Unexpected end of string file '%s'.", filename);

        return false;
    }

    return true;
}

// Parses CSF files which support UCS2 strings, essentially the BMP of unicode.
bool GameTextManager::Parse_CSF_File(const char *filename)
{
    captainslog_info("Parsing CSF file '%s'.", filename);
    CSFHeader header;
    File *file = g_theFileSystem->Open_File(filename, File::BINARY | File::READ);

    if (file == nullptr || file->Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }

    uint32_t id;
    int index = 0;

    if (file->Read(&id, sizeof(id)) != sizeof(id)) {
        file->Close();

        return false;
    }

    // Little endian "LBL " FourCC
    while (id == FourCC<' ', 'L', 'B', 'L'>::value) {
        int32_t num_strings;
        int32_t length;

        file->Read(&num_strings, sizeof(num_strings));
        file->Read(&length, sizeof(length));

        // convert to host endianess
        length = le32toh(length);
        num_strings = le32toh(num_strings);

        if (length > 0) {
            file->Read(m_bufferIn, length);
        }

        m_bufferIn[length] = '\0';
        m_stringInfo[index].label = m_bufferIn;
        m_maxLabelLen = std::max(length, m_maxLabelLen);

        // Read all strings associated with this label, Nox used multiple strings for
        // random variation, Generals only cares about first one.
        for (int i = 0; i < num_strings; ++i) {
            file->Read(&id, sizeof(id));

            if (id != FourCC<' ', 'R', 'T', 'S'>::value && id != FourCC<'W', 'R', 'T', 'S'>::value) {
                file->Close();

                return false;
            }

            file->Read(&length, sizeof(length));
            length = le32toh(length);

            if (length > 0) {
                file->Read(m_translateBuffer, sizeof(m_translateBuffer[0]) * length);
            }

            // CSF format supports multiple strings per label, but we only care about
            // first string.
            if (i == 0) {
                m_translateBuffer[length] = '\0';

                for (int j = 0; m_translateBuffer[j] != '\0'; ++j) {
                    // Correct for big endian systems
                    m_translateBuffer[j] = le16toh(m_translateBuffer[j]);

                    // Binary NOT to decode
                    m_translateBuffer[j] = ~m_translateBuffer[j];
                }

                Strip_Spaces(m_translateBuffer);
                m_stringInfo[index].text = m_translateBuffer;
            }

            // FourCC of 'STRW' rather than 'STR ' indicates extra data.
            if (id == FourCC<'W', 'R', 'T', 'S'>::value) {
                file->Read(&length, sizeof(length));
                length = le32toh(length);

                if (length > 0) {
                    file->Read(m_bufferIn, length);
                }

                m_bufferIn[length] = '\0';

                if (i == 0) {
                    m_stringInfo[index].speech = m_bufferIn;
                }
            }
        }

        ++index;

        if (file->Read(&id, sizeof(id)) != sizeof(id)) {
            break;
        }
    }

    file->Close();

    return true;
}

// Parse an additional string file for a map. Currently cannot be localized.
bool GameTextManager::Parse_Map_String_File(const char *filename)
{
    captainslog_info("Parsing map string file '%s'.", filename);
    File *file = g_theFileSystem->Open_File(filename, File::TEXT | File::READ);

    if (file == nullptr) {
        return false;
    }

    int index = 0;
    bool end = false;

    while (Read_Line(m_bufferIn, sizeof(m_bufferIn), file)) {
        Remove_Leading_And_Trailing(m_bufferIn);

        // If we got a "//" (which is 2F2F in hex) comment line or no string at all,
        // read next line
        if (*reinterpret_cast<uint16_t *>(m_bufferIn) == 0x2F2F || m_bufferIn[0] == '\0') {
            continue;
        }

        captainslog_trace("Read line '%s'.", m_bufferIn);

        end = false;

#if ASSERT_LEVEL >= ASSERTS_DEBUG
        if (index > 0) {
            for (int i = 0; i < index; ++i) {
                captainslog_dbgassert(strcasecmp(m_mapStringInfo[i].label.Str(), m_bufferIn) != 0,
                    "String label '%s' is defined multiple times!",
                    m_bufferIn);
            }
        }
#endif

        m_mapStringInfo[index].label = m_bufferIn;
        m_maxLabelLen = std::max<int>(strlen(m_bufferIn), m_maxLabelLen);

        bool read_string = false;

        while (Read_Line(m_bufferIn, sizeof(m_bufferIn) - 1, file)) {
            Remove_Leading_And_Trailing(m_bufferIn);

            captainslog_trace("Read line '%s'.", m_bufferIn);

            if (m_bufferIn[0] == '"') {
                size_t len = strlen(m_bufferIn);
                m_bufferIn[len] = '\n';
                m_bufferIn[len + 1] = '\0';
                Read_To_End_Of_Quote(file, m_bufferIn + 1, m_bufferOut, m_bufferEx, sizeof(m_bufferOut));

                if (read_string) {
                    captainslog_warn("String label '%s' has more than one string defined!", m_bufferIn);

                    continue;
                }

                Translate_Copy(m_translateBuffer, m_bufferOut);
                Strip_Spaces(m_translateBuffer);

                // TODO maybe Windows build does something extra here not done in mac version.

                m_mapStringInfo[index].text = m_translateBuffer;
                m_mapStringInfo[index].speech = m_bufferEx;

                read_string = true;
            } else if (strcasecmp(m_bufferIn, "END") == 0) {
                ++index;
                end = true;

                break;
            }
        }
    }

    file->Close();

    if (!end) {
        captainslog_error("Unexpected end of string file '%s'.", filename);

        return false;
    }

    return true;
}

GameTextManager::GameTextManager() :
    m_textCount(0),
    m_maxLabelLen(0),
    m_stringInfo(nullptr),
    m_stringLUT(nullptr),
    m_initialized(false),
    m_noStringList(nullptr),
    m_useStringFile(true),
    m_language(LanguageID::US),
    m_failed(U_CHAR("***FATAL*** String Manager failed to initialize properly")),
    m_mapStringInfo(nullptr),
    m_mapStringLUT(nullptr),
    m_mapTextCount(0),
    m_stringVector()
{
    memset(m_bufferIn, 0, sizeof(m_bufferIn));
    memset(m_bufferOut, 0, sizeof(m_bufferOut));
    memset(m_bufferEx, 0, sizeof(m_bufferEx));
}

GameTextManager::~GameTextManager()
{
    Reset(); // #BUGFIX Full cleanup on destruction.
    Deinit();
}

void GameTextManager::Init()
{
    captainslog_info("Initializing GameTextManager.");
    if (m_initialized) {
        return;
    }

    Utf8String csfpath;
    bool use_csf = true;

    csfpath.Format("data/%s/Generals.csf", Get_Registry_Language().Str());

    // Check if we can use a standard string file, if not, try the csf file.
    if (m_useStringFile && Get_String_Count("data/Generals.str", m_textCount)) {
        use_csf = false;
    } else if (!Get_CSF_Info(csfpath.Str())) {
        captainslog_error("Couldn't get STR string count or CSF info, returning.");
        return;
    }

    if (m_textCount == 0) {
        captainslog_error("Text count is %d, returning.", m_textCount);
        return;
    }

    m_stringInfo = new StringInfo[m_textCount];

    if (m_stringInfo == nullptr) {
        captainslog_error("Couldn't allocated string info array.");
        Deinit();

        return;
    }

    // Try and parse the relevant string file, cleanup if not.
    if (!use_csf) {
        if (!Parse_String_File("data/Generals.str")) {
            captainslog_error("Couldn't parse string file.");

            Deinit();

            return;
        }
    } else {
        if (!Parse_CSF_File(csfpath.Str())) {
            captainslog_error("Couldn't parse compiled string file.");
            Deinit();

            return;
        }
    }

    // Generate the lookup table and sort it for efficient search.
    m_stringLUT = new StringLookUp[m_textCount];

    for (int i = 0; i < m_textCount; ++i) {
        m_stringLUT[i].info = &m_stringInfo[i];
        m_stringLUT[i].label = &m_stringInfo[i].label;
    }

    qsort(m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT);

    // Fetch the GUI window title string and set it here.
    Utf8String ntitle;
    Utf16String wtitle;
    wtitle = U_CHAR("Thyme - "); // #FEATURE Add Thyme text to window name.
    wtitle += Fetch("GUI:Command&ConquerGenerals");

    ntitle.Translate(wtitle);

#ifdef BUILD_WITH_SDL2
    if (g_applicationWindow != nullptr) {
        SDL_SetWindowTitle(g_applicationWindow, ntitle.Str());
    }
#elif defined PLATFORM_WINDOWS
    if (g_applicationHWnd != 0) {
        SetWindowTextA(g_applicationHWnd, ntitle.Str());
        SetWindowTextW(g_applicationHWnd, wtitle.Str());
    }
#endif
}

// Resets the map strings, main string file is left loaded.
void GameTextManager::Reset()
{
    m_mapTextCount = 0; // #BUGFIX Reset map text count as well.

    if (m_mapStringInfo != nullptr) {
        delete[] m_mapStringInfo;
        m_mapStringInfo = nullptr;
    }

    if (m_mapStringLUT != nullptr) {
        delete[] m_mapStringLUT;
        m_mapStringLUT = nullptr;
    }
}

// Find and return the unicode string corresponding to the label provided.
// Optionally can pass a bool pointer to determine if a string was found.
Utf16String GameTextManager::Fetch(Utf8String args, bool *success)
{
    return Fetch(args.Str(), success);
}

// Find and return the unicode string corresponding to the label provided.
// Optionally can pass a bool pointer to determine if a string was found.
Utf16String GameTextManager::Fetch(const char *args, bool *success)
{
    if (m_stringInfo == nullptr) {
        if (success != nullptr) {
            *success = false;
        }

        return m_failed;
    }

    Utf8String argstr = args;
    StringLookUp key = { &argstr, nullptr };

    StringLookUp *found =
        static_cast<StringLookUp *>(bsearch(&key, m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT));

    if (found != nullptr) {
        if (success != nullptr) {
            *success = true;
        }

        return found->info->text;
    }

    if (m_mapStringLUT != nullptr && m_mapTextCount > 0) {
        found =
            static_cast<StringLookUp *>(bsearch(&key, m_mapStringLUT, m_mapTextCount, sizeof(StringLookUp), Compare_LUT));

        if (found != nullptr) {
            if (success != nullptr) {
                *success = true;
            }

            return found->info->text;
        }
    }

    if (success != nullptr) {
        *success = false;
    }

    // If we reached here, we didn't find a string from our string file.
    Utf16String missing;
    NoString *no_string;

    missing.Format(U_CHAR("MISSING: '%hs'"), args);

    // Find missing string in NoString list if it already exists.
    for (no_string = m_noStringList; no_string != nullptr; no_string = no_string->next) {
        if (missing == no_string->text) {
            break;
        }
    }

    // If it was not found or the list was empty, add a new one.
    if (no_string == nullptr) {
        no_string = new NoString;
        no_string->text = missing;
        no_string->next = m_noStringList;
        m_noStringList = no_string;
    }

    return no_string->text;
}

// List all string labels that start with the prefix passed to the function.
std::vector<Utf8String> *GameTextManager::Get_Strings_With_Prefix(Utf8String label)
{
    m_stringVector.clear();

    captainslog_trace("Searching for strings prefixed with '%s'.", label.Str());

    // Search all string labels that start with the substring provided.
    if (m_stringLUT != nullptr) {
        for (int i = 0; i < m_textCount; ++i) {
            const char *lut_label = m_stringLUT[i].label->Str();

            if (strstr(lut_label, label.Str()) == lut_label) {
                m_stringVector.push_back(*m_stringLUT[i].label);
            }
        }
    }

    // Same again for map strings.
    if (m_mapStringLUT != nullptr) {
        for (int i = 0; i < m_mapTextCount; ++i) {
            const char *lut_label = m_mapStringLUT[i].label->Str();
            if (strstr(lut_label, label.Str()) == lut_label) {
                m_stringVector.push_back(*m_mapStringLUT[i].label);
            }
        }
    }

    return &m_stringVector;
}

// Initializes a string file associated with a specific map. Resources
// allocated here are freed by GameTextManager::Reset()
void GameTextManager::Init_Map_String_File(Utf8String const &filename)
{
    Get_String_Count(filename.Str(), m_mapTextCount);
    m_mapStringInfo = new StringInfo[m_mapTextCount];
    Parse_Map_String_File(filename.Str());

    // Generate the lookup table and sort it for efficient search.
    m_mapStringLUT = new StringLookUp[m_mapTextCount];

    for (int i = 0; i < m_mapTextCount; ++i) {
        m_mapStringLUT[i].info = &m_mapStringInfo[i];
        m_mapStringLUT[i].label = &m_mapStringInfo[i].label;
    }

    qsort(m_mapStringLUT, m_mapTextCount, sizeof(StringLookUp), Compare_LUT);
}

// Destroys the main string file, doesn't affect loaded map strings.
void GameTextManager::Deinit()
{
    if (m_stringInfo != nullptr) {
        delete[] m_stringInfo;
        m_stringInfo = nullptr;
    }

    if (m_stringLUT != nullptr) {
        delete[] m_stringLUT;
        m_stringLUT = nullptr;
    }

    m_textCount = 0;

    for (NoString *ns = m_noStringList; ns != nullptr;) {
        NoString *tmp = ns;
        ns = tmp->next;

        delete tmp;
    }

    m_noStringList = nullptr;
    m_initialized = false;
}
