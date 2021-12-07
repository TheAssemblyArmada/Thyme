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
#include "matpass.h"
#include "dx8wrapper.h"
#include "texture.h"
#include "vertmaterial.h"

bool MaterialPassClass::m_enablePerPolygonCulling = true;

MaterialPassClass::MaterialPassClass() :
    m_shader(0), m_material(nullptr), m_enableOnTranslucentMeshes(true), m_cullVolume(nullptr)
{
    for (int i = 0; i < MAX_TEX_STAGES; ++i) {
        m_texture[i] = nullptr;
    }
}

MaterialPassClass::~MaterialPassClass()
{
    for (int i = 0; i < MAX_TEX_STAGES; ++i) {
        Ref_Ptr_Release(m_texture[i]);
    }

    Ref_Ptr_Release(m_material);
}

void MaterialPassClass::Install_Materials()
{
    DX8Wrapper::Set_Material(Peek_Material());
    DX8Wrapper::Set_Shader(Peek_Shader());
    for (uint32_t i = 0; i < DX8Wrapper::Get_Current_Caps()->Max_Textures_Per_Pass(); ++i) {
        DX8Wrapper::Set_Texture(i, Peek_Texture(i));
    }
}

void MaterialPassClass::Set_Texture(TextureClass *texture, int stage)
{
    Ref_Ptr_Set(m_texture[stage], texture);
}

void MaterialPassClass::Set_Shader(ShaderClass shader)
{
    m_shader = shader;
    m_shader.Enable_Fog("MaterialPassClass");
}

void MaterialPassClass::Set_Material(VertexMaterialClass *material)
{
    Ref_Ptr_Set(m_material, material);
}

TextureClass *MaterialPassClass::Get_Texture(int stage)
{
    if (m_texture[stage] != nullptr) {
        m_texture[stage]->Add_Ref();
    }

    return m_texture[stage];
}

VertexMaterialClass *MaterialPassClass::Get_Material()
{
    if (m_material != nullptr) {
        m_material->Add_Ref();
    }

    return m_material;
}

TextureClass *MaterialPassClass::Peek_Texture(int stage)
{
    captainslog_assert(stage >= 0);
    captainslog_assert(stage < MAX_TEX_STAGES);

    return m_texture[stage];
}
