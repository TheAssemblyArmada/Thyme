/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Mat Desc Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshmatdesc.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

ShaderClass MeshMatDescClass::Get_Shader(int pidx, int pass) const
{
    if (m_shaderArray[pass]) {
        return m_shaderArray[pass]->Get_Element(pidx);
    }

    return m_shader[pass];
}

VertexMaterialClass *MeshMatDescClass::Peek_Material(int vidx, int pass) const
{
    if (m_materialArray[pass]) {
        return m_materialArray[pass]->Peek_Element(vidx);
    }

    return m_material[pass];
}

TextureClass *MeshMatDescClass::Peek_Texture(int pidx, int pass, int stage) const
{
    if (m_textureArray[pass][stage]) {
        return m_textureArray[pass][stage]->Peek_Element(pidx);
    }

    return m_texture[pass][stage];
}

VertexMaterialClass *MatBufferClass::Peek_Element(int index)
{
    return m_array[index];
}

TextureClass *TexBufferClass::Peek_Element(int index)
{
    return m_array[index];
}
