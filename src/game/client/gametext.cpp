////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMETEXT.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: String file handler.
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
#include "gametext.h"
#include "filesystem.h"
#include "hookcrt.h"
#include "main.h"   // For ApplicationHWnd
#include "rtsutils.h"

using rts::FourCC;

int GameTextManager::Compare_LUT(void const *a, void const *b)
{
    char const *ac = static_cast<StringLookUp const*>(a)->label->Str();
    char const *bc = static_cast<StringLookUp const*>(b)->label->Str();

    return strcasecmp(ac, bc);
}

char GameTextManager::Read_Char(File *file)
{
    char tmp;

    if ( file->Read(&tmp, sizeof(tmp)) == sizeof(tmp) ) {
        return tmp;
    }

    return '\0';
}

void GameTextManager::Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len)
{
    DEBUG_LOG("Reading to end of quote");
    bool escape = false;
    int i;

    for ( i = 0; i < buff_len; ++i ) {
        char current;

        // if in pointer is valid, read data from that, otherwise read from file.
        if ( in != nullptr ) {
            current = *in++;

            if ( current = '\0' ) {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // -1 and we are done?
        if ( current == '\xFF' ) {
            return;
        }

        // Handle some special characters.
        if ( current == '\n' ) {
            escape = false;
            current = ' ';
        } else if ( current == '\\') {
            escape = !escape;
        } else if ( current = '"' && !escape ) {
            break;
        } else {
            escape = false;
        }

        // Treat any white space as a space char.
        if ( iswspace(current) ) {
            current = ' ';
        }

        // Copy to output buffer.
        out[i] = current;
    }

    out[i] = '\0';

    int wave_pos = 0;
    int state = 0;

    while ( true ) {
        char current;
        
        // if in pointer is valid, read data from that, otherwise read from file.
        if ( in != nullptr ) {
            current = *in++;

            if ( current = '\0' ) {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // Stop reading on line break or -1 char.
        if ( current == '\n' || current == '\xFF' ) {
            break;
        }

        // state 0 ignores initial whitespace and '=' 
        if ( state == 0 && !iswspace(current) && current != '=' ) {
            state = 1;
        }
        
        // state 1 read so long as its alphanumeric characters or underscore
        if ( state == 1 ) {
            if ( (current < 'a' || current > 'z')
                && (current < 'A' || current > 'Z')
                && (current < '0' || current > '9')
                && current != '_' 
            ) {
                state = 2;
            } else {
                wave[wave_pos++] = current;
            }
        }

        // state 2 ignore everything and keep reading until a breaking condition is encountered
    }

    if ( wave_pos > 0 ) {
        if ( wave[wave_pos - 1] >= '0' && wave[wave_pos - 1] <= '9' ) {
            wave[wave_pos++] = 'e';
        }
    }

    wave[wave_pos] = '\0';
}

bool GameTextManager::Read_Line(char *buffer, int length, File *file)
{
    bool ret = false;
    char *putp;

    for ( int i = 0; i < length; ++i ) {
        if ( file->Read(putp, sizeof(*putp)) != sizeof(*putp) ) {
            break;
        }

        if ( *putp == '\n' ) {
            break;
        }

        ++putp;
        ret = true;
    }

    *putp = '\0';

    return ret;
}

void GameTextManager::Translate_Copy(wchar_t *out, char *in)
{
    DEBUG_LOG("Translating '%s' to wide char.\n");
    while ( *in != '\0' ) {
        // Handle escape sequences and start next iteration.
        if ( *in == '\\' ) {
            ++in;

            switch ( *in ) {
                case 't':
                    *out++ = L'\t';
                    ++in;
                    continue;
                case 'n':
                    *out++ = L'\n';
                    ++in;
                    continue;
                case '\0':
                    return;
                default:
                    *out++ = *in++;
                    continue;
            }
        }

        // Otherwise its a naive copy, assumes input is pure ascii.
        *out++ = *in++;
    }

    // Null terminate.
    *out = L'\0';
}

void GameTextManager::Remove_Leading_And_Trailing(char *buffer)
{
    int first = 0;

    // Find first none whitespace char.
    while ( buffer[first] != '\0' && iswspace(buffer[first]) ) {
        ++first;
    }

    int pos = 0;

    // Move data down to start of buffer.
    while ( buffer[first] != '\0' ) {
        buffer[pos++] = buffer[first++];
    }

    // Move pos back to last none whitespace.
    while ( --pos >= 0 && iswspace(buffer[pos]) ) {
        // Empty loop.
    }

    // Null terminate after last none whitespace
    buffer[pos + 1] = '\0';
}

void GameTextManager::Strip_Spaces(wchar_t *buffer)
{
    wchar_t *getp = buffer;
    wchar_t *putp = buffer;

    while ( *getp != L'\0' ) {
        //Move get pointer past current run of spaces, if any.
        while ( *getp != L'\0' && *getp == L' ' ) {
            ++getp;
        }

        // Break if we have a null char.
        if ( *getp == L'\0' ) {
            break;
        }

        // Copy none space characters to where they should be.
        while ( *getp != L'\0' && *getp != L' ' ) {
            *putp++ = *getp++;
        }
    }

    // Ensure we null terminate after the last shuffled character.
    *putp = L'\0';
}

void GameTextManager::Reverse_Word(char *start, char *end)
{
    bool first_char = true;

    while ( start < end ) {
        char s = *start;
        char e = *end;

        // Swap the capitalisation for first char if its within standard alphabet.
        if ( first_char ) {
            if ( s >= 'A' && s <= 'Z' && e >= 'a' && e <= 'z' ) {
                s += 32;
                e -= 32;
            }

            first_char = false;
        }

        *start++ = e;
        *end-- = s;
    }
}

bool GameTextManager::Get_String_Count(char const *filename, int &count)
{
    File *file = TheFileSystem->Open(filename, File::TEXT | File::READ);
    count = 0;

    if ( file = nullptr ) {
        return false;
    }

    while ( Read_Line(m_bufferIn, sizeof(m_bufferIn) - 1, file) ) {
        Remove_Leading_And_Trailing(m_bufferIn);

        if ( m_bufferIn[0] == '"' ) {
            size_t len = strlen(m_bufferIn);
            m_bufferIn[len] = '\n';
            m_bufferIn[len + 1] = '\0';
            Read_To_End_Of_Quote(file, &m_bufferIn[1], m_bufferOut, m_bufferEx, sizeof(m_bufferOut));
        } else if ( strcasecmp(m_bufferIn, "END") == 0 ) {
            ++count;
        }
    }

    count += 500;
    file->Close();

    return true;
}

bool GameTextManager::Get_CSF_Info(char const *filename)
{
    static_assert(sizeof(CSFHeader) == 24, "CSFHeader struct not expected size.");
    CSFHeader header;
    File *file = TheFileSystem->Open(filename, File::BINARY | File::READ);

    if ( file == nullptr || file->Read(&header, sizeof(header)) != sizeof(header) || header.id != FourCC<' ', 'F', 'S', 'C'>::value ) {
        return false;
    }

    m_textCount = header.num_labels;

    if ( header.version <= 1 ) {
        m_language = LANGUAGE_ID_US;
    } else {
        m_language = header.langid;
    }

    file->Close();

    return true;
}

bool GameTextManager::Parse_String_File(char const *filename)
{
    return false;
}

bool GameTextManager::Parse_CSF_File(char const *filename)
{
    return false;
}

bool GameTextManager::Parse_Map_String_File(char const *filename)
{
    return false;
}

GameTextManager::GameTextManager() :
    m_textCount(0),
    m_maxLabelLen(0),
    m_stringInfo(nullptr),
    m_stringLUT(nullptr),
    m_initialized(false),
    m_noStringList(nullptr),
    m_useStringFile(true),
    m_language(LANGUAGE_ID_US),
    m_failed(L"***FATAL*** String Manager failed to initilaize properly"),
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
    Deinit();
}

void GameTextManager::Init()
{
    if ( m_initialized ) {
        return;
    }

    AsciiString csfpath;
    bool use_csf = true;

    csfpath.Format("data/%s/Generals.csf", Get_Registry_Language());

    // Check if we can use a standard string file, if not, try the csf file.
    if ( m_useStringFile && Get_String_Count("data/Generals.str", m_textCount) ) {
        use_csf = false;
    } else if ( !Get_CSF_Info(csfpath.Str()) ) {
        return;
    }

    if ( m_textCount == 0 ) {
        return;
    }
    
    m_stringInfo = new StringInfo[m_textCount];

    if ( m_stringInfo == nullptr ) {
        Deinit();

        return;
    }

    // Try and parse the relevant string file, cleanup if not
    if ( use_csf ) {
        if ( !Parse_String_File("data/Generals.str") ) {
            Deinit();

            return;
        }
    } else {
        if ( !Parse_CSF_File(csfpath.Str()) ) {
            Deinit();

            return;
        }
    }

    // Generate the lookup table and sort it for efficient search.
    m_stringLUT = new StringLookUp[m_textCount];

    for ( int i = 0; i < m_textCount; ++i ) {
        m_stringLUT[i].info = &m_stringInfo[i];
        m_stringLUT[i].label = &m_stringInfo[i].label;
    }

    qsort(m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT);

    // Fetch the GUI window title string and set it here.
    AsciiString ntitle;
    UnicodeString wtitle = Fetch("GUI:Command&ConquerGenerals");
    
    ntitle.Translate(wtitle);

#ifdef PLATFORM_WINDOWS
    if ( ApplicationHWnd != 0 ) {
        SetWindowTextA(ApplicationHWnd, ntitle.Str());
        SetWindowTextW(ApplicationHWnd, wtitle.Str());
    }
#else

#endif
}

void GameTextManager::Reset()
{
    if ( m_stringInfo != nullptr ) {
        delete[] m_stringInfo;
        m_stringInfo = nullptr;
    }

    if ( m_stringLUT != nullptr ) {
        delete[] m_stringLUT;
        m_stringLUT = nullptr;
    }
}

UnicodeString GameTextManager::Fetch(AsciiString args, bool *success)
{
    return UnicodeString();
}

UnicodeString GameTextManager::Fetch(const char *args, bool *success)
{
    return UnicodeString();
}

std::vector<AsciiString> GameTextManager::Get_Strings_With_Prefix(AsciiString label)
{
    return std::vector<AsciiString>();
}

void GameTextManager::Init_Map_String_File(AsciiString const &filename)
{
}

void GameTextManager::Deinit()
{
    if ( m_stringInfo != nullptr ) {
        delete[] m_stringInfo;
        m_stringInfo = nullptr;
    }

    if ( m_stringLUT != nullptr ) {
        delete[] m_stringLUT;
        m_stringLUT = nullptr;
    }

    m_textCount = 0;

    // Cleanup NoString list.
    for ( NoString *ns = m_noStringList; ns != nullptr; ) {
        NoString *tmp = ns;
        ns = tmp->next;

        delete tmp;
    }

    m_noStringList = nullptr;
    m_initialized = false;
}
