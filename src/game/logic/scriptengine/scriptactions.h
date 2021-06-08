/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Script engine action processor.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "subsysteminterface.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class ScriptAction;

class ScriptActionsInterface : public SubsystemInterface
{
public:
    virtual void Execute_Action(ScriptAction *action) = 0;
    virtual int Close_Windows(bool unk) = 0;
    virtual void Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool bonus) = 0;
};

class ScriptActions : public ScriptActionsInterface
{
public:
    ScriptActions();
    virtual ~ScriptActions() override;

    // SubsystemInterface virtuals.
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    // ScriptActionsInterface virtuals
    virtual void Execute_Action(ScriptAction *action) override;
    virtual int Close_Windows(bool unk) override;
    virtual void Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool bonus) override;

private:
    bool m_unkBool;
    Utf8String m_unkString;
};

#ifdef GAME_DLL
extern ScriptActionsInterface *&g_theScriptActions;
#else
extern ScriptActionsInterface *g_theScriptActions;
#endif
