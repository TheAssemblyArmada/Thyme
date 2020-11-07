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
#pragma once

#include "always.h"
#include "colmath.h"
#include "frustum.h"
#include "matrix4.h"
#include "obbox.h"
#include "plane.h"
#include "rendobj.h"
#include "vector2.h"

class RenderInfoClass;

class ViewportClass
{
public:
    ViewportClass() : m_min(0.0f, 0.0f), m_max(1.0f, 1.0f) {}
    ViewportClass(const Vector2 &min, const Vector2 &max) : m_min(min), m_max(max) {}
    ViewportClass(const ViewportClass &vp) : m_min(vp.m_min), m_max(vp.m_max) {}

    float Width() const { return m_max.X - m_min.X; }
    float Height() const { return m_max.Y - m_min.Y; }

    Vector2 m_min;
    Vector2 m_max;
};

class CameraClass : public RenderObjClass
{
public:
    enum ProjectionType
    {
        PERSPECTIVE = 0,
        ORTHO
    };

    enum ProjectionResType
    {
        INSIDE_FRUSTUM,
        OUTSIDE_FRUSTUM,
        OUTSIDE_NEAR_CLIP,
        OUTSIDE_FAR_CLIP,
    };

    CameraClass();
    CameraClass(const CameraClass &src);

    virtual ~CameraClass() {}

    CameraClass &operator=(const CameraClass &);

    virtual RenderObjClass *Clone() const;

    virtual int Class_ID() const { return CLASSID_CAMERA; }

    // Rendering
    virtual void Render(RenderInfoClass &rinfo) {}

    // Scene
    virtual void Set_Transform(const Matrix3D &m);
    virtual void Set_Position(const Vector3 &v);

    // Bounding
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const;

    // Depth
    float Get_Depth() const;

    // Projection type
    void Set_Projection_Type(ProjectionType ptype);
    ProjectionType Get_Projection_Type();

    // Clipping
    void Set_Clip_Planes(float znear, float zfar);
    void Get_Clip_Planes(float &znear, float &zfar) const;

    // View plane
    void Set_View_Plane(const Vector2 &min, const Vector2 &max);
    void Set_View_Plane(float hfov, float vfov = -1);
    void Set_Aspect_Ratio(float width_to_height);
    void Get_View_Plane(Vector2 &set_min, Vector2 &set_max) const;
    float Get_Horizontal_FOV() const;
    float Get_Vertical_FOV() const;
    float Get_Aspect_Ratio() const;

    // Projection
    void Get_Projection_Matrix(Matrix4 *set_tm);
    void Get_D3D_Projection_Matrix(Matrix4 *set_tm);
    void Get_View_Matrix(Matrix3D *set_tm);
    const Matrix4 &Get_Projection_Matrix();
    const Matrix3D &Get_View_Matrix();

    // Projecting and un-projecting a point
    ProjectionResType Project(Vector3 &dest, const Vector3 &ws_point) const;
    ProjectionResType Project_Camera_Space_Point(Vector3 &dest, const Vector3 &cam_point) const;
    void Un_Project(Vector3 &dest, const Vector2 &view_point) const;
    void Transform_To_View_Space(Vector3 &dest, const Vector3 &ws_point) const;
    void Rotate_To_View_Space(Vector3 &dest, const Vector3 &ws_vector) const;

    // Viewport
    void Set_Viewport(const Vector2 &min, const Vector2 &max);
    void Get_Viewport(Vector2 &set_min, Vector2 &set_max) const;
    const ViewportClass &Get_Viewport() const;
    void Set_Depth_Range(float zstart = 0.0f, float zend = 1.0f);
    void Get_Depth_Range(float *set_zstart, float *set_zend) const;

    // Culling
    bool Cull_Sphere(const SphereClass &sphere) const;
    bool Cull_Sphere_On_Frustum_Sides(const SphereClass &sphere) const;
    bool Cull_Box(const AABoxClass &box) const;

    // Frustum
    const FrustumClass &Get_Frustum() const;
    const PlaneClass *Get_Frustum_Planes() const;
    const Vector3 *Get_Frustum_Corners() const;
    const FrustumClass &Get_View_Space_Frustum() const;
    const PlaneClass *Get_View_Space_Frustum_Planes() const;
    const Vector3 *Get_View_Space_Frustum_Corners() const;
    const OBBoxClass &Get_Near_Clip_Bounding_Box() const;

    // Coordinates
    void Device_To_View_Space(const Vector2 &device_coord, Vector3 *view_coord);
    void Device_To_World_Space(const Vector2 &device_coord, Vector3 *world_coord);
    float Compute_Projected_Sphere_Radius(float dist, float radius);

    // Apply camera to the W3D scene.
    void Apply();

    // Conversion for some old format
    static void Convert_Old(Vector3 &pos);

protected:
    void Update_Frustum() const;

    ProjectionType m_projection;
    ViewportClass m_viewport;
    ViewportClass m_viewPlane;
    float m_aspectRatio;
    float m_zNear;
    float m_zFar;
    float m_zBufferMin;
    float m_zBufferMax;

    mutable bool m_frustumValid;
    mutable FrustumClass m_frustum;
    mutable FrustumClass m_viewSpaceFrustum;
    mutable OBBoxClass m_nearClipBBox;
    mutable Matrix4 m_projectionTransform;
    mutable Matrix3D m_cameraInvTransform;
};

inline float CameraClass::Get_Depth() const
{
    return m_zFar;
}

inline void CameraClass::Set_Projection_Type(ProjectionType ptype)
{
    m_frustumValid = false;
    m_projection = ptype;
}

inline CameraClass::ProjectionType CameraClass::Get_Projection_Type()
{
    return m_projection;
}

inline void CameraClass::Set_Viewport(const Vector2 &min, const Vector2 &max)
{
    m_viewport.m_min = min;
    m_viewport.m_max = max;
    m_frustumValid = false;
}

inline void CameraClass::Get_Viewport(Vector2 &set_min, Vector2 &set_max) const
{
    set_min = m_viewport.m_min;
    set_max = m_viewport.m_max;
}

inline void CameraClass::Set_Depth_Range(float zmin, float zmax)
{
    m_zBufferMin = zmin;
    m_zBufferMax = zmax;
}

inline void CameraClass::Get_Depth_Range(float *set_zmin, float *set_zmax) const
{
    if (set_zmin != NULL) {
        *set_zmin = m_zBufferMin;
    }
    if (set_zmax != NULL) {
        *set_zmax = m_zBufferMax;
    }
}

inline const ViewportClass &CameraClass::Get_Viewport() const
{
    return m_viewport;
}

inline bool CameraClass::Cull_Sphere(const SphereClass &sphere) const
{
    const FrustumClass &frustum = Get_Frustum();
    return CollisionMath::Overlap_Test(frustum, sphere) == CollisionMath::OUTSIDE;
}

inline bool CameraClass::Cull_Sphere_On_Frustum_Sides(const SphereClass &sphere) const
{
    const FrustumClass &frustum = Get_Frustum();
    const PlaneClass *planes = frustum.m_planes;

    bool is_visible = true;

    for (int i = 1; i < ARRAY_SIZE(frustum.m_planes) - 1; i++) {
        is_visible =
            is_visible && (CollisionMath::Overlap_Test(planes[i], sphere) & (CollisionMath::INSIDE | CollisionMath::BOTH));
    }
    return !is_visible;
}

inline const FrustumClass &CameraClass::Get_Frustum() const
{
    Update_Frustum();
    return m_frustum;
}

inline const PlaneClass *CameraClass::Get_Frustum_Planes() const
{
    const FrustumClass &frustum = Get_Frustum();
    return frustum.m_planes;
}

inline const Vector3 *CameraClass::Get_Frustum_Corners() const
{
    const FrustumClass &frustum = Get_Frustum();
    return frustum.m_corners;
}

inline const FrustumClass &CameraClass::Get_View_Space_Frustum() const
{
    Update_Frustum();
    return m_viewSpaceFrustum;
}

inline const PlaneClass *CameraClass::Get_View_Space_Frustum_Planes() const
{
    const FrustumClass &frustum = Get_View_Space_Frustum();
    return frustum.m_planes;
}

inline const Vector3 *CameraClass::Get_View_Space_Frustum_Corners() const
{
    const FrustumClass &frustum = Get_View_Space_Frustum();
    return frustum.m_corners;
}
