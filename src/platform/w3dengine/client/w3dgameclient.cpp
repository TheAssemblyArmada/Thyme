/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D game client
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dgameclient.h"
#include "binkvideoplayer.h"
#include "drawable.h"
#include "w3ddisplay.h"
#include "w3ddisplaystringmanager.h"
#include "w3dgamefont.h"
#include "w3dgamewindowmanager.h"
#include "w3dmouse.h"
#include "w3dsnow.h"
#include "w3dstatuscircle.h"
#include "w3dterrainvisual.h"
#ifdef BUILD_WITH_FFMPEG
#include "ffmpegvideoplayer.h"
#endif

GameWindowManager *W3DGameClient::Create_WindowManager()
{
    return new W3DGameWindowManager;
}

Mouse *W3DGameClient::Create_Mouse()
{
    Win32Mouse *mouse = new W3DMouse();
    g_theWin32Mouse = mouse;
    return mouse;
}

Display *W3DGameClient::Create_GameDisplay()
{
    return new W3DDisplay;
}

FontLibrary *W3DGameClient::Create_FontLibrary()
{
    return new W3DFontLibrary;
}

TerrainVisual *W3DGameClient::Create_TerrainVisual()
{
    return new W3DTerrainVisual;
}

void W3DGameClient::Set_Team_Color(int red, int green, int blue)
{
    W3DStatusCircle::g_needUpdate = true;
    W3DStatusCircle::g_diffuse = (red << 16) + (green << 8) + blue;
}

Drawable *W3DGameClient::Create_Drawable(const ThingTemplate *temp, DrawableStatus status)
{
    if (temp == nullptr) {
        return nullptr;
    }

    return new Drawable(temp, status);
}

VideoPlayer *W3DGameClient::Create_VideoPlayer()
{
#ifdef BUILD_WITH_FFMPEG
    return new Thyme::FFmpegVideoPlayer();
#else
    return new BinkVideoPlayer();
#endif
}

SnowManager *W3DGameClient::Create_SnowManager()
{
    return new W3DSnowManager();
}

DisplayStringManager *W3DGameClient::Create_DisplayStringManager()
{
    return new W3DDisplayStringManager();
}
