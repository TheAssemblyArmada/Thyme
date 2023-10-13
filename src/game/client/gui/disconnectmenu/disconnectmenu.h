/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Disconnect Menu
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

class DisconnectManager;

enum DisconnectMenuStateType
{
    DISCONNECTMENUSTATETYPE_SCREENON,
    DISCONNECTMENUSTATETYPE_SCREENOFF,
};

class DisconnectMenu
{
public:
    virtual ~DisconnectMenu();

    bool Is_Screen_Visible() const { return m_menuState == DISCONNECTMENUSTATETYPE_SCREENON; }

private:
    DisconnectManager *m_disconnectManager;
    DisconnectMenuStateType m_menuState;
};

#ifdef GAME_DLL
extern DisconnectMenu *&g_theDisconnectMenu;
#else
extern DisconnectMenu *g_theDisconnectMenu;
#endif
