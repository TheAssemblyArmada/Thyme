/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Gamespy stuff
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
#include "gameinfo.h"

class GameSpyStagingRoom : public GameInfo
{
public:
    Utf16String Get_Game_Name() const { return m_gameName; }

private:
    Utf16String m_gameName;
};
