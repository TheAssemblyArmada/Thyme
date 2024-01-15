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
#pragma once
#include "always.h"
#include "aabox.h"
#include "hash.h"
#include "lightenv.h"
#include "matrix3.h"
#include "refcount.h"
#include "sphere.h"
#include "w3dshadow.h"
#include <new>

class CameraClass;
class SpecialRenderInfoClass;
class W3DShadowTextureManager;
class TexProjectClass;
class RenderObjClass;
class Drawable;
class RenderInfoClass;
class TextureClass;
class FrustumClass;

class W3DShadowTexture : public RefCountClass, public HashableClass
{
public:
    W3DShadowTexture();
    virtual ~W3DShadowTexture() override;

    virtual char const *Get_Key() override { return m_name; }
    int Init(RenderObjClass *robj);
    void Update_Bounds(Vector3 &light_pos, RenderObjClass *robj);

    const char *Get_Name() const { return m_name; }
    TextureClass *Get_Texture() const { return m_texture; }
    const Vector3 &Get_Light_Pos_History() const { return m_lastLightPosition; }
    const Matrix3 &Get_Object_Orienation_History() const { return m_lastObjectOrientation; }
    const SphereClass &Get_Bounding_Sphere() const { return m_areaEffectSphere; }
    const AABoxClass &Get_Bounding_Box() const { return m_areaEffectBox; }

    void Set_Name(const char *source);
    void Set_Texture(TextureClass *texture) { m_texture = texture; }
    void Set_Light_Pos_History(const Vector3 &pos) { m_lastLightPosition = pos; }
    void Set_Object_Orienation_History(const Matrix3 &orientation) { m_lastObjectOrientation = orientation; }
    void Set_Bounding_Sphere(const SphereClass &sphere) { m_areaEffectSphere = sphere; }
    void Set_Bounding_Box(const AABoxClass &box) { m_areaEffectBox = box; }

    void Set_Decal_UV_Axis(const Vector3 &uv0, const Vector3 &uv1)
    {
        m_shadowUV[0] = uv0;
        m_shadowUV[1] = uv1;
    }

    void Get_Decal_UV_Axis(Vector3 *u, Vector3 *v)
    {
        *u = m_shadowUV[0];
        *v = m_shadowUV[1];
    }

private:
    char m_name[32];
    TextureClass *m_texture;
    Vector3 m_lastLightPosition;
    Matrix3 m_lastObjectOrientation;
    AABoxClass m_areaEffectBox;
    SphereClass m_areaEffectSphere;
    Vector3 m_shadowUV[2];
};

class W3DShadowTextureManager
{
public:
    W3DShadowTextureManager();
    ~W3DShadowTextureManager();

    void Free_All_Textures();
    W3DShadowTexture *Peek_Texture(char const *name);
    W3DShadowTexture *Get_Texture(char const *name);
    bool Add_Texture(W3DShadowTexture *new_texture);
    void Invalidate_Cached_Light_Positions();
    int Is_Missing(char const *name);
    void Register_Missing(char const *name);
    int Create_Texture(RenderObjClass *robj, char const *name);

private:
    HashTableClass *m_texturePtrTable;
    HashTableClass *m_missingTextureTable;
    friend class W3DShadowTextureManagerIterator;
};

class W3DShadowTextureManagerIterator : public HashTableIteratorClass
{
public:
    W3DShadowTextureManagerIterator(W3DShadowTextureManager &manager) : HashTableIteratorClass(*manager.m_texturePtrTable) {}
    W3DShadowTexture *Get_Current_Texture() { return static_cast<W3DShadowTexture *>(Get_Current()); }
    virtual ~W3DShadowTextureManagerIterator() override {}
};

class W3DProjectedShadow : public Shadow
{
public:
    W3DProjectedShadow();
    ~W3DProjectedShadow();

    virtual void Release() override;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(
        DebugDrawStats *stats) override; // name of this funcion and data type of its parameter unknown
#endif

    void Init();
    void Update();
    void Update_Projection_Parameters(Matrix3D const &camera_xform);
    void Update_Texture(Vector3 &light_pos);

    W3DShadowTexture *Get_Texture(int index) { return m_shadowTexture[index]; }
    TexProjectClass *Get_Shadow_Projector() { return m_shadowProjector; }

    void Set_Render_Object(RenderObjClass *robj) { m_robj = robj; }
    void Set_Obj_Pos_History(const Vector3 &pos) { m_lastObjPosition = pos; }
    void Set_Texture(int i, W3DShadowTexture *texture) { m_shadowTexture[i] = texture; }

#ifdef GAME_DLL
    void Hook_Dtor() { W3DProjectedShadow::~W3DProjectedShadow(); }
#endif

private:
    W3DShadowTexture *m_shadowTexture[1];
    TexProjectClass *m_shadowProjector;
    RenderObjClass *m_robj;
    Vector3 m_lastObjPosition;
    W3DProjectedShadow *m_next;
    bool m_allowWorldAlign;
    float m_offsetX;
    float m_offsetY;
    int m_flags;
    friend class W3DProjectedShadowManager;
};

class ProjectedShadowManager
{
public:
    virtual ~ProjectedShadowManager() {}
    virtual Shadow *Add_Decal(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info) = 0;
    virtual Shadow *Add_Decal(Shadow::ShadowTypeInfo *shadow_info) = 0;
};

class W3DProjectedShadowManager : public ProjectedShadowManager
{
public:
    W3DProjectedShadowManager();
    virtual ~W3DProjectedShadowManager() override;
    virtual Shadow *Add_Decal(Shadow::ShadowTypeInfo *shadow_info) override;
    virtual Shadow *Add_Decal(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info) override;

    bool Re_Acquire_Resources();
    void Release_Resources();

    bool Init();
    void Reset();

    W3DProjectedShadow *Add_Shadow(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable);
    void Remove_Shadow(W3DProjectedShadow *shadow);
    void Remove_All_Shadows();

    int Render_Shadows(RenderInfoClass &rinfo);
    int Render_Projected_Terrain_Shadow(W3DProjectedShadow *shadow, AABoxClass &box);

    W3DProjectedShadow *Create_Decal_Shadow(Shadow::ShadowTypeInfo *shadow_info);
    void Queue_Decal(W3DProjectedShadow *shadow);
    void Flush_Decals(W3DShadowTexture *texture, ShadowType type);

    void Invalidate_Cached_Light_Positions();
    void Update_Render_Target_Textures();

    SpecialRenderInfoClass *Get_Render_Context() { return m_shadowContext; }
    TextureClass *Get_Render_Target() { return m_dynamicRenderTarget; }

#ifdef GAME_DLL
    W3DProjectedShadowManager *Hook_Ctor() { return new (this) W3DProjectedShadowManager(); }
#endif

private:
    W3DProjectedShadow *m_shadowList;
    W3DProjectedShadow *m_decalList;
    TextureClass *m_dynamicRenderTarget;
    bool m_renderTargetHasAlpha;
    CameraClass *m_shadowCamera;
    LightEnvironmentClass m_shadowLightEnv;
    SpecialRenderInfoClass *m_shadowContext;
    W3DShadowTextureManager *m_W3DShadowTextureManager;
    int m_numDecalShadows;
    int m_numProjectionShadows;
};

#ifdef GAME_DLL
extern W3DProjectedShadowManager *&g_theW3DProjectedShadowManager;
extern ProjectedShadowManager *&g_theProjectedShadowManager;
#else
extern W3DProjectedShadowManager *g_theW3DProjectedShadowManager;
extern ProjectedShadowManager *g_theProjectedShadowManager;
#endif
