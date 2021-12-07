/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D status circle
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
#include "rendobj.h"
#include "shader.h"

class DX8IndexBufferClass;
class VertexMaterialClass;
class DX8VertexBufferClass;

class W3DStatusCircle : public RenderObjClass
{
public:
    W3DStatusCircle();
    W3DStatusCircle(const W3DStatusCircle &src);
    W3DStatusCircle &operator=(const W3DStatusCircle &that);

    virtual ~W3DStatusCircle() override;
    virtual int Class_ID() const override;
    virtual RenderObjClass *Clone() const override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual bool Cast_Ray(RayCollisionTestClass &raytest) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

    int Free_Map_Resources();
    int Init_Data();
    int Update_Circle_VB();
    int Update_Screen_VB(int diffuse);

    static int g_diffuse;
    static bool g_needUpdate;

private:
    int m_numTriangles;
    DX8IndexBufferClass *m_indexBuffer;
    ShaderClass m_shaderClass;
    VertexMaterialClass *m_vertexMaterialClass;
    DX8VertexBufferClass *m_vertexBufferCircle;
    DX8VertexBufferClass *m_vertexBufferScreen;
};