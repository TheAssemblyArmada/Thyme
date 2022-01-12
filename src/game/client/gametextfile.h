/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Localization File. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "fileref.h"
#include "gametextcommon.h"
#include "utility/array.h"
#include "utility/arrayview.h"
#include "utility/ebitflags.h"
#include "utility/enumerator.h"
#include "utility/enumflags.h"

namespace Thyme
{

enum class GameTextOption : uint32_t
{
    NONE = 0,
    OPTIMIZE_MEMORY_SIZE = (1 << 0),
    CHECK_BUFFER_LENGTH_ON_LOAD = (1 << 1),
    CHECK_BUFFER_LENGTH_ON_SAVE = (1 << 2),
    KEEP_OBSOLETE_SPACES_ON_LOAD = (1 << 3),
    WRITE_EXTRA_LF_ON_STR_SAVE = (1 << 4),
};

bool Name_To_Game_Text_Option(const char *name, GameTextOption &option);

} // namespace Thyme

DEFINE_RTS_UNDERLYING_TYPE(Thyme::GameTextOption, uint32_t);

namespace Thyme
{

// #TODO Split the multi language functionality from 'GameTextFile' into a new 'MultiGameTextFile' class?
// #TODO Return error codes for load and save?

// #FEATURE GameTextFile contains the core file handling functionality of original GameTextManager, which allows to use it
// for more flexible localization file operations.
class GameTextFile
{
public:
    using Options = rts::ebitflags<GameTextOption>;
    using Languages = rts::enumflags<LanguageID, g_languageCount>;

public:
    GameTextFile();

    // Checks whether or not localization is loaded.
    bool Is_Loaded() const;
    bool Is_Loaded(Languages languages) const;
    bool Is_Any_Loaded(Languages languages) const;

    // Loads CSF or STR file from disk. Does not unload previous data on failure.
    bool Load(const char *filename);
    bool Load_CSF(const char *filename);
    bool Load_STR(const char *filename);
    bool Load_Multi_STR(const char *filename, Languages languages);

    // Saves CSF or STR file to disk. Will write over any existing file.
    bool Save(const char *filename);
    bool Save_CSF(const char *filename);
    bool Save_STR(const char *filename);
    bool Save_Multi_STR(const char *filename, Languages languages);

    // Unloads language data.
    void Unload();
    void Unload(Languages languages);

    // Unloads all data and resets all settings.
    void Reset();

    // Merges another game text into this one. Identical labels will write their text contents from the other to this.
    void Merge_And_Overwrite(const GameTextFile &other);
    void Merge_And_Overwrite(const GameTextFile &other, Languages languages);

    // Retrieves all localization data.
    const StringInfos &Get_String_Infos() const;
    const StringInfos &Get_String_Infos(LanguageID language) const;

    // Sets options for loading and saving files.
    void Set_Options(Options options);
    Options Get_Options() const;

    // Sets the active language used by this instance. Loading a CSF file will automatically change active language.
    void Set_Language(LanguageID language);
    LanguageID Get_Language() const;

    // Swaps strings from one language to another one.
    void Swap_String_Infos(LanguageID left, LanguageID right);

    // Optional logging stream. Works besides captains log.
    static void Set_Log_File(FILE *log);

private:
    enum class FileType
    {
        AUTO,
        CSF,
        STR,
        MULTI_STR,

        COUNT,
    };

    enum class StrReadStep
    {
        LABEL,
        SEARCH,
        TEXT,
    };

    enum class StrParseResult
    {
        IS_NOTHING,
        IS_LABEL,
        IS_PRETEXT,
        IS_SPEECH,
        IS_END,
    };

    struct LengthInfo
    {
        int max_label_len;
        int max_text8_len;
        int max_text16_len;
        int max_speech_len;
    };

    // https://www.rfc-editor.org/rfc/rfc3629
    // In UTF-8, characters from the U+0000..U+10FFFF range (the UTF-16
    // accessible range) are encoded using sequences of 1 to 4 octets.
    enum : size_t
    {
        TEXT_16_SIZE = 1024,
        TEXT_8_SIZE = TEXT_16_SIZE * 4,
    };

    using StringInfosArray = rts::array<StringInfos, g_languageCount>;
    using ConstStringInfosPtrArray = rts::array<const StringInfos *, g_languageCount>;
    using StringInfosPtrArray = rts::array<StringInfos *, g_languageCount>;
    using Utf8Array = rts::array<char, TEXT_8_SIZE>;
    using Utf16Array = rts::array<unichar_t, TEXT_16_SIZE>;
    using Utf8View = rts::array_view<char>;
    using Utf16View = rts::array_view<unichar_t>;

private:
    bool Load(const char *filename, FileType filetype, Languages languages);
    bool Save(const char *filename, FileType filetype, Languages languages);

    void Merge_And_Overwrite_Internal(const GameTextFile &other, LanguageID language);
    void Check_Buffer_Lengths(LanguageID language);

    StringInfos &Mutable_String_Infos();
    StringInfos &Mutable_String_Infos(LanguageID language);

    template<typename Functor> static void For_Each_Language(Languages languages, Functor functor);
    static StringInfosPtrArray Build_String_Infos_Ptrs_Array(StringInfosArray &string_infos_array, Languages languages);
    static ConstStringInfosPtrArray Build_Const_String_Infos_Ptrs_Array(
        StringInfosArray &string_infos_array, Languages languages);

    static size_t Get_Max_Size(const ConstStringInfosPtrArray &string_infos_ptrs);

    static void Build_Multi_String_Infos(
        MultiStringInfos &multi_string_infos, const ConstStringInfosPtrArray &string_infos_ptrs, Options options);
    static void Build_String_Infos(
        StringInfosPtrArray &string_infos_ptrs, const MultiStringInfos &multi_string_infos, Options options);

    static bool Get_Language_With_String_Infos(
        LanguageID &language, const StringInfosPtrArray &string_infos_ptrs, size_t occurence = 0);

    static Languages Filter_Usable_Languages(Languages languages);
    static bool Supports_Multi_Language(FileType filetype);
    static FileType Get_File_Type(const char *filename, FileType filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Assert_Length_Info(const LengthInfo &len_info);

    static Utf16String &Get_Text(StringInfo &string_info, LanguageID language);
    static Utf16String &Get_Text(MultiStringInfo &string_info, LanguageID language);
    static Utf8String &Get_Speech(StringInfo &string_info, LanguageID language);
    static Utf8String &Get_Speech(MultiStringInfo &string_info, LanguageID language);

    static bool Read_Multi_STR_File(
        FileRef &file, StringInfosPtrArray &string_infos_ptrs, Languages languages, Options options);
    static bool Read_STR_File(FileRef &file, StringInfos &string_infos, Options options);
    template<typename StringInfosType>
    static void Read_STR_File_T(FileRef &file, StringInfosType &string_infos, Languages languages, Options options);
    static StrParseResult Parse_STR_Label(Utf8Array &buf, Utf8String &label);
    static StrParseResult Parse_STR_Search(Utf8Array &buf);
    static void Parse_STR_Text(Utf8Array &buf, Utf16String &text, Options options);
    static void Parse_STR_Speech(Utf8View &buf, Utf8String &speech);
    static bool Parse_STR_Language(const char *cstring, LanguageID &language, size_t &parsed_count);
    static bool Is_STR_Pre_Text(Utf8View buf);
    static bool Is_STR_Comment(const char *cstring);
    static bool Is_STR_End(const char *cstring);
    static void Change_Step(StrReadStep &step, StrReadStep new_step, const char *&eol_chars);

    static bool Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language, Options options);
    static bool Read_CSF_Header(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Entry(FileRef &file, StringInfo &string_info, Options options, Utf16Array &buf);
    static bool Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts);
    static bool Read_CSF_Text(FileRef &file, StringInfo &string_info, Options options, Utf16Array &buf);

    static bool Write_Multi_STR_File(
        FileRef &file, const ConstStringInfosPtrArray &string_infos_ptrs, Languages languages, Options options);
    static bool Write_Multi_STR_Entry(FileRef &file,
        const MultiStringInfo &string_info,
        Languages languages,
        Options options,
        Utf8Array &buf,
        Utf8String &str);
    static bool Write_STR_Language(FileRef &file, LanguageID language);

    static bool Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options);
    static bool Write_STR_Entry(
        FileRef &file, const StringInfo &string_info, Options options, Utf8Array &buf, Utf8String &str);
    static bool Write_STR_Label(FileRef &file, const Utf8String &label);
    static bool Write_STR_Text(FileRef &file, const Utf16String &text, Options options, Utf8Array &buf, Utf8String &str);
    static bool Write_STR_Speech(FileRef &file, const Utf8String &speech);
    static bool Write_STR_End(FileRef &file);

    static bool Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Array &buf);
    static bool Write_CSF_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Array &buf);

    static void Log_Line(const char *prefix, const char *format, ...);

private:
    Options m_options;
    LanguageID m_language;
    StringInfosArray m_stringInfosArray;

    static FILE *s_logfile;
};

} // namespace Thyme
