/**
 * @file
 *
 * @author tomsons26
 *
 * @brief scene classes
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
#include "refcount.h"
#include "rendobj.h"
#include "vector3.h"

class RenderObjClass;
class RenderInfoClass;
class ChunkSaveClass;
class ChunkLoadClass;

class SceneIterator
{
public:
    virtual ~SceneIterator(){};
    virtual void First() = 0;
    virtual void Next() = 0;
    virtual bool Is_Done() = 0;
    virtual RenderObjClass *Current_Item() = 0;
};

class SceneClass : public RefCountClass
{
public:
    enum RegType
    {
        ON_FRAME_UPDATE = 0x0,
        LIGHT = 0x1,
        RELEASE = 0x2,
    };

    enum PolyRenderType
    {
        POINT = 0x0,
        LINE = 0x1,
        FILL = 0x2,
    };

    enum ExtraPassPolyRenderType
    {
        EXTRA_PASS_DISABLE = 0x0,
        EXTRA_PASS_LINE = 0x1,
        EXTRA_PASS_CLEAR_LINE = 0x2,
    };

    SceneClass();

    virtual ~SceneClass() {}

    virtual int Get_Scene_ID() { return 0; }

    virtual void Add_Render_Object(RenderObjClass *obj) { obj->Notify_Added(this); }
    virtual void Remove_Render_Object(RenderObjClass *obj) { obj->Notify_Removed(this); }

    virtual SceneIterator *Create_Iterator(bool onlyvisible) = 0;
    virtual void Destroy_Iterator(SceneIterator *it) = 0;

    virtual void Set_Ambient_Light(const Vector3 &color) { m_ambientLight = color; }
    virtual const Vector3 &Get_Ambient_Light() { return m_ambientLight; }
    virtual void Set_Fog_Enable(bool set) { m_fogEnabled = set; }
    virtual bool Get_Fog_Enable() { return m_fogEnabled; }
    virtual void Set_Fog_Color(const Vector3 &color) { m_fogColor = color; }
    virtual Vector3 &Get_Fog_Color() { return m_fogColor; }
    virtual void Set_Fog_Range(float start, float end)
    {
        m_fogStart = start;
        m_fogEnd = end;
    }
    virtual void Get_Fog_Range(float *start, float *end)
    {
        *start = m_fogStart;
        *end = m_fogEnd;
    }

    virtual void Register(RenderObjClass *obj, RegType for_what) = 0;
    virtual void Unregister(RenderObjClass *obj, RegType for_what) = 0;

    virtual float Compute_Point_Visibility(RenderInfoClass &rinfo, const Vector3 &point) { return 1.0f; }

    virtual void Save(ChunkSaveClass &csave);
    virtual void Load(ChunkLoadClass &cload);

    virtual void Render(RenderInfoClass &rinfo);
    virtual void Customized_Render(RenderInfoClass &rinfo) = 0;
    virtual void Pre_Render_Processing(RenderInfoClass &rinfo) {}
    virtual void Post_Render_Processing(RenderInfoClass &rinfo) {}

    void Set_Polygon_Mode(PolyRenderType type) { m_polyRenderMode = type; }
    PolyRenderType Get_Polygon_Mode() const { return m_polyRenderMode; }
    void Set_Extra_Pass_Polygon_Mode(ExtraPassPolyRenderType type) { m_extraPassPolyRenderMode = type; }
    ExtraPassPolyRenderType Get_Extra_Pass_Polygon_Mode() const { return m_extraPassPolyRenderMode; }

protected:
    Vector3 m_ambientLight;
    PolyRenderType m_polyRenderMode;
    ExtraPassPolyRenderType m_extraPassPolyRenderMode;
    bool m_fogEnabled;
    Vector3 m_fogColor;
    float m_fogStart;
    float m_fogEnd;
};

class SimpleSceneClass : public SceneClass
{
public:
    SimpleSceneClass();
    virtual ~SimpleSceneClass() override {}

    virtual int Get_Scene_ID() const { return 1; }

    virtual void Add_Render_Object(RenderObjClass *obj) override;
    virtual void Remove_Render_Object(RenderObjClass *obj) override;

    virtual SceneIterator *Create_Iterator(bool onlyvisible) override;
    virtual void Destroy_Iterator(SceneIterator *it) override;

    virtual void Register(RenderObjClass *obj, RegType for_what) override;
    virtual void Unregister(RenderObjClass *obj, RegType for_what) override;

    virtual float Compute_Point_Visibility(RenderInfoClass &rinfo, const Vector3 &point) override;

    virtual void Customized_Render(RenderInfoClass &rinfo) override;
    virtual void Post_Render_Processing(RenderInfoClass &rinfo) override;

    virtual void Remove_All_Render_Objects();
    virtual void Visibility_Check(CameraClass *);

protected:
    bool m_visibilityChecked;
    RefMultiListClass<RenderObjClass> m_renderList;
    RefMultiListClass<RenderObjClass> m_updateList;
    RefMultiListClass<RenderObjClass> m_lightList;
    RefMultiListClass<RenderObjClass> m_releaseList;
};

class SimpleSceneIterator : public SceneIterator
{
public:
    SimpleSceneIterator(RefMultiListClass<RenderObjClass> *list, bool onlyvisible) :
        m_robjIterator(list), m_onlyVisible(onlyvisible)
    {
        // #BUGFIX Initialize all members
        m_scene = nullptr;
    }
    virtual ~SimpleSceneIterator() {}
    virtual void First() { m_robjIterator.First(); }
    virtual void Next() { m_robjIterator.Next(); };
    virtual bool Is_Done() { return m_robjIterator.Is_Done(); };
    virtual RenderObjClass *Current_Item() { return m_robjIterator.Peek_Obj(); };

private:
    RefMultiListIterator<RenderObjClass> m_robjIterator;
    SimpleSceneClass *m_scene;
    bool m_onlyVisible;
};
