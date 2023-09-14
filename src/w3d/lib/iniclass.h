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
#pragma once

#include "always.h"
#include "crcengine.h"
#include "index.h"
#include "listnode.h"
#include "straw.h"
#include "wwfile.h"
#include <string.h>

#define SYS_NEW_LINE "\r\n"

// find out what this really is.
enum INIIntegerType
{
    INIINTEGER_AS_DECIMAL = 0,
    INIINTEGER_AS_HEX = 1,
    INIINTEGER_AS_MOTOROLA_HEX = 2
};

enum INILoadMode
{
    INIC_LOAD_INVALID = 0,
    INIC_LOAD_OVERWRITE = 1,
    INIC_LOAD_CREATE_OVERRIDES = 2
};

class INIEntry : public Node<INIEntry *>
{
public:
    INIEntry(const char *name, const char *value) : m_key(strdup(name)), m_value(strdup(value)) {}
    ~INIEntry();

    const char *Get_Name() { return m_key; }
    void Set_Name(const char *name);
    const char *Get_Value() { return m_value; }
    void Set_Value(const char *new_value);
    int32_t CRC(const char *value) const { return CRC::String(value, 0); }

public:
    char *m_key;
    char *m_value;
};

class INISection : public Node<INISection *>
{
public:
    INISection(const char *name) : m_sectionName(strdup(name)) {}
    ~INISection();

    INIEntry *Find_Entry(const char *entry) const;
    const char *Get_Name() const { return m_sectionName; }
    void Set_Name(const char *str);
    int Get_Entry_Count() const { return m_entryIndex.Count(); }
    int32_t CRC(const char *value) const { return CRC::String(value, 0); }

public:
    char *m_sectionName;
    List<INIEntry *> m_entryList;
    IndexClass<int32_t, INIEntry *> m_entryIndex;
};

/**
 * @brief Implements INI file handling, everything is handled in memory once loaded.
 *
 *    Here are example file contents:
 *
 *      [TankObject]
 *      Name = Super Awesome Tank
 *      MaxCount = 1
 *      Enabled = true
 *      ; Comment string
 *
 *      [General]
 *      MaxTankCount = 999
 *
 *    The "[TankObject]" part is a section called "TankObject".
 *    The "Name" part is a key called "Name".
 *    The "Super Awesome Tank" part is a value called "Super Awesome Tank".
 *    The "; Comment string" part is a comment line that has no effect.
 *
 *    Some format specification details regarding this implementation:
 *        - Leading and trailing space is removed from section, key, and value strings returned to the user.
 *        - If you want to preserve leading and trailing space, surround the text with quote characters.
 *        - Spaces are valid in section, key, and value strings.
 *        - The only reserved characters are the [] characters in the section string and the = char in the key string.
 *        - The key = section sequence need not have spaces between the key and = and the = and section.
 *        - Section and key strings are case sensitive when the user is doing reads.
 *        - Newlines can be in either Unix format ("\n") or Windows format ("\r\n").
 *        - When writing an ini, the new lines are always windows format for consistency with the original.
 *        - Supported text encodings for .ini files include ASCII, UTF8 Unicode.
 *
 *    Example usage:
 *        INIClass INIClass("somefile.ini");
 *
 *        int rating;
 *        if(iniFile.ReadEntryFormatted("User", "Rating", "%d", &rating) > 0)
 *            DEBUG_SAY("User rating is %d.", rating);
 *
 *        BoardBitmapData bbd;
 *        if(iniFile.ReadBinary("Board", "Bitmap", &bbd, sizeof(bbd));
 *            DEBUG_SAY("Board bitmap binary data read successfully.");
 *
 */
class INIClass
{
    friend class INIEntry;
    friend class INISection;

public:
    enum
    {
        MAX_LINE_LENGTH = 512, // this is 512 in Generals, 4096 in BFME
        MAX_BUF_SIZE = 4096,
        MAX_UUBLOCK_LINE_LENGTH = 70,
        MAX_TEXTBLOCK_LINE_LENGTH = 75
    };

public:
    INIClass() : m_fileName(nullptr) { Initialize(); }
    INIClass(FileClass &file);
    virtual ~INIClass();

    void Initialize();
    void Shutdown();
    bool Clear(const char *section = nullptr, const char *entry = nullptr);
    bool Is_Loaded() const { return m_sectionList->First()->Is_Valid(); }

    int Load(FileClass &file);
    int Load(Straw &straw);

    List<INISection *> &Get_Section_List() { return *m_sectionList; }
    IndexClass<int32_t, INISection *> &Get_Section_Index() { return *m_sectionIndex; }

    // Returns the section object if it exists.
    INISection *Find_Section(const char *section) const;
    int Section_Count() { return m_sectionIndex->Count(); }

    // Returns the entry object if it exists.
    INIEntry *Find_Entry(const char *section, const char *entry) const;
    int Entry_Count(const char *section) const;
    const char *Get_Entry(const char *section, int index) const;

    // Enumerate_Entries()
    //   Enumerates all entries (key/value pairs) of a given section.
    //   Returns the number of entries present or -1 upon error.
    int Enumerate_Entries(const char *section, const char *entry_prefix, uint32_t start_number, uint32_t end_number);
    bool Put_Int(const char *section, const char *entry, int value, int format = INIINTEGER_AS_DECIMAL);
    int Get_Int(const char *section, const char *entry, int defvalue = 0) const;
    bool Put_Bool(const char *section, const char *entry, bool value);
    bool Get_Bool(const char *section, const char *entry, bool defvalue = false) const;
    bool Put_Hex(const char *section, const char *entry, int value);
    int Get_Hex(const char *section, const char *entry, int defvalue = 0) const;
    bool Put_Float(const char *section, const char *entry, double value);
    float Get_Float(const char *section, const char *entry, float defvalue = 0) const;
    bool Put_Double(const char *section, const char *entry, double value);
    double Get_Double(const char *section, const char *entry, double defvalue = 0) const;
    bool Put_String(const char *section, const char *entry, const char *string);
    int Get_String(
        const char *section, const char *entry, const char *defvalue = "", char *buffer = nullptr, int length = 0) const;

private:
    static void Strip_Comments(char *line);
    static int32_t CRC(const char *value) { return CRC::String(value, 0); }
    void Duplicate_CRC_Error(const char *message, const char *section, const char *entry);

protected:
    List<INISection *> *m_sectionList;
    IndexClass<int32_t, INISection *> *m_sectionIndex;
    const char *m_fileName;
};
