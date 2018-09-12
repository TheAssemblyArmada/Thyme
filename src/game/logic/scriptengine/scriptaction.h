/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing a script action.
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
#include "mempoolobj.h"
#include "scriptparam.h"

class ScriptAction : public MemoryPoolObject
{
    IMPLEMENT_POOL(ScriptAction);

    enum
    {
        MAX_ACTION_PARAMETERS = 12,
    };

public:
    enum ScriptActionType
    {
        DEFAULT = 5,
    };

public:
    ScriptAction();
    ScriptAction(ScriptActionType type);
    virtual ~ScriptAction();

    ScriptAction *Duplicate();
    ScriptAction *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    Utf8String Get_UI_Text();

    static bool Parse_Action_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static bool Parse_False_Action_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    void Set_Action_Type(ScriptActionType type);
    static ScriptAction *Parse_Action(DataChunkInput &input, DataChunkInfo *info);

private:
    ScriptActionType m_actionType;
    int m_numParams;
    Parameter *m_params[MAX_ACTION_PARAMETERS];
    ScriptAction *m_nextAction;
    bool m_hasWarnings;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void ScriptAction::Hook_Me()
{
    Hook_Method(0x0051FF80, &Duplicate);
    Hook_Method(0x00520240, &Duplicate_And_Qualify);
    Hook_Function(0x00521240, &Parse_Action_Chunk);
    Hook_Function(0x00521280, &Parse_False_Action_Chunk);
}
#endif