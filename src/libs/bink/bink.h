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
#pragma once

#include <stdint.h>

#if !defined _MSC_VER
#if !defined(__stdcall)
#if defined __has_attribute && __has_attribute(stdcall)
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif /* !defined __stdcall */
#endif /* !defined COMPILER_MSVC */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Bink
{
    uint32_t width;
    uint32_t height;
    uint32_t frames;
    uint32_t frame_num;
    uint32_t last_frame_num;
    /* Original struct has more members, but we only need these to match the ABI*/
} Bink;

typedef void *(__stdcall *SndOpenCallback)(uintptr_t param);

Bink *__stdcall BinkOpen(const char *name, uint32_t flags);
void __stdcall BinkSetSoundTrack(uint32_t total_tracks, uint32_t *tracks);
int32_t __stdcall BinkSetSoundSystem(SndOpenCallback open, uintptr_t param);
void *__stdcall BinkOpenDirectSound(uintptr_t param);
void __stdcall BinkClose(Bink *handle);
int32_t __stdcall BinkWait(Bink *handle);
int32_t __stdcall BinkDoFrame(Bink *handle);
int32_t __stdcall BinkCopyToBuffer(
    Bink *handle, void *dest, int32_t destpitch, uint32_t destheight, uint32_t destx, uint32_t desty, uint32_t flags);
void __stdcall BinkSetVolume(Bink *handle, uint32_t trackid, int32_t volume);
void __stdcall BinkNextFrame(Bink *handle);
void __stdcall BinkGoto(Bink *handle, uint32_t frame, int32_t flags);

#ifdef __cplusplus
} // extern "C"
#endif
