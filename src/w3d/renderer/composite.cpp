/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Composite Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "composite.h"

CompositeRenderObjClass::CompositeRenderObjClass() {}

CompositeRenderObjClass::CompositeRenderObjClass(const CompositeRenderObjClass &src) : RenderObjClass(src)
{
    Set_Name(src.Get_Name());
    Set_Base_Model_Name(src.Get_Base_Model_Name());
}

CompositeRenderObjClass &CompositeRenderObjClass::operator=(const CompositeRenderObjClass &that)
{
    Set_Name(that.Get_Name());
    Set_Base_Model_Name(that.Get_Base_Model_Name());
    return *this;
}

CompositeRenderObjClass::~CompositeRenderObjClass() {}

void CompositeRenderObjClass::Restart()
{
    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        robj->Restart();
        robj->Release_Ref();
    }
}

const char *CompositeRenderObjClass::Get_Name() const
{
    return m_name;
}

void CompositeRenderObjClass::Set_Name(const char *name)
{
    m_name = name;
}

const char *CompositeRenderObjClass::Get_Base_Model_Name() const
{
    return m_baseModelName.Is_Empty() ? nullptr : m_baseModelName.Peek_Buffer();
}

void CompositeRenderObjClass::Set_Base_Model_Name(const char *name)
{
    m_baseModelName = name;
}

int CompositeRenderObjClass::Get_Num_Polys() const
{
    int polys = 0;

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        polys += robj->Get_Num_Polys();
        robj->Release_Ref();
    }

    return polys;
}

void CompositeRenderObjClass::Notify_Added(SceneClass *scene)
{
    RenderObjClass::Notify_Added(scene);

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        robj->Notify_Added(scene);
        robj->Release_Ref();
    }
}

void CompositeRenderObjClass::Notify_Removed(SceneClass *scene)
{
    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        robj->Notify_Removed(scene);
        robj->Release_Ref();
    }

    RenderObjClass::Notify_Removed(scene);
}

bool CompositeRenderObjClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    bool collides = false;

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        collides = (robj->Cast_Ray(raytest) | collides) != 0;
        robj->Release_Ref();
    }

    return collides;
}

bool CompositeRenderObjClass::Cast_AABox(AABoxCollisionTestClass &boxtest)
{
    bool collides = false;

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        collides = (robj->Cast_AABox(boxtest) | collides) != 0;
        robj->Release_Ref();
    }

    return collides;
}

bool CompositeRenderObjClass::Cast_OBBox(OBBoxCollisionTestClass &boxtest)
{
    bool collides = false;

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        collides = (robj->Cast_OBBox(boxtest) | collides) != 0;
        robj->Release_Ref();
    }

    return collides;
}

bool CompositeRenderObjClass::Intersect_AABox(AABoxIntersectionTestClass &boxtest)
{
    bool intersects = false;

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        intersects = (robj->Intersect_AABox(boxtest) | intersects) != 0;
        robj->Release_Ref();
    }

    return intersects;
}

bool CompositeRenderObjClass::Intersect_OBBox(OBBoxIntersectionTestClass &boxtest)
{
    bool intersects = false;

    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        captainslog_assert(robj);
        intersects = (robj->Intersect_OBBox(boxtest) | intersects) != 0;
        robj->Release_Ref();
    }

    return intersects;
}

void CompositeRenderObjClass::Create_Decal(DecalGeneratorClass *generator)
{
    captainslog_dbgassert(0, "decals not supported");
}

void CompositeRenderObjClass::Delete_Decal(unsigned long decal_id)
{
    captainslog_dbgassert(0, "decals not supported");
}

void CompositeRenderObjClass::Update_Obj_Space_Bounding_Volumes()
{
    if (Get_Num_Sub_Objects() > 0) {
        RenderObjClass *robj = Get_Sub_Object(0);
        captainslog_assert(robj);
        robj->Get_Obj_Space_Bounding_Sphere(m_objSphere);
        AABoxClass obj_aabox;
        robj->Get_Obj_Space_Bounding_Box(obj_aabox);
        robj->Release_Ref();
        MinMaxAABoxClass box(obj_aabox);

        for (int index = 1; index < Get_Num_Sub_Objects(); index++) {
            robj = Get_Sub_Object(index);
            captainslog_assert(robj);
            SphereClass sphere;
            robj->Get_Obj_Space_Bounding_Sphere(sphere);
            robj->Get_Obj_Space_Bounding_Box(obj_aabox);
            m_objSphere.Add_Sphere(sphere);
            box.Add_Box(obj_aabox);
            robj->Release_Ref();
        }

        m_objBox.Init(box);
        Invalidate_Cached_Bounding_Volumes();
        if (Get_Container()) {
            Get_Container()->Update_Obj_Space_Bounding_Volumes();
        }
    } else {
        m_objSphere.Init(Vector3(0, 0, 0), 0);
        m_objBox.Init(Vector3(0, 0, 0), Vector3(0, 0, 0));
    }
}

void CompositeRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box = m_objBox;
}

void CompositeRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere = m_objSphere;
}

void CompositeRenderObjClass::Set_User_Data(void *value, bool recursive)
{
    m_userData = value;

    if (recursive) {
        for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
            RenderObjClass *robj = Get_Sub_Object(i);
            captainslog_assert(robj);
            robj->Set_User_Data(value, recursive);
            robj->Release_Ref();
        }
    }
}
