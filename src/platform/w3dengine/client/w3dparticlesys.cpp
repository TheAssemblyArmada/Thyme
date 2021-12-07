/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Particle System
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dparticlesys.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void Do_Particles(RenderInfoClass &rinfo)
{
#ifdef GAME_DLL
    Call_Function<void, RenderInfoClass &>(0x00778D20, rinfo);
#endif
}
