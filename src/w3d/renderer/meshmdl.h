/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Model Class
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
#include "bittype.h"
#include "colmath.h"
#include "meshgeometry.h"
#include "meshmatdesc.h"
#include "rinfo.h"
#include "shader.h"
#include "sharebuf.h"
#include "simplevec.h"
#include "vector2.h"
#include "vector3.h"
#include "vector3i.h"
#include "vector4.h"
#include "vertmaterial.h"
#include "wwstring.h"

class TextureClass;
class RenderInfoClass;
class SpecialRenderInfoClass;
class MatBufferClass;
class TexBufferClass;
class AABoxClass;
class OBBoxClass;
class FrustumClass;
class SphereClass;
class AABTreeClass;
class MaterialInfoClass;
class MeshLoadContextClass;
class MeshSaveContextClass;
class ChunkLoadClass;
class ChunkSaveClass;
class MeshClass;
class HTreeClass;
class DecalGeneratorClass;
class LightEnvironmentClass;
class DX8MeshRendererClass;
class DX8PolygonRendererClass;
class DX8SkinFVFCategoryContainer;
class DX8FVFCategoryContainer;
class GapFillerClass;
struct VertexFormatXYZNDUV2;

class MeshModelClass : public MeshGeometryClass
{
    IMPLEMENT_W3D_POOL(MeshModelClass);

public:
    MeshModelClass();
    MeshModelClass(const MeshModelClass &that);
    virtual ~MeshModelClass() override;

    MeshModelClass &operator=(const MeshModelClass &that);
    void Reset(int polycount, int vertcount, int passcount);
    void Register_For_Rendering();
    void Shadow_Render(SpecialRenderInfoClass &rinfo, const Matrix3D &tm, const HTreeClass *htree);

    int Get_Pass_Count() const { return m_curMatDesc->Get_Pass_Count(); }
    const Vector2 *Get_UV_Array(int pass = 0, int stage = 0) { return m_curMatDesc->Get_UV_Array(pass, stage); }
    int Get_UV_Array_Count() { return m_curMatDesc->Get_UV_Array_Count(); }
    const Vector2 *Get_UV_Array_By_Index(int index) { return m_curMatDesc->Get_UV_Array_By_Index(index, false); }
    unsigned *Get_DCG_Array(int pass) { return m_curMatDesc->Get_DCG_Array(pass); }
    unsigned *Get_DIG_Array(int pass) { return m_curMatDesc->Get_DIG_Array(pass); }
    VertexMaterialClass::ColorSourceType Get_DCG_Source(int pass) { return m_curMatDesc->Get_DCG_Source(pass); }
    VertexMaterialClass::ColorSourceType Get_DIG_Source(int pass) { return m_curMatDesc->Get_DIG_Source(pass); }
    VertexMaterialClass *Get_Single_Material(int pass = 0) const { return m_curMatDesc->Get_Single_Material(pass); }
    ShaderClass Get_Single_Shader(int pass = 0) const { return m_curMatDesc->Get_Single_Shader(pass); }
    VertexMaterialClass *Get_Material(int vidx, int pass = 0) const { return m_curMatDesc->Get_Material(vidx, pass); }
    ShaderClass Get_Shader(int pidx, int pass = 0) const { return m_curMatDesc->Get_Shader(pidx, pass); }
    ShaderClass *Get_Shader_Array(int pass, bool create = true) { return m_curMatDesc->Get_Shader_Array(pass, create); }
    int Material_Pass_Count() const { return m_curMatDesc->Get_Pass_Count(); }

    TexBufferClass *Get_Texture_Array(int pass, int stage, bool create = true)
    {
        return m_curMatDesc->Get_Texture_Array(pass, stage, create);
    }

    MatBufferClass *Get_Material_Array(int pass, bool create = true)
    {
        return m_curMatDesc->Get_Material_Array(pass, create);
    }

    unsigned *Get_Color_Array(int array_index, bool create = true)
    {
        return m_curMatDesc->Get_Color_Array(array_index, create);
    }

    TextureClass *Get_Single_Texture(int pass = 0, int stage = 0) const
    {
        return m_curMatDesc->Get_Single_Texture(pass, stage);
    }

    TextureClass *Get_Texture(int pidx, int pass = 0, int stage = 0) const
    {
        return m_curMatDesc->Get_Texture(pidx, pass, stage);
    }

    void Set_Pass_Count(int passes) { m_curMatDesc->Set_Pass_Count(passes); }
    void Set_Single_Material(VertexMaterialClass *vmat, int pass = 0) { m_curMatDesc->Set_Single_Material(vmat, pass); }
    void Set_Single_Shader(ShaderClass shader, int pass = 0) { m_curMatDesc->Set_Single_Shader(shader, pass); }
    void Set_Material(int vidx, VertexMaterialClass *vmat, int pass = 0) { m_curMatDesc->Set_Material(vidx, vmat, pass); }
    void Set_Shader(int pidx, ShaderClass shader, int pass = 0) { m_curMatDesc->Set_Shader(pidx, shader, pass); }

    void Set_Single_Texture(TextureClass *tex, int pass = 0, int stage = 0)
    {
        m_curMatDesc->Set_Single_Texture(tex, pass, stage);
    }

    void Set_Texture(int pidx, TextureClass *tex, int pass = 0, int stage = 0)
    {
        m_curMatDesc->Set_Texture(pidx, tex, pass, stage);
    }

    VertexMaterialClass *Peek_Single_Material(int pass = 0) const { return m_curMatDesc->Peek_Single_Material(pass); }

    TextureClass *Peek_Single_Texture(int pass = 0, int stage = 0) const
    {
        return m_curMatDesc->Peek_Single_Texture(pass, stage);
    }

    bool Has_Material_Array(int pass) const { return m_curMatDesc->Has_Material_Array(pass); }
    bool Has_Shader_Array(int pass) const { return m_curMatDesc->Has_Shader_Array(pass); }
    bool Has_Texture_Array(int pass, int stage) const { return m_curMatDesc->Has_Texture_Array(pass, stage); }

    VertexMaterialClass *Peek_Material(int vidx, int pass = 0) const { return m_curMatDesc->Peek_Material(vidx, pass); }

    TextureClass *Peek_Texture(int pidx, int pass = 0, int stage = 0) const
    {
        return m_curMatDesc->Peek_Texture(pidx, pass, stage);
    }

    void Replace_Texture(TextureClass *texture, TextureClass *new_texture);
    void Replace_VertexMaterial(VertexMaterialClass *vmat, VertexMaterialClass *new_vmat);

    void Make_Geometry_Unique();
    void Make_UV_Array_Unique(int pass = 0, int stage = 0);
    void Make_Color_Array_Unique(int array_index = 0);

    virtual W3DErrorType Load_W3D(ChunkLoadClass &cload) override;

    void Enable_Alternate_Material_Description(bool onoff);
    bool Is_Alternate_Material_Description_Enabled();
    bool Needs_Vertex_Normals();
    void Init_For_NPatch_Rendering();
    DX8FVFCategoryContainer *Peek_FVF_Category_Container();

    MeshModelClass *Hook_Ctor() { return new (this) MeshModelClass; }
    MeshModelClass *Hook_Ctor2(const MeshModelClass &src) { return new (this) MeshModelClass(src); }

protected:
    W3DErrorType Read_Chunks(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Material_Info(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Shaders(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Vertex_Materials(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Textures(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Material_Pass(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Vertex_Material_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Shader_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_SCG(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_DIG(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_DCG(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Texture_Stage(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Texture_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Stage_Texcoords(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Per_Face_Texcoord_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context);
    W3DErrorType Read_Prelit_Material(ChunkLoadClass &cload, MeshLoadContextClass *context);

    unsigned int Get_Sort_Flags(int pass) const;
    unsigned int Get_Sort_Flags() const;

    void Post_Process();
    void Post_Process_Fog();
    void Compute_Static_Sort_Levels();
    void Install_Materials(MeshLoadContextClass *loadinfo);
    void Clone_Materials(const MeshModelClass &srcmesh);
    void Install_Alternate_Material_Desc(MeshLoadContextClass *context);
    void Modify_For_Overbright();

    MeshMatDescClass *m_defMatDesc;
    MeshMatDescClass *m_alternateMatDesc;
    MeshMatDescClass *m_curMatDesc;
    MaterialInfoClass *m_matInfo;
    MultiListClass<DX8PolygonRendererClass> m_polygonRendererList;
    GapFillerClass *m_gapFiller;
    bool m_hasBeenInUse;
    friend class MeshClass;
    friend class MeshLoadContextClass;
    friend class DX8SkinFVFCategoryContainer;
    friend class DX8MeshRendererClass;
    friend class DX8PolygonRendererClass;
};
