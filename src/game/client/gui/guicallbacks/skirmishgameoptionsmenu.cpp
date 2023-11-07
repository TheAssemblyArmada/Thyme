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
#include "skirmishgameoptionsmenu.h"
#include "gametext.h"
#include "image.h"
#include "mouse.h"

TechAndSupplyImages g_theSupplyAndTechImageLocations;

void Position_Additional_Images(MapMetaData *map, GameWindow *window, bool reset)
{
    g_theSupplyAndTechImageLocations.m_supplyPositions.clear();
    g_theSupplyAndTechImageLocations.m_techPositions.clear();

    if (map != nullptr && window != nullptr && !window->Win_Is_Hidden()) {
        static MapMetaData *prevMMD;

        if (reset) {
            prevMMD = nullptr;
        }

        if (map != prevMMD) {
            int width;
            int height;
            int screen_x;
            int screen_y;
            window->Win_Get_Size(&width, &height);
            window->Win_Get_Screen_Position(&screen_x, &screen_y);
            ICoord2D ul;
            ICoord2D lr;
            Find_Draw_Positions(0, 0, width, height, map->m_extent, &ul, &lr);
            int draw_w = lr.x - ul.x;
            int draw_h = lr.y - ul.y;

            for (auto it = map->m_supplyPositions.begin(); it != map->m_supplyPositions.end(); it++) {
                ICoord2D pos;
                float ratio = (it->x - map->m_extent.lo.x) / (map->m_extent.hi.x - map->m_extent.lo.x);
                pos.x = (draw_w * ratio - 7.0f + ul.x);
                ratio = (it->y - map->m_extent.lo.y) / (map->m_extent.hi.y - map->m_extent.lo.y);
                pos.y = ((1.0f - ratio) * draw_h - 7.0f + ul.y);
                g_theSupplyAndTechImageLocations.m_supplyPositions.push_front(pos);
            }

            for (auto it = map->m_techPositions.begin(); it != map->m_techPositions.end(); it++) {
                ICoord2D pos;
                float ratio = (it->x - map->m_extent.lo.x) / (map->m_extent.hi.x - map->m_extent.lo.x);
                pos.x = (draw_w * ratio - 7.0f + ul.x);
                ratio = (it->y - map->m_extent.lo.y) / (map->m_extent.hi.y - map->m_extent.lo.y);
                pos.y = ((1.0f - ratio) * draw_h - 7.0f + ul.y);
                g_theSupplyAndTechImageLocations.m_techPositions.push_front(pos);
            }
        }
    }
}

void Map_Selector_Tooltip(GameWindow *window, WinInstanceData *instance, unsigned int mouse)
{
    int mouse_x = (mouse & 0xffff);
    int mouse_y = ((mouse >> 16) & 0xffff);
    int screen_x;
    int screen_y;
    window->Win_Get_Screen_Position(&screen_x, &screen_y);
    Image *tech_building = g_theMappedImageCollection->Find_Image_By_Name("TecBuilding");
    Image *cash = g_theMappedImageCollection->Find_Image_By_Name("Cash");

    if (tech_building != nullptr) {
        for (auto it = g_theSupplyAndTechImageLocations.m_techPositions.begin();
             it != g_theSupplyAndTechImageLocations.m_techPositions.end();
             it++) {
            if (mouse_x > it->x + screen_x) {
                if (mouse_x < screen_x + it->x + 15) {
                    if (mouse_y > it->y + screen_y) {
                        if (mouse_y < screen_y + it->y + 15) {
                            g_theMouse->Set_Cursor_Tooltip(g_theGameText->Fetch("TOOLTIP:TechBuilding"), -1, nullptr, 1.0f);
                            return;
                        }
                    }
                }
            }
        }
    }

    if (cash != nullptr) {
        for (auto it = g_theSupplyAndTechImageLocations.m_supplyPositions.begin();
             it != g_theSupplyAndTechImageLocations.m_supplyPositions.end();
             it++) {
            if (mouse_x > it->x + screen_x) {
                if (mouse_x < screen_x + it->x + 15) {
                    if (mouse_y > it->y + screen_y) {
                        if (mouse_y < screen_y + it->y + 15) {
                            g_theMouse->Set_Cursor_Tooltip(g_theGameText->Fetch("TOOLTIP:SupplyDock"), -1, nullptr, 1.0f);
                            return;
                        }
                    }
                }
            }
        }
    }
}
