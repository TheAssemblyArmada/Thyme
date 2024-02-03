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
#include "baseheightmap.h"
#include "binkvideoplayer.h"
#include "drawable.h"
#include "globaldata.h"
#include "rayeffect.h"
#include "thingfactory.h"
#include "w3d.h"
#include "w3ddisplay.h"
#include "w3ddisplaystringmanager.h"
#include "w3dgamefont.h"
#include "w3dgamewindowmanager.h"
#include "w3dingameui.h"
#include "w3dmouse.h"
#include "w3dshadow.h"
#include "w3dsnow.h"
#include "w3dstatuscircle.h"
#include "w3dterrainvisual.h"
#include "w3dwater.h"
#ifdef BUILD_WITH_FFMPEG
#include "ffmpegvideoplayer.h"
#endif
#ifdef PLATFORM_WINDOWS
#include "win32dikeyboard.h"
#else
#include "sdl2/sdl2keybd.h"
#endif

W3DGameClient::W3DGameClient() {}

W3DGameClient::~W3DGameClient() {}

void W3DGameClient::Init()
{
    GameClient::Init();
}

void W3DGameClient::Reset()
{
    GameClient::Reset();
}

void W3DGameClient::Update()
{
    GameClient::Update();
}

void W3DGameClient::Create_Ray_Effect_From_Template(const Coord3D *src, const Coord3D *dst, const ThingTemplate *temp)
{
    Drawable *draw = g_theThingFactory->New_Drawable(temp, DRAWABLE_STATUS_UNK);

    if (draw != nullptr) {
        Coord3D pos;
        pos.x = src->x + (dst->x - src->x) * 0.5f;
        pos.y = src->y + (dst->y - src->y) * 0.5f;
        pos.z = src->z + (dst->z - src->z) * 0.5f;
        draw->Set_Position(&pos);
        g_theRayEffects->Add_Ray_Effect(draw, src, dst);
    }
}

void W3DGameClient::Add_Scorch(Coord3D *pos, float scale, Scorches scorch)
{
    if (g_theTerrainRenderObject != nullptr) {
        g_theTerrainRenderObject->Add_Scorch(Vector3(pos->x, pos->y, pos->x), scale, scorch);
    }
}

void W3DGameClient::Set_Time_Of_Day(TimeOfDayType time)
{
    GameClient::Set_Time_Of_Day(time);

    if (g_theWaterRenderObj != nullptr) {
        g_theWaterRenderObj->Set_Time_Of_Day(time);
    }

    if (g_theW3DShadowManager != nullptr) {
        g_theW3DShadowManager->Set_Time_Of_Day(time);
    }

    g_theDisplay->Set_Time_Of_Day(time);
}

void W3DGameClient::Adjust_LOD(int lod)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_textureReductionFactor += lod;

        if (g_theWriteableGlobalData->m_textureReductionFactor > 4) {
            g_theWriteableGlobalData->m_textureReductionFactor = 4;
        }

        if (g_theWriteableGlobalData->m_textureReductionFactor < 0) {
            g_theWriteableGlobalData->m_textureReductionFactor = 0;
        }

        if (W3D::Get_Texture_Reduction() != g_theWriteableGlobalData->m_textureReductionFactor) {
            W3D::Set_Texture_Reduction(g_theWriteableGlobalData->m_textureReductionFactor, 32);

            g_theGameLODManager->Set_Texture_Reduction_Factor(g_theWriteableGlobalData->m_textureReductionFactor);

            if (g_theTerrainRenderObject != nullptr) {
                g_theTerrainRenderObject->Set_Texture_LOD(g_theWriteableGlobalData->m_textureReductionFactor);
            }
        }
    }
}

void W3DGameClient::Notify_Terrain_Object_Moved(Object *obj)
{
    if (g_theTerrainRenderObject != nullptr) {
        g_theTerrainRenderObject->Unit_Moved(obj);
    }
}

Keyboard *W3DGameClient::Create_Keyboard()
{
#ifdef PLATFORM_WINDOWS
    return new DirectInputKeyboard();
#else
    return new Thyme::SDL2InputKeyboard();
#endif
}

void W3DGameClient::Set_Frame_Rate(float fps)
{
    g_theW3DFrameLengthInMsec = fps;
}

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

InGameUI *W3DGameClient::Create_InGameUI()
{
    return new W3DInGameUI;
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
#ifdef GAME_DLL
    return new BinkVideoPlayer();
#else
    return nullptr;
#endif
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
