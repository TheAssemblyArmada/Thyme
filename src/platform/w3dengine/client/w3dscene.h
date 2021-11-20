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

class RTS3DScene : public SimpleSceneClass, public SubsystemInterface
{
public:
    virtual ~RTS3DScene() override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Customized_Render(RenderInfoClass &rinfo) override;
    virtual void Visibility_Check(CameraClass *camera);
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override;

    CustomScenePassModes Get_Custom_Scene_Pass_Mode() { return m_customScenePassMode; }

protected:
    GenericMultiListClass m_dynamicLightList;
    bool m_drawTerrainOnly;
    LightClass *m_globalLight[4];
    LightClass *m_sceneLight;
    Vector3 m_ambient;
    LightClass *m_infantryLight[4];
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
    int m_unkOccludedCount;
    RenderObjClass **m_occludedBuildingsBuffer;
    RenderObjClass **m_occludedObjectsBuffer;
    RenderObjClass **m_occludedOthersBuffer;
    int m_occludedBuildingsCount;
    int m_occludedObjectsCount;
    int m_occludedOthersCount;
    CameraClass *m_camera;
};
