/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Template classes for script actions and conditions.
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
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "scriptparam.h"

class Template : public MemoryPoolObject
{
    friend class ScriptEngine;
    IMPLEMENT_POOL(Template)

    enum
    {
        PARAMETER_COUNT = 12,
    };

public:
    Template();
    virtual ~Template() override{};

    void Get_UI_Strings(Utf8String *strings) const;
    Parameter::ParameterType Get_Parameter_Type(int32_t parameter) const;
    int32_t Get_Num_Parameters() const { return m_numParameters; }

private:
    Utf8String m_uiName;
    Utf8String m_uiName2;
    Utf8String m_internalName;
    NameKeyType m_nameKey;
    int32_t m_numUiStrings;
    Utf8String m_uiStrings[PARAMETER_COUNT];
    int32_t m_numParameters;
    Parameter::ParameterType m_parameters[PARAMETER_COUNT];
    Utf8String m_helpString;
};

class ActionTemplate : public Template
{
    friend class ScriptEngine;

public:
    virtual ~ActionTemplate() override {}
};

class ConditionTemplate : public Template
{
    friend class ScriptEngine;

public:
    virtual ~ConditionTemplate() override {}
};
