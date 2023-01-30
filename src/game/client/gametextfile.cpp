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
#include "always.h"
#include "gametextfile.h"
#include "filesystem.h"
#include "gametextlookup.h"
#include "rtsutils.h"
#include "utility/arrayutil.h"
#include "utility/fileutil.h"
#include "utility/stlutil.h"
#include "utility/stringutil.h"

#define GAMETEXTLOG_TRACE(fmt, ...) \
    captainslog_trace(fmt, ##__VA_ARGS__); \
    GameTextFile::Log_Line("TRACE : ", fmt, ##__VA_ARGS__)

#define GAMETEXTLOG_DEBUG(fmt, ...) \
    captainslog_debug(fmt, ##__VA_ARGS__); \
    GameTextFile::Log_Line("DEBUG : ", fmt, ##__VA_ARGS__)

#define GAMETEXTLOG_INFO(fmt, ...) \
    captainslog_info(fmt, ##__VA_ARGS__); \
    GameTextFile::Log_Line("", fmt, ##__VA_ARGS__)

#define GAMETEXTLOG_WARN(fmt, ...) \
    captainslog_warn(fmt, ##__VA_ARGS__); \
    GameTextFile::Log_Line("WARNING : ", fmt, ##__VA_ARGS__)

#define GAMETEXTLOG_ERROR(fmt, ...) \
    captainslog_error(fmt, ##__VA_ARGS__); \
    GameTextFile::Log_Line("ERROR : ", fmt, ##__VA_ARGS__)

#define GAMETEXTLOG_FATAL(fmt, ...) \
    captainslog_fatal(fmt, ##__VA_ARGS__); \
    GameTextFile::Log_Line("FATAL : ", fmt, ##__VA_ARGS__)

namespace Thyme
{

namespace
{
template<typename IntegerType> constexpr size_t Bit_To_Index(IntegerType integer)
{
    using UnsignedInt = UnsignedIntegerT<IntegerType>;
    size_t n = 0;
    for (; n < sizeof(IntegerType) * 8; ++n) {
        if (UnsignedInt(integer) & (UnsignedInt(1) << n)) {
            return n;
        }
    }
    return n;
}

constexpr const char *const s_option_0 = "None";
constexpr const char *const s_option_1 = "Optimize_Memory_Size";
constexpr const char *const s_option_2 = "Check_Buffer_Length_On_Load";
constexpr const char *const s_option_3 = "Check_Buffer_Length_On_Save";
constexpr const char *const s_option_4 = "Keep_Obsolete_Spaces_On_Load";
constexpr const char *const s_option_5 = "Write_Extra_LF_On_STR_Save";

constexpr const char *const s_options[] = {
    s_option_0,
    s_option_1,
    s_option_2,
    s_option_3,
    s_option_4,
    s_option_5,
};

static_assert(s_option_0 == s_options[size_t(GameTextOption::NONE)]);
static_assert(s_option_1 == s_options[1 + Bit_To_Index(GameTextOption::OPTIMIZE_MEMORY_SIZE)]);
static_assert(s_option_2 == s_options[1 + Bit_To_Index(GameTextOption::CHECK_BUFFER_LENGTH_ON_LOAD)]);
static_assert(s_option_3 == s_options[1 + Bit_To_Index(GameTextOption::CHECK_BUFFER_LENGTH_ON_SAVE)]);
static_assert(s_option_4 == s_options[1 + Bit_To_Index(GameTextOption::KEEP_OBSOLETE_SPACES_ON_LOAD)]);
static_assert(s_option_5 == s_options[1 + Bit_To_Index(GameTextOption::WRITE_EXTRA_LF_ON_STR_SAVE)]);
} // namespace

bool Name_To_Game_Text_Option(const char *name, GameTextOption &option)
{
    size_t index = 0;
    for (const char *option_name : s_options) {
        if (strcasecmp(option_name, name) == 0) {
            option = (index == 0) ? GameTextOption::NONE : static_cast<GameTextOption>(1 << (index - 1));
            return true;
        }
        ++index;
    }
    return false;
}

namespace
{
// ISO 639-1 language codes - sort of.

constexpr const char *const s_langcode_invalid = "__";

constexpr const char *const s_langcode_us = "US";
constexpr const char *const s_langcode_en = s_langcode_invalid;
constexpr const char *const s_langcode_de = "DE";
constexpr const char *const s_langcode_fr = "FR";
constexpr const char *const s_langcode_es = "ES";
constexpr const char *const s_langcode_it = "IT";
constexpr const char *const s_langcode_ja = "JA";
constexpr const char *const s_langcode_jb = s_langcode_invalid;
constexpr const char *const s_langcode_ko = "KO";
constexpr const char *const s_langcode_zh = "ZH";
constexpr const char *const s_langcode___ = s_langcode_invalid;
constexpr const char *const s_langcode_bp = "BP";
constexpr const char *const s_langcode_pl = "PL";
constexpr const char *const s_langcode_uk = s_langcode_invalid;
constexpr const char *const s_langcode_ru = "RU";
constexpr const char *const s_langcode_ar = "AR";

constexpr const char *const s_langcodes[] = {
    s_langcode_us,
    s_langcode_en,
    s_langcode_de,
    s_langcode_fr,
    s_langcode_es,
    s_langcode_it,
    s_langcode_ja,
    s_langcode_jb,
    s_langcode_ko,
    s_langcode_zh,
    s_langcode___,
    s_langcode_bp,
    s_langcode_pl,
    s_langcode_uk,
    s_langcode_ru,
    s_langcode_ar,
};

static_assert(s_langcode_us == s_langcodes[size_t(LanguageID::US)]);
static_assert(s_langcode_en == s_langcodes[size_t(LanguageID::UK)]);
static_assert(s_langcode_de == s_langcodes[size_t(LanguageID::GERMAN)]);
static_assert(s_langcode_fr == s_langcodes[size_t(LanguageID::FRENCH)]);
static_assert(s_langcode_es == s_langcodes[size_t(LanguageID::SPANISH)]);
static_assert(s_langcode_it == s_langcodes[size_t(LanguageID::ITALIAN)]);
static_assert(s_langcode_ja == s_langcodes[size_t(LanguageID::JAPANESE)]);
static_assert(s_langcode_jb == s_langcodes[size_t(LanguageID::JABBER)]);
static_assert(s_langcode_ko == s_langcodes[size_t(LanguageID::KOREAN)]);
static_assert(s_langcode_zh == s_langcodes[size_t(LanguageID::CHINESE)]);
static_assert(s_langcode___ == s_langcodes[size_t(LanguageID::UNUSED_1)]);
static_assert(s_langcode_bp == s_langcodes[size_t(LanguageID::BRAZILIAN)]);
static_assert(s_langcode_pl == s_langcodes[size_t(LanguageID::POLISH)]);
static_assert(s_langcode_uk == s_langcodes[size_t(LanguageID::UNKNOWN)]);
static_assert(s_langcode_ru == s_langcodes[size_t(LanguageID::RUSSIAN)]);
static_assert(s_langcode_ar == s_langcodes[size_t(LanguageID::ARABIC)]);

static_assert(ARRAY_SIZE(s_langcodes) == g_languageCount);

constexpr const char *Get_Language_Code(LanguageID language)
{
    return s_langcodes[size_t(language)];
}

constexpr const char s_str_eol[] = { '\r', '\n' };
constexpr const char s_str_quo[] = { '"' };
constexpr const char s_str_end[] = { 'E', 'N', 'D' };
constexpr const char s_str_lng[] = { ':' };

struct CSFLabelHeader
{
    uint32_t id;
    int32_t texts;
    int32_t length;
};

struct CSFTextHeader
{
    uint32_t id;
    int32_t length;
};

struct CSFSpeechHeader
{
    int32_t length;
};

} // namespace

FILE *GameTextFile::s_logfile = nullptr;

GameTextFile::GameTextFile() :
    m_options(GameTextOption::OPTIMIZE_MEMORY_SIZE), m_language(LanguageID::UNKNOWN), m_stringInfosArray(){};

bool GameTextFile::Is_Loaded() const
{
    return !Get_String_Infos().empty();
}

bool GameTextFile::Is_Loaded(Languages languages) const
{
    bool loaded = true;
    For_Each_Language(languages, [&](LanguageID language) { loaded &= !Get_String_Infos(language).empty(); });
    return loaded;
}

bool GameTextFile::Is_Any_Loaded(Languages languages) const
{
    bool loaded = true;
    For_Each_Language(languages, [&](LanguageID language) { loaded |= !Get_String_Infos(language).empty(); });
    return loaded;
}

bool GameTextFile::Load(const char *filename)
{
    const FileType filetype = Get_File_Type(filename, FileType::AUTO);

    return Load(filename, filetype, ~Languages());
}

bool GameTextFile::Load_CSF(const char *filename)
{
    return Load(filename, FileType::CSF, Languages());
}

bool GameTextFile::Load_STR(const char *filename)
{
    return Load(filename, FileType::STR, Languages());
}

bool GameTextFile::Load_Multi_STR(const char *filename, Languages languages)
{
    return Load(filename, FileType::MULTI_STR, languages);
}

bool GameTextFile::Save(const char *filename)
{
    FileType filetype = Get_File_Type(filename, FileType::AUTO);

    return Save(filename, filetype, ~Languages());
}

bool GameTextFile::Save_CSF(const char *filename)
{
    return Save(filename, FileType::CSF, Languages());
}

bool GameTextFile::Save_STR(const char *filename)
{
    return Save(filename, FileType::STR, Languages());
}

bool GameTextFile::Save_Multi_STR(const char *filename, Languages languages)
{
    return Save(filename, FileType::MULTI_STR, languages);
}

bool GameTextFile::Load(const char *filename, FileType filetype, Languages languages)
{
    captainslog_assert(filetype != FileType::AUTO);

    if (!filename || !filename[0]) {
        GAMETEXTLOG_ERROR("File without name cannot be loaded");
        return false;
    }

    const int filemode = Encode_Buffered_File_Mode(File::READ | File::BINARY, 1024 * 32);
    FileRef file = g_theFileSystem->Open_File(filename, filemode);

    if (!file.Is_Open()) {
        GAMETEXTLOG_ERROR("File '%s' cannot be opened for read", filename);
        return false;
    }

    bool success = false;

    languages = Filter_Usable_Languages(languages);
    LanguageID read_language = m_language;
    StringInfosArray string_infos_array;
    StringInfos &string_infos = string_infos_array[size_t(read_language)];

    switch (filetype) {

        case FileType::CSF: {
            success = Read_CSF_File(file, string_infos, read_language, m_options);
            string_infos_array[size_t(read_language)].swap(string_infos);
            break;
        }
        case FileType::STR: {
            success = Read_STR_File(file, string_infos, m_options);
            break;
        }
        case FileType::MULTI_STR: {
            StringInfosPtrArray string_infos_ptrs = Build_String_Infos_Ptrs_Array(string_infos_array, languages);
            success = Read_Multi_STR_File(file, string_infos_ptrs, languages, m_options);
            Get_Language_With_String_Infos(read_language, string_infos_ptrs, 0);
            break;
        }
    }

    if (success) {
        m_language = read_language;
        const Languages used_languages = Supports_Multi_Language(filetype) ? languages : read_language;

        GAMETEXTLOG_INFO("File '%s' loaded successfully", filename);

        For_Each_Language(used_languages, [&](LanguageID language) {
            Mutable_String_Infos(language).swap(string_infos_array[size_t(language)]);

            GAMETEXTLOG_INFO("Read language: %s", Get_Language_Name(language));
            GAMETEXTLOG_INFO("Read line count: %zu", Get_String_Infos(language).size());

            if (m_options.has(Options::Value::CHECK_BUFFER_LENGTH_ON_LOAD)) {
                Check_Buffer_Lengths(language);
            }
        });
    } else {
        GAMETEXTLOG_ERROR("File '%s' failed to load", filename);
    }

    return success;
}

bool GameTextFile::Save(const char *filename, FileType filetype, Languages languages)
{
    captainslog_assert(filetype != FileType::AUTO);

    if (!filename || !filename[0]) {
        GAMETEXTLOG_ERROR("File without name cannot be saved");
        return false;
    }

    languages = Filter_Usable_Languages(languages);
    const Languages used_languages = Supports_Multi_Language(filetype) ? languages : m_language;

    if (!Is_Any_Loaded(used_languages)) {
        GAMETEXTLOG_ERROR("File without string data cannot be saved");
        return false;
    }

    const int filemode = Encode_Buffered_File_Mode(File::WRITE | File::CREATE | File::BINARY, 1024 * 32);
    FileRef file = g_theFileSystem->Open_File(filename, filemode);

    if (!file.Is_Open()) {
        GAMETEXTLOG_ERROR("File '%s' cannot be opened for write", filename);
        return false;
    }

    bool success = false;

    switch (filetype) {

        case FileType::CSF: {
            success = Write_CSF_File(file, Get_String_Infos(), m_language);
            break;
        }
        case FileType::STR: {
            success = Write_STR_File(file, Get_String_Infos(), m_options);
            break;
        }
        case FileType::MULTI_STR: {
            ConstStringInfosPtrArray string_infos_ptrs = Build_Const_String_Infos_Ptrs_Array(m_stringInfosArray, languages);
            success = Write_Multi_STR_File(file, string_infos_ptrs, languages, m_options);
            break;
        }
    }

    if (success) {
        GAMETEXTLOG_INFO("File '%s' saved successfully", filename);

        For_Each_Language(used_languages, [&](LanguageID language) {
            GAMETEXTLOG_INFO("Written language: %s", Get_Language_Name(language));
            GAMETEXTLOG_INFO("Written line count: %zu", Get_String_Infos(language).size());

            if (m_options.has(Options::Value::CHECK_BUFFER_LENGTH_ON_SAVE)) {
                Check_Buffer_Lengths(language);
            }
        });
    } else {
        GAMETEXTLOG_ERROR("File '%s' failed to save", filename);
    }

    return success;
}

void GameTextFile::Unload()
{
    Unload(m_language);
}

void GameTextFile::Unload(Languages languages)
{
    For_Each_Language(languages, [&](LanguageID language) { rts::Free_Container(Mutable_String_Infos(language)); });
}

void GameTextFile::Reset()
{
    for (StringInfos &string_infos : m_stringInfosArray) {
        rts::Free_Container(string_infos);
    }
    m_options = Options::Value::NONE;
    m_language = LanguageID::UNKNOWN;
}

void GameTextFile::Merge_And_Overwrite(const GameTextFile &other)
{
    Merge_And_Overwrite_Internal(other, m_language);
}

void GameTextFile::Merge_And_Overwrite(const GameTextFile &other, Languages languages)
{
    For_Each_Language(languages, [&](LanguageID language) { Merge_And_Overwrite_Internal(other, language); });
}

void GameTextFile::Merge_And_Overwrite_Internal(const GameTextFile &other, LanguageID language)
{
    const size_t other_size = other.Get_String_Infos(language).size();
    StringInfos &this_strings = Mutable_String_Infos(language);
    StringInfos other_new_strings;
    other_new_strings.reserve(other_size);

    {
        const MutableGameTextLookup this_lookup(this_strings);

        for (const StringInfo &other_string : other.Get_String_Infos(language)) {
            const MutableStringLookup *this_string_lookup = this_lookup.Find(other_string.label.Str());

            if (this_string_lookup == nullptr) {
                // Other string is new. Prepare to add.
                other_new_strings.push_back(other_string);
            } else {
                // Other string already exists. Update this string.
                this_string_lookup->string_info->text = other_string.text;
                this_string_lookup->string_info->speech = other_string.speech;
            }
        }
    }

    rts::Append_Container(this_strings, other_new_strings);
}

void GameTextFile::Check_Buffer_Lengths(LanguageID language)
{
    LengthInfo len_info;
    Collect_Length_Info(len_info, Get_String_Infos(language));
    Log_Length_Info(len_info);
    Assert_Length_Info(len_info);
}

const StringInfos &GameTextFile::Get_String_Infos() const
{
    return m_stringInfosArray[size_t(m_language)];
}

const StringInfos &GameTextFile::Get_String_Infos(LanguageID language) const
{
    return m_stringInfosArray[size_t(language)];
}

StringInfos &GameTextFile::Mutable_String_Infos()
{
    return m_stringInfosArray[size_t(m_language)];
}

StringInfos &GameTextFile::Mutable_String_Infos(LanguageID language)
{
    return m_stringInfosArray[size_t(language)];
}

void GameTextFile::Set_Options(Options options)
{
    m_options = options;
}

GameTextFile::Options GameTextFile::Get_Options() const
{
    return m_options;
}

void GameTextFile::Set_Language(LanguageID language)
{
    m_language = language;
}

LanguageID GameTextFile::Get_Language() const
{
    return m_language;
}

void GameTextFile::Swap_String_Infos(LanguageID left, LanguageID right)
{
    if (left != right) {
        m_stringInfosArray[size_t(left)].swap(m_stringInfosArray[size_t(right)]);
    }
}

void GameTextFile::Set_Log_File(FILE *log)
{
    s_logfile = log;
}

template<typename Functor> void GameTextFile::For_Each_Language(Languages languages, Functor functor)
{
    for (rts::enumerator<LanguageID> it; it < LanguageID(g_languageCount); ++it) {
        const LanguageID language = it.value();
        if (languages.has(language)) {
            functor(language);
        }
    }
}

GameTextFile::StringInfosPtrArray GameTextFile::Build_String_Infos_Ptrs_Array(
    StringInfosArray &string_infos_array, Languages languages)
{
    StringInfosPtrArray string_infos_ptrs = {};

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        string_infos_ptrs[index] = &string_infos_array[index];
    });
    return string_infos_ptrs;
}

GameTextFile::ConstStringInfosPtrArray GameTextFile::Build_Const_String_Infos_Ptrs_Array(
    StringInfosArray &string_infos_array, Languages languages)
{
    ConstStringInfosPtrArray string_infos_ptrs = {};

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        string_infos_ptrs[index] = &string_infos_array[index];
    });
    return string_infos_ptrs;
}

size_t GameTextFile::Get_Max_Size(const ConstStringInfosPtrArray &string_infos_ptrs)
{
    size_t size = 0;

    for (const StringInfos *string_infos : string_infos_ptrs) {
        if (string_infos != nullptr) {
            size = std::max(size, string_infos->size());
        }
    }
    return size;
}

void GameTextFile::Build_Multi_String_Infos(
    MultiStringInfos &multi_string_infos, const ConstStringInfosPtrArray &string_infos_ptrs, Options options)
{
    const size_t estimated_size = Get_Max_Size(string_infos_ptrs);
    size_t current_size = 0;
    MultiStringInfos tmp_multi_string_infos;
    tmp_multi_string_infos.reserve(estimated_size);
    multi_string_infos.clear();
    multi_string_infos.reserve(estimated_size);
    MutableMultiGameTextLookup lookup;

    size_t language_index = 0;

    for (const StringInfos *string_infos : string_infos_ptrs) {
        if (string_infos != nullptr) {
            if (current_size != multi_string_infos.size()) {
                current_size = multi_string_infos.size();
                lookup.Load(multi_string_infos);
            }

            for (const StringInfo &string_info : *string_infos) {
                const MutableMultiStringLookup *multi_string_lookup = lookup.Find(string_info.label.Str());

                if (multi_string_lookup == nullptr) {
                    MultiStringInfo multi_string_info;
                    multi_string_info.label = string_info.label;
                    multi_string_info.text[language_index] = string_info.text;
                    multi_string_info.speech[language_index] = string_info.speech;
                    tmp_multi_string_infos.push_back(multi_string_info);
                } else {
                    multi_string_lookup->string_info->text[language_index] = string_info.text;
                    multi_string_lookup->string_info->speech[language_index] = string_info.speech;
                }
            }
            rts::Append_Container(multi_string_infos, tmp_multi_string_infos);
            tmp_multi_string_infos.clear();
        }
        ++language_index;
    }

    if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
        rts::Shrink_To_Fit(multi_string_infos);
    }
}

void GameTextFile::Build_String_Infos(
    StringInfosPtrArray &string_infos_ptrs, const MultiStringInfos &multi_string_infos, Options options)
{
    size_t language_index = 0;

    for (StringInfos *string_infos_ptr : string_infos_ptrs) {
        if (string_infos_ptr != nullptr) {
            StringInfos &string_infos = *string_infos_ptr;
            if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
                rts::Free_Container(string_infos);
            }
            string_infos.resize(multi_string_infos.size());
            size_t string_index = 0;

            for (const MultiStringInfo &multi_string_info : multi_string_infos) {
                string_infos[string_index].label = multi_string_info.label;
                string_infos[string_index].text = multi_string_info.text[language_index];
                string_infos[string_index].speech = multi_string_info.speech[language_index];
                ++string_index;
            }
        }
        ++language_index;
    }
}

bool GameTextFile::Get_Language_With_String_Infos(
    LanguageID &language, const StringInfosPtrArray &string_infos_ptrs, size_t occurence)
{
    size_t num = 0;
    rts::enumerator<LanguageID> it;
    for (const StringInfosPtrArray::value_type &string_infos_ptr : string_infos_ptrs) {
        if (string_infos_ptr != nullptr && !string_infos_ptr->empty()) {
            if (occurence == num++) {
                language = it.value();
                return true;
            }
        }
        ++it;
    }
    return false;
}

GameTextFile::Languages GameTextFile::Filter_Usable_Languages(Languages languages)
{
    for (rts::enumerator<LanguageID> it; it < LanguageID::COUNT; ++it) {
        if (languages.has(it.value())) {
            const char *langcode = s_langcodes[it.underlying()];
            if (strcmp(langcode, s_langcode_invalid) == 0) {
                languages.reset(it.value());
            }
        }
    }
    return languages;
}

bool GameTextFile::Supports_Multi_Language(FileType filetype)
{
    switch (filetype) {
        case FileType::MULTI_STR:
            return true;
        default:
            return false;
    }
}

GameTextFile::FileType GameTextFile::Get_File_Type(const char *filename, FileType filetype)
{
    if (filetype == FileType::AUTO) {

        const char *fileext = rts::Get_File_Extension(rts::Make_Array_View(filename));

        if (strcasecmp(fileext, "csf") == 0) {
            filetype = FileType::CSF;
        } else if (strcasecmp(fileext, "str") == 0) {
            filetype = FileType::STR;
        } else if (strcasecmp(fileext, "multistr") == 0) {
            filetype = FileType::MULTI_STR;
        } else {
            filetype = FileType::CSF;
        }
    }
    return filetype;
}

void GameTextFile::Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings)
{
    len_info.max_label_len = 0;
    len_info.max_text8_len = 0;
    len_info.max_text16_len = 0;
    len_info.max_speech_len = 0;

    Utf8String utf8text;

    for (const StringInfo &string : strings) {
        utf8text.Translate(string.text);

        len_info.max_label_len = std::max(len_info.max_label_len, string.label.Get_Length());
        len_info.max_text8_len = std::max(len_info.max_text8_len, utf8text.Get_Length());
        len_info.max_text16_len = std::max(len_info.max_text16_len, string.text.Get_Length());
        len_info.max_speech_len = std::max(len_info.max_speech_len, string.speech.Get_Length());
    }
}

void GameTextFile::Log_Length_Info(const LengthInfo &len_info)
{
    const int label_len = len_info.max_label_len;
    const int text8_len = len_info.max_text8_len;
    const int text16_len = len_info.max_text16_len;
    const int speech_len = len_info.max_speech_len;

    const bool label_ok = (TEXT_8_SIZE - 1 > label_len);
    const bool text8_ok = (TEXT_8_SIZE - 1 > text8_len);
    const bool text16_ok = (TEXT_16_SIZE - 1 > text16_len);
    const bool speech_ok = (TEXT_8_SIZE - 1 > speech_len);

    if (label_ok) {
        GAMETEXTLOG_INFO("Checked label len: %d, max: %d", label_len, TEXT_8_SIZE - 1);
    } else {
        GAMETEXTLOG_ERROR("Checked label len: %d, max: %d", label_len, TEXT_8_SIZE - 1);
    }

    if (text8_ok) {
        GAMETEXTLOG_INFO("Checked utf8 text len: %d, max: %d", text8_len, TEXT_8_SIZE - 1);
    } else {
        GAMETEXTLOG_ERROR("Checked utf8 text len: %d, max: %d", text8_len, TEXT_8_SIZE - 1);
    }

    if (text16_ok) {
        GAMETEXTLOG_INFO("Checked utf16 text len: %d, max: %d", text16_len, TEXT_16_SIZE - 1);
    } else {
        GAMETEXTLOG_ERROR("Checked utf16 text len: %d, max: %d", text16_len, TEXT_16_SIZE - 1);
    }

    if (speech_ok) {
        GAMETEXTLOG_INFO("Checked speech len: %d, max: %d", speech_len, TEXT_8_SIZE - 1);
    } else {
        GAMETEXTLOG_ERROR("Checked speech len: %d, max: %d", speech_len, TEXT_8_SIZE - 1);
    }
}

void GameTextFile::Assert_Length_Info(const LengthInfo &len_info)
{
    captainslog_dbgassert(TEXT_8_SIZE - 1 > len_info.max_label_len, "Buffer size must be larger");
    captainslog_dbgassert(TEXT_8_SIZE - 1 > len_info.max_text8_len, "Buffer size must be larger");
    captainslog_dbgassert(TEXT_16_SIZE - 1 > len_info.max_text16_len, "Buffer size must be larger");
    captainslog_dbgassert(TEXT_8_SIZE - 1 > len_info.max_speech_len, "Buffer size must be larger");
}

Utf16String &GameTextFile::Get_Text(StringInfo &string_info, LanguageID language)
{
    (void)language;
    return string_info.text;
}

Utf16String &GameTextFile::Get_Text(MultiStringInfo &string_info, LanguageID language)
{
    return string_info.text[size_t(language)];
}

Utf8String &GameTextFile::Get_Speech(StringInfo &string_info, LanguageID language)
{
    (void)language;
    return string_info.speech;
}

Utf8String &GameTextFile::Get_Speech(MultiStringInfo &string_info, LanguageID language)
{
    return string_info.speech[size_t(language)];
}

bool GameTextFile::Read_Multi_STR_File(
    FileRef &file, StringInfosPtrArray &string_infos_ptrs, Languages languages, Options options)
{
    GAMETEXTLOG_INFO("Reading text file '%s' in Multi STR format", file.Get_File_Name().Str());

    MultiStringInfos multi_string_infos;
    multi_string_infos.reserve(8192);

    Read_STR_File_T(file, multi_string_infos, languages, options);

    Build_String_Infos(string_infos_ptrs, multi_string_infos, options);

    return !multi_string_infos.empty();
}

bool GameTextFile::Read_STR_File(FileRef &file, StringInfos &string_infos, Options options)
{
    GAMETEXTLOG_INFO("Reading text file '%s' in STR format", file.Get_File_Name().Str());

    // Instead of reading the file once from top to bottom to get the number of the entries, we will allocate a very generous
    // buffer to begin with and shrink it down afterwards. This will reduce algorithm complexity and file access.
    string_infos.reserve(8192);

    Read_STR_File_T(file, string_infos, LanguageID::UNKNOWN, options);

    if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
        rts::Shrink_To_Fit(string_infos);
    }

    return !string_infos.empty();
}

template<typename StringInfosType>
void GameTextFile::Read_STR_File_T(FileRef &file, StringInfosType &string_infos, Languages languages, Options options)
{
    using StringInfoType = typename StringInfosType::value_type;

    StringInfoType string_info;
    StrParseResult result;
    LanguageID read_language = LanguageID::UNKNOWN;
    Utf8Array buf = {};
    StrReadStep step = StrReadStep::LABEL;
    const char *eol_chars = "\n";

    while (rts::Read_Line(file.Get_File(), buf.data(), buf.size(), eol_chars)) {

        switch (step) {
            case StrReadStep::LABEL:
                string_info = StringInfoType();
                result = Parse_STR_Label(buf, string_info.label);

                if (result == StrParseResult::IS_LABEL) {
                    Change_Step(step, StrReadStep::SEARCH, eol_chars);
                }
                break;

            case StrReadStep::SEARCH:
                read_language = LanguageID::UNKNOWN;
                result = Parse_STR_Search(buf);

                if (result == StrParseResult::IS_PRETEXT) {
                    size_t parsed_count;
                    Parse_STR_Language(buf.data(), read_language, parsed_count);
                    Change_Step(step, StrReadStep::TEXT, eol_chars);

                } else if (result == StrParseResult::IS_SPEECH) {
                    size_t parsed_count;
                    Parse_STR_Language(buf.data(), read_language, parsed_count);

                    if (languages.has(read_language)) {
                        Utf8View view(buf.data() + parsed_count, buf.size() - parsed_count);
                        Parse_STR_Speech(view, Get_Speech(string_info, read_language));
                    }

                } else if (result == StrParseResult::IS_END) {
                    string_infos.push_back(string_info);
                    Change_Step(step, StrReadStep::LABEL, eol_chars);
                }
                break;

            case StrReadStep::TEXT:
                if (languages.has(read_language)) {
                    Parse_STR_Text(buf, Get_Text(string_info, read_language), options);
                }
                Change_Step(step, StrReadStep::SEARCH, eol_chars);
                break;
        }
    }
}

GameTextFile::StrParseResult GameTextFile::Parse_STR_Label(Utf8Array &buf, Utf8String &label)
{
    rts::Strip_Characters(buf.data(), "\n\r");
    const size_t len = rts::Strip_Leading_And_Trailing_Spaces(buf.data());

    if (len == 0) {
        return StrParseResult::IS_NOTHING;
    }

    if (Is_STR_Comment(buf.data())) {
        return StrParseResult::IS_NOTHING;
    }

    label = buf.data();
    return StrParseResult::IS_LABEL;
}

GameTextFile::StrParseResult GameTextFile::Parse_STR_Search(Utf8Array &buf)
{
    rts::Strip_Characters(buf.data(), "\n\r");
    const size_t len = rts::Strip_Leading_And_Trailing_Spaces(buf.data());

    if (len == 0) {
        return StrParseResult::IS_NOTHING;
    }

    if (Is_STR_Comment(buf.data())) {
        return StrParseResult::IS_NOTHING;
    }

    if (Is_STR_End(buf.data())) {
        return StrParseResult::IS_END;
    }

    Utf8View view(buf.begin(), len);

    // #TODO the original appears to have some more checks for the Speech string. Could include them here as well.

    return Is_STR_Pre_Text(view) ? StrParseResult::IS_PRETEXT : StrParseResult::IS_SPEECH;
}

void GameTextFile::Parse_STR_Text(Utf8Array &buf, Utf16String &text, Options options)
{
    // Read string can be empty.
    rts::Strip_Characters(buf.data(), "\n\r");
    rts::Replace_Characters(buf.data(), "\t\v\f", ' ');

    // STR does support escaped characters for special control characters. When written out as 2 symbol sequence, it will be
    // converted into single character here. Convert in place.
    const char *search[6] = { "\\n", "\\t", "\\\"", "\\?", "\\\'", "\\\\" };
    const char *replace[6] = { "\n", "\t", "\"", "?", "\'", "\\" };
    size_t len = rts::Replace_Character_Sequences(buf.data(), buf.size(), buf.data(), search, replace);

    // Read string is expected to close with a quote. Remove it here.
    if (buf[len - 1] == '\"') {
        buf[len - 1] = '\0';
    }

    if (!options.has(GameTextOption::KEEP_OBSOLETE_SPACES_ON_LOAD)) {
        // Strip any remaining obsolete spaces for cleaner presentation in game.
        rts::Strip_Obsolete_Spaces(buf.data());
    }

    // Translate final UTF16 string.
    text.Translate(buf.data());
}

void GameTextFile::Parse_STR_Speech(Utf8View &buf, Utf8String &speech)
{
    rts::Strip_Characters(buf.data(), "\n\r");
    rts::Strip_Leading_And_Trailing_Spaces(buf.data());
    speech = buf.data();
}

bool GameTextFile::Parse_STR_Language(const char *cstring, LanguageID &language, size_t &parsed_count)
{
    const size_t code_len = strlen(s_langcodes[0]);
    const size_t lng_len = sizeof(s_str_lng);

    for (rts::enumerator<LanguageID> it; it < LanguageID::COUNT; ++it) {
        const char *code = s_langcodes[it.underlying()];
        if (strncasecmp(cstring, code, code_len) == 0) {
            if (strncasecmp(cstring + code_len, s_str_lng, lng_len) == 0) {
                language = it.value();
                parsed_count = code_len + lng_len;
                return true;
            }
        }
    }
    parsed_count = 0;
    return false;
}

bool GameTextFile::Is_STR_Pre_Text(Utf8View buf)
{
    return !buf.empty() ? (buf.back() == '"') : false;
}

bool GameTextFile::Is_STR_Comment(const char *cstring)
{
    return (cstring[0] == '/' && cstring[1] == '/') || (cstring[0] == '\\' && cstring[1] == '\\');
}

bool GameTextFile::Is_STR_End(const char *cstring)
{
    return (strcasecmp(cstring, "END") == 0);
}

void GameTextFile::Change_Step(StrReadStep &step, StrReadStep new_step, const char *&eol_chars)
{
    step = new_step;

    switch (new_step) {
        case StrReadStep::LABEL:
            eol_chars = "\n";
            break;
        case StrReadStep::SEARCH:
            eol_chars = "\n\"";
            break;
        case StrReadStep::TEXT:
            eol_chars = "\"";
            break;
    }
}

bool GameTextFile::Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language, Options options)
{
    GAMETEXTLOG_INFO("Reading text file '%s' in CSF format", file.Get_File_Name().Str());

    bool success = false;
    Utf16Array buf = {};

    if (Read_CSF_Header(file, string_infos, language)) {
        success = true;

        for (StringInfo &string_info : string_infos) {
            if (!Read_CSF_Entry(file, string_info, options, buf)) {
                success = false;
                break;
            }
        }
    }
    return success;
}

bool GameTextFile::Read_CSF_Header(FileRef &file, StringInfos &string_infos, LanguageID &language)
{
    CSFHeader header;

    if (rts::Read_Any(file.Get_File(), header)) {
        letoh_ref(header.id);
        letoh_ref(header.langid);
        letoh_ref(header.num_labels);
        letoh_ref(header.num_strings);
        letoh_ref(header.skip);
        letoh_ref(header.version);

        if (header.id == rts::FourCC_LE<'C', 'S', 'F', ' '>::value) {
            language = (header.version > 1) ? static_cast<LanguageID>(header.langid) : LanguageID::US;
            string_infos.resize(header.num_labels);
            return true;
        }
    }

    return false;
}

bool GameTextFile::Read_CSF_Entry(FileRef &file, StringInfo &string_info, Options options, Utf16Array &buf)
{
    int32_t texts;

    if (Read_CSF_Label(file, string_info, texts)) {
        if (texts == 0) {
            return true;
        }
        if (Read_CSF_Text(file, string_info, options, buf)) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts)
{
    CSFLabelHeader header;

    if (rts::Read_Any(file.Get_File(), header)) {
        letoh_ref(header.id);
        letoh_ref(header.texts);
        letoh_ref(header.length);

        if (header.id == rts::FourCC_LE<'L', 'B', 'L', ' '>::value) {
            const auto view = rts::Make_Resized_Array_View(string_info.label, header.length);

            if (rts::Read_Str(file.Get_File(), view)) {
                texts = header.texts;
                return true;
            }
        }
    }

    return false;
}

bool GameTextFile::Read_CSF_Text(FileRef &file, StringInfo &string_info, Options options, Utf16Array &buf)
{
    bool text_ok = false;
    bool speech_ok = false;
    bool read_speech = false;

    {
        CSFTextHeader header;

        if (rts::Read_Any(file.Get_File(), header)) {
            letoh_ref(header.id);
            letoh_ref(header.length);

            const size_t capped_text_len = std::min<size_t>(header.length, buf.size() - 1);

            read_speech = (header.id == rts::FourCC_LE<'S', 'T', 'R', 'W'>::value);
            const bool read_text = (header.id == rts::FourCC_LE<'S', 'T', 'R', ' '>::value);

            if (read_speech || read_text) {
                auto bufview = rts::Make_Array_View(buf.data(), capped_text_len);

                if (rts::Read_Str(file.Get_File(), bufview)) {
                    buf[capped_text_len] = U_CHAR('\0');

                    for (size_t i = 0; i < capped_text_len; ++i) {
                        letoh_ref(bufview[i]);
                        // Every char is binary flipped here by design.
                        bufview[i] = ~bufview[i];
                    }

                    if (!options.has(GameTextOption::KEEP_OBSOLETE_SPACES_ON_LOAD)) {
                        // Strip obsolete spaces for cleaner presentation in game.
                        rts::Strip_Obsolete_Spaces(bufview.data());
                    }

                    string_info.text = buf.data();
                    text_ok = true;
                }
            }
        }
    }

    if (read_speech) {
        CSFSpeechHeader header;

        if (rts::Read_Any(file.Get_File(), header)) {
            letoh_ref(header.length);
            const auto view = rts::Make_Resized_Array_View(string_info.speech, header.length);

            if (rts::Read_Str(file.Get_File(), view)) {
                speech_ok = true;
            }
        }
    }

    return text_ok && (speech_ok || !read_speech);
}

bool GameTextFile::Write_Multi_STR_File(
    FileRef &file, const ConstStringInfosPtrArray &string_infos_ptrs, Languages languages, Options options)
{
    GAMETEXTLOG_INFO("Writing text file '%s' in Multi STR format", file.Get_File_Name().Str());

    MultiStringInfos multi_string_infos;
    Build_Multi_String_Infos(multi_string_infos, string_infos_ptrs, options);

    Utf8Array buf = {};
    Utf8String str;
    str.Get_Buffer_For_Read(TEXT_8_SIZE);

    for (const MultiStringInfo &string_info : multi_string_infos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_Multi_STR_Entry(file, string_info, languages, options, buf, str)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_Multi_STR_Entry(
    FileRef &file, const MultiStringInfo &string_info, Languages languages, Options options, Utf8Array &buf, Utf8String &str)
{
    bool ok = true;

    ok &= Write_STR_Label(file, string_info.label);

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        ok &= Write_STR_Language(file, language);
        ok &= Write_STR_Text(file, string_info.text[index], options, buf, str);
    });

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        if (!string_info.speech[index].Is_Empty()) {
            ok &= Write_STR_Language(file, language);
            ok &= Write_STR_Speech(file, string_info.speech[index]);
        }
    });

    ok &= Write_STR_End(file);

    return ok;
}

bool GameTextFile::Write_STR_Language(FileRef &file, LanguageID language)
{
    const char *code = Get_Language_Code(language);

    bool ok = true;
    ok &= rts::Write_Str(file.Get_File(), rts::Make_Array_View(code));
    ok &= rts::Write_Any(file.Get_File(), s_str_lng);
    ok &= rts::Write_Any(file.Get_File(), ' ');
    return ok;
}

bool GameTextFile::Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options)
{
    GAMETEXTLOG_INFO("Writing text file '%s' in STR format", file.Get_File_Name().Str());

    Utf8Array buf = {};
    Utf8String str;
    str.Get_Buffer_For_Read(TEXT_8_SIZE);

    for (const StringInfo &string_info : string_infos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_STR_Entry(file, string_info, options, buf, str)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_STR_Entry(
    FileRef &file, const StringInfo &string_info, Options options, Utf8Array &buf, Utf8String &str)
{
    bool ok = true;
    ok &= Write_STR_Label(file, string_info.label);
    ok &= Write_STR_Text(file, string_info.text, options, buf, str);

    if (!string_info.speech.Is_Empty()) {
        ok &= Write_STR_Speech(file, string_info.speech);
    }
    ok &= Write_STR_End(file);
    return ok;
}

bool GameTextFile::Write_STR_Label(FileRef &file, const Utf8String &label)
{
    bool ok = true;
    ok &= rts::Write_Str(file.Get_File(), rts::Make_Array_View(label));
    ok &= rts::Write_Any(file.Get_File(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_Text(FileRef &file, const Utf16String &text, Options options, Utf8Array &buf, Utf8String &str)
{
    // Convert utf16 to utf8.
    str.Translate(text.Str());

    // STR does support escaped characters for special control characters. Write them out as escaped characters so they
    // are easily modifiable in text editor.
    const char *search[4] = { "\n", "\t", "\"", "\\" };
    const char *replace[4] = { "\\n", "\\t", "\\\"", "\\\\" };

    if (options.has(Options::Value::WRITE_EXTRA_LF_ON_STR_SAVE)) {
        replace[0] = "\\n\r\n";
    }
    size_t len = rts::Replace_Character_Sequences(buf.data(), buf.size(), str.Str(), search, replace);

    bool ok = true;
    ok &= rts::Write_Any(file.Get_File(), s_str_quo);
    ok &= rts::Write_Str(file.Get_File(), rts::Make_Array_View(buf.data(), len));
    ok &= rts::Write_Any(file.Get_File(), s_str_quo);
    ok &= rts::Write_Any(file.Get_File(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_Speech(FileRef &file, const Utf8String &speech)
{
    bool ok = true;
    ok &= rts::Write_Str(file.Get_File(), rts::Make_Array_View(speech));
    ok &= rts::Write_Any(file.Get_File(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_End(FileRef &file)
{
    bool ok = true;
    ok &= rts::Write_Any(file.Get_File(), s_str_end);
    ok &= rts::Write_Any(file.Get_File(), s_str_eol);
    ok &= rts::Write_Any(file.Get_File(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language)
{
    GAMETEXTLOG_INFO("Writing text file '%s' in CSF format", file.Get_File_Name().Str());

    bool success = false;

    if (Write_CSF_Header(file, string_infos, language)) {
        success = true;
        Utf16Array buf = {};
        int string_index = 0;

        for (const StringInfo &string_info : string_infos) {
            ++string_index;

            if (string_info.label.Is_Empty()) {
                GAMETEXTLOG_ERROR("String %d has no label", string_index);
                continue;
            }

            if (!Write_CSF_Entry(file, string_info, buf)) {
                success = false;
                break;
            }
        }
    }
    return success;
}

bool GameTextFile::Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language)
{
    CSFHeader header;
    header.id = rts::FourCC_LE<'C', 'S', 'F', ' '>::value;
    header.version = 3;
    header.num_labels = string_infos.size();
    header.num_strings = string_infos.size();
    header.skip = rts::FourCC_LE<'T', 'H', 'Y', 'M'>::value;
    header.langid = language;
    htole_ref(header.id);
    htole_ref(header.version);
    htole_ref(header.num_labels);
    htole_ref(header.num_strings);
    htole_ref(header.skip);
    htole_ref(header.langid);

    return rts::Write_Any(file.Get_File(), header);
}

bool GameTextFile::Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Array &buf)
{
    if (Write_CSF_Label(file, string_info)) {
        if (Write_CSF_Text(file, string_info, buf)) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Write_CSF_Label(FileRef &file, const StringInfo &string_info)
{
    CSFLabelHeader header;
    header.id = rts::FourCC_LE<'L', 'B', 'L', ' '>::value;
    header.texts = string_info.text.Is_Empty() ? 0 : 1;
    header.length = string_info.label.Get_Length();
    htole_ref(header.id);
    htole_ref(header.texts);
    htole_ref(header.length);

    if (rts::Write_Any(file.Get_File(), header)) {
        if (rts::Write_Str(file.Get_File(), rts::Make_Array_View(string_info.label))) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Array &buf)
{
    bool text_ok = false;
    bool speech_ok = false;
    const size_t text_len = string_info.text.Get_Length();
    const size_t speech_len = string_info.speech.Get_Length();
    const bool write_text = (text_len > 0);
    const bool write_speech = (speech_len > 0);

    if (write_text) {
        const size_t capped_text_len = std::min<size_t>(text_len, buf.size() - 1);

        CSFTextHeader header;
        header.id = write_speech ? rts::FourCC_LE<'S', 'T', 'R', 'W'>::value : rts::FourCC_LE<'S', 'T', 'R', ' '>::value;
        header.length = capped_text_len;
        htole_ref(header.id);
        htole_ref(header.length);

        if (rts::Write_Any(file.Get_File(), header)) {

            for (size_t i = 0; i < capped_text_len; ++i) {
                // Every char is binary flipped here by design.
                buf[i] = ~string_info.text[i];
                htole_ref(buf[i]);
            }

            if (rts::Write(file.Get_File(), buf.data(), capped_text_len * sizeof(Utf16View::value_type))) {
                text_ok = true;
            }
        }
    }

    if (text_ok && write_speech) {
        CSFSpeechHeader header;
        header.length = string_info.speech.Get_Length();
        htole_ref(header.length);

        if (rts::Write_Any(file.Get_File(), header)) {
            if (rts::Write_Str(file.Get_File(), rts::Make_Array_View(string_info.speech))) {
                speech_ok = true;
            }
        }
    }
    return (text_ok || !write_text) && (speech_ok || !write_speech);
}

void GameTextFile::Log_Line(const char *prefix, const char *format, ...)
{
    FILE *file = s_logfile;

    if (file != nullptr) {
        va_list args;
        va_start(args, format);
        fputs(prefix, file);
        vfprintf(file, format, args);
        fputs("\n", file);
        va_end(args);
        fflush(file);
    }
}

} // namespace Thyme

#undef GAMETEXTLOG_TRACE
#undef GAMETEXTLOG_DEBUG
#undef GAMETEXTLOG_INFO
#undef GAMETEXTLOG_WARN
#undef GAMETEXTLOG_ERROR
#undef GAMETEXTLOG_FATAL
