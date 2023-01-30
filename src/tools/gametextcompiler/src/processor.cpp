/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler Processor. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "processor.h"
#include "commands.h"
#include <algorithm>
#include <utility/stringutil.h>

namespace Thyme
{
namespace
{
constexpr const char *const s_result_names[] = {
    "SUCCESS",
    "INVALID_COMMAND_ACTION",
    "INVALID_COMMAND_ARGUMENT",
    "INVALID_LANGUAGE_VALUE",
    "INVALID_OPTION_VALUE",
    "INVALID_FILE_ID_ARGUMENT",
    "MISSING_FILE_PATH_ARGUMENT",
    "MISSING_LANGUAGE_ARGUMENT",
    "EXECUTION_ERROR",
};

static_assert(ARRAY_SIZE(s_result_names) == size_t(Processor::ResultId::COUNT));
} // namespace

const char *Processor::Get_Result_Name(ResultId id)
{
    return s_result_names[size_t(id)];
}

const Processor::FileId Processor::s_defaultFileId = { 0 };

Processor::Processor() : m_fileMap(), m_commands() {}

Processor::Result Processor::Parse_Commands(const CommandTexts &command_texts)
{
    if (Has_Simple_Command(command_texts)) {
        return Parse_Simple_Commands(command_texts);
    } else {
        return Parse_Function_Commands(command_texts);
    }
}

Processor::Result Processor::Execute_Commands() const
{
    Result result(ResultId::SUCCESS);

    for (const CommandPtr &command : m_commands) {
        if (!command->Execute()) {
            result.id = ResultId::EXECUTION_ERROR;
            result.error_command_index = command->Id();
            break;
        }
    }
    return result;
}

Processor::Result Processor::Parse_Function_Commands(const CommandTexts &command_texts)
{
    Result result(ResultId::SUCCESS);
    CommandPtrs commands;

    for (size_t index = 0; index < command_texts.size(); ++index) {

        CommandAction action;
        result = Parse_Function_Command(action, command_texts[index], index);

        if (result.id != ResultId::SUCCESS) {
            result.error_command_index = index;
            break;
        }

        result = Add_New_Command(commands, m_fileMap, action);

        if (result.id != ResultId::SUCCESS) {
            result.error_command_index = index;
            break;
        }
    }

    if (result.id == ResultId::SUCCESS) {
        m_commands.swap(commands);
    }

    return result;
}

Processor::Result Processor::Parse_Simple_Commands(const CommandTexts &command_texts)
{
    Result result(ResultId::SUCCESS);
    CommandActionSequence actions;
    CommandPtrs commands;

    if (!command_texts.empty()) {

        const size_t size = command_texts.size();

        for (size_t index = 0; index < size - 1; ++index) {

            if (!Is_Simple_Command(command_texts[index])) {
                continue;
            }

            const char *command_name = command_texts[index] + 1;
            const char *command_value = command_texts[index + 1];

            result = Parse_Simple_Command(actions, command_name, command_value, index);

            if (result.id != ResultId::SUCCESS) {
                result.error_command_index = index;
                break;
            }

            ++index;
        }
    }

    if (result.id == ResultId::SUCCESS) {

        for (const CommandAction &action : actions) {
            if (action.action_id != CommandActionId::INVALID) {
                result = Add_New_Command(commands, m_fileMap, action);

                if (result.id != ResultId::SUCCESS) {
                    result.error_command_index = action.command_index;
                    break;
                }
            }
        }
    }

    if (result.id == ResultId::SUCCESS) {
        m_commands.swap(commands);
    }

    return result;
}

bool Processor::Has_Simple_Command(const CommandTexts &command_texts)
{
    for (const char *command_text : command_texts) {
        if (Is_Simple_Command(command_text)) {
            return true;
        }
    }
    return false;
}

bool Processor::Is_Simple_Command(const char *command_text)
{
    return command_text != nullptr && *command_text == '-';
}

template<size_t Size> bool Processor::Parse_Next_Word(std::string &word, const char *&str, const char (&separators)[Size])
{
    const char *reader = str;

    do {
        for (const char separator : separators) {
            if (*reader == separator && reader > str) {
                word.assign(str, reader);
                str = reader;
                if (*str != '\0') {
                    str += 1;
                }
                return true;
            }
        }
    } while (*reader++ != '\0');

    return false;
}

Processor::Result Processor::Parse_Function_Command(CommandAction &action, const char *command_text, size_t command_index)
{
    Result result(ResultId::SUCCESS);
    CommandActionId action_id = CommandActionId::INVALID;
    CommandArgumentId argument_id = CommandArgumentId::INVALID;
    CommandArguments arguments;
    const char *reader = command_text;
    std::string word;

    while (true) {
        const char *word_begin = reader;

        if (reader == command_text && Parse_Next_Word(word, reader, { '(' })) {
            if (!String_To_Command_Action_Id(word.c_str(), action_id)) {
                result.id = ResultId::INVALID_COMMAND_ACTION;
                result.error_text = ErrorText(word_begin, reader);
                break;
            }

        } else if (argument_id == CommandArgumentId::INVALID && Parse_Next_Word(word, reader, { ':' })) {
            if (!String_To_Command_Argument_Id(word.c_str(), argument_id)) {
                result.id = ResultId::INVALID_COMMAND_ARGUMENT;
                result.error_text = ErrorText(word_begin, reader);
                break;
            }
            arguments.emplace_back();

        } else if (argument_id != CommandArgumentId::INVALID && Parse_Next_Word(word, reader, { ',', '|', ')' })) {
            result = Parse_Command_Argument(arguments.back(), word, argument_id);
            if (result.id != ResultId::SUCCESS) {
                result.error_text = ErrorText(word_begin, reader);
                break;
            }
            if (*(reader - 1) == ',') {
                argument_id = CommandArgumentId::INVALID;
            }

        } else {
            break;
        }
    }

    if (action_id == CommandActionId::INVALID) {
        result.id = ResultId::INVALID_COMMAND_ACTION;
    }

    if (result.id == ResultId::SUCCESS) {
        action.action_id = action_id;
        action.arguments.swap(arguments);
        action.command_index = command_index;
    }

    return result;
}

Processor::Result Processor::Parse_Simple_Command(
    CommandActionSequence &actions, const char *command_name, const char *command_value, size_t command_index)
{
    Result result(ResultId::SUCCESS);
    SimpleActionId simple_action_id;

    if (!String_To_Simple_Action_Id(command_name, simple_action_id)) {
        result.id = ResultId::INVALID_COMMAND_ACTION;
        result.error_text = ErrorText(command_name, command_name + strlen(command_name));
        return result;
    }

    bool overwrite_action_id = false;
    SequenceId sequence_id = SequenceId::INVALID;
    CommandActionId action_id = CommandActionId::INVALID;
    CommandArgumentId argument_id = CommandArgumentId::INVALID;

    switch (simple_action_id) {
        case SimpleActionId::OPTIONS:
            sequence_id = SequenceId::SET_OPTIONS;
            action_id = CommandActionId::SET_OPTIONS;
            argument_id = CommandArgumentId::OPTIONS;
            break;
        case SimpleActionId::LOAD_CSF_FILE:
            sequence_id = SequenceId::LOAD;
            action_id = CommandActionId::LOAD_CSF;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::LOAD_STR_FILE:
            sequence_id = SequenceId::LOAD;
            action_id = CommandActionId::LOAD_STR;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::LOAD_STR_LANGUAGES:
            sequence_id = SequenceId::LOAD;
            action_id = CommandActionId::LOAD_MULTI_STR;
            argument_id = CommandArgumentId::LANGUAGES;
            overwrite_action_id = true;
            break;
        case SimpleActionId::SWAP_AND_SET_LANGUAGE:
            sequence_id = SequenceId::SWAP_AND_SET_LANGUAGE;
            action_id = CommandActionId::SWAP_AND_SET_LANGUAGE;
            argument_id = CommandArgumentId::LANGUAGES;
            break;
        case SimpleActionId::SAVE_CSF:
            sequence_id = SequenceId::SAVE;
            action_id = CommandActionId::SAVE_CSF;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::SAVE_STR:
            sequence_id = SequenceId::SAVE;
            action_id = CommandActionId::SAVE_STR;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::SAVE_STR_LANGUAGES:
            sequence_id = SequenceId::SAVE;
            action_id = CommandActionId::SAVE_MULTI_STR;
            argument_id = CommandArgumentId::LANGUAGES;
            overwrite_action_id = true;
            break;
    }

    static_assert(g_simpleActionCount == 8, "SimpleAction is missing");

    captainslog_assert(sequence_id != SequenceId::INVALID);

    if (sequence_id != SequenceId::INVALID) {
        CommandArgument argument;

        {
            const char *reader = command_value;
            const char *word_begin = reader;
            std::string word;

            while (Parse_Next_Word(word, reader, { '|', '\0' })) {
                result = Parse_Command_Argument(argument, word, argument_id);
                if (result.id != ResultId::SUCCESS) {
                    result.error_text = ErrorText(word_begin, reader);
                    break;
                }
                word_begin = reader;
            }
        }

        if (result.id == ResultId::SUCCESS) {
            CommandAction &action = actions[size_t(sequence_id)];

            if (action.action_id == CommandActionId::INVALID || overwrite_action_id) {
                action.action_id = action_id;
                action.command_index = command_index;
            }
            action.arguments.emplace_back(std::move(argument));
        }
    }

    return result;
}

Processor::Result Processor::Parse_Command_Argument(
    CommandArgument &argument, std::string &str, CommandArgumentId argument_id)
{
    Result result(ResultId::SUCCESS);

    switch (argument_id) {
        case CommandArgumentId::FILE_ID: {
            FileId file_id;
            file_id.value = std::stoi(str);
            argument.value.emplace<FileId>(std::move(file_id));
            break;
        }
        case CommandArgumentId::FILE_PATH: {
            FilePath file_path;
            file_path.value = str.c_str();
            argument.value.emplace<FilePath>(std::move(file_path));
            break;
        }
        case CommandArgumentId::LANGUAGES: {
            LanguageID language;

            if (strcasecmp(str.c_str(), "All") == 0) {
                argument.value.emplace<Languages>(~Languages());

            } else if (Name_To_Language(str.c_str(), language)) {
                Languages languages;
                const Languages *languages_ptr = std::get_if<Languages>(&argument.value);
                if (languages_ptr != nullptr) {
                    languages = *languages_ptr;
                }
                languages |= language;
                argument.value.emplace<Languages>(std::move(languages));

            } else {
                result.id = ResultId::INVALID_LANGUAGE_VALUE;
            }
            break;
        }
        case CommandArgumentId::OPTIONS: {
            GameTextOption option;

            if (Name_To_Game_Text_Option(str.c_str(), option)) {
                GameTextOptions options;
                const GameTextOptions *options_ptr = std::get_if<GameTextOptions>(&argument.value);
                if (options_ptr != nullptr) {
                    options = *options_ptr;
                }
                options |= option;
                argument.value.emplace<GameTextOptions>(std::move(options));

            } else {
                result.id = ResultId::INVALID_OPTION_VALUE;
            }
            break;
        }
    }

    static_assert(g_commandArgumentCount == 4, "CommandArgument is missing");

    return result;
}

Processor::Result Processor::Add_New_Command(CommandPtrs &commands, FileMap &file_map, const CommandAction &action)
{
    Result result(ResultId::SUCCESS);

    Populate_File_Map(file_map, action);

    switch (action.action_id) {
        case CommandActionId::LOAD_CSF:
            result = Add_Load_CSF_Command(commands, file_map, action);
            break;
        case CommandActionId::LOAD_STR:
            result = Add_Load_STR_Command(commands, file_map, action);
            break;
        case CommandActionId::LOAD_MULTI_STR:
            result = Add_Load_Multi_STR_Command(commands, file_map, action);
            break;
        case CommandActionId::SAVE_CSF:
            result = Add_Save_CSF_Command(commands, file_map, action);
            break;
        case CommandActionId::SAVE_STR:
            result = Add_Save_STR_Command(commands, file_map, action);
            break;
        case CommandActionId::SAVE_MULTI_STR:
            result = Add_Save_Multi_STR_Command(commands, file_map, action);
            break;
        case CommandActionId::UNLOAD:
            result = Add_Unload_Command(commands, file_map, action);
            break;
        case CommandActionId::RESET:
            result = Add_Reset_Command(commands, file_map, action);
            break;
        case CommandActionId::MERGE_AND_OVERWRITE:
            result = Add_Merge_Command(commands, file_map, action);
            break;
        case CommandActionId::SET_OPTIONS:
            result = Add_Set_Options_Command(commands, file_map, action);
            break;
        case CommandActionId::SET_LANGUAGE:
            result = Add_Set_Language_Command(commands, file_map, action);
            break;
        case CommandActionId::SWAP_LANGUAGE_STRINGS:
            result = Add_Swap_Language_Command(commands, file_map, action);
            break;
        case CommandActionId::SWAP_AND_SET_LANGUAGE:
            result = Add_Swap_Set_Language_Command(commands, file_map, action);
            break;
    }

    static_assert(g_commandActionCount == 13, "CommandAction is missing");

    if (result.id == ResultId::SUCCESS) {
        commands.back()->Set_Id(action.command_index);
    }

    return result;
}

Processor::Result Processor::Add_Load_CSF_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result(ResultId::MISSING_FILE_PATH_ARGUMENT);
    }

    commands.emplace_back(new LoadCsfCommand(file_ptr, file_path));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Load_STR_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result(ResultId::MISSING_FILE_PATH_ARGUMENT);
    }

    commands.emplace_back(new LoadStrCommand(file_ptr, file_path));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Load_Multi_STR_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);
    const auto languages = Get_Languages(action.arguments);

    if (file_path == nullptr) {
        return Result(ResultId::MISSING_FILE_PATH_ARGUMENT);
    }

    if (languages.none()) {
        return Result(ResultId::MISSING_LANGUAGE_ARGUMENT);
    }

    commands.emplace_back(new LoadMultiStrCommand(file_ptr, file_path, languages));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Save_CSF_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result(ResultId::MISSING_FILE_PATH_ARGUMENT);
    }

    commands.emplace_back(new SaveCsfCommand(file_ptr, file_path));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Save_STR_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result(ResultId::MISSING_FILE_PATH_ARGUMENT);
    }

    commands.emplace_back(new SaveStrCommand(file_ptr, file_path));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Save_Multi_STR_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);
    const auto languages = Get_Languages(action.arguments);

    if (file_path == nullptr) {
        return Result(ResultId::MISSING_FILE_PATH_ARGUMENT);
    }

    if (languages.none()) {
        return Result(ResultId::MISSING_LANGUAGE_ARGUMENT);
    }

    commands.emplace_back(new SaveMultiStrCommand(file_ptr, file_path, languages));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Unload_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto languages = Get_Languages(action.arguments);

    commands.emplace_back(new UnloadCommand(file_ptr, languages));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Reset_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);

    commands.emplace_back(new ResetCommand(file_ptr));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Merge_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr_a = Get_File_Ptr(action.arguments, file_map, 0);
    const auto file_ptr_b = Get_File_Ptr(action.arguments, file_map, 1);
    const auto languages = Get_Languages(action.arguments);

    if (file_ptr_a == file_ptr_b) {
        return Result(ResultId::INVALID_FILE_ID_ARGUMENT);
    }

    commands.emplace_back(new MergeAndOverwriteCommand(file_ptr_a, file_ptr_b, languages));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Set_Options_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto options = Get_Options(action.arguments);

    commands.emplace_back(new SetOptionsCommand(file_ptr, options));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Set_Language_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto language = Get_Language(action.arguments);

    if (language == LanguageID::UNKNOWN) {
        return Result(ResultId::MISSING_LANGUAGE_ARGUMENT);
    }

    commands.emplace_back(new SetLanguageCommand(file_ptr, language));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Swap_Language_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto language_a = Get_Language(action.arguments, 0);
    const auto language_b = Get_Language(action.arguments, 1);

    if (language_a == LanguageID::UNKNOWN) {
        return Result(ResultId::MISSING_LANGUAGE_ARGUMENT);
    }
    if (language_b == LanguageID::UNKNOWN) {
        return Result(ResultId::MISSING_LANGUAGE_ARGUMENT);
    }

    commands.emplace_back(new SwapLanguageStringsCommand(file_ptr, language_a, language_b));
    return Result(ResultId::SUCCESS);
}

Processor::Result Processor::Add_Swap_Set_Language_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto language = Get_Language(action.arguments, 0);

    if (language == LanguageID::UNKNOWN) {
        return Result(ResultId::MISSING_LANGUAGE_ARGUMENT);
    }

    commands.emplace_back(new SwapAndSetLanguageCommand(file_ptr, language));
    return Result(ResultId::SUCCESS);
}

void Processor::Populate_File_Map(FileMap &file_map, const CommandAction &action)
{
    const FileId *file_id_ptr = Find_Ptr<FileId>(action.arguments);
    const FileId file_id = (file_id_ptr == nullptr) ? s_defaultFileId : *file_id_ptr;
    Populate_File_Map(file_map, file_id);
}

void Processor::Populate_File_Map(FileMap &file_map, FileId file_id)
{
    const FileMap::iterator it = file_map.find(file_id);
    if (it == file_map.end()) {
        auto *file = new GameTextFile();
        file->Set_Options(GameTextOptions::Value::NONE);
        file_map.emplace(file_id, file);
    }
}

GameTextFilePtr Processor::Get_File_Ptr(const FileMap &file_map, FileId file_id)
{
    const FileMap::const_iterator it = file_map.find(file_id);
    captainslog_assert(it != file_map.end());
    return it->second;
}

template<class Type> const Type *Processor::Find_Ptr(const CommandArguments &arguments, size_t occurence)
{
    size_t num = 0;
    for (const CommandArgument &argument : arguments) {
        const Type *type_ptr = std::get_if<Type>(&argument.value);
        if (type_ptr != nullptr) {
            if (occurence == num++) {
                return type_ptr;
            }
        }
    }
    return nullptr;
}

GameTextFilePtr Processor::Get_File_Ptr(const CommandArguments &arguments, const FileMap &file_map, size_t occurence)
{
    const FileId *ptr = Find_Ptr<FileId>(arguments, occurence);
    const FileId file_id = (ptr == nullptr) ? s_defaultFileId : *ptr;
    return Get_File_Ptr(file_map, file_id);
}

const char *Processor::Get_File_Path(const CommandArguments &arguments, size_t occurence)
{
    const FilePath *ptr = Find_Ptr<FilePath>(arguments, occurence);
    return (ptr == nullptr) ? nullptr : ptr->value.c_str();
}

Languages Processor::Get_Languages(const CommandArguments &arguments, size_t occurence)
{
    const Languages *ptr = Find_Ptr<Languages>(arguments, occurence);
    return (ptr == nullptr) ? Languages() : *ptr;
}

LanguageID Processor::Get_Language(const CommandArguments &arguments, size_t occurence)
{
    LanguageID language = LanguageID::UNKNOWN;
    const Languages *ptr = Find_Ptr<Languages>(arguments, occurence);
    if (ptr != nullptr) {
        ptr->get(language, 0);
    }
    return language;
}

GameTextOptions Processor::Get_Options(const CommandArguments &arguments, size_t occurence)
{
    const GameTextOptions *ptr = Find_Ptr<GameTextOptions>(arguments, occurence);
    return (ptr == nullptr) ? GameTextOptions() : *ptr;
}

} // namespace Thyme
