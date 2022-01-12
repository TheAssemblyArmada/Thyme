/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler Commands. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "commands.h"

namespace Thyme
{
namespace
{
const char *const s_command_action_names[] = {
    "LOAD_CSF",
    "LOAD_STR",
    "LOAD_MULTI_STR",
    "SAVE_CSF",
    "SAVE_STR",
    "SAVE_MULTI_STR",
    "UNLOAD",
    "RESET",
    "MERGE_AND_OVERWRITE",
    "SET_OPTIONS",
    "SET_LANGUAGE",
    "SWAP_LANGUAGE_STRINGS",
    "SWAP_AND_SET_LANGUAGE",
};

const char *const s_command_argument_names[] = {
    "FILE_ID",
    "FILE_PATH",
    "LANGUAGE",
    "OPTION",
};

const char *const s_simple_action_names[] = {
    "OPTIONS",
    "LOAD_CSF",
    "LOAD_STR",
    "LOAD_STR_LANGUAGES",
    "SWAP_AND_SET_LANGUAGE",
    "SAVE_CSF",
    "SAVE_STR",
    "SAVE_STR_LANGUAGES",
};

static_assert(ARRAY_SIZE(s_command_action_names) == g_commandActionCount);
static_assert(ARRAY_SIZE(s_command_argument_names) == g_commandArgumentCount);
static_assert(ARRAY_SIZE(s_simple_action_names) == g_simpleActionCount);

template<typename EnumType, size_t Size>
bool String_To_Enum_Id(const char *str, EnumType &id, const char *const (&search_names)[Size])
{
    for (size_t index = 0; index < ARRAY_SIZE(search_names); ++index) {
        if (strcasecmp(str, search_names[index]) == 0) {
            id = static_cast<EnumType>(index);
            return true;
        }
    }
    return false;
}
} // namespace

bool String_To_Command_Action_Id(const char *str, CommandActionId &action_id)
{
    return String_To_Enum_Id(str, action_id, s_command_action_names);
}

bool String_To_Command_Argument_Id(const char *str, CommandArgumentId &argument_id)
{
    return String_To_Enum_Id(str, argument_id, s_command_argument_names);
}

bool String_To_Simple_Action_Id(const char *str, SimpleActionId &action_id)
{
    return String_To_Enum_Id(str, action_id, s_simple_action_names);
}

CommandId Command::s_id = 1000000000;

bool LoadCsfCommand::Execute() const
{
    return m_filePtr->Load_CSF(m_filePath.c_str());
}

bool LoadStrCommand::Execute() const
{
    return m_filePtr->Load_STR(m_filePath.c_str());
}

bool LoadMultiStrCommand::Execute() const
{
    return m_filePtr->Load_Multi_STR(m_filePath.c_str(), m_languages);
}

bool SaveCsfCommand::Execute() const
{
    return m_filePtr->Save_CSF(m_filePath.c_str());
}

bool SaveStrCommand::Execute() const
{
    return m_filePtr->Save_STR(m_filePath.c_str());
}

bool SaveMultiStrCommand::Execute() const
{
    return m_filePtr->Save_Multi_STR(m_filePath.c_str(), m_languages);
}

bool UnloadCommand::Execute() const
{
    if (m_languages.any()) {
        m_filePtr->Unload(m_languages);
    } else {
        m_filePtr->Unload();
    }
    return true;
}

bool ResetCommand::Execute() const
{
    m_filePtr->Reset();
    return true;
}

bool MergeAndOverwriteCommand::Execute() const
{
    if (m_languages.any()) {
        m_filePtrA->Merge_And_Overwrite(*m_filePtrB, m_languages);
    } else {
        m_filePtrA->Merge_And_Overwrite(*m_filePtrB);
    }
    return true;
}

bool SetOptionsCommand::Execute() const
{
    m_filePtr->Set_Options(m_options);
    return true;
}

bool SetLanguageCommand::Execute() const
{
    m_filePtr->Set_Language(m_language);
    return true;
}

bool SwapLanguageStringsCommand::Execute() const
{
    m_filePtr->Swap_String_Infos(m_languageA, m_languageB);
    return true;
}

bool SwapAndSetLanguageCommand::Execute() const
{
    const LanguageID current_language = m_filePtr->Get_Language();
    m_filePtr->Swap_String_Infos(current_language, m_language);
    m_filePtr->Set_Language(m_language);
    return true;
}

} // namespace Thyme
