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
#pragma once

#include <gametextfile.h>
#include <memory>
#include <string>

namespace Thyme
{
using CommandId = size_t;
using GameTextOption = GameTextOption;
using GameTextOptions = GameTextFile::Options;
using GameTextFile = GameTextFile;
using GameTextFilePtr = std::shared_ptr<GameTextFile>;
using Languages = GameTextFile::Languages;

enum class CommandActionId
{
    INVALID = -1,
    LOAD_CSF,
    LOAD_STR,
    LOAD_MULTI_STR,
    SAVE_CSF,
    SAVE_STR,
    SAVE_MULTI_STR,
    UNLOAD,
    RESET,
    MERGE_AND_OVERWRITE,
    SET_OPTIONS,
    SET_LANGUAGE,
    SWAP_LANGUAGE_STRINGS,
    SWAP_AND_SET_LANGUAGE,

    COUNT
};

enum class CommandArgumentId
{
    INVALID = -1,
    FILE_ID,
    FILE_PATH,
    LANGUAGES,
    OPTIONS,

    COUNT
};

enum class SimpleActionId
{
    OPTIONS,
    LOAD_CSF_FILE,
    LOAD_STR_FILE,
    LOAD_STR_LANGUAGES,
    SWAP_AND_SET_LANGUAGE,
    SAVE_CSF,
    SAVE_STR,
    SAVE_STR_LANGUAGES,

    COUNT
};

constexpr size_t g_commandActionCount = size_t(CommandActionId::COUNT);
constexpr size_t g_commandArgumentCount = size_t(CommandArgumentId::COUNT);
constexpr size_t g_simpleActionCount = size_t(SimpleActionId::COUNT);

bool String_To_Command_Action_Id(const char *str, CommandActionId &action_id);
bool String_To_Command_Argument_Id(const char *str, CommandArgumentId &argument_id);
bool String_To_Simple_Action_Id(const char *str, SimpleActionId &action_id);

class Command
{
public:
    Command() : m_id(s_id++) {}
    virtual ~Command() {}

    CommandId Id() const { return m_id; }
    void Set_Id(CommandId id) { m_id = id; }

    virtual CommandActionId Type() const = 0;
    virtual bool Execute() const = 0;

private:
    CommandId m_id;
    static CommandId s_id;
};

class LoadCsfCommand : public Command
{
public:
    LoadCsfCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD_CSF; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class LoadStrCommand : public Command
{
public:
    LoadStrCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}
    virtual CommandActionId Type() const override { return CommandActionId::LOAD_STR; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class LoadMultiStrCommand : public Command
{
public:
    LoadMultiStrCommand(const GameTextFilePtr &file_ptr, const char *path, Languages languages) :
        m_filePtr(file_ptr), m_filePath(path), m_languages(languages)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::LOAD_MULTI_STR; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
    Languages m_languages;
};

class SaveCsfCommand : public Command
{
public:
    SaveCsfCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::SAVE_CSF; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveStrCommand : public Command
{
public:
    SaveStrCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}
    virtual CommandActionId Type() const override { return CommandActionId::SAVE_STR; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveMultiStrCommand : public Command
{
public:
    SaveMultiStrCommand(const GameTextFilePtr &file_ptr, const char *path, Languages languages) :
        m_filePtr(file_ptr), m_filePath(path), m_languages(languages)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::SAVE_MULTI_STR; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
    Languages m_languages;
};

class UnloadCommand : public Command
{
public:
    UnloadCommand(const GameTextFilePtr &file_ptr, Languages languages) : m_filePtr(file_ptr), m_languages(languages) {}

    virtual CommandActionId Type() const override { return CommandActionId::UNLOAD; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    Languages m_languages;
};

class ResetCommand : public Command
{
public:
    ResetCommand(const GameTextFilePtr &file_ptr) : m_filePtr(file_ptr) {}

    virtual CommandActionId Type() const override { return CommandActionId::RESET; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
};

class MergeAndOverwriteCommand : public Command
{
public:
    MergeAndOverwriteCommand(const GameTextFilePtr &file_ptr_a, const GameTextFilePtr &file_ptr_b, Languages languages) :
        m_filePtrA(file_ptr_a), m_filePtrB(file_ptr_b), m_languages(languages)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::MERGE_AND_OVERWRITE; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtrA;
    GameTextFilePtr m_filePtrB;
    Languages m_languages;
};

class SetOptionsCommand : public Command
{
public:
    SetOptionsCommand(const GameTextFilePtr &file_ptr, GameTextOptions options) : m_filePtr(file_ptr), m_options(options) {}

    virtual CommandActionId Type() const override { return CommandActionId::SET_OPTIONS; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    GameTextOptions m_options;
};

class SetLanguageCommand : public Command
{
public:
    SetLanguageCommand(const GameTextFilePtr &file_ptr, LanguageID language) : m_filePtr(file_ptr), m_language(language) {}

    virtual CommandActionId Type() const override { return CommandActionId::SET_LANGUAGE; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    LanguageID m_language;
};

class SwapLanguageStringsCommand : public Command
{
public:
    SwapLanguageStringsCommand(const GameTextFilePtr &file_ptr, LanguageID language_a, LanguageID language_b) :
        m_filePtr(file_ptr), m_languageA(language_a), m_languageB(language_b)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::SWAP_LANGUAGE_STRINGS; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    LanguageID m_languageA;
    LanguageID m_languageB;
};

class SwapAndSetLanguageCommand : public Command
{
public:
    SwapAndSetLanguageCommand(const GameTextFilePtr &file_ptr, LanguageID language) :
        m_filePtr(file_ptr), m_language(language)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::SWAP_AND_SET_LANGUAGE; }
    virtual bool Execute() const override;

private:
    GameTextFilePtr m_filePtr;
    LanguageID m_language;
};

} // namespace Thyme
