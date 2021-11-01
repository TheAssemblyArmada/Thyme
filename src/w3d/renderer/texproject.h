/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Texture Projector Class
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
#include "cullsys.h"
#include "matpass.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "matrixmapper.h"
#include "multilist.h"
#include "obbox.h"
#include "projector.h"
class SpecialRenderInfoClass;
class RenderObjClass;
class MaterialPassClass;
class SurfaceClass;
class ZTextureClass;

class TexProjectClass : public ProjectorClass, public CullableClass, public MultiListObjectClass
{
public:
    TexProjectClass();
    virtual ~TexProjectClass() override;

    void Set_Texture_Size(int size);
    int Get_Texture_Size();
    void Init_Multiplicative();
    void Init_Additive();
    void Set_Intensity(float intensity, bool immediate = false);
    float Get_Intensity();
    bool Is_Intensity_Zero();
    void Set_Attenuation(float attenuation);
    float Get_Attenuation();
    void Enable_Attenuation(bool onoff);
    bool Is_Attenuation_Enabled();
    MaterialPassClass *Peek_Material_Pass();

    void Enable_Affect_Dynamic_Objects(bool onoff) { Set_Flag(AFFECT_DYNAMIC_OBJS, onoff); }
    bool Is_Affect_Dynamic_Objects_Enabled() { return Get_Flag(AFFECT_DYNAMIC_OBJS); }
    void Enable_Affect_Static_Objects(bool onoff) { Set_Flag(AFFECT_STATIC_OBJS, onoff); }
    bool Is_Affect_Static_Objects_Enabled() { return Get_Flag(AFFECT_STATIC_OBJS); }
    void Enable_Depth_Gradient(bool onoff);
    bool Is_Depth_Gradient_Enabled(bool onoff);

    virtual void Set_Perspective_Projection(float hfov, float vfov, float znear, float zfar) override;
    virtual void Set_Ortho_Projection(float xmin, float xmax, float ymin, float ymax, float znear, float zfar) override;

    void Set_Texture(TextureClass *texture);
    TextureClass *Get_Texture() const;
    TextureClass *Peek_Texture() const;

    bool Compute_Perspective_Projection(
        RenderObjClass *obj, const Vector3 &lightpos, float znear = -1.0f, float zfar = -1.0f);
    bool Compute_Perspective_Projection(
        const AABoxClass &obj_box, const Matrix3D &tm, const Vector3 &lightpos, float znear = -1.0f, float zfar = -1.0f);

    bool Compute_Ortho_Projection(RenderObjClass *obj, const Vector3 &lightdir, float znear = -1.0f, float zfar = -1.0f);
    bool Compute_Ortho_Projection(
        const AABoxClass &obj_box, const Matrix3D &tm, const Vector3 &lightdir, float znear = -1.0f, float zfar = -1.0f);

    bool Needs_Render_Target();
    void Set_Render_Target(TextureClass *render_target, ZTextureClass *z_render_target);
    void Peek_Render_Target(TextureClass **render_target, ZTextureClass **z_render_target);

    bool Compute_Texture(RenderObjClass *model, SpecialRenderInfoClass *context);

    virtual void Pre_Render_Update(const Matrix3D &camera);

    virtual void *Get_Projection_Object_ID() const { return nullptr; }

    TexProjectClass *Hook_Ctor() { return new (this) TexProjectClass; }

protected:
    void Set_Flag(uint32_t flag, bool onoff);
    bool Get_Flag(uint32_t flag) const;
    virtual void Update_WS_Bounding_Volume();
    void Configure_Camera(CameraClass &camera);

    enum FlagsType
    {
        PERSPECTIVE = 0x00000001,
        ADDITIVE = 0x00000002,
        TEXTURE_DIRTY = 0x00000004,
        VOLATILE = 0x00000008,
        ATTENUATE = 0x00000010,
        AFFECT_DYNAMIC_OBJS = 0x00000020,
        AFFECT_STATIC_OBJS = 0x00000040,
        USE_DEPTH_GRADIENT = 0x00000080,
        HAS_RENDER_TARGET = 0x00000100,
        SIZE_MASK = 0xFFF00000,
        SIZE_SHIFT = 20,

        DEFAULT_FLAGS = ATTENUATE | AFFECT_DYNAMIC_OBJS | AFFECT_STATIC_OBJS
    };

    uint32_t m_flags;

    float m_desiredIntensity;
    float m_intensity;
    float m_attenuation;

    MaterialPassClass *m_materialPass;
    MatrixMapperClass *m_mapper1;
    TextureClass *m_renderTarget;
    ZTextureClass *m_zRenderTarget;

    float m_hFov;
    float m_vFov;
    float m_xMin;
    float m_xMax;
    float m_yMin;
    float m_yMax;
    float m_zNear;
    float m_zFar;
};
