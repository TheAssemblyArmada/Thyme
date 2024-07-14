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
#include "datatreeview.h"
#include "emitterinstancelist.h"
#include "ffactory.h"
#include "graphicview.h"
#include "hanim.h"
#include "hlod.h"
#include "light.h"
#include "mainfrm.h"
#include "mesh.h"
#include "meshmdl.h"
#include "part_emt.h"
#include "resource.h"
#include "restrictedfiledialog.h"
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
    m_model(nullptr),
    m_backgroundObject(nullptr),
    m_animation(nullptr),
    m_light(nullptr),
    m_backgroundBitmap(nullptr),
    m_textureCamera(nullptr),
    m_docCamera(nullptr),
    m_fogColor(0.5f, 0.5f, 0.5f),
    m_frameCount(0.0f),
    m_time(0.0f),
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

        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

        if (frame != nullptr) {
            CDataTreeView *view = static_cast<CDataTreeView *>(frame->m_splitter.GetPane(0, 0));

            if (view != nullptr) {
                view->GetTreeCtrl().DeleteItem(TVI_ROOT);
                view->AddCategories();
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

        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

        if (frame != nullptr) {
            CDataTreeView *tree = static_cast<CDataTreeView *>(frame->m_splitter.GetPane(0, 0));

            if (tree != nullptr) {
                tree->AddRenderObjects();
            }
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
        ParticleEmitterClass *emitter = static_cast<ParticleEmitterClass *>(robj);
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

    if (strrchr(filename, '\\') != nullptr) {
        CString str = filename;
        str = str.Left(strrchr(filename, '\\') - filename);
        SetCurrentDirectory(str);
        g_theSimpleFileFactory->Append_Sub_Directory(str);
    }

    const char *c = strrchr(filename, '.');

    if (lstrcmpi(c, ".tga") != 0 && lstrcmpi(c, ".dds") != 0) {
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
            m_textureCamera->Set_View_Plane(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
            m_textureCamera->Set_Position(Vector3(0.0f, 0.0f, 1.0f));
            m_textureCamera->Set_Clip_Planes(0.1f, 10.0f);
        }

        CString path1 = theApp.GetProfileString("Config", "TexturePath1", "");
        CString path2 = theApp.GetProfileString("Config", "TexturePath2", "");

        SetTexturePath1(path1);
        SetTexturePath2(path2);

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

    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

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
    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        return static_cast<CDataTreeView *>(frame->m_splitter.GetPane(0, 0));
    } else {
        return nullptr;
    }
}

CGraphicView *CW3DViewDoc::GetGraphicView()
{
    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        return static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));
    } else {
        return nullptr;
    }
}

void CW3DViewDoc::UpdateParticleCount()
{
    int count = GetNumParticles(nullptr);

    static_cast<CMainFrame *>(AfxGetMainWnd())->UpdateParticleCount(count);
}

int CW3DViewDoc::GetNumParticles(RenderObjClass *robj)
{
    int objcount = 0;

    if (robj == nullptr) {
        robj = m_model;
    }

    if (robj != nullptr) {
        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *o = robj->Get_Sub_Object(i);

            if (o != nullptr) {
                objcount += GetNumParticles(o);
                o->Release_Ref();
            }
        }

        if (robj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {

            if (static_cast<ParticleEmitterClass *>(robj)->Peek_Buffer() != nullptr) {
                objcount += static_cast<ParticleEmitterClass *>(robj)->Peek_Buffer()->Get_Particle_Count();
            }
        }
    }

    return objcount;
}

void CW3DViewDoc::SetRenderObject(RenderObjClass *robj, bool useRegularCameraReset, bool resetCamera, bool preserveModel)
{
    if (m_scene == nullptr) {
        return;
    }

    Ref_Ptr_Release(m_animation);

    if (!preserveModel && m_model != nullptr) {
        RemoveRenderObject(m_model);
        Ref_Ptr_Release(m_model);
    }

    m_scene->Remove_All_LOD_Objects();

    if (robj == nullptr) {
        return;
    }

    if (preserveModel) {
        robj->Set_Animation();
        Matrix3D tm(true);
        robj->Set_Transform(tm);
        SceneClass *scene;

        if (robj->Class_ID() == RenderObjClass::CLASSID_BITMAP2D) {
            scene = m_textureScene;
        } else {
            m_scene->Remove_All_LOD_Objects();
            scene = m_scene;
        }

        scene->Add_Render_Object(robj);

        if (m_scene->Get_Auto_Switch_LOD() && robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            robj->Set_LOD_Level(0);
        }

        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

        if (frame == nullptr) {
            return;
        }

        CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

        if (view == nullptr) {
            return;
        }

        bool b = useRegularCameraReset ? this->m_resetCamera : resetCamera;

        if (b || this->m_forceCameraReset) {
            view->ResetCamera(robj);
            m_forceCameraReset = false;
        }

        return;
    }

    robj->Set_Animation();
    captainslog_dbgassert(m_model == nullptr, "Expected nullptr, otherwise leaks"); // Thyme specific
    m_model = robj;
    m_model->Add_Ref();
    Matrix3D tm(true);
    m_model->Set_Transform(tm);
    SceneClass *scene;

    if (robj->Class_ID() == RenderObjClass::CLASSID_BITMAP2D) {
        scene = m_textureScene;
    } else {
        scene = m_scene;
    }

    scene->Add_Render_Object(robj);

    if (m_scene->Get_Auto_Switch_LOD() && robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        robj->Set_LOD_Level(0);
    }

    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame == nullptr) {
        return;
    }

    CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

    if (view == nullptr) {
        return;
    }

    bool b = useRegularCameraReset ? this->m_resetCamera : resetCamera;

    if (b || this->m_forceCameraReset) {
        view->ResetCamera(robj);
        m_forceCameraReset = false;
    }
}

void CW3DViewDoc::SetParticleEmitter(ParticleEmitterClass *emitter, bool useRegularCameraReset, bool resetCamera)
{
    if (m_scene != nullptr) {
        Ref_Ptr_Release(m_animation);

        if (m_model != nullptr) {
            RemoveRenderObject(m_model);
            Ref_Ptr_Release(m_model);
        }

        m_scene->Remove_All_LOD_Objects();

        if (emitter != nullptr) {
            Matrix3D tm(true);
            emitter->Set_Transform(tm);
            RenderObjClass *robj = emitter;
            Ref_Ptr_Set(m_model, robj);
            m_scene->Add_Render_Object(emitter);
            emitter->Restart();

            CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

            if (frame != nullptr) {
                CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

                if (view != nullptr) {

                    bool b = useRegularCameraReset ? this->m_resetCamera : resetCamera;

                    if (b || this->m_forceCameraReset) {
                        view->ResetParticleEmitterCamera(emitter);
                        m_forceCameraReset = false;
                    }
                }
            }
        }
    }
}

bool GetCameraBoneTransform(RenderObjClass *robj, Matrix3D *tm)
{
    if (robj != nullptr) {
        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *o = robj->Get_Sub_Object(i);

            if (GetCameraBoneTransform(o, tm)) {
                o->Release_Ref();
                return true;
            }
        }

        int index = robj->Get_Bone_Index("CAMERA");

        if (index > 0) {
            *tm = robj->Get_Bone_Transform(index);
            return true;
        }
    }

    return false;
}

void CW3DViewDoc::SetAnimationByName(RenderObjClass *robj, const char *name, bool useRegularCameraReset, bool resetCamera)
{
    if (m_scene == nullptr || robj == nullptr || name == nullptr) {
        return;
    }

    SetRenderObject(robj, true, true, false);
    Ref_Ptr_Release(m_animation);
    m_animation = W3DAssetManager::Get_Instance()->Get_HAnim(name);
    m_frameCount = 0.0f;
    m_time = 0.0f;

    if (m_model != nullptr) {
        m_model->Set_Animation(m_animation, 0.0f, 0);
        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

        if (frame != nullptr) {
            CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

            if (view != nullptr) {

                bool b;

                if (useRegularCameraReset) {
                    b = m_resetCamera;
                } else {
                    b = resetCamera;
                }

                if (b || this->m_forceCameraReset) {
                    view->ResetCamera(robj);
                    m_forceCameraReset = false;
                }

                static_cast<CMainFrame *>(AfxGetMainWnd())->PostMessage(WM_COMMAND, ID_ANIMATION_PLAY);
            }
        }
    }

    if (m_animateCamera) {
        if (m_model != nullptr) {
            Matrix3D tm(true);

            if (GetCameraBoneTransform(m_model, &tm)) {
                Matrix3D m;
                m[0].X = 0.0f;
                m[0].Y = 0.0f;
                m[0].Z = -1.0f;
                m[0].W = 0.0f;
                m[1].X = -1.0f;
                m[1].Y = 0.0f;
                m[1].Z = 0.0f;
                m[1].W = 0.0f;
                m[2].X = 0.0f;
                m[2].Y = 1.0f;
                m[2].Z = 0.0f;
                m[2].W = 0.0f;
                tm = tm * m;
                CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

                if (frame != nullptr) {
                    CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

                    if (view != nullptr) {
                        view->m_camera->Set_Transform(tm);
                    }
                }
            }
        }
    }
}

void CW3DViewDoc::Deselect()
{
    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        CDataTreeView *view = static_cast<CDataTreeView *>(frame->m_splitter.GetPane(0, 0));

        if (view != nullptr) {
            view->Select(0);
        }
    }
}

void CW3DViewDoc::UpdateFrameCount()
{
    if (m_animation != nullptr) {
        m_frameCount = 0.0f;
        m_time = 0.0f;
        float rate = m_animation->Get_Frame_Rate();
        float speed = GetCurrentGraphicView()->m_animationSpeed;
        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());
        frame->UpdateFrameCount(0, m_animation->Get_Num_Frames() - 1, speed * rate);
    }
}

void CW3DViewDoc::UpdateAnimation(float tm)
{
    if (m_model != nullptr) {
        if (m_animation != nullptr) {
            int frames = m_animation->Get_Num_Frames();
            float rate = m_animation->Get_Frame_Rate();
            float f1 = (frames - 1) / rate;
            float f2 = tm + m_time;
            m_time = f2;

            if (f1 < f2) {
                m_time = f2 - f1;
            }

            m_frameCount = rate * m_time;
            float speed = GetCurrentGraphicView()->m_animationSpeed;
            CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());
            frame->UpdateFrameCount(m_frameCount, frames - 1, speed * rate);

            if (m_blendFrames) {
                m_model->Set_Animation(m_animation, m_frameCount);
            } else {
                m_model->Set_Animation(m_animation, (int)m_frameCount);
            }

            if (m_animateCamera) {
                if (m_model != nullptr) {
                    Matrix3D tm(true);

                    if (GetCameraBoneTransform(m_model, &tm)) {
                        Matrix3D m;
                        m[0].X = 0.0f;
                        m[0].Y = 0.0f;
                        m[0].Z = -1.0f;
                        m[0].W = 0.0f;
                        m[1].X = -1.0f;
                        m[1].Y = 0.0f;
                        m[1].Z = 0.0f;
                        m[1].W = 0.0f;
                        m[2].X = 0.0f;
                        m[2].Y = 1.0f;
                        m[2].Z = 0.0f;
                        m[2].W = 0.0f;
                        tm = tm * m;
                        CMainFrame *frame2 = static_cast<CMainFrame *>(AfxGetMainWnd());

                        if (frame2 != nullptr) {
                            CGraphicView *view = static_cast<CGraphicView *>(frame2->m_splitter.GetPane(0, 1));

                            if (view != nullptr) {
                                view->m_camera->Set_Transform(tm);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CW3DViewDoc::AnimateCamera(bool animate)
{
    m_animateCamera = animate;

    if (!animate) {
        CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());
        frame->SendMessage(WM_COMMAND, ID_CAMERA_RESET);
    }
}

void CW3DViewDoc::EnableFog(bool enable)
{
    if (m_scene != nullptr) {
        m_scene->Set_Fog_Enable(enable);
        m_fogEnabled = enable;
    }
}

void CW3DViewDoc::OnStep(int step)
{
    if (m_model != nullptr) {
        if (m_animation != nullptr) {
            int frames = m_animation->Get_Num_Frames();
            m_frameCount += step;

            if (m_frameCount < frames) {
                if (m_frameCount < 0.0f) {
                    m_frameCount = frames - 1;
                }
            } else {
                m_frameCount = 0.0f;
                m_time = 0.0f;
            }

            float speed = GetCurrentGraphicView()->m_animationSpeed;
            CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());
            float rate = m_animation->Get_Frame_Rate();
            frame->UpdateFrameCount(m_frameCount, frames - 1, speed * rate);

            if (m_blendFrames) {
                m_model->Set_Animation(m_animation, m_frameCount);
            } else {
                m_model->Set_Animation(m_animation, (int)m_frameCount);
            }

            if (m_animateCamera) {
                if (m_model != nullptr) {
                    Matrix3D tm(true);

                    if (GetCameraBoneTransform(m_model, &tm)) {
                        Matrix3D m;
                        m[0].X = 0.0f;
                        m[0].Y = 0.0f;
                        m[0].Z = -1.0f;
                        m[0].W = 0.0f;
                        m[1].X = -1.0f;
                        m[1].Y = 0.0f;
                        m[1].Z = 0.0f;
                        m[1].W = 0.0f;
                        m[2].X = 0.0f;
                        m[2].Y = 1.0f;
                        m[2].Z = 0.0f;
                        m[2].W = 0.0f;
                        tm = tm * m;
                        CMainFrame *frame2 = static_cast<CMainFrame *>(AfxGetMainWnd());

                        if (frame2 != nullptr) {
                            CGraphicView *view = static_cast<CGraphicView *>(frame2->m_splitter.GetPane(0, 1));

                            if (view != nullptr) {
                                view->m_camera->Set_Transform(tm);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CW3DViewDoc::ToggleAlternateMaterials(RenderObjClass *robj)
{
    if (robj == nullptr) {
        robj = m_model;
    }

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_MESH) {
            MeshClass *mesh = static_cast<MeshClass *>(robj);
            MeshModelClass *model = mesh->Get_Model();
            model->Enable_Alternate_Material_Description(!model->Is_Alternate_Material_Description_Enabled());
        }

        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            ToggleAlternateMaterials(robj->Get_Sub_Object(i));
        }
    }
}

void CW3DViewDoc::AddEmittersToList(EmitterInstanceList *list, const char *name, RenderObjClass *robj)
{
    if (robj == nullptr) {
        robj = m_model;
    }

    for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
        RenderObjClass *o = robj->Get_Sub_Object(i);

        if (o != nullptr) {
            AddEmittersToList(list, name, o);
            o->Release_Ref();
        }
    }

    if (robj != nullptr && robj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
        if (lstrcmpi(name, robj->Get_Name()) == 0) {
            list->Add_Emitter(static_cast<ParticleEmitterClass *>(robj));
        }
    }
}

bool CW3DViewDoc::ExportEmitter()
{
    bool ret = false;

    if (m_model == nullptr || m_model->Class_ID() != RenderObjClass::CLASSID_PARTICLEEMITTER) {
        return false;
    }

    CDataTreeView *view = nullptr;
    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        view = static_cast<CDataTreeView *>(frame->m_splitter.GetPane(0, 0));
    }

    CString str = view->GetSelectedItemName();
    str += ".w3d";

    RestrictedFileDialogClass dlg(FALSE,
        ".w3d",
        str,
        OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Westwood 3D Files (*.w3d)|*.~xyzabc||",
        AfxGetMainWnd());

    dlg.m_ofn.lpstrTitle = "Export Emitter";

    if (dlg.DoModal() == IDOK) {
        ret = SaveEmitter(dlg.GetPathName());
    }

    return ret;
}

bool CW3DViewDoc::SaveEmitter(const char *name)
{
    bool ret = false;
    ParticleEmitterPrototypeClass *proto =
        static_cast<ParticleEmitterPrototypeClass *>(W3DAssetManager::Get_Instance()->Find_Prototype(m_model->Get_Name()));

    if (proto != nullptr) {
        ParticleEmitterDefClass *def = proto->Get_Definition();

        if (def != nullptr) {
            FileClass *file = g_theFileFactory->Get_File(name);

            if (file != nullptr) {
                file->Open(FM_WRITE);
                ChunkSaveClass csave(file);
                ret = def->Save_W3D(csave) == W3D_ERROR_OK;
                file->Close();
                g_theFileFactory->Return_File(file);
            }
        }
    }

    return ret;
}

void CW3DViewDoc::SetTexturePath1(const char *path)
{
    if (m_texturePath1 != path) {
        if (strlen(path) > 0) {
            g_theSimpleFileFactory->Append_Sub_Directory(path);
        }

        m_texturePath1 = path;
        theApp.WriteProfileString("Config", "TexturePath1", m_texturePath1);
    }
}

void CW3DViewDoc::SetTexturePath2(const char *path)
{
    if (m_texturePath2 != path) {
        if (strlen(path) > 0) {
            g_theSimpleFileFactory->Append_Sub_Directory(path);
        }

        m_texturePath2 = path;
        theApp.WriteProfileString("Config", "TexturePath2", m_texturePath2);
    }
}

void CW3DViewDoc::CreateBackgroundBitmap(const char *name)
{
    if (m_backgroundScene != nullptr) {
        if (m_backgroundBitmap != nullptr) {
            m_backgroundBitmap->Remove();
            Ref_Ptr_Release(m_backgroundBitmap);
        }

        if (name != nullptr && strcmpi(m_backgroundBitmapName, name) != 0) {
            m_backgroundBitmap = new Bitmap2D(name, 0.5, 0.5, true, false, false, -1, -1, false);

            if (m_backgroundBitmap != nullptr) {
                m_textureScene->Add_Render_Object(m_backgroundBitmap);
            }
        }

        m_backgroundBitmapName = name;
    }
}

void CW3DViewDoc::SetBackgroundObject(const char *name)
{
    if (m_backgroundScene != nullptr && m_docCamera != nullptr) {
        if (m_backgroundObject != nullptr) {
            m_backgroundObject->Remove();
            Ref_Ptr_Release(m_backgroundObject);
        }

        if (name != nullptr) {
            m_backgroundObject = W3DAssetManager::Get_Instance()->Create_Render_Obj(name);

            if (m_backgroundObject != nullptr) {
                m_backgroundObject->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
                float plane = m_backgroundObject->Get_Bounding_Sphere().Radius * 4.0f;

                CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

                if (frame != nullptr) {
                    CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

                    if (view != nullptr) {
                        m_docCamera->Set_Transform(view->m_camera->Get_Transform());
                    }
                }

                m_docCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
                m_docCamera->Set_Clip_Planes(1.0f, plane);
                m_backgroundScene->Add_Render_Object(m_backgroundObject);
            }
        }

        m_backgroundObjectName = name;
    }
}

void CW3DViewDoc::SaveCameraSettings()
{
    theApp.WriteProfileInt("Config", "UseManualFOV", m_useManualFov);
    theApp.WriteProfileInt("Config", "UseManualClipPlanes", m_useManualClipPlanes);

    CameraClass *camera = GetCurrentGraphicView()->m_camera;

    if (camera != nullptr) {
        float hfov = camera->Get_Horizontal_FOV();
        float vfov = camera->Get_Vertical_FOV();
        float zfar;
        float znear;
        camera->Get_Clip_Planes(znear, zfar);
        CString shfov;
        CString svfov;
        CString sznear;
        CString szfar;
        shfov.Format("%f", hfov);
        svfov.Format("%f", vfov);
        sznear.Format("%f", znear);
        szfar.Format("%f", zfar);
        theApp.WriteProfileString("Config", "hfov", shfov);
        theApp.WriteProfileString("Config", "vfov", svfov);
        theApp.WriteProfileString("Config", "znear", sznear);
        theApp.WriteProfileString("Config", "zfar", szfar);
    }
}

void CW3DViewDoc::SetFogColor(Vector3 &color)
{
    m_fogColor = color;

    if (m_scene != nullptr) {
        m_scene->Set_Fog_Color(color);
    }
}

PrototypeClass *CW3DViewDoc::GenerateLOD(const char *name, int type)
{
    RenderObjIterator *iter = W3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator();

    if (iter == nullptr) {
        return nullptr;
    }

    int count = 0;
    int index = 0xFFFF;
    char c = 'Z';

    for (iter->First(); !iter->Is_Done(); iter->Next()) {
        const char *robjname = iter->Current_Item_Name();

        if (W3DAssetManager::Get_Instance()->Render_Obj_Exists(robjname)
            && iter->Current_Item_Class_ID() == RenderObjClass::CLASSID_HLOD && strstr(robjname, name) == robjname) {
            const char *str = &robjname[strlen(name)];

            if (type != 0) {
                if (type == 1) {
                    if (*str >= 'a' && *str <= 'z' || *str >= 'A' && *str <= 'Z') {
                        char c1 = str[1];

                        if (!c1) {
                            count++;

                            if (type != 0) {
                                char c2 = toupper(robjname[strlen(robjname) - 1]);
                                if (c >= c2) {
                                    c = c2;
                                }
                            } else {
                                int i2 = atoi(&robjname[strlen(robjname) - 1]);
                                if (index >= i2) {
                                    index = i2;
                                }
                            }
                        }
                    }
                }
            } else if (*str == 'L' || *str == 'l') {
                if (str[1] >= '0' && str[1] <= '9') {
                    char c1 = str[2];

                    if (!c1) {
                        count++;

                        if (type != 0) {
                            char c2 = toupper(robjname[strlen(robjname) - 1]);
                            if (c >= c2) {
                                c = c2;
                            }
                        } else {
                            int i2 = atoi(&robjname[strlen(robjname) - 1]);
                            if (index >= i2) {
                                index = i2;
                            }
                        }
                    }
                }
            }
        }
    }

    RenderObjClass **objs = new RenderObjClass *[count];
    RenderObjClass **objs2 = &objs[count - 1];

    for (int i = 0; i < count; i++) {
        StringClass lodname;

        if (type) {
            lodname.Format("%s%c", name, c++);
        } else {
            lodname.Format("%sL%d", name, i + index);
        }

        *objs2 = W3DAssetManager::Get_Instance()->Create_Render_Obj(lodname);
        objs2--;
    }

    HLodClass *hlod = new HLodClass(name, objs, count);
    HLodDefClass *def = new HLodDefClass(*hlod);
    HLodPrototypeClass *proto = new HLodPrototypeClass(def);
    Ref_Ptr_Release(hlod);

    for (int i = 0; i < count; i++) {
        Ref_Ptr_Release(objs[i]);
    }

    delete[] objs;
    W3DAssetManager::Get_Instance()->Release_Render_Obj_Iterator(iter);
    return proto;
}

bool CW3DViewDoc::ExportLOD()
{
    if (m_model == nullptr || m_model->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    CString str = GetDataTreeView()->GetSelectedItemName();
    str += ".w3d";

    RestrictedFileDialogClass dlg(FALSE,
        ".w3d",
        str,
        OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Westwood 3D Files (*.w3d)|*.~xyzabc||",
        AfxGetMainWnd());

    dlg.m_ofn.lpstrTitle = "Export LOD";

    if (dlg.DoModal() == IDOK) {
        return SaveLOD(dlg.GetPathName());
    }

    return false;
}

bool CW3DViewDoc::SaveLOD(const char *name)
{
    bool ret = false;
    HLodPrototypeClass *proto =
        static_cast<HLodPrototypeClass *>(W3DAssetManager::Get_Instance()->Find_Prototype(m_model->Get_Name()));

    if (proto != nullptr) {
        if (proto->Get_Definition() != nullptr) {
            FileClass *f = g_theFileFactory->Get_File(name);

            if (f != nullptr) {
                f->Open(FM_WRITE);
                ChunkSaveClass csave(f);
                ret = proto->Get_Definition()->Save(csave) == W3D_ERROR_OK;
                f->Close();
                g_theFileFactory->Return_File(f);
            }
        }
    }

    return ret;
}

void CW3DViewDoc::CreateHLodPrototype(HLodClass &lod)
{
    HLodDefClass *def = new HLodDefClass(lod);
    HLodPrototypeClass *proto = new HLodPrototypeClass(def);
    W3DAssetManager::Get_Instance()->Remove_Prototype(def->Get_Name());
    W3DAssetManager::Get_Instance()->Add_Prototype(proto);
}

void CW3DViewDoc::SetLODLevel(int lod, RenderObjClass *robj)
{
    if (robj == nullptr) {
        robj = m_model;
    }

    if (robj != nullptr) {
        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *o = robj->Get_Sub_Object(i);

            if (o != nullptr) {
                SetLODLevel(lod, o);
                o->Release_Ref();
            }
        }

        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            robj->Set_LOD_Level(lod + robj->Get_LOD_Level());
        }
    }
}
