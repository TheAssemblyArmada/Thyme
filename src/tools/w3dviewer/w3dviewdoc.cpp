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
#include "w3dviewdoc.h"
#include "scene.h"
#include "w3dview.h"

IMPLEMENT_DYNCREATE(CW3DViewDoc, CDocument)

// clang-format off
BEGIN_MESSAGE_MAP(CW3DViewDoc, CDocument)
END_MESSAGE_MAP()
// clang-format on

CW3DViewDoc::CW3DViewDoc() :
    m_scene(nullptr),
    m_textureScene(nullptr),
    m_cursorScene(nullptr),
    m_backgroundScene(nullptr),
    m_dazzleLayer(nullptr),
    m_model(nullptr),
    m_backgroundObject(nullptr),
    m_animation(nullptr),
    m_light(nullptr),
    m_backgroundBitmap(nullptr),
    m_textureCamera(nullptr),
    m_docCamera(nullptr),
    m_fogColor(0.5f, 0.5f, 0.5f),
    m_isQCompressed(false),
    m_compressType(2),
    m_frameCount(0.0f),
    m_blendFrames(TRUE),
    m_animateCamera(false),
    m_resetCamera(true),
    m_forceCameraReset(true),
    m_useManualFov(false),
    m_useManualClipPlanes(false),
    m_initialized(false),
    m_fogEnabled(false)
{
    m_animateCamera = theApp.GetProfileInt("Config", "AnimateCamera", 0) == 1;
    m_resetCamera = theApp.GetProfileInt("Config", "ResetCamera", 1) == 1;
}

CW3DViewDoc::~CW3DViewDoc()
{
    Free();
}

BOOL CW3DViewDoc::OnNewDocument()
{
    // TODO
    return FALSE;
}

BOOL CW3DViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    // TODO
    return FALSE;
}

void CW3DViewDoc::Free()
{
    // TODO
}

void CW3DViewDoc::SaveSettings(const char *name)
{
    // TODO
}

#if 0
void CW3DViewDoc::Initialize()
{
    // TODO
}

void CW3DViewDoc::LoadFile(CString pathName)
{
    // TODO
}

void CW3DViewDoc::Deselect()
{
    // TODO
}

void CW3DViewDoc::SetParticleEmitter(ParticleEmitterClass *emitter, bool unk1, bool unk2)
{
    // TODO
}

void CW3DViewDoc::SetRenderObject(RenderObjClass *robj, bool unk1, bool unk2, bool unk3)
{
    // TODO
}

void CW3DViewDoc::UpdateFrameCount()
{
    // TODO
}

void CW3DViewDoc::OnStep(int step)
{
    // TODO
}

void CW3DViewDoc::SetAnimationByName(RenderObjClass *robj, const char *name, bool unk1, bool unk2)
{
    // TODO
}

void CW3DViewDoc::PlayAnimationSound()
{
    // TODO
}

void CW3DViewDoc::UpdateAnimation(float tm)
{
    // TODO
}

void CW3DViewDoc::GenerateLOD(const char *name, int type)
{
    // TODO
}

void CW3DViewDoc::CreateBackgroundBitmap(const char *name)
{
    // TODO
}

void CW3DViewDoc::WriteSettingsFile(const char *name, int flags)
{
    // TODO
}

bool CW3DViewDoc::ExportLOD()
{
    // TODO
}

bool CW3DViewDoc::SaveLOD(const char *name)
{
    // TODO
}

void CW3DViewDoc::SetBackgroundObject(const char *name)
{
    // TODO
}

void CW3DViewDoc::RemoveRenderObject(RenderObjClass *robj)
{
    // TODO
}

bool CW3DViewDoc::ExportPrimitive()
{
    // TODO
}

bool CW3DViewDoc::ExportEmitter()
{
    // TODO
}

bool CW3DViewDoc::SavePrototype(const char *name)
{
    // TODO
}

bool CW3DViewDoc::ExportSound()
{
    // TODO
}

bool CW3DViewDoc::SaveSound(const char *name)
{
    // TODO
}

void CW3DViewDoc::AutoAssignBone()
{
    // TODO
}

bool CW3DViewDoc::ExportAggregate()
{
    // TODO
}

void CW3DViewDoc::CreateAggregatePrototype(RenderObjClass &robj)
{
    // TODO
}

void CW3DViewDoc::CreateHLodPrototype(HLodClass &lod)
{
    // TODO
}

void CW3DViewDoc::AnimateCamera(bool animate)
{
    // TODO
}

void CW3DViewDoc::CaptureMovie()
{
    // TODO
}

void CW3DViewDoc::AddEmittersToDef(EditorParticleEmitterDefClass *def, const char *name, RenderObjClass *robj)
{
    // TODO
}

int CW3DViewDoc::GetNumParticles(RenderObjClass *robj)
{
    // TODO
}

void CW3DViewDoc::UpdateParticleCount()
{
    // TODO
}

void CW3DViewDoc::SetLODLevel(int lod, RenderObjClass *robj)
{
    // TODO
}

void CW3DViewDoc::ToggleAlternateMaterials(RenderObjClass *robj)
{
    // TODO
}

void CW3DViewDoc::CopyDeps(CString directorty)
{
    // TODO
}

void CW3DViewDoc::SetTexturePath1(const char *path)
{
    // TODO
}

void CW3DViewDoc::SetTexturePath2(const char *path)
{
    // TODO
}

void CW3DViewDoc::ImportFacial(CString &hname, CString &fname)
{
    // TODO
}

HTreeClass *CW3DViewDoc::GetCurrentHTree()
{
    // TODO
}

void CW3DViewDoc::SaveCameraSettings()
{
    // TODO
}

void CW3DViewDoc::ReadSettings()
{
    // TODO
}

void CW3DViewDoc::RenderDazzleLayer(CameraClass *camera)
{
    // TODO
}

void CW3DViewDoc::SetFogColor(Vector3 &color)
{
    // TODO
}

void CW3DViewDoc::EnableFog(bool enable)
{
    // TODO
}
#endif
