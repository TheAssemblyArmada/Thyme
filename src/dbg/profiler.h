/**
 * @file
 *
 * @author feliwir
 *
 * @brief Profiler abstraction header.
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

#ifdef BUILD_WITH_TRACY
#define USE_PROFILER
#include <tracy/Tracy.hpp>

// Misc
#define PROFILER_MSG(M) TracyMessage(M, strlen(M));
#define PROFILER_FRAME_START(N) FrameMarkStart(N);
#define PROFILER_FRAME_END(N) FrameMarkEnd(N);

// Blocks
#define PROFILER_BLOCK_SCOPED ZoneScoped;
#define PROFILER_BLOCK_TEXT(N, S) ZoneText(N, S);

// Memory profiling
#define PROFILER_ALLOC(P, S) TracySecureAlloc(P, S);
#define PROFILER_FREE(P) TracySecureFree(P);

#define PROFILER_ALLOC_NAMED(P, S, N) TracySecureAllocN(P, S, N);
#define PROFILER_FREE_NAMED(P, N) TracySecureFreeN(P, N);
#endif
