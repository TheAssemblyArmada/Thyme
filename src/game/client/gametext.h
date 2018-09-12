/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief String file handler.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "file.h"
#include "subsysteminterface.h"
#include "unicodestring.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

// This enum applies to RA2/YR and Generals/ZH, BFME ID's are slightly different.
enum LanguageID : int32_t
{
    LANGUAGE_ID_US = 0,
    LANGUAGE_ID_UK,
    LANGUAGE_ID_GERMAN,
    LANGUAGE_ID_FRENCH,
    LANGUAGE_ID_SPANISH,
    LANGUAGE_ID_ITALIAN,
    LANGUAGE_ID_JAPANSE,
    LANGUAGE_ID_JABBER,
    LANGUAGE_ID_KOREAN,
    LANGUAGE_ID_CHINESE,
    LANGUAGE_ID_UNK1,
    LANGUAGE_ID_UNK2,
    LANGUAGE_ID_POLISH,
    LANGUAGE_ID_UNKNOWN,
};

struct CSFHeader
{
    uint32_t id;
    int32_t version;
    int32_t num_labels;
    int32_t num_strings;
    int32_t skip;
    LanguageID langid;
};

struct NoString
{
    NoString *next;
    Utf16String text;
};

struct StringInfo
{
    Utf8String label;
    Utf16String text;
    Utf8String speech;
};

struct StringLookUp
{
    Utf8String *label;
    StringInfo *info;
};

class GameTextInterface : public SubsystemInterface
{
public:
    virtual ~GameTextInterface() {}

    virtual Utf16String Fetch(const char *args, bool *success = nullptr) = 0;
    virtual Utf16String Fetch(Utf8String args, bool *success = nullptr) = 0;
    virtual std::vector<Utf8String> *Get_Strings_With_Prefix(Utf8String label) = 0;
    virtual void Init_Map_String_File(Utf8String const &filename) = 0;
    virtual void Deinit() = 0;
};

class GameTextManager : public GameTextInterface
{
public:
    GameTextManager();
    virtual ~GameTextManager();

    virtual void Init();
    virtual void Reset();
    virtual void Update() {}

    virtual Utf16String Fetch(const char *args, bool *success = nullptr);
    virtual Utf16String Fetch(Utf8String args, bool *success = nullptr);
    virtual std::vector<Utf8String> *Get_Strings_With_Prefix(Utf8String label);
    virtual void Init_Map_String_File(Utf8String const &filename);
    virtual void Deinit();

    static int Compare_LUT(void const *a, void const *b);
    static GameTextInterface *Create_Game_Text_Interface();
#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif
private:
    void Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len);
    void Translate_Copy(unichar_t *out, char *in);
    void Remove_Leading_And_Trailing(char *buffer);
    void Strip_Spaces(unichar_t *buffer);
    void Reverse_Word(char *start, char *end);
    char Read_Char(File *file);
    bool Read_Line(char *buffer, int length, File *file);
    bool Get_String_Count(const char *filename, int &count);
    bool Get_CSF_Info(const char *filename);
    bool Parse_String_File(const char *filename);
    bool Parse_CSF_File(const char *filename);
    bool Parse_Map_String_File(const char *filename);

private:
    int m_textCount;
    int m_maxLabelLen;
    char m_bufferIn[10240];
    char m_bufferOut[10240];
    char m_bufferEx[10240];
    unichar_t m_translateBuffer[20480];
    StringInfo *m_stringInfo;
    StringLookUp *m_stringLUT;
    bool m_initialized;
    // pad 3 chars
    NoString *m_noStringList;
    bool m_useStringFile;
    // pad 3 chars
    LanguageID m_language;
    Utf16String m_failed;
    StringInfo *m_mapStringInfo;
    StringLookUp *m_mapStringLUT;
    int m_mapTextCount;
    std::vector<Utf8String> m_stringVector;
};

#ifndef THYME_STANDALONE
//#define g_theGameText Make_Global<GameTextInterface*>(0x00A2A2AC)
extern GameTextInterface *&g_theGameText;
inline void GameTextManager::Hook_Me()
{
    Hook_Function(0x00418320, &Create_Game_Text_Interface);
    Hook_Function(0x0041A020, &Compare_LUT);
}
#else
extern GameTextInterface *g_theGameText;
#endif