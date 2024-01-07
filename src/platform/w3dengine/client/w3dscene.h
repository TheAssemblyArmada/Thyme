/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Scene
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
#include "lightenv.h"
#include "scene.h"
#include "shadermanager.h"
#include "subsysteminterface.h"

class W3DShroudMaterialPassClass;
class W3DMaskMaterialPassClass;
class MaterialPassClass;
class W3DDynamicLight;

class RTS3DScene : public SimpleSceneClass, public SubsystemInterface
{
public:
    RTS3DScene();
    virtual ~RTS3DScene() override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Customized_Render(RenderInfoClass &rinfo) override;
    virtual void Visibility_Check(CameraClass *camera);
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}
    virtual void Draw() override;

    void Flush(RenderInfoClass &rinfo);
    void Add_Dynamic_Light(W3DDynamicLight *obj);
    bool Cast_Ray(RayCollisionTestClass &ray_test, bool test_all, int collision_type);
    RefMultiListIterator<RenderObjClass> *Create_Lights_Iterator();
    void Destroy_Lights_Iterator(RefMultiListIterator<RenderObjClass> *it);
    void Do_Render(CameraClass *camera);
    void Flag_Occluded_Objects(CameraClass *camera);
    void Flush_Occluded_Objects(RenderInfoClass &rinfo);
    void Flush_Occluded_Objects_Into_Stencil(RenderInfoClass &rinfo);
    void flush_Translucent_Objects(RenderInfoClass &rinfo);
    W3DDynamicLight *Get_A_Dynamic_Light();
    void Remove_Dynamic_Light(W3DDynamicLight *obj);
    void Render_One_Object(RenderInfoClass &rinfo, RenderObjClass *robj, int local_player_index);
    void Render_Specific_Drawables(RenderInfoClass &rinfo, int num_drawables, Drawable **drawables);
    void Set_Global_Light(LightClass *light, int light_index);
    void Update_Fixed_Light_Environments(RenderInfoClass &rinfo);
    void Update_Player_Color_Passes();

    void Draw_Terrain_Only(bool draw) { m_drawTerrainOnly = draw; }
    void Set_Custom_Scene_Pass_Mode(CustomScenePassModes mode) { m_customScenePassMode = mode; }
    CustomScenePassModes Get_Custom_Scene_Pass_Mode() { return m_customScenePassMode; }
    RefMultiListClass<RenderObjClass> *Get_Dynamic_Lights() { return &m_dynamicLightList; }

#ifdef GAME_DLL
    RTS3DScene *Hook_Ctor() { return new (this) RTS3DScene(); }
#endif

protected:
    RefMultiListClass<RenderObjClass> m_dynamicLightList;
    bool m_drawTerrainOnly;
    LightClass *m_globalLight[LightEnvironmentClass::MAX_LIGHTS];
    LightClass *m_sceneLight;
    Vector3 m_ambient;
    LightClass *m_infantryLight[LightEnvironmentClass::MAX_LIGHTS];
    int m_numGlobalLights;
    LightEnvironmentClass m_globalLightEnv;
    LightEnvironmentClass m_sceneLightEnv;
    W3DShroudMaterialPassClass *m_shroudMaterialPass;
    W3DMaskMaterialPassClass *m_maskMaterialPass;
    MaterialPassClass *m_normalMatPass;
    MaterialPassClass *m_stealthDetectedMatPass;
    int m_unk;
    MaterialPassClass *m_occludedMatPassesPerPlayer[16];
    CustomScenePassModes m_customScenePassMode;
    int m_translucentObjectsCount;
    RenderObjClass **m_translucentObjectsBuffer;
    int m_flaggedOccludedCount;
    RenderObjClass **m_occludedBuildingsBuffer;
    RenderObjClass **m_occludedObjectsBuffer;
    RenderObjClass **m_occludedOthersBuffer;
    int m_occludedBuildingsCount;
    int m_occludedObjectsCount;
    int m_occludedOthersCount;
    CameraClass *m_camera;
};

class RTS2DScene : public SimpleSceneClass, public SubsystemInterface
{
public:
    RTS2DScene();
    virtual ~RTS2DScene() override;
    virtual void Customized_Render(RenderInfoClass &rinfo) override;
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}
    virtual void Draw() override;

    void Do_Render(CameraClass *camera);

#ifdef GAME_DLL
    RTS2DScene *Hook_Ctor() { return new (this) RTS2DScene(); }
#endif

private:
    RenderObjClass *m_status;
    CameraClass *m_camera;
};

class RTS3DInterfaceScene : public SimpleSceneClass
{
public:
    RTS3DInterfaceScene();
    virtual ~RTS3DInterfaceScene() override;
    virtual void Customized_Render(RenderInfoClass &rinfo) override;

#ifdef GAME_DLL
    RTS3DInterfaceScene *Hook_Ctor() { return new (this) RTS3DInterfaceScene(); }
#endif
};
