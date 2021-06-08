/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Script engine condition processor.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "mempoolobj.h"
#include "subsysteminterface.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class Condition;
class Parameter;

class TransportStatus : public MemoryPoolObject
{
    IMPLEMENT_POOL(TransportStatus);

public:
    TransportStatus() : m_nextStatus(nullptr), m_objID(0), m_frameNumber(0), m_unitCount(0) {}
    virtual ~TransportStatus() override {}

    TransportStatus *m_nextStatus;
    unsigned int m_objID;
    unsigned int m_frameNumber;
    int m_unitCount;
};

class ScriptConditionsInterface : public SubsystemInterface
{
public:
    virtual bool Evaluate_Condition(Condition *condition) = 0;
    virtual bool Evaluate_Skirmish_Command_Button_Is_Ready(
        Parameter *param_one, Parameter *param_two, Parameter *param_three, bool unk) = 0;
    virtual bool Evaluate_Team_Is_Contained(Parameter *param, bool unk) = 0;
};

class ScriptConditions : public ScriptConditionsInterface
{
    ScriptConditions();
    virtual ~ScriptConditions() override;

    // SubsystemInterface virtuals.
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    virtual bool Evaluate_Condition(Condition *condition) override;
    virtual bool Evaluate_Skirmish_Command_Button_Is_Ready(
        Parameter *param_one, Parameter *param_two, Parameter *param_three, bool unk) override;
    virtual bool Evaluate_Team_Is_Contained(Parameter *param, bool unk) override;

private:
#ifdef GAME_DLL
    static TransportStatus *&s_transportStatuses;
#else
    static TransportStatus *s_transportStatuses;
#endif
};

#ifdef GAME_DLL
extern ScriptConditionsInterface *&g_theScriptConditions;
#else
extern ScriptConditionsInterface *g_theScriptConditions;
#endif
