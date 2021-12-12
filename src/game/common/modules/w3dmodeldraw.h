/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Model Draw Module
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
#include "drawmodule.h"

class ModelConditionInfo;
class RenderObjClass;
class Shadow;
class TerrainTracksRenderObjClass;

class W3DModelDraw : public DrawModule, public ObjectDrawInterface
{
    IMPLEMENT_POOL(W3DModelDraw);

public:
    struct RecoilInfo
    {
        enum RecoilState
        {
            IDLE,
            RECOIL_START,
            RECOIL,
            SETTLE,
        };

        RecoilState m_state;
        float m_shift;
        float m_recoilRate;
    };

    struct SubObject
    {
        Utf8String name;
        bool visible;
    };

    virtual ~W3DModelDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Drawable_Bound_To_Object() override;
    virtual void Preload_Assets(TimeOfDayType time_of_day) override;
    virtual void Do_Draw_Module(Marix3D *transform) override;
    virtual void Set_Shadows_Enabled(bool enable) override;
    virtual void Release_Shadows() override;
    virtual void Allocate_Shadows() override;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(DebugDrawStats *stats) override;
#endif
    virtual void Set_Terrain_Decal(TerrainDecalType decal) override;
    virtual void Set_Terrain_Decal_Size(float width, float height) override;
    virtual void Set_Terrain_Decal_Opacity(float opacity) override;
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    virtual bool Is_Visible() const override;
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    virtual void React_To_Geometry_Change() override;
    virtual ObjectDrawInterface *Get_Object_Draw_Interface() override;
    virtual const ObjectDrawInterface *Get_Object_Draw_Interface() const override;
    virtual void On_Render_Obj_Recreated();
    virtual void Adjust_Transform_Mtx(Matrix3D &) const;

    RenderObjClass *getRenderObject() { return m_renderObject; }

private:
    ModelConditionInfo *m_curState;
    ModelConditionInfo *m_nextState;
    int m_loopDuration;
    int m_hexColor;
    int m_whichAnimInCurState;
    std::vector<RecoilInfo> m_weaponRecoilInfoVec[3];
    bool m_recalcBones;
    bool m_fullyObscuredByShroud;
    bool m_shadowState;
    RenderObjClass *m_renderObject;
    Shadow *m_shadow;
    Shadow *m_decalShadow;
    TerrainTracksRenderObjClass *m_trackRenderObject;
    std::vector<unsigned int> m_particleSystemIDs;
    std::vector<SubObject> m_subObjects;
    bool m_isDaytime;
    bool m_pauseAnimation;
    int m_animMode;
};
