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
#pragma once
#include "always.h"
#include "gameclient.h"

class W3DGameClient : public GameClient
{
public:
    ~W3DGameClient() override;
    void Init() override;
    void Reset() override;
    void Update() override;
    RayEffectData *Create_Ray_Effect_From_Template(
        const Coord3D *src, const Coord3D *dst, const ThingTemplate *temp) override;
    void Add_Scorch(Coord3D *pos, float scale, Scorches scorch) override;
    Drawable *Create_Drawable(const ThingTemplate *temp, DrawableStatus status) override;
    void Set_Time_Of_Day(TimeOfDayType time) override;
    void Set_Team_Color(int red, int blue, int green) override;
    void Adjust_LOD(int lod) override;
    int Notify_Terrain_Object_Moved(Object *obj) override;
    Display *Create_GameDisplay() override;
    InGameUI *Create_InGameUI() override;
    GameWindowManager *Create_WindowManager() override;
    FontLibrary *Create_FontLibrary() override;
    DisplayStringManager *Create_DisplayStringManager() override;
    VideoPlayer *Create_VideoPlayer() override;
    TerrainVisual *Create_TerrainVisual() override;
    Keyboard *Create_Keyboard() override;
    Mouse *Create_Mouse() override;
    SnowManager *Create_SnowManager() override;
    void Set_Frame_Rate(float fps) override;
};