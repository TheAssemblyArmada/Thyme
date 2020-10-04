/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base client object providing overall control of client IO and rendering.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameclient.h"
#include "rayeffect.h"

#ifndef GAME_DLL
GameClient *g_theGameClient;
#endif

void GameClient::Remove_From_Ray_Effects(Drawable *drawable)
{
    g_theRayEffects->Delete_Ray_Effect(drawable);
}

void GameClient::Get_Ray_Effect_Data(Drawable *drawable, RayEffectData *data)
{
    g_theRayEffects->Get_Ray_Effect_Data(drawable, data);
}
