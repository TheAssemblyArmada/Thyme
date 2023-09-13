/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief INI file parsing class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "iniclass.h"
#include "filestraw.h"
#include "nstrdup.h"
#include "rawfile.h"
#include "readline.h"
#include <captainslog.h>
#include <cctype>
#include <cstdio>

using std::free;
using std::strchr;
using std::strlen;

INIEntry::~INIEntry()
{
    if (m_key != nullptr) {
        free((void *)m_key);
    }

    m_key = nullptr;

    if (m_value != nullptr) {
        free((void *)m_value);
    }

    m_value = nullptr;
}

void INIEntry::Set_Name(const char *new_name)
{
    if (m_key != nullptr) {
        free((void *)m_key);
    }

    m_key = strdup(new_name);
}

void INIEntry::Set_Value(const char *str)
{
    if (m_value != nullptr) {
        free((void *)m_value);
    }

    m_value = strdup(str);
}

INISection::~INISection()
{
    if (m_sectionName != nullptr) {
        free((void *)m_sectionName);
    }

    m_sectionName = nullptr;
    m_entryList.Delete();
}

INIEntry *INISection::Find_Entry(const char *entry) const
{
    int crc;

    if (entry != nullptr && (crc = CRC(entry), m_entryIndex.Is_Present(crc))) {
        // captainslog_trace("Fetching entry %s with CRC %08x", entry, crc);
        return m_entryIndex.Fetch_Index(crc);
    }

    return nullptr;
}

void INISection::Set_Name(const char *str)
{
    if (m_sectionName != nullptr) {
        free((void *)m_sectionName);
    }

    m_sectionName = strdup(str);
}

INIClass::INIClass(FileClass &file) : m_fileName(nullptr)
{
    Initialize();
    Load(file);
}

INIClass::~INIClass()
{
    Clear();
    Shutdown();
}

void INIClass::Shutdown()
{
    delete m_sectionList;
    delete m_sectionIndex;
    delete[] m_fileName;
}

void INIClass::Initialize()
{
    m_sectionList = new List<INISection *>;
    m_sectionIndex = new IndexClass<int32_t, INISection *>;
    m_fileName = nstrdup("<unknown>");
}

bool INIClass::Clear(const char *section, const char *entry)
{
    INISection *sectionptr;
    INIEntry *entryptr;

    if (section) {
        if ((sectionptr = Find_Section(section)) != nullptr) {
            if (entry) {
                if ((entryptr = sectionptr->Find_Entry(entry)) == nullptr) {
                    return true;
                }

                sectionptr->m_entryIndex.Remove_Index(CRC(entryptr->Get_Name()));
                delete entryptr;
            } else {
                if (!sectionptr) {
                    return true;
                }

                m_sectionIndex->Remove_Index(CRC(sectionptr->Get_Name()));
                delete sectionptr;
            }

            return true;
        }

        return false;
    }

    m_sectionList->Delete();
    m_sectionIndex->Clear();

    return true;
}

int INIClass::Load(FileClass &file)
{
    delete[] m_fileName;
    m_fileName = nstrdup(file.File_Name());
    FileStraw fstraw(file);

    return Load(fstraw);
}

int INIClass::Load(Straw &straw)
{
    char buffer[MAX_LINE_LENGTH];
    // char section[64];
    bool merge = false;
    bool end_of_file = false;

    if (m_sectionIndex->Count() > 0) {
        merge = true;
    }

    // Read all lines until we find the first section.
    while (!end_of_file) {
        Read_Line(straw, buffer, MAX_LINE_LENGTH, end_of_file);
        if (end_of_file) {
            captainslog_trace("INIClass::Load() - reached end of file before finding a section");
            return INIC_LOAD_INVALID;
        }

        if (buffer[0] == '[' && strchr(buffer, ']') != nullptr) {
            break;
        }
    }

    while (!end_of_file) {
        captainslog_dbgassert(
            buffer[0] == '[' && strchr(buffer, ']'), ".ini file is badly formatted."); // at start of section
        // Remove square brackets to get section name and create new section.
        buffer[0] = ' ';
        *strchr(buffer, ']') = '\0';
        strtrim(buffer);
        INISection *section = new INISection(buffer);

        if (section == nullptr) {
            captainslog_trace("INIClass::Load() - failed to create section!");

            Clear();

            return INIC_LOAD_INVALID;
        }

        while (!end_of_file) {
            int count = Read_Line(straw, buffer, sizeof(buffer), end_of_file);
            // Check we don't have another section.
            if (buffer[0] == '[' && strchr(buffer, ']')) {
                break;
            }

            // Strip comments from the line.
            Strip_Comments(buffer);
            char *delimiter = strchr(buffer, '=');

            if (count > 0 && buffer[0] != ';' && buffer[0] != '=') {
                if (delimiter != nullptr) {
                    delimiter[0] = '\0';
                    char *entry = buffer;
                    char *value = delimiter + 1;

                    strtrim(entry);

                    if (entry[0] != '\0') {
                        strtrim(value);

                        if (value[0] == '\0') {
                            continue;
                        }

                        INIEntry *entryptr = new INIEntry(entry, value);
                        if (!entryptr) {
                            captainslog_trace("Failed to create entry '%s = %s'.", entry, value);

                            delete section;
                            Clear();

                            return INIC_LOAD_INVALID;
                        }

                        // Is this Name, Value or something?
                        CRC(entryptr->Get_Name());
                        int32_t crc = CRC(entryptr->Get_Name());

                        if (section->m_entryIndex.Is_Present(crc)) {
                            // Duplicate_CRC_Error(__FUNCTION__, section->Get_Name(), entryptr->Get_Name());
                        }

                        section->m_entryIndex.Add_Index(crc, entryptr);
                        section->m_entryList.Add_Tail(entryptr);
                    }
                }
            }
        }

        if (section->m_entryList.Is_Empty()) {
            delete section;
        } else {
            int32_t crc = CRC(section->Get_Name());
            m_sectionIndex->Add_Index(crc, section);
            m_sectionList->Add_Tail(section);
        }
    }

    return INIC_LOAD_OVERWRITE;
}

INISection *INIClass::Find_Section(const char *section) const
{
    captainslog_assert(section != nullptr);

    int crc;

    if (section != nullptr && (crc = CRC(section)) != 0 && m_sectionIndex->Is_Present(crc)) {
        return m_sectionIndex->Fetch_Index(crc);
    }

    return nullptr;
}

INIEntry *INIClass::Find_Entry(const char *section, const char *entry) const
{
    captainslog_assert(section != nullptr);
    captainslog_assert(entry != nullptr);

    INISection *sectionptr = Find_Section(section);

    if (sectionptr != nullptr) {
        return sectionptr->Find_Entry(entry);
    }

    return nullptr;
}

int INIClass::Entry_Count(const char *section) const
{
    captainslog_assert(section != nullptr);

    INISection *sectionptr = Find_Section(section);

    if (sectionptr != nullptr) {
        return sectionptr->Get_Entry_Count();
    }

    return 0;
}

const char *INIClass::Get_Entry(const char *section, int index) const
{
    captainslog_assert(section != nullptr);

    INISection *sectionptr = Find_Section(section);

    if (sectionptr != nullptr) {
        int count = index;

        if (index < sectionptr->Get_Entry_Count()) {
            for (INIEntry *entryptr = sectionptr->m_entryList.First(); entryptr != nullptr; entryptr = entryptr->Next()) {
                if (!entryptr->Is_Valid()) {
                    break;
                }

                if (!count) {
                    return entryptr->Get_Name(); // TODO
                }

                count--;
            }
        }
    }

    return nullptr;
}

int INIClass::Enumerate_Entries(const char *section, const char *entry_prefix, uint32_t start_number, uint32_t end_number)
{
    char buffer[256];
    uint32_t i = start_number;

    captainslog_assert(section != nullptr);
    captainslog_assert(!start_number && !end_number);

    for (; i < end_number; ++i) {
        snprintf(buffer, sizeof(buffer), "%s%d", entry_prefix, i);

        if (!Find_Entry(section, buffer)) {
            break;
        }
    }

    return i - start_number;
}

bool INIClass::Put_Int(const char *section, const char *entry, int value, int format)
{
    char buffer[512];

    if (format == INIINTEGER_AS_HEX) {
        sprintf(buffer, "%Xh", value);
    } else if (format == INIINTEGER_AS_MOTOROLA_HEX) {
        sprintf(buffer, "$%X", value);
    } else {
        sprintf(buffer, "%d", value);
    }

    return Put_String(section, entry, buffer);
}

int INIClass::Get_Int(const char *section, const char *entry, int defvalue) const
{
    INIEntry *entryptr;
    const char *value;

    if (section != nullptr && entry != nullptr && (entryptr = Find_Entry(section, entry)) != nullptr && entryptr->Get_Name()
        && (value = entryptr->Get_Value()) != nullptr) {
        if (value[0] == '$') {
            sscanf(value, "$%x", &defvalue);
            return defvalue;
        }

        if (tolower(value[strlen(value) - 1]) == 'h') {
            sscanf(value, "%xh", &defvalue);
            return defvalue;
        }

        // Convert the value to a base 10 integer.
        return strtol(value, nullptr, 10);
    }

    return defvalue;
}

bool INIClass::Put_Hex(const char *section, const char *entry, int value)
{
    char buffer[32];

    sprintf(buffer, "%X", (unsigned)value);

    return Put_String(section, entry, buffer);
}

int INIClass::Get_Hex(const char *section, const char *entry, int defvalue) const
{
    INIEntry *entryptr;

    if (section && entry && (entryptr = Find_Entry(section, entry)) != nullptr && *(entryptr->Get_Value())) {
        return sscanf(entryptr->Get_Name(), "%x", reinterpret_cast<unsigned *>(&defvalue));
    }

    return defvalue;
}

bool INIClass::Put_Float(const char *section, const char *entry, double value)
{
    char buffer[32];

    sprintf(buffer, "%f", value);

    return Put_String(section, entry, buffer);
}

float INIClass::Get_Float(const char *section, const char *entry, float defvalue) const
{
    INIEntry *entryptr;

    if (section != nullptr && entry && (entryptr = Find_Entry(section, entry)) != nullptr && *(entryptr->Get_Value())) {
        sscanf(entryptr->Get_Value(), "%f", &defvalue);

        // Is this actually a percentage? if so, divide it by 100
        if (strchr(entryptr->Get_Value(), '%')) {
            return defvalue / 100.0f;
        }
    }

    return defvalue;
}

bool INIClass::Put_Double(const char *section, const char *entry, double value)
{
    char buffer[32];

    sprintf(buffer, "%lf", value);

    return Put_String(section, entry, buffer);
}

double INIClass::Get_Double(const char *section, const char *entry, double defvalue) const
{
    INIEntry *entryptr;

    if (section && entry && (entryptr = Find_Entry(section, entry)) != nullptr && *(entryptr->Get_Value())) {
        sscanf(entryptr->Get_Value(), "%lf", &defvalue);

        //
        // Is this value really a percentage? if so, divide it by 100.
        //
        if (strchr(entryptr->Get_Value(), '%')) {
            return defvalue / 100.0;
        }
    }

    return defvalue;
}

bool INIClass::Put_String(const char *section, const char *entry, const char *string)
{
    INISection *sectionptr;
    INIEntry *entryptr;

    if (section != nullptr && entry != nullptr) {
        // If this string is for a section that doesn't exist yet, create it.
        if ((sectionptr = Find_Section(section)) == nullptr) {
            captainslog_trace("INIClass::Put_String() Creating new section [%s]", section);
            sectionptr = new INISection(section);

            if (sectionptr == nullptr) {
                return false;
            }

            m_sectionList->Add_Tail(sectionptr);
            m_sectionIndex->Add_Index(CRC(sectionptr->Get_Name()), sectionptr);
        }

        // Check if we already have a matching entry, either just CRC collision or same name.
        if ((entryptr = sectionptr->Find_Entry(entry)) != nullptr) {
            if (strcmp(entryptr->Get_Name(), entry) == 0) {
                captainslog_trace("%s  - Duplicate Entry \"%s\"", entry);
            } else {
                Duplicate_CRC_Error(__CURRENT_FUNCTION__, section, entry);
            }

            // If we already have the entry, we are replacing it so delete
            sectionptr->m_entryIndex.Remove_Index(CRC(entryptr->Get_Name()));

            if (entryptr != nullptr) {
                delete entryptr;
            }
        }

        // Insert if we have a valid string that is not empty.
        if (string != nullptr && strlen(string) > 0) {
            captainslog_assert(strlen(string) < MAX_LINE_LENGTH - 1);
            entryptr = new INIEntry(entry, string);

            if (entryptr != nullptr) {
                sectionptr->m_entryList.Add_Tail(entryptr);
                sectionptr->m_entryIndex.Add_Index(CRC(entryptr->Get_Name()), entryptr);
            }
        }

        return true;
    }

    return false;
}

int INIClass::Get_String(const char *section, const char *entry, const char *defvalue, char *buffer, int length) const
{
    INIEntry *entryptr;
    const char *value = defvalue;

    captainslog_assert(section != nullptr);
    captainslog_assert(entry != nullptr);
    captainslog_assert(buffer != nullptr);
    captainslog_assert(length > 0);

    if (buffer != nullptr && length > 0 && section != nullptr && entry != nullptr) {
        if ((entryptr = Find_Entry(section, entry)) == nullptr || (value = entryptr->Get_Value()) == nullptr) {
            if (defvalue == nullptr) {
                buffer[0] = '\0'; // nullify the first byte of char
                return 0;
            }

            value = defvalue;
        }

        // copy string to return result buffer
        strlcpy(buffer, value, length);
        strtrim(buffer);

        return strlen(buffer);
    }

    return 0;
}

bool INIClass::Put_Bool(const char *section, const char *entry, bool value)
{

    if (value) {
        return Put_String(section, entry, "true");
    }

    return Put_String(section, entry, "false");
}

bool INIClass::Get_Bool(const char *section, const char *entry, bool defvalue) const
{
    INIEntry *entryptr;
    const char *value;

    if (section != nullptr && entry != nullptr) {
        if ((entryptr = Find_Entry(section, entry)) != nullptr && entryptr->Get_Name()
            && (value = entryptr->Get_Value()) != nullptr) {
            switch (toupper(value[0])) {
                // 1, true, yes...
                case '1':
                case 'T':
                case 'Y':
                    return true;

                // 0, false, no...
                case '0':
                case 'F':
                case 'N':
                    return false;

                default:
                    captainslog_trace("Invalid boolean entry in INIClass::Get_Bool()!");
                    return false;
            }
        }
    }

    return defvalue;
}

void INIClass::Strip_Comments(char *line)
{
    if (line != nullptr) {
        // fine first instance of ';'
        char *comment = strchr(line, ';');

        // If we found a comment, replace the delimiter with a null char and trim.
        if (comment != nullptr) {
            *comment = '\0';
            strtrim(line);
        }
    }
}

void INIClass::Duplicate_CRC_Error(const char *message, const char *section, const char *entry)
{
    captainslog_warn("%s - Duplicate Entry \"%s\" in section \"%s\" (%s)", message, entry, section, m_fileName);
}
