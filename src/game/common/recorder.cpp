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
#endif

RecorderClass *Create_Recorder()
{
#ifdef GAME_DLL
    return Call_Function<RecorderClass *>(PICK_ADDRESS(0x0048DAC0, 0x00903EDE));
#else
    return nullptr;
#endif
}
