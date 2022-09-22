/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View document
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "w3dafx.h"
#include "vector.h"
#include "vector3.h"

class Bitmap2D;
class CDataTreeView;
class CGraphicView;
class CameraClass;
class EmitterInstanceList;
class HAnimClass;
class HLodClass;
class HTreeClass;
class LightClass;
class ParticleEmitterClass;
class RenderObjClass;
class SimpleSceneClass;
class ViewerSceneClass;
class PrototypeClass;

class CW3DViewDoc : public CDocument
{
public:
    virtual ~CW3DViewDoc() override;
    virtual void Serialize(CArchive &ar) override {}
    virtual BOOL IsModified() override { return m_bModified; }
    virtual BOOL OnNewDocument() override;
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;

    void Free();
    void Initialize();
    void LoadFile(CString pathName);
    void Deselect();
    void SetParticleEmitter(ParticleEmitterClass *emitter, bool useRegularCameraReset, bool resetCamera);
    void SetRenderObject(RenderObjClass *robj, bool useRegularCameraReset, bool resetCamera, bool preserveModel);
    void UpdateFrameCount();
    void OnStep(int step);
    void SetAnimationByName(RenderObjClass *robj, const char *name, bool useRegularCameraReset, bool resetCamera);
    void UpdateAnimation(float tm);
    CDataTreeView *GetDataTreeView();
    CGraphicView *GetGraphicView();
    PrototypeClass *GenerateLOD(const char *name, int type);
    void CreateBackgroundBitmap(const char *name);
    bool ExportLOD();
    bool SaveLOD(const char *name);
    void SetBackgroundObject(const char *name);
    void RemoveRenderObject(RenderObjClass *robj);
    bool ExportEmitter();
    bool SaveEmitter(const char *name);
    void CreateHLodPrototype(HLodClass &lod);
    void AnimateCamera(bool animate);
    void AddEmittersToList(EmitterInstanceList *list, const char *name, RenderObjClass *robj);
    int GetNumParticles(RenderObjClass *robj);
    void UpdateParticleCount();
    void SetLODLevel(int lod, RenderObjClass *robj);
    void ToggleAlternateMaterials(RenderObjClass *robj);
    void SetTexturePath1(const char *path);
    void SetTexturePath2(const char *path);
    void SaveCameraSettings();
    void ReadSettings();
    void SetFogColor(Vector3 &color);
    void EnableFog(bool enable);

protected:
    CW3DViewDoc();
    DECLARE_DYNCREATE(CW3DViewDoc)
    DECLARE_MESSAGE_MAP()

public:
    ViewerSceneClass *m_scene;
    SimpleSceneClass *m_textureScene;
    SimpleSceneClass *m_backgroundScene;
    RenderObjClass *m_model;
    RenderObjClass *m_backgroundObject;
    HAnimClass *m_animation;
    LightClass *m_light;
    Bitmap2D *m_backgroundBitmap;
    CameraClass *m_textureCamera;
    CameraClass *m_docCamera;
    Vector3 m_fogColor;
    CString m_backgroundBitmapName;
    CString m_backgroundObjectName;
    float m_frameCount;
    float m_time;
    BOOL m_blendFrames;
    bool m_animateCamera;
    bool m_resetCamera;
    bool m_forceCameraReset;
    bool m_useManualFov;
    bool m_useManualClipPlanes;
    bool m_initialized;
    bool m_fogEnabled;
    CString m_texturePath1;
    CString m_texturePath2;
    CString m_filePath;
    DynamicVectorClass<CString> m_loadedFiles;
};
