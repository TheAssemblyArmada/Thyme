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
    W3DGameClient();
    virtual ~W3DGameClient() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Create_Ray_Effect_From_Template(const Coord3D *src, const Coord3D *dst, const ThingTemplate *temp) override;
    virtual void Add_Scorch(Coord3D *pos, float scale, Scorches scorch) override;
    virtual Drawable *Create_Drawable(const ThingTemplate *temp, DrawableStatus status) override;
    virtual void Set_Time_Of_Day(TimeOfDayType time) override;
    virtual void Set_Team_Color(int red, int blue, int green) override;
    virtual void Adjust_LOD(int lod) override;
    virtual void Notify_Terrain_Object_Moved(Object *obj) override;
    virtual Display *Create_GameDisplay() override;
    virtual InGameUI *Create_InGameUI() override;
    virtual GameWindowManager *Create_WindowManager() override;
    virtual FontLibrary *Create_FontLibrary() override;
    virtual DisplayStringManager *Create_DisplayStringManager() override;
    virtual VideoPlayer *Create_VideoPlayer() override;
    virtual TerrainVisual *Create_TerrainVisual() override;
    virtual Keyboard *Create_Keyboard() override;
    virtual Mouse *Create_Mouse() override;
    virtual SnowManager *Create_SnowManager() override;
    virtual void Set_Frame_Rate(float fps) override;
};
