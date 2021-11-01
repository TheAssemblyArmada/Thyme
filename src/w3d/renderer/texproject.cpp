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
#include "texproject.h"
#include "assetmgr.h"
#include "bwrender.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "matpass.h"
#include "rendobj.h"
#include "rinfo.h"
#include "shader.h"
#include "texture.h"
#include "vertmaterial.h"
const float INTENSITY_RATE_OF_CHANGE = 1.0f;

TexProjectClass::TexProjectClass() :
    m_flags(DEFAULT_FLAGS),
    m_desiredIntensity(1.0f),
    m_intensity(1.0f),
    m_attenuation(1.0f),
    m_materialPass(nullptr),
    m_mapper1(nullptr),
    m_renderTarget(nullptr),
    m_zRenderTarget(nullptr),
    m_hFov(90.0f),
    m_vFov(90.0f),
    m_xMin(-10.0f),
    m_xMax(10.0f),
    m_yMin(-10.0f),
    m_yMax(10.0f),
    m_zNear(1.0f),
    m_zFar(1000.0f)
{
    m_materialPass = new MaterialPassClass();
    m_materialPass->Set_Cull_Volume(&m_worldBoundingVolume);

    VertexMaterialClass *vmtl = new VertexMaterialClass();
    captainslog_assert(vmtl != nullptr);

    vmtl->Set_Mapper(m_mapper);

    m_materialPass->Set_Material(vmtl);
    vmtl->Release_Ref();
    vmtl = nullptr;

    Init_Multiplicative();
}

TexProjectClass::~TexProjectClass()
{
    Ref_Ptr_Release(m_mapper1);
    Ref_Ptr_Release(m_materialPass);
    Ref_Ptr_Release(m_renderTarget);
    captainslog_assert(!m_zRenderTarget);
}

void TexProjectClass::Set_Texture_Size(int size)
{
    captainslog_assert(size > 0);
    captainslog_assert(size <= 512);
    m_flags &= ~SIZE_MASK;
    m_flags |= (size << SIZE_SHIFT);
}

int TexProjectClass::Get_Texture_Size()
{
    return (m_flags & SIZE_MASK) >> SIZE_SHIFT;
}

void TexProjectClass::Set_Flag(uint32_t flag, bool onoff)
{
    if (onoff) {
        m_flags |= flag;
    } else {
        m_flags &= ~flag;
    }
}

bool TexProjectClass::Get_Flag(uint32_t flag) const
{
    return (m_flags & flag) == flag;
}

void TexProjectClass::Set_Intensity(float intensity, bool immediate)
{
    captainslog_assert(intensity <= 1.0f);
    captainslog_assert(intensity >= 0.0f);

    m_desiredIntensity = intensity;

    if (immediate) {
        m_intensity = m_desiredIntensity;
    }
}

float TexProjectClass::Get_Intensity()
{
    return m_desiredIntensity;
}

bool TexProjectClass::Is_Intensity_Zero()
{
    return (m_intensity == 0.0f) && (m_desiredIntensity == 0.0f);
}

void TexProjectClass::Set_Attenuation(float attenuation)
{
    captainslog_assert(attenuation >= 0.0f);
    captainslog_assert(attenuation <= 1.0f);
    m_attenuation = attenuation;
}

float TexProjectClass::Get_Attenuation()
{
    return m_attenuation;
}

void TexProjectClass::Enable_Attenuation(bool onoff)
{
    Set_Flag(ATTENUATE, onoff);
}

bool TexProjectClass::Is_Attenuation_Enabled()
{
    return Get_Flag(ATTENUATE);
}

void TexProjectClass::Enable_Depth_Gradient(bool onoff)
{
    Set_Flag(USE_DEPTH_GRADIENT, onoff);

    if (Get_Flag(ADDITIVE)) {
        Init_Additive();
    } else {
        Init_Multiplicative();
    }
}

bool TexProjectClass::Is_Depth_Gradient_Enabled(bool onoff)
{
    return Get_Flag(USE_DEPTH_GRADIENT);
}

void TexProjectClass::Init_Multiplicative()
{
    Set_Flag(ADDITIVE, false);

    static ShaderClass mult_shader(Shader_Const(ShaderClass::PASS_LEQUAL,
        ShaderClass::DEPTH_WRITE_DISABLE,
        ShaderClass::COLOR_WRITE_ENABLE,
        ShaderClass::DSTBLEND_SRC_COLOR,
        ShaderClass::FOG_DISABLE,
        ShaderClass::GRADIENT_ADD,
        ShaderClass::SECONDARY_GRADIENT_DISABLE,
        ShaderClass::SRCBLEND_ZERO,
        ShaderClass::TEXTURING_ENABLE,
        ShaderClass::NPATCH_DISABLE,
        ShaderClass::ALPHATEST_DISABLE,
        ShaderClass::CULL_MODE_ENABLE,
        ShaderClass::DETAILCOLOR_DISABLE,
        ShaderClass::DETAILALPHA_DISABLE));

    if (W3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
        mult_shader.Enable_Fog("TexProjectClass");
    }

    if (Get_Flag(USE_DEPTH_GRADIENT)) {

        mult_shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_ADD);

        TextureClass *grad_tex = W3DAssetManager::Get_Instance()->Get_Texture("MultProjectorGradient.tga");
        if (grad_tex) {
            grad_tex->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            grad_tex->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            m_materialPass->Set_Texture(grad_tex, 1);
            grad_tex->Release_Ref();
        } else {
            captainslog_warn("Could not find texture: MultProjectorGradient.tga!\n");
        }

    } else {

        mult_shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_DISABLE);

        m_materialPass->Set_Texture(nullptr, 1);
    }

    m_materialPass->Set_Shader(mult_shader);

    VertexMaterialClass *vmtl = m_materialPass->Peek_Material();
    vmtl->Set_Ambient(0, 0, 0);
    vmtl->Set_Diffuse(0, 0, 0);
    vmtl->Set_Specular(0, 0, 0);
    vmtl->Set_Emissive(0.0f, 0.0f, 0.0f);
    vmtl->Set_Opacity(1.0f);
    vmtl->Set_Lighting(true);

    if (Get_Flag(USE_DEPTH_GRADIENT)) {
        if (m_mapper1 == nullptr) {
            m_mapper1 = new MatrixMapperClass(1);
        }
        m_mapper1->Set_Type(MatrixMapperClass::DEPTH_GRADIENT);
        vmtl->Set_Mapper(m_mapper1, 1);
    } else {
        vmtl->Set_Mapper(nullptr, 1);
    }
}

void TexProjectClass::Init_Additive()
{
    Set_Flag(ADDITIVE, true);

    static ShaderClass add_shader(Shader_Const(ShaderClass::PASS_LEQUAL,
        ShaderClass::DEPTH_WRITE_DISABLE,
        ShaderClass::COLOR_WRITE_ENABLE,
        ShaderClass::DSTBLEND_ONE,
        ShaderClass::FOG_DISABLE,
        ShaderClass::GRADIENT_MODULATE,
        ShaderClass::SECONDARY_GRADIENT_DISABLE,
        ShaderClass::SRCBLEND_ONE,
        ShaderClass::TEXTURING_ENABLE,
        ShaderClass::NPATCH_DISABLE,
        ShaderClass::ALPHATEST_DISABLE,
        ShaderClass::CULL_MODE_ENABLE,
        ShaderClass::DETAILCOLOR_DISABLE,
        ShaderClass::DETAILALPHA_DISABLE));

    if (W3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
        add_shader.Enable_Fog("TexProjectClass");
    }

    add_shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_SCALE);
    TextureClass *grad_tex = W3DAssetManager::Get_Instance()->Get_Texture("AddProjectorGradient.tga");

    if (grad_tex) {
        grad_tex->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        grad_tex->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        m_materialPass->Set_Texture(grad_tex, 1);
        grad_tex->Release_Ref();
    } else {
        captainslog_warn("Could not find texture: AddProjectorGradient.tga!\n");
    }

    m_materialPass->Set_Shader(add_shader);

    VertexMaterialClass *vmtl = m_materialPass->Peek_Material();
    vmtl->Set_Ambient(0, 0, 0);
    vmtl->Set_Diffuse(0, 0, 0);
    vmtl->Set_Specular(0, 0, 0);
    vmtl->Set_Emissive(1, 1, 1);
    vmtl->Set_Opacity(1.0f);
    vmtl->Set_Lighting(true);

    if (m_mapper1 == nullptr) {
        m_mapper1 = new MatrixMapperClass(1);
    }

    m_mapper1->Set_Type(MatrixMapperClass::NORMAL_GRADIENT);
    vmtl->Set_Mapper(m_mapper1, 1);
}

void TexProjectClass::Set_Texture(TextureClass *texture)
{
    if (texture != nullptr) {
        texture->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        texture->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        m_materialPass->Set_Texture(texture);
    }
}

TextureClass *TexProjectClass::Get_Texture() const
{
    return m_materialPass->Get_Texture();
}

TextureClass *TexProjectClass::Peek_Texture() const
{
    return m_materialPass->Peek_Texture();
}

MaterialPassClass *TexProjectClass::Peek_Material_Pass()
{
    return m_materialPass;
}

void TexProjectClass::Set_Perspective_Projection(float hfov, float vfov, float znear, float zfar)
{
    m_hFov = hfov;
    m_vFov = vfov;
    m_zNear = znear;
    m_zFar = zfar;

    ProjectorClass::Set_Perspective_Projection(hfov, vfov, znear, zfar);
    Set_Flag(PERSPECTIVE, true);
}

void TexProjectClass::Set_Ortho_Projection(float xmin, float xmax, float ymin, float ymax, float znear, float zfar)
{
    m_xMin = xmin;
    m_xMax = xmax;
    m_yMin = ymin;
    m_yMax = ymax;
    m_zNear = znear;
    m_zFar = zfar;

    ProjectorClass::Set_Ortho_Projection(xmin, xmax, ymin, ymax, znear, zfar);
    Set_Flag(PERSPECTIVE, false);
}

bool TexProjectClass::Compute_Perspective_Projection(RenderObjClass *model, const Vector3 &lightpos, float znear, float zfar)
{
    if (model == nullptr) {
        captainslog_warn("Attempting to generate projection for a nullptr model\r\n");
        return false;
    }

    AABoxClass box;
    model->Get_Obj_Space_Bounding_Box(box);
    const Matrix3D &tm = model->Get_Transform();

    return Compute_Perspective_Projection(box, tm, lightpos, znear, zfar);
}

bool TexProjectClass::Compute_Perspective_Projection(
    const AABoxClass &obj_box, const Matrix3D &tm, const Vector3 &lightpos, float user_znear, float user_zfar)
{
    Vector3 wrld_center;
    Matrix3D::Transform_Vector(tm, obj_box.m_center, &wrld_center);

    Matrix3D texture_tm, texture_tm_inv;
    texture_tm.Look_At(lightpos, wrld_center, 0.0f);
    texture_tm.Get_Orthogonal_Inverse(texture_tm_inv);

    AABoxClass box = obj_box;
    Matrix3D obj_to_world = tm;
    Matrix3D obj_to_texture;
    Matrix3D::Multiply(texture_tm_inv, obj_to_world, &obj_to_texture);
    box.Transform(obj_to_texture);

    if ((box.m_center.Z > 0.0f) || (box.m_extent.Z > GameMath::Fabs(box.m_center.Z))) {
        return false;
    }

    float znear = -box.m_center.Z;
    float zfar = -(box.m_center.Z - obj_box.m_extent.Quick_Length()) * 2.0f;

    if (user_znear != -1.0f) {
        znear = box.m_center.Z + user_znear;
    }
    if (user_zfar != -1.0f) {
        zfar = box.m_center.Z + user_zfar;
    }

    float tan_hfov2 = GameMath::Fabs(box.m_extent.X / (box.m_center.Z + box.m_extent.Z));
    float tan_vfov2 = GameMath::Fabs(box.m_extent.Y / (box.m_center.Z + box.m_extent.Z));
    float hfov = 2.0f * GameMath::Atan(tan_hfov2);
    float vfov = 2.0f * GameMath::Atan(tan_vfov2);

    Set_Perspective_Projection(hfov, vfov, znear, zfar);
    Set_Transform(texture_tm);
    return true;
}

bool TexProjectClass::Compute_Ortho_Projection(RenderObjClass *model, const Vector3 &lightdir, float znear, float zfar)
{
    if (model == nullptr) {
        captainslog_warn(("Attempting to generate projection for a nullptr model\r\n"));
        return false;
    }

    AABoxClass box;
    model->Get_Obj_Space_Bounding_Box(box);
    const Matrix3D &tm = model->Get_Transform();

    return Compute_Ortho_Projection(box, tm, lightdir, znear, zfar);
}

bool TexProjectClass::Compute_Ortho_Projection(
    const AABoxClass &obj_box, const Matrix3D &tm, const Vector3 &lightdir, float user_znear, float user_zfar)
{
    AABoxClass wrldbox = obj_box;
    wrldbox.Transform(tm);

    Vector3 camera_target = wrldbox.m_center;
    Vector3 camera_position = camera_target - 2.0f * wrldbox.m_extent.Length() * lightdir;

    Matrix3D texture_tm, texture_tm_inv;
    texture_tm.Look_At(camera_position, camera_target, 0.0f);
    texture_tm.Get_Orthogonal_Inverse(texture_tm_inv);

    AABoxClass box = obj_box;
    Matrix3D obj_to_world = tm;
    Matrix3D obj_to_texture;
    Matrix3D::Multiply(texture_tm_inv, obj_to_world, &obj_to_texture);
    box.Transform(obj_to_texture);

    box.m_extent *= 1.0f;

    float znear = -box.m_center.Z;
    float zfar = -(box.m_center.Z - obj_box.m_extent.Quick_Length()) * 2.0f;

    if (user_znear != -1.0f) {
        znear = -box.m_center.Z + user_znear;
    }
    if (user_zfar != -1.0f) {
        zfar = -box.m_center.Z + user_zfar;
    }

    Set_Ortho_Projection(box.m_center.X - box.m_extent.X,
        box.m_center.X + box.m_extent.X,
        box.m_center.Y - box.m_extent.Y,
        box.m_center.Y + box.m_extent.Y,
        znear,
        zfar);
    Set_Transform(texture_tm);
    return true;
}

bool TexProjectClass::Compute_Texture(RenderObjClass *model, SpecialRenderInfoClass *context)
{
#ifdef BUILD_WITH_D3D8
    if ((model == nullptr) || (context == nullptr)) {
        return false;
    }
    TextureClass *rtarget;
    ZTextureClass *zrtarget;
    Peek_Render_Target(&rtarget, &zrtarget);

    if (rtarget != nullptr) {
        context->m_texProject = this;
        DX8Wrapper::Set_Render_Target_With_Z(rtarget, zrtarget);

        Configure_Camera(context->m_camera);

        Vector3 color(0.0f, 0.0f, 0.0f);
        if (Get_Flag(ADDITIVE) == false) {
            color.Set(1.0f, 1.0f, 1.0f);
        }

        W3D::Begin_Render(true, zrtarget != nullptr, color);
        W3D::Render(*model, *context);
        W3D::End_Render(false);

        DX8Wrapper::Set_Render_Target((w3dsurface_t) nullptr);
    }
#endif
    return true;
}

bool TexProjectClass::Needs_Render_Target()
{
    return Get_Flag(TEXTURE_DIRTY);
}

void TexProjectClass::Set_Render_Target(TextureClass *render_target, ZTextureClass *z_render_target)
{
    Ref_Ptr_Set(m_renderTarget, render_target);
    Set_Texture(m_renderTarget);
    captainslog_assert(!z_render_target);
}

void TexProjectClass::Peek_Render_Target(TextureClass **render_target, ZTextureClass **z_render_target)
{
    if (render_target) {
        *render_target = m_renderTarget;

        if (z_render_target) {
            *z_render_target = m_zRenderTarget;
        }
    }
}

void TexProjectClass::Configure_Camera(CameraClass &camera)
{
    camera.Set_Transform(m_transform);
    camera.Set_Clip_Planes(0.01f, m_zFar);

    if (Get_Flag(PERSPECTIVE)) {
        camera.Set_Projection_Type(CameraClass::PERSPECTIVE);
        camera.Set_View_Plane(m_hFov, m_vFov);

    } else {
        camera.Set_Projection_Type(CameraClass::ORTHO);
        camera.Set_View_Plane(Vector2(m_xMin, m_yMin), Vector2(m_xMax, m_yMax));
    }

    float size = Get_Texture_Size();
    float inv_size = 1.0f / size;
    Vector2 vmin(1.0f * inv_size, 1.0f * inv_size);
    Vector2 vmax((size - 1.0f) * inv_size, (size - 1.0f) * inv_size);
    camera.Set_Viewport(vmin, vmax);
}

void TexProjectClass::Pre_Render_Update(const Matrix3D &camera)
{
    Matrix3D world_to_texture;
    Matrix3D tmp;
    Matrix4 view_to_texture;

    m_transform.Get_Orthogonal_Inverse(world_to_texture);
    Matrix3D::Multiply(world_to_texture, camera, &tmp);
    Matrix4::Multiply(m_projection, tmp, &view_to_texture);

    float frame_time = (float)W3D::Get_Frame_Time() / 1000.0f;
    float intensity_delta = m_desiredIntensity - m_intensity;
    float max_intensity_delta = INTENSITY_RATE_OF_CHANGE * frame_time;

    if (intensity_delta > max_intensity_delta) {
        m_intensity += max_intensity_delta;
    } else if (intensity_delta < -max_intensity_delta) {
        m_intensity -= max_intensity_delta;
    } else {
        m_intensity = m_desiredIntensity;
    }

    float actual_intensity = m_intensity * m_attenuation;

    VertexMaterialClass *vmat = m_materialPass->Peek_Material();
    if (Get_Flag(ADDITIVE)) {
        vmat->Set_Emissive(actual_intensity, actual_intensity, actual_intensity);
    } else {
        vmat->Set_Emissive(1.0f - actual_intensity, 1.0f - actual_intensity, 1.0f - actual_intensity);
    }

    if (Get_Flag(PERSPECTIVE)) {
        m_mapper->Set_Type(MatrixMapperClass::PERSPECTIVE_PROJECTION);
    } else {
        m_mapper->Set_Type(MatrixMapperClass::ORTHO_PROJECTION);
    }

    if (Get_Texture_Size() == 0) {
        Set_Texture_Size(m_materialPass->Peek_Texture()->Get_Width());
        captainslog_assert(Get_Texture_Size() != 0);
    }

    m_mapper->Set_Texture_Transform(view_to_texture, Get_Texture_Size());

    if (m_mapper1) {
        m_mapper1->Set_Texture_Transform(view_to_texture, Get_Texture_Size());
    }
}

void TexProjectClass::Update_WS_Bounding_Volume()
{
    ProjectorClass::Update_WS_Bounding_Volume();

    Vector3 extent;
    m_worldBoundingVolume.Compute_Axis_Aligned_Extent(&extent);
    Set_Cull_Box(AABoxClass(m_worldBoundingVolume.m_center, extent));
}
