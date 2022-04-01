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
#include "assetmgr.h"
#include "bmp2d.h"
#include "camera.h"
#include "ffactory.h"
#include "graphicview.h"
#include "hanim.h"
#include "light.h"
#include "mainfrm.h"
#include "part_emt.h"
#include "texture.h"
#include "utils.h"
#include "viewerscene.h"
#include "w3dview.h"

IMPLEMENT_DYNCREATE(CW3DViewDoc, CDocument)

// clang-format off
BEGIN_MESSAGE_MAP(CW3DViewDoc, CDocument)
END_MESSAGE_MAP()
// clang-format on

CW3DViewDoc::CW3DViewDoc() :
    m_scene(nullptr),
    m_textureScene(nullptr),
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
    BOOL result = CDocument::OnNewDocument();

    if (result) {
        m_loadedFiles.Delete_All();
        m_forceCameraReset = true;

        if (m_scene != nullptr && m_model != nullptr) {
            RemoveRenderObject(m_model);
        }

        if (m_scene != nullptr) {
            m_scene->Remove_All_LOD_Objects();
            m_scene->Set_Fog_Color(m_fogColor);
        }

        if (m_model != nullptr) {
            Ref_Ptr_Release(m_animation);
            Ref_Ptr_Release(m_model);
        }

        CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

        if (frame != nullptr) {
            CDataTreeView *view = (CDataTreeView *)frame->m_splitter.GetPane(0, 0);

            if (view != nullptr) {
                // TODO xxxx
                // view->GetTreeCtrl()->DeleteItem(TVI_ROOT);
                // view->AddCategories();
            }
        }

        Free();
        W3DAssetManager::Get_Instance()->Free_Assets();

        return TRUE;
    }

    return result;
}

BOOL CW3DViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    BOOL result = CDocument::OnOpenDocument(lpszPathName);
    if (result) {
        CGraphicView *view = GetCurrentGraphicView();

        if (view != nullptr) {
            view->EnableRendering(false);
        }

        LoadFile(lpszPathName);

        CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

        if (frame != nullptr) {
            CDataTreeView *tree = (CDataTreeView *)frame->m_splitter.GetPane(0, 0);

            // TODO xxxx
            // if (tree != nullptr) {
            //     tree->AddRenderObjects();
            // }
        }

        if (view != nullptr) {
            view->EnableRendering(true);
        }

        return TRUE;
    }

    return result;
}

void CW3DViewDoc::Free()
{
    if (m_textureScene != nullptr) {
        if (m_backgroundBitmap != nullptr) {
            m_backgroundBitmap->Remove();
        }

        Ref_Ptr_Release(m_textureScene);
    }

    if (m_backgroundScene != nullptr) {
        if (m_backgroundObject != nullptr) {
            m_backgroundObject->Remove();
        }

        Ref_Ptr_Release(m_backgroundScene);
    }

    if (m_scene != nullptr) {
        if (m_model != nullptr) {
            RemoveRenderObject(m_model);
        }

        if (m_light != nullptr) {
            RemoveRenderObject(m_light);
        }

        m_scene->Remove_All_LOD_Objects();
        Ref_Ptr_Release(m_scene);
    }

    Ref_Ptr_Release(m_textureCamera);
    Ref_Ptr_Release(m_docCamera);
    Ref_Ptr_Release(m_backgroundBitmap);
    Ref_Ptr_Release(m_light);

    if (m_model != nullptr) {
        Ref_Ptr_Release(m_animation);
        Ref_Ptr_Release(m_model);
    }
}

void CW3DViewDoc::RemoveRenderObject(RenderObjClass *robj)
{
    if (robj == nullptr) {
        robj = m_model;
    }

    while (robj->Get_Num_Sub_Objects() > 0) {
        RenderObjClass *o = robj->Get_Sub_Object(0);

        if (o != nullptr) {
            RemoveRenderObject(o);
            Ref_Ptr_Release(o);
        }
    }

    if (robj != nullptr && robj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
        ParticleEmitterClass *emitter = (ParticleEmitterClass *)robj;
        emitter->Stop();
        emitter->Remove_Buffer_From_Scene();
    }

    if (m_scene != nullptr) {
        robj->Remove();
    }
}

void CW3DViewDoc::LoadFile(CString pathName)
{
    if (m_scene == nullptr) {
        Initialize();
    }

    const char *filename = pathName;

    m_filePath = GetFilePath(pathName);
    m_loadedFiles.Add(pathName);

    CGraphicView *view = GetCurrentGraphicView();

    if (view != nullptr) {
        view->EnableRendering(false);
    }

    if (strrchr(filename, '\\')) {
        CString str = filename;
        str = str.Left(strrchr(filename, '\\') - filename);
        SetCurrentDirectory(str);
        g_theSimpleFileFactory->Append_Sub_Directory(str);
    }

    const char *c = strrchr(filename, '.');

    if (lstrcmpi(c, ".tga") && lstrcmpi(c, ".dds")) {
        W3DAssetManager::Get_Instance()->Load_3D_Assets(GetFilenameFromPath(filename));
    } else {
        TextureClass *texture = W3DAssetManager::Get_Instance()->Get_Texture(GetFilenameFromPath(filename));
        Ref_Ptr_Release(texture);
    }

    if (view != nullptr) {
        view->EnableRendering(true);
    }
}

void CW3DViewDoc::Initialize()
{
    if (m_scene == nullptr) {
        m_scene = new ViewerSceneClass();

        if (m_scene != nullptr) {
            m_scene->Set_Ambient_Light(Vector3(0.5f, 0.5f, 0.5f));
            m_scene->Set_Fog_Color(m_fogColor);
            m_light = new LightClass();

            if (m_light != nullptr) {
                m_light->Set_Position(Vector3(0.0f, 5000.0f, 3000.0f));
                m_light->Set_Intensity(1.0f);
                m_light->Set_Force_Visible(true);
                m_light->Set_Far_Attenuation_Range(1000000.0f, 1000000.0f);
                m_light->Set_Ambient(Vector3(0.0f, 0.0f, 0.0f));
                m_light->Set_Diffuse(Vector3(1.0f, 1.0f, 1.0f));
                m_light->Set_Specular(Vector3(1.0f, 1.0f, 1.0f));
                m_scene->Add_Render_Object(m_light);
            }
        }

        m_textureScene = new SimpleSceneClass();
        m_backgroundScene = new SimpleSceneClass();

        if (m_backgroundScene != nullptr) {
            m_backgroundScene->Set_Ambient_Light(Vector3(0.5f, 0.5f, 0.5f));
        }

        m_docCamera = new CameraClass();

        if (m_docCamera != nullptr) {
            m_docCamera->Set_View_Plane(Vector2(1.0f, 1.0f), Vector2(-1.0f, -1.0f));
            m_docCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
            m_docCamera->Set_Clip_Planes(0.1f, 10.0f);
        }

        m_textureCamera = new CameraClass();

        if (m_textureCamera != nullptr) {
            m_textureCamera->Set_View_Plane(Vector2(1.0f, 1.0f), Vector2(-1.0f, -1.0f));
            m_textureCamera->Set_Position(Vector3(0.0f, 0.0f, 1.0f));
            m_textureCamera->Set_Clip_Planes(0.1f, 10.0f);
        }

        CString path1 = theApp.GetProfileString("Config", "TexturePath1", "");
        CString path2 = theApp.GetProfileString("Config", "TexturePath2", "");

        if (m_texturePath1 != path1) {
            if (lstrlen(path1) > 0) {
                g_theSimpleFileFactory->Append_Sub_Directory(path1);
            }

            m_texturePath1 = path1;
            theApp.WriteProfileString("Config", "TexturePath1", m_texturePath1);
        }

        if (m_texturePath2 != path2) {
            if (lstrlen(path2) > 0) {
                g_theSimpleFileFactory->Append_Sub_Directory(path2);
            }

            m_texturePath2 = path2;
            theApp.WriteProfileString("Config", "TexturePath2", m_texturePath2);
        }

        if (m_fogEnabled) {
            m_scene->Set_Fog_Enable(true);
        }
    }

    ReadSettings();
    m_initialized = true;
}

void CW3DViewDoc::ReadSettings()
{
    m_useManualFov = theApp.GetProfileInt("Config", "UseManualFOV", 0) == 1;
    m_useManualClipPlanes = theApp.GetProfileInt("Config", "UseManualClipPlanes", 0) == 1;

    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        CGraphicView *view = (CGraphicView *)frame->m_splitter.GetPane(0, 1);

        if (view != nullptr) {
            CameraClass *camera = view->m_camera;

            if (camera != nullptr) {
                if (m_useManualFov) {
                    float hfov = atof(theApp.GetProfileString("Config", "hfov", "0"));
                    float vfov = atof(theApp.GetProfileString("Config", "vfov", "0"));
                    camera->Set_View_Plane(hfov, vfov);
                }

                if (m_useManualClipPlanes) {
                    float znear = atof(theApp.GetProfileString("Config", "znear", "0.1F"));
                    float zfar = atof(theApp.GetProfileString("Config", "zfar", "100.0F"));
                    camera->Set_Clip_Planes(znear, zfar);

                    if (m_scene != nullptr) {
                        m_scene->Set_Fog_Range(znear, zfar);
                        m_scene->Update_Fog_Range();
                    }
                }
            }
        }
    }
}

CDataTreeView *CW3DViewDoc::GetDataTreeView()
{
    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        return (CDataTreeView *)frame->m_splitter.GetPane(0, 0);
    } else {
        return nullptr;
    }
}

CGraphicView *CW3DViewDoc::GetGraphicView()
{
    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        return (CGraphicView *)frame->m_splitter.GetPane(0, 1);
    } else {
        return nullptr;
    }
}

void CW3DViewDoc::UpdateParticleCount()
{
    int count = GetNumParticles(nullptr);

    ((CMainFrame *)AfxGetMainWnd())->UpdateParticleCount(count);
}

int CW3DViewDoc::GetNumParticles(RenderObjClass *robj)
{
    int count = 0;

    if (robj == nullptr) {
        robj = m_model;
    }

    if (robj != nullptr) {
        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *o = robj->Get_Sub_Object(i);

            if (o != nullptr) {
                count += GetNumParticles(o);
                o->Release_Ref();
            }
        }

        if (robj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {

            if (((ParticleEmitterClass *)robj)->Peek_Buffer() != nullptr) {
                count += ((ParticleEmitterClass *)robj)->Peek_Buffer()->Get_Particle_Count();
            }
        }
    }

    return count;
}

#if 0
void CW3DViewDoc::Deselect()
{
    // TODO
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
