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

#ifndef SCRIPTACTION_H
#define SCRIPTACTION_H

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
    virtual ~ScriptAction();

private:
    ScriptActionType m_actionType;
    int m_numParams;
    Parameter *m_params[MAX_ACTION_PARAMETERS];
    ScriptAction *m_nextAction;
    bool m_hasWarnings;
};

#endif // SCRIPTACTION_H
