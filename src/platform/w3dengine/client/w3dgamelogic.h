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
#pragma once
#include "always.h"
#include "gamelogic.h"

class W3DGameLogic : public GameLogic
{
public:
    W3DGameLogic() {}
    virtual ~W3DGameLogic() override {}
    virtual TerrainLogic *Create_Terrain_Logic() override;
    virtual GhostObjectManager *Create_Ghost_Object_Manager() override;
};
