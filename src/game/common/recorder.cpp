/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Recorder
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "recorder.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
RecorderClass *g_theRecorder;
int g_replayCRCInterval;
#endif

bool RecorderClass::Playback_File(Utf8String name)
{
#ifdef GAME_DLL
    return Call_Method<bool, RecorderClass, Utf8String>(PICK_ADDRESS(0x0048CDB0, 0x00902519), this, name);
#else
    return false;
#endif
}

RecorderClass *Create_Recorder()
{
#ifdef GAME_DLL
    return Call_Function<RecorderClass *>(PICK_ADDRESS(0x0048DAC0, 0x00903EDE));
#else
    return nullptr;
#endif
}

bool RecorderClass::Is_Multiplayer()
{
#ifdef GAME_DLL
    return Call_Method<bool, RecorderClass>(PICK_ADDRESS(0x0048DA50, 0x00903E42), this);
#else
    return false;
#endif
}

void RecorderClass::Init_Controls()
{
#ifdef GAME_DLL
    Call_Method<void, RecorderClass>(PICK_ADDRESS(0x0048D970, 0x00903D94), this);
#endif
}
