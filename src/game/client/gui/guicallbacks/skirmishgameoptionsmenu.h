/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Skirmish Game Options Menu
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
#include "coord.h"
#include "gamewindow.h"
#include "maputil.h"
#include <list>

class TechAndSupplyImages
{
public:
    std::list<ICoord2D> m_techPositions;
    std::list<ICoord2D> m_supplyPositions;
};

void Position_Additional_Images(MapMetaData *map, GameWindow *window, bool reset);
void Map_Selector_Tooltip(GameWindow *window, WinInstanceData *instance, unsigned int mouse);

extern TechAndSupplyImages g_theSupplyAndTechImageLocations;
