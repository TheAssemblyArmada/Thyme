/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stub library containing subset of functions from binkw32.dll as used by the SAGE engine.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bink.h"

/*
 * All of these function definitions are empty, they are just here to generate a dummy dll that
 * exports the same symbols as the real binkw32.dll so we can link against it without the commercial
 * SDK.
 */

Bink *__stdcall BinkOpen(const char *name, uint32_t flags)
{
    return NULL;
}

void __stdcall BinkSetSoundTrack(uint32_t total_tracks, uint32_t *tracks)
{
    
}

int32_t __stdcall BinkSetSoundSystem(SndOpenCallback open, uintptr_t param)
{
    return 0;
}

void *__stdcall BinkOpenDirectSound(uintptr_t param)
{
    return NULL;
}

void __stdcall BinkClose(Bink *handle)
{
    
}

int32_t __stdcall BinkWait(Bink *handle)
{
    return 0;
}

int32_t __stdcall BinkDoFrame(Bink *handle)
{
    return 0;
}

int32_t __stdcall BinkCopyToBuffer(
    Bink *handle, void *dest, int32_t destpitch, uint32_t destheight, uint32_t destx, uint32_t desty, uint32_t flags)
{
    return 0;
}

void __stdcall BinkSetVolume(Bink *handle, uint32_t trackid, int32_t volume)
{
    
}

void __stdcall BinkNextFrame(Bink *handle)
{
    
}

void __stdcall BinkGoto(Bink *handle, uint32_t frame, int32_t flags)
{
    
}
