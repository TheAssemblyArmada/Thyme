/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief particle emitter loader
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "part_ldr.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

PrototypeClass *ParticleEmitterLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
#ifdef GAME_DLL
    return Call_Method<PrototypeClass *, ParticleEmitterLoaderClass, ChunkLoadClass &>(0x00812120, this, cload);
#else
    return nullptr;
#endif
}
