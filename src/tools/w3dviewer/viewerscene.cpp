/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View scene
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "viewerscene.h"
#include "camera.h"

void ViewerSceneClass::Add_Render_Object(RenderObjClass *obj)
{
    SceneClass::Add_Render_Object(obj);

    if (obj->Class_ID() == RenderObjClass::CLASSID_LIGHT) {
        m_lightList.Add(obj);
    } else {
        m_renderList.Add(obj);
    }

    Update_Fog_Range();
}

void ViewerSceneClass::Customized_Render(RenderInfoClass &rinfo)
{
    SimpleSceneClass::Customized_Render(rinfo);
}

void ViewerSceneClass::Visibility_Check(CameraClass *camera)
{
    RefMultiListIterator<RenderObjClass> iter(&m_renderList);

    for (iter.First(); !iter.Is_Done(); iter.Next()) {
        RenderObjClass *obj = iter.Peek_Obj();

        if (obj->Is_Force_Visible()) {
            obj->Set_Visible(true);
        } else {
            obj->Set_Visible(camera->Cull_Sphere(obj->Get_Bounding_Sphere()) == 0);
        }

        int lod = obj->Get_LOD_Level();

        if (obj->Is_Really_Visible()) {
            obj->Prepare_LOD(*camera);
        }

        if (!m_autoSwitchLod) {
            obj->Set_LOD_Level(lod);
        }
    }

    m_visibilityChecked = true;
}

void ViewerSceneClass::Add_LOD_Object(RenderObjClass *obj)
{
    if (obj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        AABoxClass lod_box;
        obj->Get_Obj_Space_Bounding_Box(lod_box);
        float f1 = lod_box.m_extent.Y + lod_box.m_extent.Y;
        AABoxClass this_box;
        Get_Bounding_Box(&this_box);
        float f2 = this_box.m_extent.Y + this_box.m_extent.Y;
        float f3 = f1 * 1.3333334f;
        int count = 0;
        float f4 = f3 + f2;
        SceneIterator *i;

        for (i = Create_Iterator(false); !i->Is_Done(); i->Next()) {
            RenderObjClass *o = i->Current_Item();

            if (o->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                Vector3 pos = o->Get_Position();
                pos.Y -= (f4 - f2) * 0.5f;
                o->Set_Position(pos);
                count++;
            }
        }

        Destroy_Iterator(i);

        if (count <= 0) {
            Vector3 pos(0.0f, 0.0f, 0.0f);
            obj->Set_Position(pos);
        } else {
            Vector3 pos(0.0f, f4 * 0.5f - lod_box.m_extent.Y, 0.0f);
            obj->Set_Position(pos);
        }

        Add_Render_Object(obj);
        m_lodObjectList.Add(obj);
    }
}

void ViewerSceneClass::Remove_All_LOD_Objects()
{
    for (auto *robj = m_lodObjectList.Remove_Head(); robj != nullptr; robj = m_lodObjectList.Remove_Head()) {
        Remove_Render_Object(robj);
    }
}

void ViewerSceneClass::Get_Bounding_Box(AABoxClass *box)
{
    AABoxClass box2;
    SceneIterator *i;

    for (i = Create_Iterator(false); !i->Is_Done(); i->Next()) {
        RenderObjClass *o = i->Current_Item();

        if (o->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            box2.Add_Box(o->Get_Bounding_Box());
        }
    }

    Destroy_Iterator(i);
    *box = box2;
}

void ViewerSceneClass::Get_Bounding_Sphere(SphereClass *sphere)
{
    SphereClass sphere2(Vector3(0.0f, 0.0f, 0.0f), 0.0f);
    SceneIterator *i;

    for (i = Create_Iterator(false); !i->Is_Done(); i->Next()) {
        RenderObjClass *o = i->Current_Item();

        if (o->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            sphere2.Add_Sphere(o->Get_Bounding_Sphere());
        }
    }

    Destroy_Iterator(i);
    *sphere = sphere2;
}

SceneIterator *ViewerSceneClass::Get_LOD_Iterator()
{
    return new ViewerSceneIterator(&m_lodObjectList);
}

void ViewerSceneClass::Destroy_LOD_Iterator(SceneIterator *iterator)
{
    delete iterator;
}

void ViewerSceneClass::Update_Fog_Range()
{
    float start = 0.0f;
    float end = 0.0f;
    Get_Fog_Range(&start, &end);
    SphereClass sphere;
    Get_Bounding_Sphere(&sphere);
    end = sphere.Radius * 8.0f;

    if (end < start + 200.0f) {
        end = start + 200.0f;
    }

    Set_Fog_Range(start, end);
}
