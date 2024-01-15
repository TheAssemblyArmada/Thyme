/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Projected Shadow Code
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dprojectedshadow.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "camera.h"
#include "colmath.h"
#include "drawable.h"
#include "drawmodule.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "frustum.h"
#include "globaldata.h"
#include "object.h"
#include "partitionmanager.h"
#include "rendobj.h"
#include "rinfo.h"
#include "simpleobjectiterator.h"
#include "terrainlogic.h"
#include "texproject.h"
#include "texture.h"
#include "w3dmodeldraw.h"
#include "w3dvolumetricshadow.h"
#include "worldheightmap.h"
#include <cstring>
#ifdef BUILD_WITH_D3D8
#include <d3dx8math.h>
#endif

using std::memset;
using std::strcpy;
using std::strlen;
using std::strncpy;

float g_heightMapScale = 0.645f;

int g_shadowDecalVertexSize = 32768;
int g_shadowDecalIndexSize = 65536;

#ifndef GAME_DLL
#ifdef BUILD_WITH_D3D8
IDirect3DVertexBuffer8 *g_shadowDecalVertexBufferD3D;
IDirect3DIndexBuffer8 *g_shadowDecalIndexBufferD3D;
#endif
W3DProjectedShadowManager *g_theW3DProjectedShadowManager = nullptr;
ProjectedShadowManager *g_theProjectedShadowManager = nullptr;
int g_nShadowDecalVertsInBuf;
int g_nShadowDecalStartBatchVertex;
int g_nShadowDecalIndicesInBuf;
int g_nShadowDecalStartBatchIndex;
int g_nShadowDecalPolysInBatch;
int g_nShadowDecalVertsInBatch;
int g_drawStartX;
int g_drawEdgeX;
int g_drawStartY;
int g_drawEdgeY;
#else
#ifdef BUILD_WITH_D3D8
extern IDirect3DVertexBuffer8 *&g_shadowDecalVertexBufferD3D;
extern IDirect3DIndexBuffer8 *&g_shadowDecalIndexBufferD3D;
#endif
extern int &g_nShadowDecalVertsInBuf;
extern int &g_nShadowDecalStartBatchVertex;
extern int &g_nShadowDecalIndicesInBuf;
extern int &g_nShadowDecalStartBatchIndex;
extern int &g_nShadowDecalPolysInBatch;
extern int &g_nShadowDecalVertsInBatch;
extern int &g_drawStartX;
extern int &g_drawEdgeX;
extern int &g_drawStartY;
extern int &g_drawEdgeY;
#endif

struct SHADOW_DECAL_VERTEX
{
    float x;
    float y;
    float z;
    int c;
    float u;
    float v;
};

// BUGFIX init all members
W3DShadowTexture::W3DShadowTexture() : m_texture(nullptr)
{
    m_lastLightPosition.Set(0.0f, 0.0f, 0.0f);
    m_lastObjectOrientation.Make_Identity();
    m_shadowUV[0].Set(1.0f, 0.0f, 0.0f);
    m_shadowUV[1].Set(0.0f, -1.0f, 0.0f);
}

int W3DShadowTexture::Init(RenderObjClass *robj)
{
    TextureClass *tex = g_theW3DProjectedShadowManager->Get_Render_Target();
    SurfaceClass::SurfaceDescription surface_desc;
    tex->Get_Level_Description(surface_desc, 0);
    Set_Texture(new TextureClass(
        surface_desc.width, surface_desc.height, surface_desc.format, MIP_LEVELS_1, POOL_MANAGED, false, true));
    return 1;
}

void W3DShadowTexture::Update_Bounds(Vector3 &light_pos, RenderObjClass *robj)
{
    AABoxClass *box = &m_areaEffectBox;
    Vector3 objPos;
    Vector3 Corners[8];
    Vector3 lightRay;

    objPos = robj->Get_Position();
    *box = robj->Get_Bounding_Box();

    float floorZ = objPos.Z - 2.0f;

    Corners[0] = box->m_center + box->m_extent;
    Corners[1] = Corners[0];
    Corners[1].X = Corners[1].X - 2.0f * box->m_extent.X;
    Corners[2] = Corners[1];
    Corners[2].Y = Corners[2].Y - 2.0f * box->m_extent.Y;
    Corners[3] = Corners[2];
    Corners[3].X = 2.0f * box->m_extent.X + Corners[3].X;

    lightRay = Corners[0] - light_pos;
    float f1 = 1.0f / lightRay.Length();
    lightRay *= f1;
    float f2 = GameMath::Fabs((Corners[0].Z - floorZ) / lightRay.Z);
    Corners[4] = Corners[0] + (lightRay * f2);
    f2 *= f1;

    lightRay = Corners[1] - light_pos;
    f1 = 1.0f / lightRay.Length();
    lightRay *= f1;
    float f3 = GameMath::Fabs((Corners[1].Z - floorZ) / lightRay.Z);
    Corners[5] = Corners[1] + (lightRay * f3);
    f3 *= f1;

    if (f3 > f2) {
        f2 = f3;
    }

    lightRay = Corners[2] - light_pos;
    f1 = 1.0f / lightRay.Length();
    lightRay *= f1;
    f2 = GameMath::Fabs((Corners[2].Z - floorZ) / lightRay.Z);
    Corners[6] = Corners[2] + (lightRay * f2);
    f2 *= f1;

    lightRay = Corners[3] - light_pos;
    f1 = 1.0f / lightRay.Length();
    lightRay *= f1;
    f3 = GameMath::Fabs((Corners[3].Z - floorZ) / lightRay.Z);
    Corners[7] = Corners[3] + (lightRay * f3);
    f3 *= f1;

    if (f3 > f2) {
        f2 = f3;
    }

    box->Init(Corners, 8);
    m_areaEffectSphere.Init(box->m_center, box->m_extent.Length());
    m_areaEffectSphere.Center -= objPos;
    box->Translate(-objPos);
}

W3DShadowTexture::~W3DShadowTexture()
{
    Ref_Ptr_Release(m_texture);
}

void W3DShadowTexture::Set_Name(const char *source)
{
    strlcpy_tpl(m_name, source);
}

W3DShadowTextureManager::W3DShadowTextureManager()
{
    m_texturePtrTable = new HashTableClass(2048);
    m_missingTextureTable = new HashTableClass(2048);
}

W3DShadowTextureManager::~W3DShadowTextureManager()
{
    Free_All_Textures();

    if (m_texturePtrTable) {
        delete m_texturePtrTable;
    }

    m_texturePtrTable = nullptr;

    if (m_missingTextureTable) {
        delete m_missingTextureTable;
    }

    m_missingTextureTable = nullptr;
}

void W3DShadowTextureManager::Free_All_Textures()
{
    W3DShadowTextureManagerIterator it(*this);

    for (it.First(); !it.Is_Done(); it.Next()) {
        it.Get_Current_Texture()->Release_Ref();
    }

    m_texturePtrTable->Reset();
}

W3DShadowTexture *W3DShadowTextureManager::Peek_Texture(char const *name)
{
    return static_cast<W3DShadowTexture *>(m_texturePtrTable->Find(name));
}

W3DShadowTexture *W3DShadowTextureManager::Get_Texture(char const *name)
{
    W3DShadowTexture *tex = Peek_Texture(name);

    if (tex) {
        tex->Add_Ref();
    }

    return tex;
}

bool W3DShadowTextureManager::Add_Texture(W3DShadowTexture *new_texture)
{
    captainslog_assert(new_texture != nullptr);
    new_texture->Add_Ref();
    m_texturePtrTable->Add(new_texture);
    return true;
}

void W3DShadowTextureManager::Invalidate_Cached_Light_Positions()
{
    Vector3 v(0.0f, 0.0f, 0.0f);

    W3DShadowTextureManagerIterator it(*this);

    for (it.First(); !it.Is_Done(); it.Next()) {
        it.Get_Current_Texture()->Set_Light_Pos_History(v);
    }
}

void W3DShadowTextureManager::Register_Missing(char const *name)
{
    m_missingTextureTable->Add(new MissingTextureClass(name));
}

int W3DShadowTextureManager::Is_Missing(char const *name)
{
    return m_missingTextureTable->Find(name) != nullptr;
}

int W3DShadowTextureManager::Create_Texture(RenderObjClass *robj, char const *name)
{
    W3DShadowTexture *tex = new W3DShadowTexture();

    if (tex) {
        tex->Set_Name(name);

        if (tex->Init(robj) != 0) {
            if (!Peek_Texture(tex->Get_Name())) {
                Add_Texture(tex);
                tex->Release_Ref();
                return 0;
            }
        }
        tex->Release_Ref();
    }
    return 1;
}

W3DProjectedShadow::W3DProjectedShadow()
{
    m_color2 = 0xFFFFFFFF;
    m_shadowProjector = nullptr;
    m_lastObjPosition.Set(0.0f, 0.0f, 0.0f);
    m_type = SHADOW_NONE;
    m_allowWorldAlign = false;
    m_isEnabled = true;
    m_isInvisibleEnabled = false;

    // BUGFIX init all members
    m_offsetX = 0;
    m_offsetY = 0;
    m_robj = nullptr;
    m_next = nullptr;
    m_flags = 0;

    for (int i = 0; i < 1; i++) {
        m_shadowTexture[i] = nullptr;
    }
}

void W3DProjectedShadow::Init()
{
    captainslog_dbgassert(!m_shadowProjector, "Init of existing shadow projector");

    if (m_type == SHADOW_PROJECTION) {
        m_shadowProjector = new TexProjectClass();
        m_shadowProjector->Set_Intensity(0.4f, true);
        m_shadowProjector->Set_Texture(m_shadowTexture[0]->Get_Texture());
    }
}

void W3DProjectedShadow::Update_Texture(Vector3 &light_pos)
{
    static Vector2 uvData[4] = { { -0.5f, -0.5f }, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { -0.5f, -0.5f } };

    if (m_type == SHADOW_PROJECTION) {
        Vector3 objPos = m_robj->Get_Position();

        if (objPos == Vector3(0.0f, 0.0f, 0.0f)) {
            return;
        }

        Vector3 objToLight = light_pos - objPos;
        objToLight.Normalize();
        objToLight = objPos + (objToLight * 2000.0f);
        m_shadowProjector->Compute_Perspective_Projection(m_robj, objToLight, -1.0f, -1.0f);
        m_shadowProjector->Set_Render_Target(g_theW3DProjectedShadowManager->Get_Render_Target(), nullptr);
        SpecialRenderInfoClass *rinfo = g_theW3DProjectedShadowManager->Get_Render_Context();

        rinfo->m_lightEnvironment->Reset(m_robj->Get_Position(), Vector3(0.0f, 0.0f, 0.0f));
        m_shadowProjector->Compute_Texture(m_robj, rinfo);
        SurfaceClass *src = m_shadowTexture[0]->Get_Texture()->Get_Surface_Level(0);
        SurfaceClass *dst = g_theW3DProjectedShadowManager->Get_Render_Target()->Get_Surface_Level(0);

        src->Copy(0, 0, 0, 0, 512, 512, dst);
        Ref_Ptr_Release(dst);
        Ref_Ptr_Release(src);

        m_shadowTexture[0]->Update_Bounds(g_theW3DShadowManager->Get_Light_Pos_World(0), m_robj);
    } else if (m_type == SHADOW_DECAL) {
        Vector3 objPos = m_robj->Get_Position();
        Vector3 objectToLight;

        if ((m_flags & 16) != 0) {
            objectToLight = light_pos - objPos;
            objectToLight.Z = 0.0f;
            objectToLight.Normalize();
        } else {
            objectToLight.Set(1.0f, 0.0f, 0.0f);
        }

        SurfaceClass::SurfaceDescription surface_desc;
        m_shadowTexture[0]->Get_Texture()->Get_Level_Description(surface_desc, 0);
        Vector3 uVec = (objectToLight * 3.2f) / (float)surface_desc.width;
        objectToLight.Rotate_Z(-1.0f, 0.0f);
        Vector3 vVec = (objectToLight * 3.2f) / (float)surface_desc.height;
        m_shadowTexture[0]->Set_Decal_UV_Axis(uVec, vVec);

        AABoxClass box;
        SphereClass sphere;
        m_robj->Get_Obj_Space_Bounding_Box(box);
        m_robj->Get_Obj_Space_Bounding_Sphere(sphere);
        m_shadowTexture[0]->Set_Bounding_Sphere(sphere);
        m_shadowTexture[0]->Set_Bounding_Box(box);
    }

    m_shadowTexture[0]->Set_Light_Pos_History(light_pos);
}

void W3DProjectedShadow::Update_Projection_Parameters(Matrix3D const &camera_xform)
{
    m_shadowProjector->Pre_Render_Update(camera_xform);
}

void W3DProjectedShadow::Update()
{
    if (m_shadowTexture[0]->Get_Light_Pos_History() != g_theW3DShadowManager->Get_Light_Pos_World(0)) {
        Update_Texture(g_theW3DShadowManager->Get_Light_Pos_World(0));
    }

    if (m_lastObjPosition != m_robj->Get_Position()) {
        if (m_type == SHADOW_PROJECTION) {
            Vector3 objToLight = g_theW3DShadowManager->Get_Light_Pos_World(0) - m_robj->Get_Position();
            objToLight.Normalize();
            objToLight = m_robj->Get_Position() + (objToLight * 2000.0f);
            m_shadowProjector->Compute_Perspective_Projection(m_robj, objToLight, -1.0f, -1.0f);
        }

        Set_Obj_Pos_History(m_robj->Get_Position());
    }
}

void W3DProjectedShadow::Release()
{
    g_theW3DProjectedShadowManager->Remove_Shadow(this);
}

#ifdef GAME_DEBUG_STRUCTS
void W3DProjectedShadow::Gather_Draw_Stats(DebugDrawStats *stats)
{
    if (g_theWriteableGlobalData->m_shadowDecals) {
        if (m_isEnabled) {
            if (m_isInvisibleEnabled) {
                stats->Add_Extra_Draw_Calls(1);
            }
        }
    }
}
#endif

W3DProjectedShadow::~W3DProjectedShadow()
{
    Ref_Ptr_Release(m_shadowProjector);

    for (int i = 0; i < 1; i++) {
        Ref_Ptr_Release(m_shadowTexture[i]);
    }
}

// BUGFIX init all members
W3DProjectedShadowManager::W3DProjectedShadowManager() :
    m_shadowList(nullptr),
    m_decalList(nullptr),
    m_dynamicRenderTarget(nullptr),
    m_renderTargetHasAlpha(false),
    m_shadowCamera(nullptr),
    m_shadowContext(nullptr),
    m_W3DShadowTextureManager(nullptr),
    m_numDecalShadows(0),
    m_numProjectionShadows(0)
{
}

W3DProjectedShadowManager::~W3DProjectedShadowManager()
{
    Release_Resources();
    m_dynamicRenderTarget = nullptr;
    m_renderTargetHasAlpha = false;

    if (m_shadowContext) {
        delete m_shadowContext;
    }

    if (m_shadowCamera) {
        delete m_shadowCamera;
    }

    m_shadowCamera = nullptr;

    if (m_W3DShadowTextureManager) {
        delete m_W3DShadowTextureManager;
    }

    m_W3DShadowTextureManager = nullptr;

    captainslog_dbgassert(!m_shadowList, "Destroy of non-empty projected shadow list");
    captainslog_dbgassert(!m_decalList, "Destroy of non-empty projected decal list");
}

void W3DProjectedShadowManager::Reset()
{
    captainslog_dbgassert(!m_shadowList, "Reset of non-empty projected shadow list");
    captainslog_dbgassert(!m_decalList, "Reset of non-empty projected decal list");
    m_W3DShadowTextureManager->Free_All_Textures();
}

bool W3DProjectedShadowManager::Init()
{
    m_W3DShadowTextureManager = new W3DShadowTextureManager();
    m_shadowCamera = new CameraClass();
    m_shadowContext = new SpecialRenderInfoClass(*m_shadowCamera, SpecialRenderInfoClass::RENDER_SHADOW);
    m_shadowContext->m_lightEnvironment = &m_shadowLightEnv;
    return true;
}

bool W3DProjectedShadowManager::Re_Acquire_Resources()
{
#ifdef BUILD_WITH_D3D8
    captainslog_dbgassert(!m_dynamicRenderTarget, "Acquire of existing shadow render target");

    m_renderTargetHasAlpha = true;
    m_dynamicRenderTarget = DX8Wrapper::Create_Render_Target(512, 512, WW3D_FORMAT_A8R8G8B8);

    if (!m_dynamicRenderTarget) {
        m_renderTargetHasAlpha = false;
        m_dynamicRenderTarget = DX8Wrapper::Create_Render_Target(512, 512, WW3D_FORMAT_UNKNOWN);
    }

    IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();
    captainslog_dbgassert(dev, "Trying to ReAquireResources on W3DProjectedShadowManager without device");
    captainslog_dbgassert(!g_shadowDecalIndexBufferD3D, "ReAquireResources not released in W3DProjectedShadowManager");

    if (FAILED(dev->CreateIndexBuffer(2 * g_shadowDecalIndexSize,
            D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
            D3DFMT_INDEX16,
            D3DPOOL_DEFAULT,
            &g_shadowDecalIndexBufferD3D))) {
        return false;
    }

    return g_shadowDecalVertexBufferD3D
        || SUCCEEDED(dev->CreateVertexBuffer(sizeof(SHADOW_DECAL_VERTEX) * g_shadowDecalVertexSize,
            D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
            0,
            D3DPOOL_DEFAULT,
            &g_shadowDecalVertexBufferD3D));
#else
    return true;
#endif
}

void W3DProjectedShadowManager::Release_Resources()
{
    Invalidate_Cached_Light_Positions();
    Ref_Ptr_Release(m_dynamicRenderTarget);

#ifdef BUILD_WITH_D3D8
    if (g_shadowDecalIndexBufferD3D) {
        g_shadowDecalIndexBufferD3D->Release();
    }

    if (g_shadowDecalVertexBufferD3D) {
        g_shadowDecalVertexBufferD3D->Release();
    }

    g_shadowDecalIndexBufferD3D = nullptr;
    g_shadowDecalVertexBufferD3D = nullptr;
#endif
}

void W3DProjectedShadowManager::Invalidate_Cached_Light_Positions()
{
    m_W3DShadowTextureManager->Invalidate_Cached_Light_Positions();
}

void W3DProjectedShadowManager::Update_Render_Target_Textures()
{
    if (m_shadowList) {
        if (g_theWriteableGlobalData->m_shadowDecals) {
            if (m_numProjectionShadows) {
                for (W3DProjectedShadow *shadow = m_shadowList; shadow; shadow = shadow->m_next) {
                    if (shadow->m_type != SHADOW_DECAL) {
                        shadow->Update();
                    }
                }
            }
        }
    }
}

int W3DProjectedShadowManager::Render_Projected_Terrain_Shadow(W3DProjectedShadow *shadow, AABoxClass &box)
{
#ifdef BUILD_WITH_D3D8
    struct SHADOW_VOLUME_VERTEX
    {
        float x;
        float y;
        float z;
    };

    static Matrix4 mWorld(true);

    if (g_theTerrainRenderObject == nullptr) {
        return 0;
    }

    WorldHeightMap *map = g_theTerrainRenderObject->Get_Map();
    float cx = box.m_center.X;
    float cy = box.m_center.Y;
    float dx = box.m_extent.X;
    float dy = box.m_extent.Y;
    IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();

    if (dev == nullptr) {
        return 0;
    }

    int start_x = GameMath::Fast_To_Int_Floor((cx - dx) * 0.1f);
    int end_x = GameMath::Fast_To_Int_Floor((cx + dx) * 0.1f);
    int start_y = GameMath::Fast_To_Int_Floor((cy - dy) * 0.1f);
    int end_y = GameMath::Fast_To_Int_Floor((cy + dy) * 0.1f);
    start_x &= (start_x <= 0) - 1;

    if (end_x >= map->Get_X_Extent() - 1) {
        end_x = map->Get_X_Extent() - 1;
    }

    start_y &= (start_y <= 0) - 1;

    if (end_y >= map->Get_Y_Extent() - 1) {
        end_y = map->Get_Y_Extent() - 1;
    }

    int width = end_x - start_x + 1;
    int height = end_y - start_y + 1;

    if (end_x == start_x || height == 1) {
        return 0;
    }

    int num_verts = height * width;
    SHADOW_VOLUME_VERTEX *vertices;

    if (g_nShadowVertsInBuf <= SHADOW_VERTEX_SIZE - num_verts) {

        if (g_shadowVertexBufferD3D->Lock(
                12 * g_nShadowVertsInBuf, 12 * num_verts, reinterpret_cast<BYTE **>(&vertices), D3DLOCK_NOOVERWRITE)
            != S_OK) {
            return 0;
        }
    } else {
        if (g_shadowVertexBufferD3D->Lock(0, 12 * num_verts, reinterpret_cast<BYTE **>(&vertices), D3DLOCK_DISCARD)
            != S_OK) {
            return 0;
        }

        g_nShadowVertsInBuf = 0;
        g_nShadowStartBatchVertex = 0;
    }

    if (vertices != nullptr) {
        for (int y = start_y; y <= end_y; y++) {
            float f1 = y * 10.0f;

            for (int x = start_x; x <= end_x; x++) {
                vertices->x = x * 10.0f;
                vertices->y = f1;
                vertices->z = map->Get_Height(x, y) * HEIGHTMAP_SCALE;
                vertices++;
            }
        }
    }

    g_shadowVertexBufferD3D->Unlock();
    int num_indices = 6 * (end_y - start_y) * (end_x - start_x);
    short *indices;

    if (g_nShadowIndicesInBuf <= SHADOW_INDEX_SIZE - num_indices) {
        if (g_shadowIndexBufferD3D->Lock(
                2 * g_nShadowIndicesInBuf, 2 * num_indices, reinterpret_cast<BYTE **>(&indices), D3DLOCK_NOOVERWRITE)
            != S_OK) {
            return 0;
        }
    } else {
        if (g_shadowIndexBufferD3D->Lock(0, 2 * num_indices, reinterpret_cast<BYTE **>(&indices), D3DLOCK_DISCARD) != S_OK) {
            return 0;
        }

        g_nShadowIndicesInBuf = 0;
        g_nShadowStartBatchIndex = 0;
    }

    if (indices != nullptr) {
        int y = start_y;
        int x = 0;

        while (y < end_y) {
            for (int x_index = start_x; x_index < end_x; x_index++) {
                float ua[4];
                float va[4];
                unsigned char alpha[4];
                bool flip_for_blend;
                map->Get_Alpha_UV_Data(x_index, y, ua, va, alpha, &flip_for_blend, false);

                if (flip_for_blend) {
                    indices[0] = x + 1;
                    indices[1] = width + x;
                    indices[2] = x;
                    indices[3] = x + 1;
                    indices[4] = x + width + 1;
                    indices[5] = width + x;
                } else {
                    indices[0] = x;
                    indices[1] = x + width + 1;
                    indices[2] = width + x;
                    indices[3] = x;
                    indices[4] = x + 1;
                    indices[5] = x + width + 1;
                }

                indices += 6;
                x++;
            }

            y++;
            x += width;
        }
    }

    g_shadowIndexBufferD3D->Unlock();
    dev->SetIndices(g_shadowIndexBufferD3D, g_nShadowStartBatchVertex);
    dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX *>(&mWorld));
    dev->SetStreamSource(0, g_shadowVertexBufferD3D, sizeof(SHADOW_VOLUME_VERTEX));
    dev->SetVertexShader(D3DFVF_XYZ);
    int count = 2 * (end_y - start_y) * (end_x - start_x);
    dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    dev->SetRenderState(D3DRS_STENCILREF, 1);
    dev->SetRenderState(D3DRS_STENCILMASK, -1);
    dev->SetRenderState(D3DRS_STENCILWRITEMASK, -1);
    dev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    dev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
    dev->SetRenderState(D3DRS_LIGHTING, FALSE);
    dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
    dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

    if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
        dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, num_verts, g_nShadowStartBatchIndex, count);
    }

    dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    dev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    dev->SetRenderState(D3DRS_LIGHTING, TRUE);
    g_nShadowVertsInBuf += num_verts;
    g_nShadowStartBatchVertex = g_nShadowVertsInBuf;
    g_nShadowIndicesInBuf += num_indices;
    g_nShadowStartBatchIndex = g_nShadowIndicesInBuf;
#endif
    return 1;
}

void W3DProjectedShadowManager::Flush_Decals(W3DShadowTexture *texture, ShadowType type)
{
#ifdef BUILD_WITH_D3D8
    static D3DXMATRIX mWorld = *D3DXMatrixIdentity(&mWorld);

    if (g_nShadowDecalVertsInBatch || g_nShadowDecalPolysInBatch) {
        IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();

        if (dev) {
            VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
            DX8Wrapper::Set_Material(material);
            Ref_Ptr_Release(material);
            DX8Wrapper::Set_Texture(0, texture->Get_Texture());

            switch (type) {
                case SHADOW_DECAL:
                    DX8Wrapper::Set_Shader(ShaderClass::s_presetMultiplicativeShader);
                    break;
                case SHADOW_ALPHA_DECAL:
                    DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);
                    break;
                case SHADOW_ADDITIVE_DECAL:
                    DX8Wrapper::Set_Shader(ShaderClass::s_presetAdditiveShader);
                    break;
            }

            DX8Wrapper::Apply_Render_State_Changes();
            dev->SetIndices(g_shadowDecalIndexBufferD3D, g_nShadowDecalStartBatchVertex);
            dev->SetTransform(D3DTS_WORLD, &mWorld);
            dev->SetStreamSource(0, g_shadowDecalVertexBufferD3D, sizeof(SHADOW_DECAL_VERTEX));
            dev->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ);

            if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
                // Debug_Statistics::Record_DX8_Polys_And_Vertices(g_nShadowDecalPolysInBatch, g_nShadowDecalVertsInBatch,
                // &ShaderClass::s_presetOpaqueShader);
                dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
                    0,
                    g_nShadowDecalVertsInBatch,
                    g_nShadowDecalStartBatchIndex,
                    g_nShadowDecalPolysInBatch);
            }

            g_nShadowDecalStartBatchVertex = g_nShadowDecalVertsInBuf;
            g_nShadowDecalStartBatchIndex = g_nShadowDecalIndicesInBuf;
            g_nShadowDecalPolysInBatch = 0;
            g_nShadowDecalVertsInBatch = 0;
        }
    }
#endif
}

void W3DProjectedShadowManager::Queue_Decal(W3DProjectedShadow *shadow)
{
#ifdef BUILD_WITH_D3D8
    Vector3 hmapVertex;
    Vector3 objPos;
    AABoxClass box;
    Matrix3D objXform(true);
    float f1 = 0.1f;
    static Vector3 objCenter(0.0f, 0.0f, 0.0f);
    Vector3 uVector;
    Vector3 vVector;
    float f2 = 0.0f;
    RenderObjClass *robj = shadow->m_robj;

    if (g_theTerrainRenderObject) {
        if (DX8Wrapper::Get_D3D_Device8() != nullptr) {
            WorldHeightMap *hmap = g_theTerrainRenderObject->Get_Map();
            int size = hmap->Border_Size();

            if (robj) {
                objPos = robj->Get_Position();
                objXform = robj->Get_Transform();

                if (robj->Get_User_Data()) {
                    const DrawableInfo *info = static_cast<DrawableInfo *>(robj->Get_User_Data());
                    const Drawable *drawable = info->drawable;

                    if (drawable != nullptr) {
                        const Object *obj = drawable->Get_Object();

                        if (obj) {
                            PathfindLayerEnum layer = obj->Get_Layer();

                            if (layer != LAYER_GROUND) {
                                f2 = g_theTerrainLogic->Get_Layer_Height(objPos.X, objPos.Y, layer, nullptr, true) + 1.5f;
                            }
                        }
                    }
                }
            } else {
                objPos.Set(shadow->m_position.x, shadow->m_position.y, shadow->m_position.z);
                objXform.Rotate_Z(shadow->m_angle);
            }

            objPos.Z = 0.0f;
            uVector = objXform.Get_X_Vector();
            uVector.Z = 0.0f;
            float len = uVector.Length();

            if (len == 0.0f) {
                vVector = objXform.Get_Y_Vector();
                vVector.Z = 0.0f;
                len = vVector.Length();

                if (len == 0.0f) {
                    vVector.Set(0.0f, -1.0f, 0.0f);
                } else {
                    vVector *= 1.0f / len;
                }

                uVector = vVector;
                uVector.Rotate_Z(1.0f, 0.0f);
            } else {
                uVector *= 1.0f / len;
                vVector = uVector;
                vVector.Rotate_Z(-1.0f, 0.0f);
            }

            Vector3 boxCorners[4];
            Vector3 left_x = -shadow->m_decalOffsetU * (uVector * (shadow->m_offsetX + 0.5f));
            Vector3 right_x = shadow->m_decalOffsetU * (uVector * (0.5f - shadow->m_offsetX));
            Vector3 top_y = -shadow->m_decalOffsetV * (vVector * (shadow->m_offsetY + 0.5f));
            Vector3 bottom_y = shadow->m_decalOffsetV * (vVector * (0.5f - shadow->m_offsetY));
            boxCorners[0] = left_x + top_y;
            boxCorners[1] = right_x + top_y;
            boxCorners[2] = right_x + bottom_y;
            boxCorners[3] = left_x + bottom_y;
            float min_x = boxCorners[0].X;
            float max_x = boxCorners[0].X;
            float min_y = boxCorners[0].Y;
            float max_y = boxCorners[0].Y;

            for (int i = 1; i < 4; i++) {
                if (boxCorners[i].X >= max_x) {
                    max_x = boxCorners[i].X;
                }

                if (boxCorners[i].X <= min_x) {
                    min_x = boxCorners[i].X;
                }

                if (boxCorners[i].Y >= max_y) {
                    max_y = boxCorners[i].Y;
                }

                if (boxCorners[i].Y <= min_y) {
                    min_y = boxCorners[i].Y;
                }
            }

            uVector *= shadow->m_sizeX;
            vVector *= shadow->m_sizeY;
            float uOffset = shadow->m_offsetX + 0.5f;
            float vOffset = shadow->m_offsetY + 0.5f;
            int startX = size + GameMath::Fast_To_Int_Floor((objPos.X + min_x) * f1);
            int endX = size + GameMath::Fast_To_Int_Ceil((objPos.X + max_x) * f1);
            int startY = size + GameMath::Fast_To_Int_Floor((objPos.Y + min_y) * f1);
            int endY = size + GameMath::Fast_To_Int_Ceil((objPos.Y + max_y) * f1);

            if (startX <= g_drawStartX) {
                startX = g_drawStartX;
            }

            if (startX >= g_drawEdgeX) {
                startX = g_drawEdgeX;
            }

            if (startY <= g_drawStartY) {
                startY = g_drawStartY;
            }

            if (startY >= g_drawEdgeY) {
                startY = g_drawEdgeY;
            }

            if (endX <= g_drawStartX) {
                endX = g_drawStartX;
            }

            if (endX >= g_drawEdgeX) {
                endX = g_drawEdgeX;
            }

            if (endY <= g_drawStartY) {
                endY = g_drawStartY;
            }

            if (endY >= g_drawEdgeY) {
                endY = g_drawEdgeY;
            }

            int i5 = endX - startX - 103;

            if (i5 > 0) {
                int i6 = GameMath::Fast_To_Int_Floor((float)i5 / 2.0f);
                startX += i6;
                endX -= i5 - i6;
            }

            int i7 = endY - startY - 103;

            if (i7 > 0) {
                int i8 = GameMath::Fast_To_Int_Floor((float)i7 / 2.0f);
                startY += i8;
                endY -= i7 - i8;
            }

            int i9 = endX - startX + 1;
            int i10 = endY - startY + 1;

            if (i9 > 1 && i10 > 1) {
                int numVerts = i10 * i9;
                int numIndex = 6 * (endY - startY) * (endX - startX);
                SHADOW_DECAL_VERTEX *pvVertices;

                if (g_nShadowDecalVertsInBuf <= g_shadowDecalVertexSize - i10 * i9) {
                    if (FAILED(g_shadowDecalVertexBufferD3D->Lock(sizeof(SHADOW_DECAL_VERTEX) * g_nShadowDecalVertsInBuf,
                            sizeof(SHADOW_DECAL_VERTEX) * numVerts,
                            reinterpret_cast<BYTE **>(&pvVertices),
                            D3DLOCK_NOOVERWRITE))) {
                        return;
                    }
                } else {
                    Flush_Decals(shadow->m_shadowTexture[0], shadow->m_type);

                    if (FAILED(g_shadowDecalVertexBufferD3D->Lock(0,
                            sizeof(SHADOW_DECAL_VERTEX) * numVerts,
                            reinterpret_cast<BYTE **>(&pvVertices),
                            D3DLOCK_DISCARD))) {
                        return;
                    }

                    g_nShadowDecalStartBatchVertex = 0;
                    g_nShadowDecalPolysInBatch = 0;
                    g_nShadowDecalVertsInBatch = 0;
                    g_nShadowDecalVertsInBuf = 0;
                }

                captainslog_dbgassert(numVerts == (endX - startX + 1) * (endY - startY + 1), "queueDecal VB size mismatch");

                if (pvVertices) {
                    if (f2 == 0.0f) {
                        for (int i = startY; i <= endY; i++) {
                            hmapVertex.Y = (float)(i - size) * 10.0f;

                            for (int j = startX; j <= endX; j++) {
                                hmapVertex.X = (float)(j - size) * 10.0f;
                                hmapVertex.Z = (float)hmap->Get_Height(j, i) * g_heightMapScale + 0.1f;
                                pvVertices->x = hmapVertex.X;
                                pvVertices->y = hmapVertex.Y;
                                pvVertices->z = hmapVertex.Z;
                                pvVertices->c = shadow->m_color2;
                                pvVertices->u = (uVector * (hmapVertex - objPos)) + uOffset;
                                pvVertices->v = (vVector * (hmapVertex - objPos)) + vOffset;
                                pvVertices++;
                            }
                        }
                    } else {
                        for (int i = startY; i <= endY; i++) {
                            hmapVertex.Y = (float)(i - size) * 10.0f;

                            for (int j = startX; j <= endX; j++) {
                                hmapVertex.X = (float)(j - size) * 10.0f;
                                char height = (float)hmap->Get_Height(j, i);

                                if (height * g_heightMapScale <= f2) {
                                    hmapVertex.Z = f2;
                                } else {
                                    hmapVertex.Z = (float)hmap->Get_Height(j, i) * g_heightMapScale + 0.1f;
                                }

                                pvVertices->x = hmapVertex.X;
                                pvVertices->y = hmapVertex.Y;
                                pvVertices->z = hmapVertex.Z;
                                pvVertices->c = shadow->m_color2;
                                pvVertices->u = (uVector * (hmapVertex - objPos)) + uOffset;
                                pvVertices->v = (vVector * (hmapVertex - objPos)) + vOffset;
                                pvVertices++;
                            }
                        }
                    }
                }

                g_shadowDecalVertexBufferD3D->Unlock();

                unsigned short *pvIndices;

                if (g_nShadowDecalIndicesInBuf <= g_shadowDecalIndexSize - numIndex) {
                    if (FAILED(g_shadowDecalIndexBufferD3D->Lock(2 * g_nShadowDecalIndicesInBuf,
                            2 * numIndex,
                            reinterpret_cast<BYTE **>(&pvIndices),
                            D3DLOCK_NOOVERWRITE))) {
                        return;
                    }
                } else {
                    Flush_Decals(shadow->m_shadowTexture[0], shadow->m_type);

                    if (FAILED(g_shadowDecalIndexBufferD3D->Lock(
                            0, 2 * numIndex, reinterpret_cast<BYTE **>(&pvIndices), D3DLOCK_DISCARD))) {
                        return;
                    }

                    g_nShadowDecalStartBatchIndex = 0;
                    g_nShadowDecalPolysInBatch = 0;
                    g_nShadowDecalVertsInBatch = 0;
                    g_nShadowDecalIndicesInBuf = 0;
                }

                if (pvIndices) {
                    int i13 = 0;

                    for (int i = startY; i < endY; i++) {
                        int i14 = i13;

                        for (int j = startX; j < endX; j++) {
                            if (hmap->Get_Flip_State(j, i)) {
                                pvIndices[0] = i14 + g_nShadowDecalVertsInBatch + 1;
                                pvIndices[1] = g_nShadowDecalVertsInBatch + i9 + i14;
                                pvIndices[2] = g_nShadowDecalVertsInBatch + i14;
                                pvIndices[3] = i14 + g_nShadowDecalVertsInBatch + 1;
                                pvIndices[4] = g_nShadowDecalVertsInBatch + i14 + i9 + 1;
                                pvIndices[5] = g_nShadowDecalVertsInBatch + i9 + i14;
                            } else {
                                pvIndices[0] = g_nShadowDecalVertsInBatch + i14;
                                pvIndices[1] = g_nShadowDecalVertsInBatch + i14 + i9 + 1;
                                pvIndices[2] = g_nShadowDecalVertsInBatch + i9 + i14;
                                pvIndices[3] = g_nShadowDecalVertsInBatch + i14;
                                pvIndices[4] = i14 + g_nShadowDecalVertsInBatch + 1;
                                pvIndices[5] = g_nShadowDecalVertsInBatch + i14 + i9 + 1;
                            }

                            pvIndices += 6;
                            i14++;
                        }

                        i13 += i9;
                    }
                }

                g_shadowDecalIndexBufferD3D->Unlock();
                g_nShadowDecalPolysInBatch += 2 * (endY - startY) * (endX - startX);
                g_nShadowDecalVertsInBuf += numVerts;
                g_nShadowDecalVertsInBatch += numVerts;
                g_nShadowDecalIndicesInBuf += numIndex;
            }
        }
    }
#endif
}

int W3DProjectedShadowManager::Render_Shadows(RenderInfoClass &rinfo)
{
    static AABoxClass aaBox;
    static SphereClass sphere;

    int count = 0;

    if (!m_shadowList && !m_decalList) {
        return count;
    }

    if (!g_theTerrainRenderObject) {
        return count;
    }

    WorldHeightMap *hmap = g_theTerrainRenderObject->Get_Map();
    g_drawEdgeY = hmap->Get_Draw_Origin_Y() + hmap->Get_Draw_Height() - 1;
    g_drawEdgeX = hmap->Get_Draw_Origin_X() + hmap->Get_Draw_Width() - 1;

    if (g_drawEdgeX > hmap->Get_X_Extent() - 1) {
        g_drawEdgeX = hmap->Get_X_Extent() - 1;
    }

    if (g_drawEdgeY > hmap->Get_Y_Extent() - 1) {
        g_drawEdgeY = hmap->Get_Y_Extent() - 1;
    }

    g_drawStartX = hmap->Get_Draw_Origin_X();
    g_drawStartY = hmap->Get_Draw_Origin_Y();
    g_nShadowDecalVertsInBuf = 0xFFFF;
    g_nShadowDecalIndicesInBuf = 0xFFFF;

    if (g_theWriteableGlobalData->m_shadowDecals) {
        g_theDX8MeshRenderer.Set_Camera(&rinfo.m_camera);
        W3DShadowTexture *texture = 0;
        ShadowType type = SHADOW_NONE;

        for (W3DProjectedShadow *shadow = m_shadowList;; shadow = shadow->m_next) {
            if (!shadow) {
                Flush_Decals(texture, type);
                g_theDX8MeshRenderer.Flush();
                break;
            }

            if (shadow->m_isEnabled && !shadow->m_isInvisibleEnabled) {
                if ((shadow->m_type & SHADOW_DECAL) != 0) {
                    if (!texture) {
                        texture = m_shadowList->m_shadowTexture[0];
                    }

                    if (type == SHADOW_NONE) {
                        type = m_shadowList->m_type;
                    }

                    if (shadow->m_shadowTexture[0] != texture || shadow->m_type != type) {
                        Flush_Decals(texture, type);
                        texture = shadow->m_shadowTexture[0];
                        type = shadow->m_type;
                    }

                    if (shadow->m_robj->Is_Really_Visible()) {
                        Queue_Decal(shadow);
                        count++;
                    }
                } else {
                    sphere = shadow->m_shadowTexture[0]->Get_Bounding_Sphere();
                    sphere.Center += shadow->m_robj->Get_Position();
                    switch (CollisionMath::Overlap_Test(*g_shadowCameraFrustum, sphere)) {
                        case CollisionMath::OVERLAPPED:
                            aaBox = shadow->m_shadowTexture[0]->Get_Bounding_Box();
                            aaBox.Translate(shadow->m_robj->Get_Position());

                            if (CollisionMath::Overlap_Test(*g_shadowCameraFrustum, aaBox) == CollisionMath::OUTSIDE) {
                                continue;
                            }

                            break;
                        case CollisionMath::OUTSIDE:
                            continue;
                        case CollisionMath::INSIDE:
                            aaBox = shadow->m_shadowTexture[0]->Get_Bounding_Box();
                            aaBox.Translate(shadow->m_robj->Get_Position());
                            break;
                    }

                    if (shadow->m_type == SHADOW_PROJECTION) {
                        shadow->Update_Projection_Parameters(rinfo.m_camera.Get_Transform());
                        TexProjectClass *projector = shadow->Get_Shadow_Projector();
                        projector->Peek_Material_Pass()->Install_Materials();
                        DX8Wrapper::Apply_Render_State_Changes();

                        if (Render_Projected_Terrain_Shadow(shadow, aaBox)) {
                            count++;
                        }

                        projector->Peek_Material_Pass()->UnInstall_Materials();

                        Coord3D sphere_pos;
                        sphere_pos.x = sphere.Center.X;
                        sphere_pos.y = sphere.Center.Y;
                        sphere_pos.z = sphere.Center.Z;

                        SimpleObjectIterator *iter = g_thePartitionManager->Iterate_Objects_In_Range(
                            &sphere_pos, sphere.Radius, FROM_CENTER_3D, nullptr, ITER_FASTEST);
                        MemoryPoolObjectHolder holder(iter);
                        AABoxIntersectionTestClass intersect(aaBox, COLLISION_TYPE_ALL);

                        for (Object *o = iter->First(); o; o = iter->Next()) {
                            for (DrawModule **d = o->Get_Drawable()->Get_Draw_Modules(); *d; d++) {
                                ObjectDrawInterface *intf = (*d)->Get_Object_Draw_Interface();

                                if (intf) {
                                    W3DModelDraw *model = static_cast<W3DModelDraw *>(intf);
                                    RenderObjClass *robj = model->Get_Render_Object();

                                    if (robj) {
                                        if ((!robj->Is_Alpha() || !o->Is_KindOf(KINDOF_IMMOBILE))
                                            && robj != shadow->m_robj) {
                                            if (robj->Is_Really_Visible()) {
                                                if (robj->Intersect_AABox(intersect)) {
                                                    rinfo.Push_Material_Pass(projector->Peek_Material_Pass());
                                                    rinfo.Push_Override_Flags(
                                                        RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
                                                    robj->Render(rinfo);
                                                    rinfo.Pop_Override_Flags();
                                                    rinfo.Pop_Material_Pass();
                                                    count++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_decalList) {
        W3DShadowTexture *texture = 0;
        ShadowType type = SHADOW_NONE;

        for (W3DProjectedShadow *shadow = m_decalList; shadow; shadow = shadow->m_next) {
            if (shadow->m_isEnabled) {
                if (!shadow->m_isInvisibleEnabled) {
                    if (!texture) {
                        texture = m_decalList->m_shadowTexture[0];
                    }

                    if (type == SHADOW_NONE) {
                        type = m_decalList->m_type;
                    }

                    if (shadow->m_shadowTexture[0] != texture || shadow->m_type != type) {
                        Flush_Decals(texture, type);
                        texture = shadow->m_shadowTexture[0];
                        type = shadow->m_type;
                    }

                    if (!shadow->m_robj || shadow->m_robj->Is_Really_Visible()) {
                        Queue_Decal(shadow);
                        count++;
                    }
                }
            }
        }

        Flush_Decals(texture, type);
    }

    return count;
}

Shadow *W3DProjectedShadowManager::Add_Decal(Shadow::ShadowTypeInfo *shadow_info)
{
    W3DShadowTexture *tex = nullptr;
    float x = 0.0f;
    float y = 0.0f;

    if (!shadow_info) {
        return nullptr;
    }

    char fname[64];
    int len = strlen(shadow_info->m_shadowName);
    strncpy(fname, shadow_info->m_shadowName, len);
    strcpy(&fname[len], ".tga");
    tex = m_W3DShadowTextureManager->Get_Texture(fname);

    if (!tex) {
        TextureClass *t = W3DAssetManager::Get_Instance()->Get_Texture(fname);
        t->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        t->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        t->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
        captainslog_dbgassert(t, "Could not load decal texture: %s", fname);

        if (!t) {
            return nullptr;
        }

        tex = new W3DShadowTexture();
        tex->Set_Name(fname);
        m_W3DShadowTextureManager->Add_Texture(tex);
        tex->Set_Texture(t);
    }

    ShadowType type = shadow_info->m_type;
    bool align = shadow_info->m_allowWorldAlign;
    x = shadow_info->m_sizeX;
    y = shadow_info->m_sizeY;
    W3DProjectedShadow *shadow = new W3DProjectedShadow();

    if (!shadow) {
        return nullptr;
    }

    shadow->Set_Render_Object(nullptr);
    shadow->Set_Texture(0, tex);
    shadow->m_type = type;
    shadow->m_allowWorldAlign = align;
    shadow->m_sizeX = 1.0f / x;
    shadow->m_sizeY = 1.0f / y;
    shadow->m_decalOffsetU = x;
    shadow->m_decalOffsetV = y;
    shadow->m_offsetX = 0.0f;
    shadow->m_offsetY = 0.0f;
    shadow->m_flags = (type & SHADOW_DIRECTIONAL_PROJECTION) != 0;
    shadow->Init();
    W3DProjectedShadow *s = nullptr;
    W3DProjectedShadow *i;

    for (i = m_decalList; i; i = i->m_next) {
        if (i->m_shadowTexture[0] == tex) {
            shadow->m_next = i;

            if (s) {
                s->m_next = shadow;
            } else {
                m_decalList = shadow;
            }

            break;
        }

        s = i;
    }

    if (!i) {
        shadow->m_next = m_decalList;
        m_decalList = shadow;
    }

    switch (shadow->m_type) {
        case SHADOW_DECAL:
        case SHADOW_ALPHA_DECAL:
        case SHADOW_ADDITIVE_DECAL:
            m_numDecalShadows++;
            break;
        case SHADOW_PROJECTION:
            m_numProjectionShadows++;
            break;
    }
    return shadow;
}

Shadow *W3DProjectedShadowManager::Add_Decal(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info)
{
    W3DShadowTexture *tex = nullptr;
    ShadowType type = SHADOW_NONE;
    float x = 0.0f;
    float y = 0.0f;

    if (!robj || !shadow_info) {
        return nullptr;
    }

    char fname[64];
    int len = strlen(shadow_info->m_shadowName);
    strncpy(fname, shadow_info->m_shadowName, len);
    strcpy(&fname[len], ".tga");
    tex = m_W3DShadowTextureManager->Get_Texture(fname);

    if (!tex) {
        TextureClass *t = W3DAssetManager::Get_Instance()->Get_Texture(fname);
        t->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        t->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        t->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
        captainslog_dbgassert(t, "Could not load decal texture: %s", fname);

        if (!t) {
            return nullptr;
        }

        tex = new W3DShadowTexture();
        tex->Set_Name(fname);
        m_W3DShadowTextureManager->Add_Texture(tex);
        tex->Set_Texture(t);
    }

    type = shadow_info->m_type;
    bool align = shadow_info->m_allowWorldAlign;
    int flags = (type & SHADOW_DIRECTIONAL_PROJECTION) != 0;
    x = shadow_info->m_sizeX;
    y = shadow_info->m_sizeY;
    float offsetx = shadow_info->m_offsetX;
    float offsety = shadow_info->m_offsetY;
    W3DProjectedShadow *shadow = new W3DProjectedShadow();

    if (!shadow) {
        return nullptr;
    }

    shadow->Set_Render_Object(robj);
    shadow->Set_Texture(0, tex);
    shadow->m_type = type;
    shadow->m_allowWorldAlign = align;
    AABoxClass box;
    robj->Get_Obj_Space_Bounding_Box(box);

    if (x == 0.0f) {
        x = box.m_extent.X + box.m_extent.X;
    }

    shadow->m_sizeX = 1.0f / x;

    if (y == 0.0f) {
        y = box.m_extent.Y + box.m_extent.Y;
    }

    shadow->m_sizeY = 1.0f / y;

    if (offsetx != 0.0f) {
        offsetx = offsetx * shadow->m_sizeX;
    }

    if (offsety != 0.0f) {
        offsety = offsety * shadow->m_sizeY;
    }

    shadow->m_decalOffsetU = x;
    shadow->m_decalOffsetV = y;
    shadow->m_offsetX = offsetx;
    shadow->m_offsetY = offsety;
    shadow->m_flags = flags;
    shadow->Init();
    W3DProjectedShadow *s = nullptr;
    W3DProjectedShadow *i;

    for (i = m_decalList; i; i = i->m_next) {
        if (i->m_shadowTexture[0] == tex) {
            shadow->m_next = i;

            if (s) {
                s->m_next = shadow;
            } else {
                m_decalList = shadow;
            }

            break;
        }

        s = i;
    }

    if (!i) {
        shadow->m_next = m_decalList;
        m_decalList = shadow;
    }

    switch (shadow->m_type) {
        case SHADOW_DECAL:
        case SHADOW_ALPHA_DECAL:
        case SHADOW_ADDITIVE_DECAL:
            m_numDecalShadows++;
            break;
        case SHADOW_PROJECTION:
            m_numProjectionShadows++;
            break;
    }
    return shadow;
}

W3DProjectedShadow *W3DProjectedShadowManager::Add_Shadow(
    RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable)
{
    W3DShadowTexture *tex = nullptr;
    ShadowType type = SHADOW_NONE;
    float x = 0.0f;
    float y = 0.0f;
    float offsetx = 0.0f;
    float offsety = 0.0f;
    int flags = 0;
    if (!m_dynamicRenderTarget || !robj || !g_theWriteableGlobalData->m_shadowDecals) {
        return nullptr;
    }

    if (shadow_info) {
        if (shadow_info->m_type == SHADOW_DECAL) {
            char fname[64];
            int len = strlen(shadow_info->m_shadowName);

            if (len > 1) {
                strncpy(fname, shadow_info->m_shadowName, len);
                strcpy(&fname[len], ".tga");
            } else {
                strcpy(fname, "shadow.tga");
            }

            tex = m_W3DShadowTextureManager->Get_Texture(fname);

            if (!tex) {
                TextureClass *t = W3DAssetManager::Get_Instance()->Get_Texture(fname);
                t->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                t->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                t->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
                captainslog_dbgassert(t, "Could not load decal texture: %s", fname);

                if (!t) {
                    return nullptr;
                }

                tex = new W3DShadowTexture();
                tex->Set_Name(fname);
                m_W3DShadowTextureManager->Add_Texture(tex);
                tex->Set_Texture(t);
            }

            type = SHADOW_DECAL;
            flags = (type & SHADOW_DIRECTIONAL_PROJECTION) != 0;
            x = shadow_info->m_sizeX;
            y = shadow_info->m_sizeY;
            offsetx = shadow_info->m_offsetX;
            offsety = shadow_info->m_offsetY;
        } else if (shadow_info->m_type == SHADOW_PROJECTION) {
            char fname[64];

            if (shadow_info->m_shadowName[0]) {
                strcpy(fname, shadow_info->m_shadowName);
            } else {
                strcpy(fname, robj->Get_Name());
            }

            tex = m_W3DShadowTextureManager->Get_Texture(fname);

            if (!tex) {
                m_W3DShadowTextureManager->Create_Texture(robj, fname);
                tex = m_W3DShadowTextureManager->Get_Texture(fname);
                captainslog_dbgassert(tex, "Could not create shadow texture");

                if (!tex) {
                    return 0;
                }
            }

            type = SHADOW_PROJECTION;
        }

    } else {
        char fname[64];
        strcpy(fname, robj->Get_Name());
        tex = m_W3DShadowTextureManager->Get_Texture(fname);

        if (!tex) {
            m_W3DShadowTextureManager->Create_Texture(robj, fname);
            tex = m_W3DShadowTextureManager->Get_Texture(fname);

            if (!tex) {
                return nullptr;
            }
        }

        type = SHADOW_PROJECTION;
    }

    W3DProjectedShadow *shadow = new W3DProjectedShadow();

    if (!shadow) {
        return nullptr;
    }

    shadow->Set_Render_Object(robj);
    shadow->Set_Texture(0, tex);
    shadow->m_type = type;
    shadow->m_allowWorldAlign = false;
    AABoxClass box;
    robj->Get_Obj_Space_Bounding_Box(box);

    if (x == 0.0f) {
        x = 1.0f / box.m_extent.X + box.m_extent.X;
    } else {
        x = 1.0f / x;
    }

    if (y == 0.0f) {
        y = -1.0f / box.m_extent.Y + box.m_extent.Y;
    } else {
        y = -1.0f / y;
    }

    if (offsetx != 0.0f) {
        offsetx = -offsetx * x;
    }

    if (offsety != 0.0f) {
        offsety = -offsety * y;
    }

    shadow->m_sizeX = x;
    shadow->m_sizeY = y;
    shadow->m_decalOffsetU = 1.0f / x;
    shadow->m_decalOffsetV = 1.0f / y;
    shadow->m_offsetX = offsetx;
    shadow->m_offsetY = offsety;
    shadow->m_flags = flags;
    shadow->Init();
    W3DProjectedShadow *s = nullptr;
    W3DProjectedShadow *i;

    for (i = m_shadowList; i; i = i->m_next) {
        if (i->m_shadowTexture[0] == tex) {
            shadow->m_next = i;

            if (s) {
                s->m_next = shadow;
            } else {
                m_shadowList = shadow;
            }

            break;
        }

        s = i;
    }

    if (!i) {
        shadow->m_next = m_shadowList;
        m_shadowList = shadow;
    }

    if (shadow->m_type == SHADOW_DECAL) {
        m_numDecalShadows++;
    } else if (shadow->m_type == SHADOW_PROJECTION) {
        m_numProjectionShadows++;
    }

    return shadow;
}

W3DProjectedShadow *W3DProjectedShadowManager::Create_Decal_Shadow(Shadow::ShadowTypeInfo *shadow_info)
{
    W3DShadowTexture *tex = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    float f1 = 10.0f;

    char fname[64];
    int len = strlen(shadow_info->m_shadowName);

    if (len > 1) {
        strncpy(fname, shadow_info->m_shadowName, len);
        strcpy(&fname[len], ".tga");
    } else {
        strcpy(fname, "shadow.tga");
    }

    tex = m_W3DShadowTextureManager->Get_Texture(fname);

    if (!tex) {
        TextureClass *t = W3DAssetManager::Get_Instance()->Get_Texture(fname);
        t->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        t->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        t->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
        captainslog_dbgassert(t, "Could not load decal texture");

        if (!t) {
            return nullptr;
        }

        tex = new W3DShadowTexture();
        tex->Set_Name(fname);
        m_W3DShadowTextureManager->Add_Texture(tex);
        tex->Set_Texture(t);
    }

    x = shadow_info->m_sizeX;
    y = shadow_info->m_sizeY;
    float offsetx = shadow_info->m_offsetX;
    float offsety = shadow_info->m_offsetY;
    W3DProjectedShadow *shadow = new W3DProjectedShadow();

    if (!shadow) {
        return nullptr;
    }

    shadow->Set_Texture(0, tex);
    shadow->m_type = SHADOW_DECAL;
    shadow->m_allowWorldAlign = false;

    if (x == 0.0f) {
        x = 1.0f / (f1 + f1);
    } else {
        x = 1.0f / x;
    }

    if (y == 0.0f) {
        y = -1.0f / (f1 + f1);
    } else {
        y = -1.0f / y;
    }

    if (offsetx == 0.0f) {
        offsetx = 0.0f;
    } else {
        offsetx = -offsetx * x;
    }

    if (offsety == 0.0f) {
        offsety = 0.0f;
    } else {
        offsety = -offsety * y;
    }

    shadow->m_sizeX = x;
    shadow->m_sizeY = y;
    shadow->m_decalOffsetU = 1.0f / x;
    shadow->m_decalOffsetV = 1.0f / y;
    shadow->m_offsetX = offsetx;
    shadow->m_offsetY = offsety;
    shadow->m_flags = 0;
    shadow->Init();
    return shadow;
}

void W3DProjectedShadowManager::Remove_Shadow(W3DProjectedShadow *shadow)
{
    W3DProjectedShadow *s = nullptr;

    if ((shadow->m_type & (SHADOW_ALPHA_DECAL | SHADOW_ADDITIVE_DECAL)) != 0) {
        for (W3DProjectedShadow *i = m_decalList; i; i = i->m_next) {
            if (i == shadow) {
                if (s) {
                    s->m_next = shadow->m_next;
                } else {
                    m_decalList = shadow->m_next;
                }

                if (shadow->m_type == SHADOW_DECAL) {
                    m_numDecalShadows--;
                } else if (shadow->m_type == SHADOW_PROJECTION) {
                    m_numProjectionShadows--;
                }

                if (shadow) {
                    delete shadow;
                    return;
                }
            }

            s = i;
        }
    }

    for (W3DProjectedShadow *i = m_shadowList; i; i = i->m_next) {
        if (i == shadow) {
            if (s) {
                s->m_next = shadow->m_next;
            } else {
                m_shadowList = shadow->m_next;
            }

            if (shadow->m_type == SHADOW_DECAL) {
                m_numDecalShadows--;
            } else if (shadow->m_type == SHADOW_PROJECTION) {
                m_numProjectionShadows--;
            }

            if (shadow) {
                delete shadow;
            }
            return;
        }

        s = i;
    }
}

void W3DProjectedShadowManager::Remove_All_Shadows()
{
    W3DProjectedShadow *list = m_shadowList;
    m_shadowList = nullptr;
    m_numDecalShadows = 0;
    m_numProjectionShadows = 0;
    W3DProjectedShadow *next;

    for (W3DProjectedShadow *i = list; i; i = next) {
        next = i->m_next;
        i->m_next = nullptr;
        delete i;
    }

    list = m_decalList;
    m_decalList = nullptr;

    for (W3DProjectedShadow *i = list; i; i = next) {
        next = i->m_next;
        i->m_next = nullptr;
        delete i;
    }
}
