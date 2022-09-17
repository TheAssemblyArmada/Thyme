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
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "shader.h"
#include "sharebuf.h"
#include "vector2.h"
#include "vertmaterial.h"

class TextureClass;
class MeshModelClass;

class MatBufferClass : public ShareBufferClass<VertexMaterialClass *>
{
    IMPLEMENT_W3D_POOL(MatBufferClass);

public:
public:
#ifdef BUILD_EDITOR
    MatBufferClass(int count, const char *name) : ShareBufferClass<VertexMaterialClass *>(count, name) { Clear(); }
#else
    MatBufferClass(int count) : ShareBufferClass<VertexMaterialClass *>(count) { Clear(); }
#endif

    MatBufferClass(const MatBufferClass &that);
    ~MatBufferClass();

    void Set_Element(int index, VertexMaterialClass *mat);
    VertexMaterialClass *Get_Element(int index);
    VertexMaterialClass *Peek_Element(int index);

private:
    MatBufferClass &operator=(const MatBufferClass &that);
};

inline MatBufferClass *New_Mat_Buffer(int count, const char *name = nullptr)
{
#ifdef BUILD_EDITOR
    return new MatBufferClass(count, name);
#else
    return new MatBufferClass(count);
#endif
}

class TexBufferClass : public ShareBufferClass<TextureClass *>
{
    IMPLEMENT_W3D_POOL(TexBufferClass);

public:
#ifdef BUILD_EDITOR
    TexBufferClass(int count, const char *name) : ShareBufferClass<TextureClass *>(count, name) { Clear(); }
#else
    TexBufferClass(int count) : ShareBufferClass<TextureClass *>(count) { Clear(); }
#endif

    TexBufferClass(const TexBufferClass &that);
    ~TexBufferClass();

    void Set_Element(int index, TextureClass *tex);
    TextureClass *Get_Element(int index);
    TextureClass *Peek_Element(int index);

private:
    TexBufferClass &operator=(const TexBufferClass &that);
};

inline TexBufferClass *New_Tex_Buffer(int count, const char *name = nullptr)
{
#ifdef BUILD_EDITOR
    return new TexBufferClass(count, name);
#else
    return new TexBufferClass(count);
#endif
}

class UVBufferClass : public ShareBufferClass<Vector2>
{
    IMPLEMENT_W3D_POOL(UVBufferClass);

public:
#ifdef BUILD_EDITOR
    UVBufferClass(int count, const char *name) : ShareBufferClass<Vector2>(count, name), m_CRC(0xFFFFFFFF) {}
#else
    UVBufferClass(int count) : ShareBufferClass<Vector2>(count), m_CRC(0xFFFFFFFF) {}
#endif

    UVBufferClass(const UVBufferClass &that);

    bool operator==(const UVBufferClass &that);
    bool Is_Equal_To(const UVBufferClass &that);

    void Update_CRC();
    unsigned int Get_CRC() { return m_CRC; }

private:
    unsigned int m_CRC;

    UVBufferClass &operator=(const UVBufferClass &that);
};

inline UVBufferClass *New_UV_Buffer(int count, const char *name = nullptr)
{
#ifdef BUILD_EDITOR
    return new UVBufferClass(count, name);
#else
    return new UVBufferClass(count);
#endif
}

class MeshMatDescClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(MeshMatDescClass);

public:
    enum
    {
        MAX_PASSES = 4,
        MAX_TEX_STAGES = 2,
        MAX_COLOR_ARRAYS = 2,
        MAX_UV_ARRAYS = MAX_PASSES * MAX_TEX_STAGES
    };

    MeshMatDescClass();
    MeshMatDescClass(const MeshMatDescClass &that);
    ~MeshMatDescClass();

    void Reset(int polycount, int vertcount, int passcount);
    MeshMatDescClass &operator=(const MeshMatDescClass &that);

    void Set_UV_Source(int pass, int stage, int sourceindex) { m_UVSource[pass][stage] = sourceindex; }
    void Set_Pass_Count(int passes) { m_passCount = passes; }
    void Set_Vertex_Count(int vertcount) { m_vertexCount = vertcount; }
    void Set_Polygon_Count(int polycount) { m_polyCount = polycount; }
    void Set_DCG_Source(int pass, VertexMaterialClass::ColorSourceType source) { m_DCGSource[pass] = source; }
    void Set_DIG_Source(int pass, VertexMaterialClass::ColorSourceType source) { m_DIGSource[pass] = source; }
    void Set_Single_Material(VertexMaterialClass *vmat, int pass = 0);
    void Set_Single_Texture(TextureClass *tex, int pass = 0, int stage = 0);
    void Set_Single_Shader(ShaderClass shader, int pass = 0);
    void Set_Material(int vidx, VertexMaterialClass *vmat, int pass = 0);
    void Set_Shader(int pidx, ShaderClass shader, int pass = 0);
    void Set_Texture(int pidx, TextureClass *tex, int pass = 0, int stage = 0);

    int Get_Pass_Count() const { return m_passCount; }
    int Get_Vertex_Count() const { return m_vertexCount; }
    int Get_Polygon_Count() const { return m_polyCount; }
    VertexMaterialClass::ColorSourceType Get_DCG_Source(int pass) { return m_DCGSource[pass]; }
    VertexMaterialClass::ColorSourceType Get_DIG_Source(int pass) { return m_DIGSource[pass]; }
    TextureClass *Get_Single_Texture(int pass = 0, int stage = 0) const;
    ShaderClass Get_Single_Shader(int pass = 0) const { return m_shader[pass]; }
    VertexMaterialClass *Get_Material(int vidx, int pass = 0) const;
    TextureClass *Get_Texture(int pidx, int pass = 0, int stage = 0) const;
    ShaderClass Get_Shader(int pidx, int pass = 0) const;
    TexBufferClass *Get_Texture_Array(int pass, int stage, bool create = true);
    MatBufferClass *Get_Material_Array(int pass, bool create = true);
    ShaderClass *Get_Shader_Array(int pass, bool create = true);
    Vector2 *Get_UV_Array(int pass, int stage);
    int Get_UV_Source(int pass, int stage) { return m_UVSource[pass][stage]; }
    int Get_UV_Array_Count();
    Vector2 *Get_UV_Array_By_Index(int index, bool create = true);
    unsigned *Get_DCG_Array(int pass);
    unsigned *Get_DIG_Array(int pass);
    unsigned *Get_Color_Array(int index, bool create = true);
    VertexMaterialClass *Get_Single_Material(int pass = 0) const;

    VertexMaterialClass *Peek_Single_Material(int pass = 0) const { return m_material[pass]; }
    TextureClass *Peek_Single_Texture(int pass = 0, int stage = 0) const { return m_texture[pass][stage]; }
    VertexMaterialClass *Peek_Material(int vidx, int pass = 0) const;
    TextureClass *Peek_Texture(int pidx, int pass = 0, int stage = 0) const;

    bool Has_Material_Array(int pass) const { return (m_materialArray[pass] != nullptr); }
    bool Has_Shader_Array(int pass) const { return (m_shaderArray[pass] != nullptr); }
    bool Has_Texture_Array(int pass, int stage) const { return (m_textureArray[pass][stage] != nullptr); }
    bool Has_UV(int pass, int stage) { return m_UVSource[pass][stage] != -1; }
    bool Has_Color_Array(int array) { return m_colorArray[array] != nullptr; }
    bool Has_Shader_Data(int pass) { return (m_shader[pass] != s_NullShader) || (m_shaderArray[pass] != nullptr); }
    bool Has_Material_Data(int pass) { return (m_material[pass] != nullptr) || (m_materialArray[pass] != nullptr); }
    bool Has_Texture_Data(int pass, int stage);

    void Init_Alternate(MeshMatDescClass &def_mat_desc, MeshMatDescClass &alternate_desc);
    bool Is_Empty();

    void Install_UV_Array(int pass, int stage, Vector2 *uvs, int count);
    void Make_UV_Array_Unique(int pass, int stage);
    void Make_Color_Array_Unique(int index);

    void Post_Load_Process(bool enable_lighting = true, MeshModelClass *parent = nullptr);

    bool Do_Mappers_Need_Normals();

    MeshMatDescClass *Hook_Ctor() { return new (this) MeshMatDescClass; }
    MeshMatDescClass *Hook_Ctor2(const MeshMatDescClass &src) { return new (this) MeshMatDescClass(src); }

protected:
    void Configure_Material(VertexMaterialClass *mtl, int pass, bool lighting_enabled);
    void Disable_Backface_Culling();

    int m_passCount;
    int m_vertexCount;
    int m_polyCount;
    UVBufferClass *m_UV[MAX_UV_ARRAYS];
    int m_UVSource[MAX_PASSES][MAX_TEX_STAGES];
    ShareBufferClass<unsigned> *m_colorArray[MAX_COLOR_ARRAYS];
    VertexMaterialClass::ColorSourceType m_DCGSource[MAX_PASSES];
    VertexMaterialClass::ColorSourceType m_DIGSource[MAX_PASSES];
    TextureClass *m_texture[MAX_PASSES][MAX_TEX_STAGES];
    ShaderClass m_shader[MAX_PASSES];
    VertexMaterialClass *m_material[MAX_PASSES];
    TexBufferClass *m_textureArray[MAX_PASSES][MAX_TEX_STAGES];
    MatBufferClass *m_materialArray[MAX_PASSES];
    ShareBufferClass<ShaderClass> *m_shaderArray[MAX_PASSES];

#ifdef GAME_DLL
    static ShaderClass &s_NullShader;
#else
    static ShaderClass s_NullShader;
#endif
    friend class MeshModelClass;
};

inline Vector2 *MeshMatDescClass::Get_UV_Array(int pass, int stage)
{
    if (m_UVSource[pass][stage] == -1) {
        return nullptr;
    }

    if (m_UV[m_UVSource[pass][stage]] != nullptr) {
        return m_UV[m_UVSource[pass][stage]]->Get_Array();
    }

    return nullptr;
}

inline int MeshMatDescClass::Get_UV_Array_Count()
{
    int count = 0;

    while ((m_UV[count] != nullptr) && (count < MAX_UV_ARRAYS)) {
        count++;
    }
    return count;
}

inline Vector2 *MeshMatDescClass::Get_UV_Array_By_Index(int index, bool create)
{
    if (create && !m_UV[index]) {
        m_UV[index] = New_UV_Buffer(m_vertexCount, "MeshMatDescClass::UV");
    }

    if (m_UV[index] != nullptr) {
        return m_UV[index]->Get_Array();
    }

    return nullptr;
}

inline unsigned *MeshMatDescClass::Get_DCG_Array(int pass)
{
    switch (m_DCGSource[pass]) {
        case VertexMaterialClass::MATERIAL:
            break;
        case VertexMaterialClass::COLOR1:
            if (m_colorArray[0]) {
                return m_colorArray[0]->Get_Array();
            }
            break;
        case VertexMaterialClass::COLOR2:
            if (m_colorArray[1]) {
                return m_colorArray[1]->Get_Array();
            }
            break;
        default:
            break;
    };

    return nullptr;
}

inline unsigned *MeshMatDescClass::Get_DIG_Array(int pass)
{
    switch (m_DIGSource[pass]) {
        case VertexMaterialClass::MATERIAL:
            break;
        case VertexMaterialClass::COLOR1:
            if (m_colorArray[0]) {
                return m_colorArray[0]->Get_Array();
            }
            break;
        case VertexMaterialClass::COLOR2:
            if (m_colorArray[1]) {
                return m_colorArray[1]->Get_Array();
            }
            break;
        default:
            break;
    };

    return nullptr;
}

inline unsigned *MeshMatDescClass::Get_Color_Array(int index, bool create)
{
    if (create && !m_colorArray[index]) {
        m_colorArray[index] = New_Share_Buffer<unsigned>(m_vertexCount, "MeshMatDescClass::ColorArray");
    }

    if (m_colorArray[index]) {
        return m_colorArray[index]->Get_Array();
    }

    return nullptr;
}

inline VertexMaterialClass *MeshMatDescClass::Get_Single_Material(int pass) const
{
    if (m_material[pass]) {
        m_material[pass]->Add_Ref();
    }

    return m_material[pass];
}

inline bool MeshMatDescClass::Has_Texture_Data(int pass, int stage)
{
    return (m_texture[pass][stage] != nullptr) || (m_textureArray[pass][stage] != nullptr);
}

inline void MeshMatDescClass::Disable_Backface_Culling()
{
    for (int pass = 0; pass < m_passCount; pass++) {
        m_shader[pass].Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);

        if (m_shaderArray[pass]) {
            for (int i = 0; i < m_shaderArray[pass]->Get_Count(); i++) {
                m_shaderArray[pass]->Get_Element(i).Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
            }
        }
    }
}
