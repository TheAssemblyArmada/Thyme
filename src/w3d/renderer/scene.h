/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief scene class
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
#include "vector3.h"

class RenderObjClass;
class RenderInfoClass;
class RGBColor;

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

    virtual ~SceneClass();
    virtual int Get_Scene_ID();
    virtual void Add_Render_Object(RenderObjClass *obj);
    virtual void Remove_Render_Object(RenderObjClass *obj);
    virtual SceneIterator *Create_Iterator(bool onlyvisible) = 0;
    virtual void Destroy_Iterator(SceneIterator *it) = 0;
    virtual void Set_Ambient_Light(const RGBColor &color);
    virtual const Vector3 &Get_Ambient_Light();
    virtual void Set_Fog_Enable(bool set);
    virtual bool Get_Fog_Enable();
    virtual void Set_Fog_Color(const RGBColor &color);
    virtual Vector3 &Get_Fog_Color();
    virtual void Set_Fog_Range(float start, float end);
    virtual void Get_Fog_Range(float *start, float *end);
    virtual void Register(class RenderObjClass *obj, RegType for_what) = 0;
    virtual void Unregister(class RenderObjClass *obj, RegType for_what) = 0;
    virtual float Compute_Point_Visibility(RenderInfoClass &rinfo, const Vector3 &point);
    virtual void Save(ChunkSaveClass &csave);
    virtual void Load(ChunkLoadClass &cload);
    virtual void Render(RenderInfoClass &rinfo);
    virtual void Customized_Render(RenderInfoClass &rinfo) = 0;
    virtual void Pre_Render_Processing(RenderInfoClass &rinfo);
    virtual void Post_Render_Processing(RenderInfoClass &rinfo);

private:
    Vector3 m_ambientLight;
    PolyRenderType m_polyRenderMode;
    ExtraPassPolyRenderType m_extraPassPolyRenderMode;
    bool m_fogEnabled;
    Vector3 m_fogColor;
    float m_fogStart;
    float m_fogEnd;
};
