/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Box render objects
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
#include "obbox.h"
#include "proto.h"
#include "rendobj.h"
#include "vector3.h"
#include "w3d_file.h"
#include "w3dmpo.h"

class BoxRenderObjClass : public RenderObjClass
{
protected:
    char m_name[32];
    Vector3 m_color;
    Vector3 m_objSpaceCenter;
    Vector3 m_objSpaceExtent;
    float m_opacity;
    static bool s_isInitted;
    static int s_displayMask;

public:
    static void Init();
    static void Shutdown();
    static void Set_Box_Display_Mask(int mask);

    BoxRenderObjClass(const BoxRenderObjClass &src);
    BoxRenderObjClass(const W3dBoxStruct &src);
    BoxRenderObjClass();

    BoxRenderObjClass &operator=(const BoxRenderObjClass &that);

    virtual ~BoxRenderObjClass() override {}
    virtual const char *Get_Name() const override { return m_name; }
    virtual void Set_Name(const char *name) override;
    virtual int Get_Num_Polys() const override { return 12; }
    virtual void Update_Cached_Box() = 0;

    void Set_Color(const Vector3 &color);
    void Render_Box(RenderInfoClass &rinfo, const Vector3 &center, const Vector3 &extent);
    void Vis_Render_Box(RenderInfoClass &rinfo, const Vector3 &center, const Vector3 &extent);

    void Set_Opacity(float opacity) { m_opacity = opacity; }
    const Vector3 &Get_Local_Center() const { return m_objSpaceCenter; }
    const Vector3 &Get_Local_Extent() const { return m_objSpaceExtent; }

    void Set_Local_Center_Extent(const Vector3 &center, const Vector3 &extent)
    {
        m_objSpaceCenter = center;
        m_objSpaceExtent = extent;
        Update_Cached_Box();
    }
};

class OBBoxRenderObjClass : public W3DMPO, public BoxRenderObjClass
{
    IMPLEMENT_W3D_POOL(OBBoxRenderObjClass);

private:
    OBBoxClass m_cachedBox;

public:
    OBBoxRenderObjClass(const OBBoxRenderObjClass &src);
    OBBoxRenderObjClass(const W3dBoxStruct &src);
    OBBoxRenderObjClass(const OBBoxClass &src);
    OBBoxRenderObjClass();

    OBBoxRenderObjClass &operator=(const OBBoxRenderObjClass &that);

    virtual ~OBBoxRenderObjClass() override {}
    virtual RenderObjClass *Clone() const override;
    virtual int Class_ID() const override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Special_Render(SpecialRenderInfoClass &rinfo) override;
    virtual void Set_Transform(const Matrix3D &tm) override;
    virtual void Set_Position(const Vector3 &pos) override;
    virtual bool Cast_Ray(RayCollisionTestClass &col) override;
    virtual bool Cast_AABox(AABoxCollisionTestClass &col) override;
    virtual bool Cast_OBBox(OBBoxCollisionTestClass &col) override;
    virtual bool Intersect_AABox(AABoxIntersectionTestClass &col) override;
    virtual bool Intersect_OBBox(OBBoxIntersectionTestClass &col) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual void Update_Cached_Box() override;

    OBBoxClass &Get_Box()
    {
        Validate_Transform();
        Update_Cached_Box();
        return m_cachedBox;
    }
};

class AABoxRenderObjClass : public W3DMPO, public BoxRenderObjClass
{
    IMPLEMENT_W3D_POOL(AABoxRenderObjClass);

private:
    AABoxClass m_cachedBox;

public:
    AABoxRenderObjClass(const AABoxRenderObjClass &src);
    AABoxRenderObjClass(const AABoxClass &src);
    AABoxRenderObjClass(const W3dBoxStruct &src);
    AABoxRenderObjClass();

    AABoxRenderObjClass &operator=(const AABoxRenderObjClass &that);

    virtual ~AABoxRenderObjClass() override {}
    virtual RenderObjClass *Clone() const override;
    virtual int Class_ID() const override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Special_Render(SpecialRenderInfoClass &rinfo) override;
    virtual void Set_Transform(const Matrix3D &tm) override;
    virtual void Set_Position(const Vector3 &pos) override;
    virtual bool Cast_Ray(RayCollisionTestClass &col) override;
    virtual bool Cast_AABox(AABoxCollisionTestClass &col) override;
    virtual bool Cast_OBBox(OBBoxCollisionTestClass &col) override;
    virtual bool Intersect_AABox(AABoxIntersectionTestClass &col) override;
    virtual bool Intersect_OBBox(OBBoxIntersectionTestClass &col) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual void Update_Cached_Box() override;

    AABoxClass &Get_Box()
    {
        Validate_Transform();
        Update_Cached_Box();
        return m_cachedBox;
    }
};

class BoxPrototypeClass : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(BoxPrototypeClass)
public:
    W3dBoxStruct box;
    BoxPrototypeClass(W3dBoxStruct &src) { box = src; }
    virtual ~BoxPrototypeClass() override {}
    virtual const char *Get_Name() const override { return box.Name; }

    virtual int Get_Class_ID() const override
    {
        if (box.Attributes & W3D_BOX_ATTRIBUTE_ORIENTED) {
            return RenderObjClass::CLASSID_OBBOX;
        } else {
            return RenderObjClass::CLASSID_AABOX;
        }
    }

    virtual RenderObjClass *Create() override
    {
        if (box.Attributes & W3D_BOX_ATTRIBUTE_ORIENTED) {
            return new OBBoxRenderObjClass(box);
        } else {
            return new AABoxRenderObjClass(box);
        }
    }

    virtual void Delete_Self() override { delete this; };
};

class BoxLoaderClass : public PrototypeLoaderClass
{
public:
    virtual int Chunk_Type() override { return W3D_CHUNK_BOX; }
    virtual PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};
