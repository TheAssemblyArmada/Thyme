/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "camera.h"
#include "dx8wrapper.h"
#include "gamemath.h"
#include "matrix4.h"
#include "w3d.h"

CameraClass::CameraClass() :
    m_projection(PERSPECTIVE),
    m_viewport(),
    m_aspectRatio(4.0f / 3.0f),
    m_zNear(1.0f),
    m_zFar(1000.0f),
    m_zBufferMin(0.0f),
    m_zBufferMax(1.0f),
    m_frustumValid(false)
{
    Set_Transform(Matrix3D(true));
    Set_View_Plane(DEG_TO_RADF(50.f));
}

CameraClass::CameraClass(const CameraClass &src) :
    RenderObjClass(src),
    m_projection(src.m_projection),
    m_viewport(src.m_viewport),
    m_viewPlane(src.m_viewPlane),
    m_aspectRatio(src.m_aspectRatio),
    m_zNear(src.m_zNear),
    m_zFar(src.m_zFar),
    m_zBufferMin(src.m_zBufferMin),
    m_zBufferMax(src.m_zBufferMax),
    m_frustumValid(src.m_frustumValid),
    m_frustum(src.m_frustum),
    m_nearClipBBox(src.m_nearClipBBox),
    m_projectionTransform(src.m_projectionTransform),
    m_cameraInvTransform(src.m_cameraInvTransform)
{
    m_frustumValid = false;
}

CameraClass &CameraClass::operator=(const CameraClass &that)
{
    if (this != &that) {
        RenderObjClass::operator=(that);

        m_projection = that.m_projection;
        m_viewport = that.m_viewport;
        m_viewPlane = that.m_viewPlane;
        m_zNear = that.m_zNear;
        m_zFar = that.m_zFar;
        m_frustumValid = that.m_frustumValid;
        m_frustum = that.m_frustum;
        m_nearClipBBox = that.m_nearClipBBox;
        m_projectionTransform = that.m_projectionTransform;
        m_cameraInvTransform = that.m_cameraInvTransform;

        m_frustumValid = false;
    }

    return *this;
}

RenderObjClass *CameraClass::Clone() const
{
    return new CameraClass(*this);
}

void CameraClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Center.Set(0.0f, 0.0f, 0.0f);
    sphere.Radius = m_zFar;
}

void CameraClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.m_center.Set(0.0f, 0.0f, 0.0f);
    box.m_extent.Set(m_zFar, m_zFar, m_zFar);
}

void CameraClass::Set_Transform(const Matrix3D &m)
{
    RenderObjClass::Set_Transform(m);
    m_frustumValid = false;
}

void CameraClass::Set_Position(const Vector3 &v)
{
    RenderObjClass::Set_Position(v);
    m_frustumValid = false;
}

void CameraClass::Set_View_Plane(const Vector2 &vmin, const Vector2 &vmax)
{
    m_viewPlane.m_min = vmin;
    m_viewPlane.m_max = vmax;
    m_aspectRatio = (vmax.X - vmin.X) / (vmax.Y - vmin.Y);
    m_frustumValid = false;
}

void CameraClass::Set_View_Plane(float hfov, float vfov)
{
    float width_half = GameMath::Tan(hfov / 2.0f);
    float height_half = 0.0f;

    if (vfov == -1) {
        height_half = (1.0f / m_aspectRatio) * width_half;
    } else {
        height_half = GameMath::Tan(vfov / 2.0f);
        m_aspectRatio = width_half / height_half;
    }

    m_viewPlane.m_min.Set(-width_half, -height_half);
    m_viewPlane.m_max.Set(width_half, height_half);

    m_frustumValid = false;
}

void CameraClass::Set_Aspect_Ratio(float width_to_height)
{
    m_aspectRatio = width_to_height;
    m_viewPlane.m_min.Y = m_viewPlane.m_min.X / m_aspectRatio;
    m_viewPlane.m_max.Y = m_viewPlane.m_max.X / m_aspectRatio;
    m_frustumValid = false;
}

void CameraClass::Get_View_Plane(Vector2 &set_min, Vector2 &set_max) const
{
    set_min = m_viewPlane.m_min;
    set_max = m_viewPlane.m_max;
}

CameraClass::ProjectionResType CameraClass::Project(Vector3 &dest, const Vector3 &ws_point) const
{
    Update_Frustum();

    Vector3 cam_point;
    Matrix3D::Transform_Vector(m_cameraInvTransform, ws_point, &cam_point);

    if (cam_point.Z > -m_zNear) {
        dest.Set(0, 0, 0);
        return OUTSIDE_NEAR_CLIP;
    }

    Vector4 view_point = m_projectionTransform * cam_point;
    float oow = 1.0f / view_point.W;
    dest.X = view_point.X * oow;
    dest.Y = view_point.Y * oow;
    dest.Z = view_point.Z * oow;

    if (dest.Z > 1.0f) {
        return OUTSIDE_FAR_CLIP;
    }

    if ((dest.X < -1.0f) || (dest.X > 1.0f) || (dest.Y < -1.0f) || (dest.Y > 1.0f)) {
        return OUTSIDE_FRUSTUM;
    }

    return INSIDE_FRUSTUM;
}

CameraClass::ProjectionResType CameraClass::Project_Camera_Space_Point(Vector3 &dest, const Vector3 &cam_point) const
{
    Update_Frustum();

    if (cam_point.Z > -m_zNear + GAMEMATH_EPSILON) {
        dest.Set(0, 0, 0);
        return OUTSIDE_NEAR_CLIP;
    }

    Vector4 view_point = m_projectionTransform * cam_point;
    float oow = 1.0f / view_point.W;
    dest.X = view_point.X * oow;
    dest.Y = view_point.Y * oow;
    dest.Z = view_point.Z * oow;

    if (dest.Z > 1.0f) {
        return OUTSIDE_FAR_CLIP;
    }

    if ((dest.X < -1.0f) || (dest.X > 1.0f) || (dest.Y < -1.0f) || (dest.Y > 1.0f)) {
        return OUTSIDE_FRUSTUM;
    }

    return INSIDE_FRUSTUM;
}

void CameraClass::Un_Project(Vector3 &dest, const Vector2 &view_point) const
{
    float vpdx = m_viewPlane.Width();
    float vpdy = m_viewPlane.Height();

    Vector3 point;
    point.X = m_viewPlane.m_min.X + vpdx * (view_point.X + 1.0f) * 0.5f;
    point.Y = m_viewPlane.m_min.Y + vpdy * (view_point.Y + 1.0f) * 0.5f;
    point.Z = -1.0f;

    Matrix3D::Transform_Vector(m_transform, point, &dest);
}

void CameraClass::Transform_To_View_Space(Vector3 &dest, const Vector3 &ws_point) const
{
    Update_Frustum();
    Matrix3D::Transform_Vector(m_cameraInvTransform, ws_point, &dest);
}

void CameraClass::Rotate_To_View_Space(Vector3 &dest, const Vector3 &ws_vector) const
{
    Update_Frustum();
    Matrix3D::Rotate_Vector(m_cameraInvTransform, ws_vector, &dest);
}

const OBBoxClass &CameraClass::Get_Near_Clip_Bounding_Box() const
{
    Update_Frustum();
    return m_nearClipBBox;
}

bool CameraClass::Cull_Box(const AABoxClass &box) const
{
    const FrustumClass &frustum = Get_Frustum();
    return CollisionMath::Overlap_Test(frustum, box) == CollisionMath::OUTSIDE;
}

void CameraClass::Update_Frustum() const
{
    if (m_frustumValid) {
        return;
    }

    Vector2 vpmin, vpmax;
    float znear, zfar;
    float znear_dist, zfar_dist;

    Matrix3D cam_mat = Get_Transform();
    Get_View_Plane(vpmin, vpmax);
    Get_Clip_Planes(znear_dist, zfar_dist);

    znear = -znear_dist;
    zfar = -zfar_dist;

    m_frustumValid = true;
    m_frustum.Init(cam_mat, vpmin, vpmax, znear, zfar);
    m_viewSpaceFrustum.Init(Matrix3D(true), vpmin, vpmax, znear, zfar);

    m_nearClipBBox.m_center = cam_mat * Vector3(0, 0, znear);
    m_nearClipBBox.m_extent.X = (vpmax.X - vpmin.X) * (-znear) * 0.5f;
    m_nearClipBBox.m_extent.Y = (vpmax.Y - vpmin.Y) * (-znear) * 0.5f;
    m_nearClipBBox.m_extent.Z = 0.01f;
    m_nearClipBBox.m_basis.Set(cam_mat);

    m_transform.Get_Inverse(m_cameraInvTransform);

    if (m_projection == PERSPECTIVE) {
        m_projectionTransform.Init_Perspective(
            vpmin.X * znear_dist, vpmax.X * znear_dist, vpmin.Y * znear_dist, vpmax.Y * znear_dist, znear_dist, zfar_dist);

    } else {
        m_projectionTransform.Init_Ortho(vpmin.X, vpmax.X, vpmin.Y, vpmax.Y, znear_dist, zfar_dist);
    }
}

void CameraClass::Device_To_View_Space(const Vector2 &device_coord, Vector3 *set_view)
{
    int res_width;
    int res_height;
    int res_bits;
    bool windowed;

    W3D::Get_Render_Target_Resolution(res_width, res_height, res_bits, windowed);

    Vector2 ndev;
    ndev.X = device_coord.X / (float)res_width;
    ndev.Y = device_coord.Y / (float)res_height;

    Vector2 vs_min;
    Vector2 vs_max;
    Get_View_Plane(vs_min, vs_max);

    set_view->X = vs_min.X + (ndev.X - m_viewport.m_min.X) * (vs_max.X - vs_min.X) / (m_viewport.Width());
    set_view->Y = vs_max.Y - (ndev.Y - m_viewport.m_min.Y) * (vs_max.Y - vs_min.Y) / (m_viewport.Height());
    set_view->Z = -1.0f;
}

void CameraClass::Device_To_World_Space(const Vector2 &device_coord, Vector3 *world_coord)
{
    Vector3 vs;
    Device_To_View_Space(device_coord, &vs);
    Matrix3D::Transform_Vector(m_transform, vs, world_coord);
}

void CameraClass::Apply()
{
    Update_Frustum();

    int width, height, bits;
    bool windowed;
    W3D::Get_Render_Target_Resolution(width, height, bits, windowed);

#ifdef BUILD_WITH_D3D8
    D3DVIEWPORT8 vp;
    vp.X = (DWORD)(m_viewport.m_min.X * (float)width);
    vp.Y = (DWORD)(m_viewport.m_min.Y * (float)height);
    vp.Width = (DWORD)(m_viewport.Width() * (float)width);
    vp.Height = (DWORD)(m_viewport.Height() * (float)height);
    vp.MinZ = m_zBufferMin;
    vp.MaxZ = m_zBufferMax;
    DX8Wrapper::Set_Viewport(&vp);

    Matrix4 d3dprojection;
    Get_D3D_Projection_Matrix(&d3dprojection);
    DX8Wrapper::Set_Projection_Transform_With_Z_Bias(d3dprojection, m_zNear, m_zFar);
    DX8Wrapper::Set_Transform(D3DTS_VIEW, m_cameraInvTransform);
#endif
}

void CameraClass::Set_Clip_Planes(float znear, float zfar)
{
    m_frustumValid = false;
    m_zNear = znear;
    m_zFar = zfar;
}

void CameraClass::Get_Clip_Planes(float &znear, float &zfar) const
{
    znear = m_zNear;
    zfar = m_zFar;
}

float CameraClass::Get_Horizontal_FOV() const
{
    return 2 * GameMath::Atan2(m_viewPlane.Width(), 2.0f);
}

float CameraClass::Get_Vertical_FOV() const
{
    return 2 * GameMath::Atan2(m_viewPlane.Height(), 2.0f);
}

float CameraClass::Get_Aspect_Ratio() const
{
    return m_aspectRatio;
}

void CameraClass::Get_Projection_Matrix(Matrix4 *set_tm)
{
    captainslog_assert(set_tm != NULL);

    Update_Frustum();
    *set_tm = m_projectionTransform;
}

void CameraClass::Get_D3D_Projection_Matrix(Matrix4 *set_tm)
{
    captainslog_assert(set_tm != NULL);

    Update_Frustum();

    *set_tm = m_projectionTransform;

    float oozdiff = 1.0 / (m_zFar - m_zNear); // Original uses double.
    if (m_projection == PERSPECTIVE) {
        (*set_tm)[2][2] = -(m_zFar)*oozdiff;
        (*set_tm)[2][3] = -(m_zFar * m_zNear) * oozdiff;
    } else {
        (*set_tm)[2][2] = -oozdiff;
        (*set_tm)[2][3] = -m_zNear * oozdiff;
    }
}

void CameraClass::Get_View_Matrix(Matrix3D *set_tm)
{
    captainslog_assert(set_tm != NULL);

    Update_Frustum();
    *set_tm = m_cameraInvTransform;
}

const Matrix4 &CameraClass::Get_Projection_Matrix()
{
    Update_Frustum();
    return m_projectionTransform;
}

const Matrix3D &CameraClass::Get_View_Matrix()
{
    Update_Frustum();
    return m_cameraInvTransform;
}

void CameraClass::Convert_Old(Vector3 &pos)
{
    pos.X = (pos.X + 1) / 2;
    pos.Y = (pos.Y + 1) / 2;
}

float CameraClass::Compute_Projected_Sphere_Radius(float dist, float radius)
{
    Vector4 result = m_projectionTransform * Vector4(radius, 0.0f, dist, 1.0f);
    return result.X / result.W;
}
