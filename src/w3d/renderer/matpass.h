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
#include "refcount.h"
#include "shader.h"

class VertexMaterialClass;
class OBBoxClass;
class TextureClass;

class MaterialPassClass : public RefCountClass
{
public:
    enum
    {
        MAX_TEX_STAGES = 8,
    };

    MaterialPassClass();
    virtual ~MaterialPassClass();

    virtual void Install_Materials();
    virtual void UnInstall_Materials() {}

    void Set_Texture(TextureClass *texture, int stage = 0);
    void Set_Shader(ShaderClass shader);
    void Set_Material(VertexMaterialClass *material);

    TextureClass *Get_Texture(int stage = 0);
    VertexMaterialClass *Get_Material();

    TextureClass *Peek_Texture(int stage = 0);
    ShaderClass Peek_Shader() { return m_shader; }
    VertexMaterialClass *Peek_Material() { return m_material; }

    void Enable_On_Translucent_Meshes(bool state) { m_enableOnTranslucentMeshes = state; }
    bool Is_Enabled_On_Translucent_Meshes() { return m_enableOnTranslucentMeshes; }

    void Set_Cull_Volume(OBBoxClass *box) { m_cullVolume = box; }
    OBBoxClass *Get_Cull_Volume() { return m_cullVolume; }

    static void Enable_Per_Polygon_Culling(bool state) { m_enablePerPolygonCulling = state; }
    static bool Is_Per_Polygon_Culling_Enabled() { return m_enablePerPolygonCulling; }

    static bool m_enablePerPolygonCulling;

protected:
    TextureClass *m_texture[MAX_TEX_STAGES];
    ShaderClass m_shader;
    VertexMaterialClass *m_material;
    bool m_enableOnTranslucentMeshes;
    OBBoxClass *m_cullVolume;
};
