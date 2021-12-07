/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Script engine
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
#include "snapshot.h"
#include "subsysteminterface.h"

class ScriptEngine : public SubsystemInterface, public SnapShot
{
    // dummy implementation for now
public:
    enum TFade
    {
        FADE_NONE,
        FADE_SUBTRACT,
        FADE_ADD,
        FADE_SATURATE,
        FADE_MULTIPLY,
    };

    TFade Get_Fade() { return m_fade; }
    int Get_Fade_Value() { return m_curFadeValue; }

private:
    char unk[0x10ACC];
    TFade m_fade;
    int m_minFade;
    int m_maxFade;
    int m_curFadeValue;
    int m_curFadeFrame;
};

#ifdef GAME_DLL
extern ScriptEngine *&g_theScriptEngine;
#else
extern ScriptEngine *g_theScriptEngine;
#endif
