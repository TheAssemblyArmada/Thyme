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
#pragma once
#include "w3dafx.h"
#include "scene.h"

class ViewerSceneIterator : public SceneIterator
{
public:
    ViewerSceneIterator(RefMultiListClass<RenderObjClass> *list) : m_robjIterator(list) {}
    virtual ~ViewerSceneIterator() {}
    virtual void First() { m_robjIterator.First(); }
    virtual void Next() { m_robjIterator.Next(); };
    virtual bool Is_Done() { return m_robjIterator.Is_Done(); };
    virtual RenderObjClass *Current_Item() { return m_robjIterator.Peek_Obj(); };

private:
    RefMultiListIterator<RenderObjClass> m_robjIterator;
};

class ViewerSceneClass : public SimpleSceneClass
{
public:
    ViewerSceneClass() : m_autoSwitchLod(false) {}
    virtual ~ViewerSceneClass() override {}
    virtual void Add_Render_Object(RenderObjClass *obj) override;
    virtual void Customized_Render(RenderInfoClass &rinfo) override;
    virtual void Visibility_Check(CameraClass *camera) override;
    virtual void Set_Auto_Switch_LOD(bool set) { m_autoSwitchLod = set; }
    virtual bool Get_Auto_Switch_LOD() { return m_autoSwitchLod; }
    virtual void Add_LOD_Object(RenderObjClass *obj);
    virtual void Remove_All_LOD_Objects();
    virtual void Get_Bounding_Box(AABoxClass *box);
    virtual void Get_Bounding_Sphere(SphereClass *sphere);
    virtual SceneIterator *Get_LOD_Iterator();
    virtual void Destroy_LOD_Iterator(SceneIterator *iterator);

    void Update_Fog_Range();

    bool Is_LOD(int id) const { return id == RenderObjClass::CLASSID_HMODEL || id == RenderObjClass::CLASSID_HLOD; }

private:
    bool m_autoSwitchLod;
    RefMultiListClass<RenderObjClass> m_lodObjectList;
    RefMultiListClass<RenderObjClass> m_lightList;
};
