/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief Class that holds mesh material data.
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
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "meshmdl.h"
#include "realcrc.h"
#include "refcount.h"
#include "texture.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ShaderClass MeshMatDescClass::s_NullShader(0);
#endif

MatBufferClass::MatBufferClass(const MatBufferClass &that) : ShareBufferClass<VertexMaterialClass *>(that)
{
    for (int i = 0; i < m_count; ++i) {
        if (m_array[i] != nullptr) {
            m_array[i]->Add_Ref();
        }
    }
}

MatBufferClass::~MatBufferClass()
{
    for (int i = 0; i < m_count; ++i) {
        Ref_Ptr_Release(m_array[i]);
    }
}

void MatBufferClass::Set_Element(int index, VertexMaterialClass *mat)
{
    Ref_Ptr_Set(m_array[index], mat);
}

VertexMaterialClass *MatBufferClass::Get_Element(int index)
{
    if (m_array[index] != nullptr) {
        m_array[index]->Add_Ref();
    }

    return m_array[index];
}

VertexMaterialClass *MatBufferClass::Peek_Element(int index)
{
    return m_array[index];
}

TexBufferClass::TexBufferClass(const TexBufferClass &that) : ShareBufferClass<TextureClass *>(that)
{
    for (int i = 0; i < m_count; ++i) {
        if (m_array[i]) {
            m_array[i]->Add_Ref();
        }
    }
}

TexBufferClass::~TexBufferClass()
{
    for (int i = 0; i < m_count; ++i) {
        Ref_Ptr_Release(m_array[i]);
    }
}

void TexBufferClass::Set_Element(int index, TextureClass *tex)
{
    Ref_Ptr_Set(m_array[index], tex);
}

TextureClass *TexBufferClass::Get_Element(int index)
{
    if (m_array[index] != nullptr) {
        m_array[index]->Add_Ref();
    }

    return m_array[index];
}

TextureClass *TexBufferClass::Peek_Element(int index)
{
    return m_array[index];
}

UVBufferClass::UVBufferClass(const UVBufferClass &that) : ShareBufferClass<Vector2>(that)
{
    m_CRC = that.m_CRC;
}

bool UVBufferClass::operator==(const UVBufferClass &that)
{
    return m_CRC == that.m_CRC;
}

bool UVBufferClass::Is_Equal_To(const UVBufferClass &that)
{
    return m_CRC == that.m_CRC;
}

void UVBufferClass::Update_CRC(void)
{
    m_CRC = CRC_Memory(Get_Array(), Get_Count() * sizeof(Vector2), 0);
}

TextureClass *MeshMatDescClass::Get_Single_Texture(int pass, int stage) const
{
    if (m_texture[pass][stage] != nullptr) {
        m_texture[pass][stage]->Add_Ref();
    }

    return m_texture[pass][stage];
}

VertexMaterialClass *MeshMatDescClass::Get_Material(int vidx, int pass) const
{
    if (m_materialArray[pass] != nullptr) {
        return m_materialArray[pass]->Get_Element(vidx);
    }

    if (m_material[pass] != nullptr) {
        m_material[pass]->Add_Ref();
        return m_material[pass];
    }

    return nullptr;
}

TextureClass *MeshMatDescClass::Get_Texture(int pidx, int pass, int stage) const
{
    if (m_textureArray[pass][stage] != nullptr) {
        return m_textureArray[pass][stage]->Get_Element(pidx);
    }

    if (m_texture[pass][stage] != nullptr) {
        m_texture[pass][stage]->Add_Ref();
        return m_texture[pass][stage];
    }

    return nullptr;
}

ShaderClass MeshMatDescClass::Get_Shader(int pidx, int pass) const
{
    if (m_shaderArray[pass] != nullptr) {
        return m_shaderArray[pass]->Get_Element(pidx);
    }

    return m_shader[pass];
}

VertexMaterialClass *MeshMatDescClass::Peek_Material(int vidx, int pass) const
{
    if (m_materialArray[pass] != nullptr) {
        return m_materialArray[pass]->Peek_Element(vidx);
    }

    return m_material[pass];
}

TextureClass *MeshMatDescClass::Peek_Texture(int pidx, int pass, int stage) const
{
    if (m_textureArray[pass][stage] != nullptr) {
        return m_textureArray[pass][stage]->Peek_Element(pidx);
    }

    return m_texture[pass][stage];
}

void MeshMatDescClass::Configure_Material(VertexMaterialClass *mtl, int pass, bool lighting_enabled)
{
    mtl->Set_Diffuse_Color_Source(m_DCGSource[pass]);
    mtl->Set_Emissive_Color_Source(m_DIGSource[pass]);

    mtl->Set_Lighting(lighting_enabled);

    for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {
        int src = m_UVSource[pass][stage];

        if (src == -1) {
            src = 0;
        }

        mtl->Set_UV_Source(stage, src);
    }
}

void MeshMatDescClass::Set_Single_Material(VertexMaterialClass *vmat, int pass)
{
    Ref_Ptr_Set(m_material[pass], vmat);
}

void MeshMatDescClass::Set_Single_Texture(TextureClass *tex, int pass, int stage)
{
    Ref_Ptr_Set(m_texture[pass][stage], tex);
}

void MeshMatDescClass::Set_Single_Shader(ShaderClass shader, int pass)
{
    m_shader[pass] = shader;
}

void MeshMatDescClass::Set_Material(int vidx, VertexMaterialClass *vmat, int pass)
{
    MatBufferClass *mats = Get_Material_Array(pass);
    mats->Set_Element(vidx, vmat);
}

void MeshMatDescClass::Set_Shader(int pidx, ShaderClass shader, int pass)
{
    ShaderClass *shaders = Get_Shader_Array(pass);
    shaders[pidx] = shader;
}

void MeshMatDescClass::Set_Texture(int pidx, TextureClass *tex, int pass, int stage)
{
    TexBufferClass *textures = Get_Texture_Array(pass, stage);
    textures->Set_Element(pidx, tex);
}

TexBufferClass *MeshMatDescClass::Get_Texture_Array(int pass, int stage, bool create)
{
    if (create && m_textureArray[pass][stage] == nullptr) {
        m_textureArray[pass][stage] = New_Tex_Buffer(m_polyCount, "MeshMatDescClass::TextureArray");
    }

    return m_textureArray[pass][stage];
}

MatBufferClass *MeshMatDescClass::Get_Material_Array(int pass, bool create)
{
    if (create && m_materialArray[pass] == nullptr) {
        m_materialArray[pass] = New_Mat_Buffer(m_vertexCount, "MeshMatDescClass::MaterialArray");
    }

    return m_materialArray[pass];
}

ShaderClass *MeshMatDescClass::Get_Shader_Array(int pass, bool create)
{
    if (create && m_shaderArray[pass] == nullptr) {
        m_shaderArray[pass] = New_Share_Buffer<ShaderClass>(m_polyCount, "MeshMatDescClass::ShaderArray");
        m_shaderArray[pass]->Clear();
    }

    if (m_shaderArray[pass] != nullptr) {
        return m_shaderArray[pass]->Get_Array();
    }

    return nullptr;
}

MeshMatDescClass::MeshMatDescClass(void) : m_passCount(1), m_vertexCount(0), m_polyCount(0)
{
    for (int array = 0; array < MAX_COLOR_ARRAYS; array++) {
        m_colorArray[array] = nullptr;
    }

    for (int uvarray = 0; uvarray < MAX_UV_ARRAYS; uvarray++) {
        m_UV[uvarray] = nullptr;
    }

    for (int pass = 0; pass < MAX_PASSES; pass++) {
        for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {
            m_UVSource[pass][stage] = -1;
            m_texture[pass][stage] = nullptr;
            m_textureArray[pass][stage] = nullptr;
        }

        m_DCGSource[pass] = VertexMaterialClass::MATERIAL;
        m_DIGSource[pass] = VertexMaterialClass::MATERIAL;

        m_shader[pass] = 0;
        m_material[pass] = nullptr;
        m_shaderArray[pass] = nullptr;
        m_materialArray[pass] = nullptr;
    }
}

MeshMatDescClass::MeshMatDescClass(const MeshMatDescClass &that) : m_passCount(1), m_vertexCount(0), m_polyCount(0)
{
    int pass;
    int stage;
    int array;

    for (array = 0; array < MAX_COLOR_ARRAYS; array++) {
        m_colorArray[array] = nullptr;
    }

    for (array = 0; array < MAX_UV_ARRAYS; array++) {
        m_UV[array] = nullptr;
    }

    for (pass = 0; pass < MAX_PASSES; pass++) {
        for (stage = 0; stage < MAX_TEX_STAGES; stage++) {
            m_UVSource[pass][stage] = -1;
            m_texture[pass][stage] = nullptr;
            m_textureArray[pass][stage] = nullptr;
        }

        m_DCGSource[pass] = VertexMaterialClass::MATERIAL;
        m_DIGSource[pass] = VertexMaterialClass::MATERIAL;

        m_shader[pass] = 0;
        m_material[pass] = nullptr;
        m_shaderArray[pass] = nullptr;
        m_materialArray[pass] = nullptr;
    }

    *this = that;
}

MeshMatDescClass &MeshMatDescClass::operator=(const MeshMatDescClass &that)
{
    if (this != &that) {

        m_passCount = that.m_passCount;
        m_vertexCount = that.m_vertexCount;
        m_polyCount = that.m_polyCount;

        for (int array = 0; array < MAX_COLOR_ARRAYS; array++) {
            ShareBufferClass<unsigned int> *b = const_cast<ShareBufferClass<unsigned int> *>(that.m_colorArray[array]);
            Ref_Ptr_Set(m_colorArray[array], b);
        }

        for (int uvarray = 0; uvarray < MAX_UV_ARRAYS; uvarray++) {
            UVBufferClass *b = const_cast<UVBufferClass *>(that.m_UV[uvarray]);
            Ref_Ptr_Set(m_UV[uvarray], b);
        }

        for (int pass = 0; pass < MAX_PASSES; pass++) {
            for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {
                m_UVSource[pass][stage] = that.m_UVSource[pass][stage];
                TextureClass *t = const_cast<TextureClass *>(that.m_texture[pass][stage]);
                Ref_Ptr_Set(m_texture[pass][stage], t);

                Ref_Ptr_Release(m_textureArray[pass][stage]);

                if (that.m_textureArray[pass][stage] != nullptr) {
                    m_textureArray[pass][stage] = new TexBufferClass(*that.m_textureArray[pass][stage]);
                }
            }

            m_DCGSource[pass] = that.m_DCGSource[pass];
            m_DIGSource[pass] = that.m_DIGSource[pass];

            m_shader[pass] = that.m_shader[pass];
            VertexMaterialClass *v = const_cast<VertexMaterialClass *>(that.m_material[pass]);
            Ref_Ptr_Set(m_material[pass], v);

            Ref_Ptr_Release(m_materialArray[pass]);

            if (that.m_materialArray[pass] != nullptr) {
                m_materialArray[pass] = new MatBufferClass(*that.m_materialArray[pass]);
            }

            Ref_Ptr_Release(m_shaderArray[pass]);
            if (that.m_shaderArray[pass] != nullptr) {
                m_shaderArray[pass] = new ShareBufferClass<ShaderClass>(*that.m_shaderArray[pass]);
            }
        }
    }

    return *this;
}

MeshMatDescClass::~MeshMatDescClass(void)
{
    Reset(0, 0, 0);
}

void MeshMatDescClass::Reset(int polycount, int vertcount, int passcount)
{
    m_polyCount = polycount;
    m_vertexCount = vertcount;
    m_passCount = passcount;

    for (int array = 0; array < MAX_COLOR_ARRAYS; array++) {
        Ref_Ptr_Release(m_colorArray[array]);
    }

    for (int uvarray = 0; uvarray < MAX_UV_ARRAYS; uvarray++) {
        Ref_Ptr_Release(m_UV[uvarray]);
    }

    for (int pass = 0; pass < MAX_PASSES; pass++) {
        for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {
            m_UVSource[pass][stage] = -1;
            Ref_Ptr_Release(m_texture[pass][stage]);
            Ref_Ptr_Release(m_textureArray[pass][stage]);
        }

        m_DCGSource[pass] = VertexMaterialClass::MATERIAL;
        m_DIGSource[pass] = VertexMaterialClass::MATERIAL;
        m_shader[pass] = 0;
        Ref_Ptr_Release(m_shaderArray[pass]);

        Ref_Ptr_Release(m_material[pass]);
        Ref_Ptr_Release(m_materialArray[pass]);
    }
}

void MeshMatDescClass::Init_Alternate(MeshMatDescClass &default_materials, MeshMatDescClass &alternate_materials)
{
    m_passCount = default_materials.m_passCount;
    m_vertexCount = default_materials.m_vertexCount;
    m_polyCount = default_materials.m_polyCount;

    for (int array = 0; array < MAX_COLOR_ARRAYS; array++) {
        if (alternate_materials.m_colorArray[array] != nullptr) {
            Ref_Ptr_Set(m_colorArray[array], alternate_materials.m_colorArray[array]);
        } else {
            Ref_Ptr_Set(m_colorArray[array], default_materials.m_colorArray[array]);
        }
    }

    for (int i = 0; i < alternate_materials.Get_UV_Array_Count(); i++) {
        Ref_Ptr_Set(m_UV[i], alternate_materials.m_UV[i]);
    }

    for (int pass = 0; pass < MAX_PASSES; pass++) {
        for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {

            if (alternate_materials.m_UVSource[pass][stage] == -1) {
                if (default_materials.m_UVSource[pass][stage] != -1) {

                    int default_uv_source = default_materials.m_UVSource[pass][stage];
                    UVBufferClass *uvarray = default_materials.m_UV[default_uv_source];
                    int found_index = -1;

                    for (int i = 0; i < Get_UV_Array_Count(); i++) {
                        if (uvarray->Get_CRC() == m_UV[i]->Get_CRC()) {
                            found_index = i;
                            break;
                        }
                    }

                    if (found_index != -1) {
                        m_UVSource[pass][stage] = found_index;
                    } else {
                        int new_index = Get_UV_Array_Count();
                        Ref_Ptr_Set(m_UV[new_index], default_materials.m_UV[default_uv_source]);
                        m_UVSource[pass][stage] = new_index;
                    }
                }
            } else {
                m_UVSource[pass][stage] = alternate_materials.m_UVSource[pass][stage];
            }

            if ((alternate_materials.m_texture[pass][stage] != nullptr)
                || (alternate_materials.m_textureArray[pass][stage])) {
                Ref_Ptr_Set(m_texture[pass][stage], alternate_materials.m_texture[pass][stage]);
                Ref_Ptr_Set(m_textureArray[pass][stage], alternate_materials.m_textureArray[pass][stage]);
            } else {
                Ref_Ptr_Set(m_texture[pass][stage], default_materials.m_texture[pass][stage]);
                Ref_Ptr_Set(m_textureArray[pass][stage], default_materials.m_textureArray[pass][stage]);
            }
        }

        if (alternate_materials.m_DCGSource[pass] == VertexMaterialClass::MATERIAL) {
            m_DCGSource[pass] = default_materials.m_DCGSource[pass];
        } else {
            m_DCGSource[pass] = alternate_materials.m_DCGSource[pass];
        }

        m_shader[pass] = default_materials.m_shader[pass];
        Ref_Ptr_Set(m_shaderArray[pass], default_materials.m_shaderArray[pass]);

        if ((alternate_materials.m_material[pass] != nullptr) || (alternate_materials.m_materialArray[pass] != nullptr)) {
            Ref_Ptr_Set(m_material[pass], alternate_materials.m_material[pass]);
            Ref_Ptr_Set(m_materialArray[pass], alternate_materials.m_materialArray[pass]);
        } else {
            if (default_materials.m_material[pass] != nullptr) {
                m_material[pass] = new VertexMaterialClass(*(default_materials.m_material[pass]));
            } else {
                if (default_materials.m_materialArray[pass] != nullptr) {
                    captainslog_warn(("Unimplemented case: mesh has more than one default vertex material but no alternate "
                                      "vertex materials have been defined.\r\n"));
                }
                m_material[pass] = nullptr;
            }
        }
    }
}

bool MeshMatDescClass::Is_Empty(void)
{
    for (int array = 0; array < MAX_COLOR_ARRAYS; array++) {
        if (m_colorArray[array] != nullptr) {
            return false;
        }
    }

    for (int uvarray = 0; uvarray < MAX_UV_ARRAYS; uvarray++) {
        if (m_UV[uvarray] != nullptr) {
            return false;
        }
    }

    for (int pass = 0; pass < MAX_PASSES; pass++) {
        for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {
            if (m_texture[pass][stage] != nullptr) {
                return false;
            }

            if (m_textureArray[pass][stage] != nullptr) {
                return false;
            }
        }

        if (m_material[pass] != nullptr) {
            return false;
        }

        if (m_materialArray[pass] != nullptr) {
            return false;
        }
    }

    return true;
}

void MeshMatDescClass::Make_UV_Array_Unique(int pass, int stage)
{
    int uvindex = m_UVSource[pass][stage];

    if (m_UV[uvindex]->Num_Refs() > 1) {
        UVBufferClass *unique_uv = new UVBufferClass(*m_UV[uvindex]);
        m_UV[uvindex]->Release_Ref();
        m_UV[uvindex] = unique_uv;
    }
}

void MeshMatDescClass::Make_Color_Array_Unique(int array)
{
    if ((m_colorArray[array] != nullptr) && (m_colorArray[array]->Num_Refs() > 1)) {
        ShareBufferClass<unsigned int> *unique_color_array = new ShareBufferClass<unsigned int>(*m_colorArray[array]);
        m_colorArray[array]->Release_Ref();
        m_colorArray[array] = unique_color_array;
    }
}

void MeshMatDescClass::Install_UV_Array(int pass, int stage, Vector2 *uvs, int count)
{
    unsigned int crc = CRC_Memory(uvs, count * sizeof(Vector2), 0);
    bool found = false;

    for (int i = 0; i < Get_UV_Array_Count(); i++) {
        if (m_UV[i]->Get_CRC() == crc) {
            found = true;
            Set_UV_Source(pass, stage, i);
            break;
        }
    }

    if (found == false) {
        int new_index = 0;

        while ((m_UV[new_index] != nullptr) && (new_index < MAX_UV_ARRAYS)) {
            new_index++;
        }

        if (new_index < MAX_UV_ARRAYS) {

            captainslog_assert(m_UV[new_index] == nullptr);
            m_UV[new_index] = New_UV_Buffer(count, "MeshMatDescClass::UV");
            memcpy(m_UV[new_index]->Get_Array(), uvs, count * sizeof(Vector2));
            m_UV[new_index]->Update_CRC();
            Set_UV_Source(pass, stage, new_index);
        }
    }
}

void MeshMatDescClass::Post_Load_Process(bool lighting_enabled, MeshModelClass *parent)
{
#ifdef BUILD_WITH_D3D8
    bool set_lighting_to_false = true;

    for (int pass = 0; pass < m_passCount; pass++) {

        if ((m_material[pass] == nullptr) && (m_materialArray[pass] == nullptr)) {
            m_material[pass] = new VertexMaterialClass();
        }

        if (m_material[pass] != nullptr) {
            Configure_Material(m_material[pass], pass, lighting_enabled);
        } else {
            VertexMaterialClass *prev_mtl = nullptr;
            VertexMaterialClass *mtl = Peek_Material(pass, 0);

            for (int vidx = 0; vidx < m_vertexCount; vidx++) {
                mtl = Peek_Material(vidx, pass);

                if ((mtl != prev_mtl) && (mtl != nullptr)) {
                    Configure_Material(mtl, pass, lighting_enabled);
                    prev_mtl = mtl;
                }
            }
        }

        if (m_colorArray[0] == nullptr && m_colorArray[1] == nullptr) {
            continue;
        }

        Vector3 single_diffuse(0.0f, 0.0f, 0.0f);
        Vector3 single_ambient(0.0f, 0.0f, 0.0f);
        Vector3 single_emissive(0.0f, 0.0f, 0.0f);
        float single_opacity = 1.0f;
        bool single_diffuse_used = true;
        bool single_ambient_used = true;
        bool single_emissive_used = true;
        bool single_opacity_used = true;
        bool diffuse_used = false;
        bool ambient_used = false;
        bool emissive_used = false;
        bool opacity_used = false;

        Vector3 mtl_diffuse;
        Vector3 mtl_ambient;
        Vector3 mtl_emissive;
        float mtl_opacity = 1.0f;

        VertexMaterialClass *prev_mtl = nullptr;
        VertexMaterialClass *mtl = Peek_Material(0, pass);

        if (mtl != nullptr) {
            mtl->Get_Diffuse(&single_diffuse);
            single_opacity = mtl->Get_Opacity();
            mtl->Get_Ambient(&single_ambient);
            mtl->Get_Emissive(&single_emissive);

            if (single_diffuse.X || single_diffuse.Y || single_diffuse.Z) {
                diffuse_used = true;
            }

            if (single_ambient.X || single_ambient.Y || single_ambient.Z) {
                ambient_used = true;
            }

            if (single_emissive.X || single_emissive.Y || single_emissive.Z) {
                emissive_used = true;
            }

            if (single_opacity != 1.0f) {
                opacity_used = true;
            }
        }

        for (int vidx = 0; vidx < m_vertexCount; vidx++) {
            mtl = Peek_Material(vidx, pass);

            if (mtl != prev_mtl) {
                prev_mtl = mtl;
                mtl->Get_Diffuse(&mtl_diffuse);
                mtl_opacity = mtl->Get_Opacity();
                mtl->Get_Ambient(&mtl_ambient);
                mtl->Get_Emissive(&mtl_emissive);
            }

            if (mtl_diffuse.X != single_diffuse.X || mtl_diffuse.Y != single_diffuse.Y
                || mtl_diffuse.Z != single_diffuse.Z) {
                single_diffuse_used = false;
            }
            if (mtl_ambient.X != single_ambient.X || mtl_ambient.Y != single_ambient.Y
                || mtl_ambient.Z != single_ambient.Z) {
                single_ambient_used = false;
            }
            if (mtl_emissive.X != single_emissive.X || mtl_emissive.Y != single_emissive.Y
                || mtl_emissive.Z != single_emissive.Z) {
                single_emissive_used = false;
            }
            if (mtl_opacity != single_opacity) {
                single_opacity_used = false;
            }

            if (mtl_diffuse.X || mtl_diffuse.Y || mtl_diffuse.Z) {
                diffuse_used = true;
            }

            if (mtl_ambient.X || mtl_ambient.Y || mtl_ambient.Z) {
                ambient_used = true;
            }

            if (mtl_emissive.X || mtl_emissive.Y || mtl_emissive.Z) {
                emissive_used = true;
            }

            if (mtl_opacity != 1.0f) {
                opacity_used = true;
            }
        }

        if ((m_DCGSource[pass] != VertexMaterialClass::MATERIAL) && (m_colorArray[0] != nullptr)
            && (m_DIGSource[pass] != VertexMaterialClass::MATERIAL) && (m_colorArray[1] != nullptr)) {
            unsigned int *diffuse_array = m_colorArray[0]->Get_Array();
            unsigned int *emissive_array = m_colorArray[1]->Get_Array();

            for (int vidx = 0; vidx < m_vertexCount; vidx++) {
                Vector4 diffuse = DX8Wrapper::Convert_Color(diffuse_array[vidx]);
                Vector4 emissive = DX8Wrapper::Convert_Color(emissive_array[vidx]);
                diffuse.X *= emissive.X;
                diffuse.Y *= emissive.Y;
                diffuse.Z *= emissive.Z;
                diffuse_array[vidx] = DX8Wrapper::Convert_Color(diffuse);
            }
        }

        m_DIGSource[pass] = VertexMaterialClass::MATERIAL;

        if ((m_DCGSource[pass] != VertexMaterialClass::MATERIAL) && (m_colorArray[0] != nullptr)) {
            unsigned int *diffuse_array = m_colorArray[0]->Get_Array();
            Vector3 mtl_diffuse2;
            float mtl_opacity2 = 1.0f;

            VertexMaterialClass *prev_mtl2 = nullptr;
            VertexMaterialClass *mtl2 = Peek_Material(0, pass);

            for (int vidx = 0; vidx < m_vertexCount; vidx++) {
                mtl2 = Peek_Material(vidx, pass);

                if (mtl2 != prev_mtl2) {
                    prev_mtl2 = mtl2;
                    mtl2->Get_Diffuse(&mtl_diffuse2);
                    mtl_opacity2 = mtl2->Get_Opacity();
                }

                if (diffuse_used && !ambient_used && !emissive_used) {
                    Vector4 diffuse = DX8Wrapper::Convert_Color(diffuse_array[vidx]);
                    diffuse.X *= mtl_diffuse2.X;
                    diffuse.Y *= mtl_diffuse2.Y;
                    diffuse.Z *= mtl_diffuse2.Z;
                    diffuse.W *= mtl_opacity2;
                    diffuse_array[vidx] = DX8Wrapper::Convert_Color(diffuse);

                    mtl2->Set_Ambient_Color_Source(VertexMaterialClass::MATERIAL);
                    mtl2->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
                    mtl2->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
                }

                if (diffuse_used && ambient_used && !emissive_used) {
                    Vector4 diffuse = DX8Wrapper::Convert_Color(diffuse_array[vidx]);
                    diffuse.X *= mtl_diffuse2.X;
                    diffuse.Y *= mtl_diffuse2.Y;
                    diffuse.Z *= mtl_diffuse2.Z;
                    diffuse.W *= mtl_opacity2;
                    diffuse_array[vidx] = DX8Wrapper::Convert_Color(diffuse);

                    mtl2->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
                    mtl2->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
                    mtl2->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
                }

                if (!diffuse_used && ambient_used && !emissive_used) {
                    Vector4 diffuse = DX8Wrapper::Convert_Color(diffuse_array[vidx]);
                    diffuse.X *= mtl_ambient.X;
                    diffuse.Y *= mtl_ambient.Y;
                    diffuse.Z *= mtl_ambient.Z;
                    diffuse.W *= mtl_opacity2;
                    diffuse_array[vidx] = DX8Wrapper::Convert_Color(diffuse);

                    mtl2->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
                    mtl2->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
                    mtl2->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
                }

                if (!diffuse_used && !ambient_used && emissive_used) {
                    Vector4 diffuse = DX8Wrapper::Convert_Color(diffuse_array[vidx]);
                    diffuse.X *= mtl_emissive.X;
                    diffuse.Y *= mtl_emissive.Y;
                    diffuse.Z *= mtl_emissive.Z;
                    diffuse.W *= mtl_opacity2;
                    diffuse_array[vidx] = DX8Wrapper::Convert_Color(diffuse);

                    mtl2->Set_Ambient_Color_Source(VertexMaterialClass::MATERIAL);
                    mtl2->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
                    mtl2->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
                } else {
                    if (m_passCount != 1) {
                        set_lighting_to_false = false;
                    }
                }
            }
        }
    }

    for (int pass = 0; pass < m_passCount; pass++) {
        bool kill_pass = false;

        if (set_lighting_to_false) {
            Vector3 single_diffuse(0.0f, 0.0f, 0.0f);
            Vector3 single_ambient(0.0f, 0.0f, 0.0f);
            Vector3 single_emissive(0.0f, 0.0f, 0.0f);
            bool diffuse_used = false;
            bool ambient_used = false;
            bool emissive_used = false;

            Vector3 mtl_diffuse;
            Vector3 mtl_ambient;
            Vector3 mtl_emissive;

            VertexMaterialClass *prev_mtl = nullptr;
            VertexMaterialClass *mtl = Peek_Material(0, pass);

            if (mtl != nullptr) {
                mtl->Get_Diffuse(&single_diffuse);
                mtl->Get_Ambient(&single_ambient);
                mtl->Get_Emissive(&single_emissive);

                if (single_diffuse.X || single_diffuse.Y || single_diffuse.Z) {
                    diffuse_used = true;
                }

                if (single_ambient.X || single_ambient.Y || single_ambient.Z) {
                    ambient_used = true;
                }

                if (single_emissive.X || single_emissive.Y || single_emissive.Z) {
                    emissive_used = true;
                }
            }

            for (int vidx = 0; vidx < m_vertexCount; vidx++) {
                mtl = Peek_Material(vidx, pass);

                if (mtl != prev_mtl) {
                    prev_mtl = mtl;
                    mtl->Get_Diffuse(&mtl_diffuse);
                    mtl->Get_Ambient(&mtl_ambient);
                    mtl->Get_Emissive(&mtl_emissive);
                }

                if (mtl_diffuse.X || mtl_diffuse.Y || mtl_diffuse.Z) {
                    diffuse_used = true;
                }

                if (mtl_ambient.X || mtl_ambient.Y || mtl_ambient.Z) {
                    ambient_used = true;
                }

                if (mtl_emissive.X || mtl_emissive.Y || mtl_emissive.Z) {
                    emissive_used = true;
                }
            }

            if ((m_DCGSource[pass] != VertexMaterialClass::MATERIAL) && (m_colorArray[0] != nullptr)) {
                VertexMaterialClass *prev_mtl2 = nullptr;
                VertexMaterialClass *mtl2 = Peek_Material(0, pass);

                for (int vidx = 0; vidx < m_vertexCount; vidx++) {
                    mtl2 = Peek_Material(vidx, pass);

                    if (mtl2 != prev_mtl2) {
                        prev_mtl2 = mtl2;

                        if (!diffuse_used && !ambient_used && emissive_used) {
                            mtl2->Set_Lighting(false);
                        }
                    }
                }
            }
        }
    }
#endif
}

bool MeshMatDescClass::Do_Mappers_Need_Normals(void)
{
    for (int pass = 0; pass < m_passCount; pass++) {
        if (m_material[pass] != nullptr) {
            if (m_material[pass]->Do_Mappers_Need_Normals()) {
                return true;
            }
        } else {
            VertexMaterialClass *prev_mtl = nullptr;
            VertexMaterialClass *mtl = Peek_Material(pass, 0);

            for (int vidx = 0; vidx < m_vertexCount; vidx++) {
                mtl = Peek_Material(vidx, pass);

                if ((mtl != prev_mtl) && (mtl != nullptr)) {
                    if (mtl->Do_Mappers_Need_Normals()) {
                        return true;
                    }

                    prev_mtl = mtl;
                }
            }
        }
    }

    return false;
}
