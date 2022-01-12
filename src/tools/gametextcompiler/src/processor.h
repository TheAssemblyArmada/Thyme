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
#pragma once

#include "commands.h"
#include <array>
#include <unordered_map>
#include <utility/arrayview.h>
#include <variant>
#include <vector>

namespace Thyme
{
class Processor
{
public:
    enum class ResultId
    {
        SUCCESS,
        INVALID_COMMAND_ACTION,
        INVALID_COMMAND_ARGUMENT,
        INVALID_LANGUAGE_VALUE,
        INVALID_OPTION_VALUE,
        INVALID_FILE_ID_ARGUMENT,
        MISSING_FILE_PATH_ARGUMENT,
        MISSING_LANGUAGE_ARGUMENT,
        EXECUTION_ERROR,

        // #TODO Add more detailed execution errors once GameTextFile supports it

        COUNT
    };

    static const char *Get_Result_Name(ResultId id);

    using ErrorText = rts::array_view<const char>;
    using CommandTexts = rts::array_view<const char *>;

    struct Result
    {
        explicit Result(ResultId id) : id(id), error_command_index(0), error_text() {}

        ResultId id;
        size_t error_command_index;
        ErrorText error_text;
    };

public:
    Processor();

    Result Parse_Commands(const CommandTexts &command_texts);
    Result Execute_Commands() const;

private:
    using CommandPtr = std::shared_ptr<Command>;
    using CommandPtrs = std::vector<CommandPtr>;

    struct FileId
    {
        bool operator==(const FileId &other) const { return value == other.value; }
        int value;
    };

    struct FileIdHash
    {
        std::size_t operator()(const FileId &key) const { return std::hash<int>{}(key.value); }
    };

    using FileMap = std::unordered_map<FileId, GameTextFilePtr, FileIdHash>;

    struct FilePath
    {
        std::string value;
    };

    struct CommandArgument
    {
        std::variant<FileId, FilePath, Languages, GameTextOptions> value;
    };

    using CommandArguments = std::vector<CommandArgument>;

    struct CommandAction
    {
        CommandActionId action_id = CommandActionId::INVALID;
        CommandArguments arguments;
        size_t command_index = 0;
    };

    enum class SequenceId
    {
        INVALID = -1,
        SET_OPTIONS,
        LOAD,
        SWAP_AND_SET_LANGUAGE,
        SAVE,

        COUNT
    };

    using CommandActionSequence = std::array<CommandAction, size_t(SequenceId::COUNT)>;

private:
    Result Parse_Function_Commands(const CommandTexts &command_texts);
    Result Parse_Simple_Commands(const CommandTexts &command_texts);

    static bool Has_Simple_Command(const CommandTexts &command_texts);
    static bool Is_Simple_Command(const char *command_text);

    template<size_t Size> bool static Parse_Next_Word(std::string &word, const char *&str, const char (&separators)[Size]);

    static Result Parse_Function_Command(CommandAction &action, const char *command_text, size_t command_index);
    static Result Parse_Simple_Command(
        CommandActionSequence &actions, const char *command_name, const char *command_value, size_t command_index);
    static Result Parse_Command_Argument(CommandArgument &argument, std::string &str, CommandArgumentId argument_id);

    static Result Add_New_Command(CommandPtrs &commands, FileMap &file_map, const CommandAction &action);
    static Result Add_Load_CSF_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Load_STR_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Load_Multi_STR_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Save_CSF_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Save_STR_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Save_Multi_STR_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Unload_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Reset_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Merge_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Set_Options_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Set_Language_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Swap_Language_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);
    static Result Add_Swap_Set_Language_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action);

    static void Populate_File_Map(FileMap &file_map, const CommandAction &action);
    static void Populate_File_Map(FileMap &file_map, FileId file_id);
    static GameTextFilePtr Get_File_Ptr(const FileMap &file_map, FileId file_id);

    template<class Type> static const Type *Find_Ptr(const CommandArguments &arguments, size_t occurence = 0);
    static GameTextFilePtr Get_File_Ptr(const CommandArguments &arguments, const FileMap &file_map, size_t occurence = 0);
    static const char *Get_File_Path(const CommandArguments &arguments, size_t occurence = 0);
    static Languages Get_Languages(const CommandArguments &arguments, size_t occurence = 0);
    static LanguageID Get_Language(const CommandArguments &arguments, size_t occurence = 0);
    static GameTextOptions Get_Options(const CommandArguments &arguments, size_t occurence = 0);

private:
    FileMap m_fileMap;
    CommandPtrs m_commands;

    static const FileId s_defaultFileId;
};

} // namespace Thyme
