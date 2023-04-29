/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Game Logic
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dgamelogic.h"
#include "w3dghostobject.h"
#include "w3dterrainlogic.h"

TerrainLogic *W3DGameLogic::Create_Terrain_Logic()
{
    return new W3DTerrainLogic();
}

GhostObjectManager *W3DGameLogic::Create_Ghost_Object_Manager()
{
    return new W3DGhostObjectManager();
}
