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
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshmdl.h"
#include "aabtree.h"
#include "assetmgr.h"
#include "bwrender.h"
#include "camera.h"
#include "dx8polygonrenderer.h"
#include "dx8renderer.h"
#include "hashtemplate.h"
#include "htree.h"
#include "matinfo.h"
#include "vp.h"
#include "w3d_util.h"

static DynamicVectorClass<Vector4> _TempTransformedVertexBuffer;

MeshModelClass::MeshModelClass() :
    m_defMatDesc(nullptr), m_alternateMatDesc(nullptr), m_curMatDesc(nullptr), m_matInfo(nullptr), m_gapFiller(nullptr)
{
    Set_Flag(DIRTY_BOUNDS, true);

    m_defMatDesc = new MeshMatDescClass;
    m_curMatDesc = m_defMatDesc;

    m_matInfo = new MaterialInfoClass();

    // #BUGFIX Initialize all members
    m_hasBeenInUse = false;
}

MeshModelClass::MeshModelClass(const MeshModelClass &that) :
    MeshGeometryClass(that),
    m_defMatDesc(nullptr),
    m_alternateMatDesc(nullptr),
    m_curMatDesc(nullptr),
    m_matInfo(nullptr),
    m_gapFiller(nullptr),
    m_hasBeenInUse(false)
{
    m_defMatDesc = new MeshMatDescClass(*(that.m_defMatDesc));

    if (that.m_alternateMatDesc != nullptr) {
        m_alternateMatDesc = new MeshMatDescClass(*(that.m_alternateMatDesc));
    }

    m_curMatDesc = m_defMatDesc;
    Clone_Materials(that);
}

MeshModelClass::~MeshModelClass()
{
    g_theDX8MeshRenderer.Unregister_Mesh_Type(this);
    Reset(0, 0, 0);
    Ref_Ptr_Release(m_matInfo);

    if (m_defMatDesc != nullptr) {
        delete m_defMatDesc;
    }

    if (m_alternateMatDesc != nullptr) {
        delete m_alternateMatDesc;
    }
}

MeshModelClass &MeshModelClass::operator=(const MeshModelClass &that)
{
    if (this != &that) {
        g_theDX8MeshRenderer.Unregister_Mesh_Type(this);
        MeshGeometryClass::operator=(that);

        *m_defMatDesc = *(that.m_defMatDesc);
        m_curMatDesc = m_defMatDesc;

        if (m_alternateMatDesc != nullptr) {
            delete m_alternateMatDesc;
            m_alternateMatDesc = nullptr;
        }

        if (that.m_alternateMatDesc != nullptr) {
            m_alternateMatDesc = new MeshMatDescClass(*(that.m_alternateMatDesc));
        }

        Clone_Materials(that);
    }

    return *this;
}

void MeshModelClass::Reset(int polycount, int vertcount, int passcount)
{
    Reset_Geometry(polycount, vertcount);
    g_theDX8MeshRenderer.Unregister_Mesh_Type(this);
    m_matInfo->Reset();
    m_defMatDesc->Reset(polycount, vertcount, passcount);

    if (m_alternateMatDesc != nullptr) {
        delete m_alternateMatDesc;
        m_alternateMatDesc = nullptr;
    }

    m_curMatDesc = m_defMatDesc;
}

void MeshModelClass::Register_For_Rendering()
{
    m_hasBeenInUse = true;
    g_theDX8MeshRenderer.Register_Mesh_Type(this);
}

void MeshModelClass::Replace_Texture(TextureClass *texture, TextureClass *new_texture)
{
    captainslog_assert(texture);
    captainslog_assert(new_texture);

    for (int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; ++stage) {
        for (int pass = 0; pass < Get_Pass_Count(); ++pass) {
            if (Has_Texture_Array(pass, stage)) {
                for (int i = 0; i < Get_Polygon_Count(); ++i) {
                    if (Peek_Texture(i, pass, stage) == texture) {
                        Set_Texture(i, new_texture, pass, stage);
                    }
                }
            } else {
                if (Peek_Single_Texture(pass, stage) == texture) {
                    Set_Single_Texture(new_texture, pass, stage);
                }
            }

            DX8FVFCategoryContainer *fvf_category = Peek_FVF_Category_Container();

            if (fvf_category) {
                fvf_category->Change_Polygon_Renderer_Texture(m_polygonRendererList, texture, new_texture, pass, stage);
            }
        }
    }
}

void MeshModelClass::Replace_VertexMaterial(VertexMaterialClass *vmat, VertexMaterialClass *new_vmat)
{
    captainslog_assert(vmat);
    captainslog_assert(new_vmat);

    for (int pass = 0; pass < Get_Pass_Count(); ++pass) {
        if (Has_Material_Array(pass)) {
            for (int i = 0; i < Get_Vertex_Count(); ++i) {
                if (Peek_Material(i, pass) == vmat) {
                    Set_Material(i, new_vmat, pass);
                }
            }
        } else {
            if (Peek_Single_Material(pass) == vmat) {
                Set_Single_Material(new_vmat, pass);
            }
        }

        DX8FVFCategoryContainer *fvf_category = Peek_FVF_Category_Container();

        if (fvf_category) {
            fvf_category->Change_Polygon_Renderer_Material(m_polygonRendererList, vmat, new_vmat, pass);
        }
    }
}

void MeshModelClass::Shadow_Render(SpecialRenderInfoClass &rinfo, const Matrix3D &tm, const HTreeClass *htree)
{
    if (rinfo.m_bwRenderer != nullptr) {
        if (_TempTransformedVertexBuffer.Length() < m_vertexCount) {
            _TempTransformedVertexBuffer.Resize(m_vertexCount);
        }

        Vector4 *transf_ptr = &(_TempTransformedVertexBuffer[0]);
        Get_Deformed_Screenspace_Vertices(transf_ptr, rinfo, tm, htree);

        Vector2 *tptr = reinterpret_cast<Vector2 *>(transf_ptr);
        Vector4 *optr = transf_ptr;

        for (int a = 0; a < m_vertexCount; ++a, ++optr) {
            *tptr++ = Vector2((*optr)[0], -(*optr)[1]);
        }

        rinfo.m_bwRenderer->Set_Vertex_Locations(reinterpret_cast<Vector2 *>(transf_ptr), m_vertexCount);
        rinfo.m_bwRenderer->Render_Triangles(reinterpret_cast<const unsigned int *>(m_poly->Get_Array()), m_polyCount * 3);
    }
}

void MeshModelClass::Make_Geometry_Unique()
{
    captainslog_assert(m_vertex);

    ShareBufferClass<Vector3> *unique_verts = new ShareBufferClass<Vector3>(*m_vertex);
    Ref_Ptr_Set(m_vertex, unique_verts);
    Ref_Ptr_Release(unique_verts);

    ShareBufferClass<Vector3> *norms = new ShareBufferClass<Vector3>(*m_vertexNorm);
    Ref_Ptr_Set(m_vertexNorm, norms);
    Ref_Ptr_Release(norms);
}

void MeshModelClass::Make_UV_Array_Unique(int pass, int stage)
{
    m_curMatDesc->Make_UV_Array_Unique(pass, stage);
}

void MeshModelClass::Make_Color_Array_Unique(int array_index)
{
    m_curMatDesc->Make_Color_Array_Unique(array_index);
}

void MeshModelClass::Enable_Alternate_Material_Description(bool onoff)
{
    if ((onoff == true) && (m_alternateMatDesc != nullptr)) {
        if (m_curMatDesc != m_alternateMatDesc) {
            m_curMatDesc = m_alternateMatDesc;

            if (Get_Flag(SORT) && W3D::Is_Munge_Sort_On_Load_Enabled()) {
                Compute_Static_Sort_Levels();
            }

            if (W3D::Is_Overbright_Modify_On_Load_Enabled()) {
                Modify_For_Overbright();
            }

            g_theDX8MeshRenderer.Invalidate(false);
        }
    } else {
        if (m_curMatDesc != m_defMatDesc) {
            m_curMatDesc = m_defMatDesc;

            if (Get_Flag(SORT) && W3D::Is_Munge_Sort_On_Load_Enabled()) {
                Compute_Static_Sort_Levels();
            }

            if (W3D::Is_Overbright_Modify_On_Load_Enabled()) {
                Modify_For_Overbright();
            }

            g_theDX8MeshRenderer.Invalidate(false);
        }
    }
}

bool MeshModelClass::Is_Alternate_Material_Description_Enabled(void)
{
    return m_curMatDesc == m_alternateMatDesc;
}

bool MeshModelClass::Needs_Vertex_Normals(void)
{
    if (Get_Flag(MeshModelClass::PRELIT_MASK) == 0) {
        return true;
    }

    return m_curMatDesc->Do_Mappers_Need_Normals();
}

void MeshModelClass::Init_For_NPatch_Rendering() {}

class MeshLoadContextClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(MeshLoadContextClass);

private:
    MeshLoadContextClass();
    virtual ~MeshLoadContextClass() override;

    W3dTexCoordStruct *Get_Texcoord_Array();

    int Add_Shader(ShaderClass shader);
    int Add_Vertex_Material(VertexMaterialClass *vmat);
    int Add_Texture(TextureClass *tex);

    ShaderClass Peek_Shader(int index) { return m_shaders[index]; }
    VertexMaterialClass *Peek_Vertex_Material(int index) { return m_vertexMaterials[index]; }
    TextureClass *Peek_Texture(int index) { return m_textures[index]; }

    int Shader_Count(void) { return m_shaders.Count(); }
    int Vertex_Material_Count(void) { return m_vertexMaterials.Count(); }
    int Texture_Count(void) { return m_textures.Count(); }

    ShaderClass Peek_Legacy_Shader(int legacy_material_index);
    VertexMaterialClass *Peek_Legacy_Vertex_Material(int legacy_material_index);
    TextureClass *Peek_Legacy_Texture(int legacy_material_index);

    Vector2 *Get_Temporary_UV_Array(int elementcount);

    void Notify_Loaded_DIG_Chunk(bool onoff = true) { m_loadedDIG = onoff; }
    bool Already_Loaded_DIG(void) { return m_loadedDIG; }

private:
    struct LegacyMaterialClass
    {
        LegacyMaterialClass(void) : m_vertexMaterialIdx(0), m_shaderIdx(0), m_textureIdx(0) {}
        ~LegacyMaterialClass(void) {}
        void Set_Name(const char *name) { m_name = name; }

        StringClass m_name;
        int m_vertexMaterialIdx;
        int m_shaderIdx;
        int m_textureIdx;
    };

    W3dMeshHeader3Struct m_header;
    W3dTexCoordStruct *m_texCoords;
    W3dMaterialInfoStruct m_matInfo;

    uint32_t m_prelitChunkID;

    int m_curPass;
    int m_curTexStage;

    DynamicVectorClass<LegacyMaterialClass *> m_legacyMaterials;
    DynamicVectorClass<ShaderClass> m_shaders;
    DynamicVectorClass<VertexMaterialClass *> m_vertexMaterials;
    DynamicVectorClass<unsigned long> m_vertexMaterialCrcs;
    DynamicVectorClass<TextureClass *> m_textures;
    MeshMatDescClass m_alternateMatDesc;
    SimpleVecClass<Vector2> m_tempUVArray;
    bool m_loadedDIG;

    friend class MeshClass;
    friend class MeshModelClass;
};

W3DErrorType MeshModelClass::Load_W3D(ChunkLoadClass &cload)
{
    MeshLoadContextClass *context = nullptr;

    cload.Open_Chunk();

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_MESH_HEADER3) {
        captainslog_warn(("Old format mesh mesh, no longer supported.\n"));
        goto Error;
    }

    context = new MeshLoadContextClass;

    if (cload.Read(&(context->m_header), sizeof(W3dMeshHeader3Struct)) != sizeof(W3dMeshHeader3Struct)) {
        goto Error;
    }

    cload.Close_Chunk();

    char *tmpname;
    int namelen;

    Reset(context->m_header.NumTris, context->m_header.NumVertices, 1);

    namelen = strlen(context->m_header.ContainerName);
    namelen += strlen(context->m_header.MeshName);
    namelen += 2;
    m_w3dAttributes = context->m_header.Attributes;
    m_sortLevel = context->m_header.SortLevel;
    tmpname = new char[namelen];
    memset(tmpname, 0, sizeof(char) * namelen);

    if (strlen(context->m_header.ContainerName) > 0) {
        strcpy(tmpname, context->m_header.ContainerName);
        strcat(tmpname, ".");
    }

    strcat(tmpname, context->m_header.MeshName);

    Set_Name(tmpname);

    delete[] tmpname;
    tmpname = NULL;

    context->m_alternateMatDesc.Set_Vertex_Count(m_vertexCount);
    context->m_alternateMatDesc.Set_Polygon_Count(m_polyCount);

    m_boundBoxMin.Set(context->m_header.Min.x, context->m_header.Min.y, context->m_header.Min.z);
    m_boundBoxMax.Set(context->m_header.Max.x, context->m_header.Max.y, context->m_header.Max.z);

    m_boundSphereCenter.Set(context->m_header.SphCenter.x, context->m_header.SphCenter.y, context->m_header.SphCenter.z);
    m_boundSphereRadius = context->m_header.SphRadius;

    if (context->m_header.Version >= 0x40001) {
        int geometry_type = context->m_header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;

        switch (geometry_type) {
            case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
                break;
            case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
                Set_Flag(ALIGNED, true);
                break;
            case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
                Set_Flag(ORIENTED, true);
                break;
            case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
                Set_Flag(SKIN, true);
                Set_Flag(ALLOW_NPATCHES, true);
                break;
        }
    }

    if (context->m_header.Attributes & W3D_MESH_FLAG_TWO_SIDED) {
        Set_Flag(TWO_SIDED, true);
    }

    if (context->m_header.Attributes & W3D_MESH_FLAG_CAST_SHADOW) {
        Set_Flag(CAST_SHADOW, true);
    }

    if (context->m_header.Attributes & W3D_MESH_FLAG_NPATCHABLE) {
        Set_Flag(ALLOW_NPATCHES, true);
    }

    if (context->m_header.Attributes & W3D_MESH_FLAG_PRELIT_MASK) {
        switch (W3D::Get_Prelit_Mode()) {
            case W3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE:
                if (context->m_header.Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) {
                    context->m_prelitChunkID = W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE;
                    Set_Flag(PRELIT_LIGHTMAP_MULTI_TEXTURE, true);
                }
                break;
            case W3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS:
                if (context->m_header.Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) {
                    context->m_prelitChunkID = W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS;
                    Set_Flag(PRELIT_LIGHTMAP_MULTI_PASS, true);
                }
                break;
            case W3D::PRELIT_MODE_VERTEX:
                if (context->m_header.Attributes & W3D_MESH_FLAG_PRELIT_VERTEX) {
                    context->m_prelitChunkID = W3D_CHUNK_PRELIT_VERTEX;
                    Set_Flag(PRELIT_VERTEX, true);
                }
                break;
            default:
                captainslog_assert(context->m_header.Attributes & W3D_MESH_FLAG_PRELIT_UNLIT);
                context->m_prelitChunkID = W3D_CHUNK_PRELIT_UNLIT;
                break;
        }
    } else {
        if (context->m_header.Attributes & OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED) {
            Set_Flag(PRELIT_LIGHTMAP_MULTI_PASS, true);
        }
    }

    Read_Chunks(cload, context);

    if ((context->m_header.Version < 0x30000) && (Get_Flag(SKIN))) {
        uint16_t *links = Get_Bone_Links();
        captainslog_assert(links);

        for (int bi = 0; bi < Get_Vertex_Count(); bi++) {
            links[bi] += 1;
        }
    }

    if ((((m_w3dAttributes & W3D_MESH_FLAG_COLLISION_TYPE_MASK) >> W3D_MESH_FLAG_COLLISION_TYPE_SHIFT) != 0)
        && (m_cullTree == NULL)) {
        Generate_Culling_Tree();
    }

    Install_Materials(context);
    delete context;
    Post_Process();

    return W3D_ERROR_OK;

Error:

    return W3D_ERROR_LOAD_FAILED;
}

W3DErrorType MeshModelClass::Read_Chunks(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    while (cload.Open_Chunk()) {
        W3DErrorType error = W3D_ERROR_OK;

        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_VERTICES:
                error = Read_Vertices(cload);
                break;
            case W3D_CHUNK_SURRENDER_NORMALS:
            case W3D_CHUNK_VERTEX_NORMALS:
                error = Read_Vertex_Normals(cload);
                break;
            case W3D_CHUNK_TRIANGLES:
                error = Read_Triangles(cload);
                break;
            case W3D_CHUNK_MESH_USER_TEXT:
                error = Read_User_Text(cload);
                break;
            case W3D_CHUNK_VERTEX_INFLUENCES:
                error = Read_Vertex_Influences(cload);
                break;
            case W3D_CHUNK_VERTEX_SHADE_INDICES:
                error = Read_Vertex_Shade_Indices(cload);
                break;
            case W3D_CHUNK_MATERIAL_INFO:
                error = Read_Material_Info(cload, context);
                break;
            case W3D_CHUNK_SHADERS:
                error = Read_Shaders(cload, context);
                break;
            case W3D_CHUNK_VERTEX_MATERIALS:
                error = Read_Vertex_Materials(cload, context);
                break;
            case W3D_CHUNK_TEXTURES:
                error = Read_Textures(cload, context);
                break;
            case W3D_CHUNK_MATERIAL_PASS:
                error = Read_Material_Pass(cload, context);
                break;
            case W3D_CHUNK_PRELIT_UNLIT:
            case W3D_CHUNK_PRELIT_VERTEX:
            case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS:
            case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE:
                Read_Prelit_Material(cload, context);
                break;
            case W3D_CHUNK_AABTREE:
                Read_AABTree(cload);
                break;
            default:
                captainslog_warn("Obsolete/unsupported chunk %d encoutered in mesh: %s.%s",
                    cload.Cur_Chunk_ID(),
                    context->m_header.ContainerName,
                    context->m_header.MeshName);
                break;
        }
        cload.Close_Chunk();

        if (error != W3D_ERROR_OK) {
            return error;
        }
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Material_Info(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    if (cload.Read(&(context->m_matInfo), sizeof(W3dMaterialInfoStruct)) != sizeof(W3dMaterialInfoStruct)) {
        return W3D_ERROR_LOAD_FAILED;
    }

    Set_Pass_Count(context->m_matInfo.PassCount);
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Shaders(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    W3dShaderStruct shader;

    for (unsigned int i = 0; i < context->m_matInfo.ShaderCount; i++) {
        if (cload.Read(&shader, sizeof(shader)) != sizeof(shader)) {
            return W3D_ERROR_LOAD_FAILED;
        }

        ShaderClass newshader;
        W3dUtilityClass::Convert_Shader(shader, &newshader);

        int index = context->Add_Shader(newshader);
        captainslog_assert(index == (int)i);
    }
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Vertex_Materials(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    while (cload.Open_Chunk()) {
        captainslog_assert(cload.Cur_Chunk_ID() == W3D_CHUNK_VERTEX_MATERIAL);
        VertexMaterialClass *vmat = new VertexMaterialClass();
        W3DErrorType error = vmat->Load_W3D(cload);

        if (error != W3D_ERROR_OK) {
            return error;
        }

        context->Add_Vertex_Material(vmat);
        vmat->Release_Ref();
        cload.Close_Chunk();
    }

    return W3D_ERROR_OK;
}

TextureClass *Load_Texture(ChunkLoadClass &cload)
{
    if (!cload.Open_Chunk() || (cload.Cur_Chunk_ID() != W3D_CHUNK_TEXTURE)) {
        return 0;
    }

    bool texinfo = false;

    W3dTextureInfoStruct info;
    char name[256];

    while (cload.Open_Chunk()) {
        int id = cload.Cur_Chunk_ID();
        switch (id) {
            case W3D_CHUNK_TEXTURE_NAME:
                cload.Read(name, cload.Cur_Chunk_Length());
                break;
            case W3D_CHUNK_TEXTURE_INFO:
                cload.Read(&info, sizeof(info));
                texinfo = true;
                break;
        }

        cload.Close_Chunk();
    }

    cload.Close_Chunk();

    if (!texinfo) {
        return W3DAssetManager::Get_Instance()->Get_Texture(name);
    }

    bool nolod = (info.Attributes & W3DTEXTURE_NO_LOD) == W3DTEXTURE_NO_LOD;
    MipCountType mips;

    if (nolod) {
        mips = MipCountType::MIP_LEVELS_1;
    } else {
        switch (info.Attributes & W3DTEXTURE_MIP_LEVELS_MASK) {
            case W3DTEXTURE_MIP_LEVELS_2:
                mips = MipCountType::MIP_LEVELS_2;
                break;
            case W3DTEXTURE_MIP_LEVELS_3:
                mips = MipCountType::MIP_LEVELS_3;
                break;
            case W3DTEXTURE_MIP_LEVELS_4:
                mips = MipCountType::MIP_LEVELS_4;
                break;
            case W3DTEXTURE_MIP_LEVELS_ALL:
            default:
                mips = MipCountType::MIP_LEVELS_ALL;
                break;
        }
    }

    WW3DFormat format = WW3D_FORMAT_UNKNOWN;

    switch (info.Attributes & W3DTEXTURE_TYPE_MASK) {
        case W3DTEXTURE_TYPE_COLORMAP:
            break;
        case W3DTEXTURE_TYPE_BUMPMAP:
            if (DX8Wrapper::Is_Initted()) {
                if (DX8Wrapper::Get_Current_Caps()->Support_Bump_Envmap()) {
                    mips = MipCountType::MIP_LEVELS_1;

                    if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_U8V8)) {
                        format = WW3D_FORMAT_U8V8;
                    } else if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_X8L8V8U8)) {
                        format = WW3D_FORMAT_X8L8V8U8;
                    } else if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_L6V5U5)) {
                        format = WW3D_FORMAT_L6V5U5;
                    }
                }
            }
            break;
        default:
            captainslog_assert(false);
            break;
    }

    TextureClass *tex = W3DAssetManager::Get_Instance()->Get_Texture(name, mips, format);

    if (nolod) {
        tex->Get_Texture_Filter()->Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
    }

    tex->Get_Texture_Filter()->Set_U_Address_Mode(info.Attributes & W3DTEXTURE_CLAMP_U ?
            TextureFilterClass::TEXTURE_ADDRESS_CLAMP :
            TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
    tex->Get_Texture_Filter()->Set_V_Address_Mode(info.Attributes & W3DTEXTURE_CLAMP_V ?
            TextureFilterClass::TEXTURE_ADDRESS_CLAMP :
            TextureFilterClass::TEXTURE_ADDRESS_REPEAT);

    return tex;
}

W3DErrorType MeshModelClass::Read_Textures(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    for (TextureClass *newtex = Load_Texture(cload); newtex != nullptr; newtex = Load_Texture(cload)) {

        context->Add_Texture(newtex);
        newtex->Release_Ref();
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Material_Pass(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    context->m_curTexStage = 0;

    while (cload.Open_Chunk()) {
        W3DErrorType error = W3D_ERROR_OK;

        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_VERTEX_MATERIAL_IDS:
                error = Read_Vertex_Material_Ids(cload, context);
                break;
            case W3D_CHUNK_SHADER_IDS:
                error = Read_Shader_Ids(cload, context);
                break;
            case W3D_CHUNK_DCG:
                error = Read_DCG(cload, context);
                break;
            case W3D_CHUNK_DIG:
                error = Read_DIG(cload, context);
                break;
            case W3D_CHUNK_SCG:
                error = Read_SCG(cload, context);
                break;
            case W3D_CHUNK_TEXTURE_STAGE:
                error = Read_Texture_Stage(cload, context);
                break;
        };

        if (error != W3D_ERROR_OK) {
            return error;
        }

        cload.Close_Chunk();
    }

    context->m_curPass++;
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Vertex_Material_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    MeshMatDescClass *matdesc = m_defMatDesc;

    if (m_defMatDesc->Has_Material_Data(context->m_curPass)) {
        matdesc = &(context->m_alternateMatDesc);
    }

    uint32_t vmat;

    if (cload.Cur_Chunk_Length() == 1 * sizeof(uint32_t)) {
        cload.Read(&vmat, sizeof(uint32_t));
        matdesc->Set_Single_Material(context->Peek_Vertex_Material(vmat), context->m_curPass);
    } else {
        for (int i = 0; i < Get_Vertex_Count(); i++) {
            cload.Read(&vmat, sizeof(uint32_t));
            matdesc->Set_Material(i, context->Peek_Vertex_Material(vmat), context->m_curPass);
        }
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Shader_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    MeshMatDescClass *matdesc = m_defMatDesc;

    if (m_defMatDesc->Has_Shader_Data(context->m_curPass)) {
        matdesc = &(context->m_alternateMatDesc);
    }

    uint32_t shaderid;

    if (cload.Cur_Chunk_Length() == 1 * sizeof(uint32_t)) {
        cload.Read(&shaderid, sizeof(shaderid));
        ShaderClass shader = context->Peek_Shader(shaderid);
        matdesc->Set_Single_Shader(shader, context->m_curPass);

        if ((context->m_curPass == 0) && (shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO)
            && (shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) && (m_sortLevel == SORT_LEVEL_NONE)) {
            Set_Flag(SORT, true);
        }
    } else {
        for (int i = 0; i < Get_Polygon_Count(); i++) {
            cload.Read(&shaderid, sizeof(uint32_t));
            ShaderClass shader = context->Peek_Shader(shaderid);
            matdesc->Set_Shader(i, shader, context->m_curPass);

            if ((context->m_curPass == 0) && (shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO)
                && (shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) && (m_sortLevel == SORT_LEVEL_NONE)) {
                Set_Flag(SORT, true);
            }
        }
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_DCG(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
#ifdef BUILD_WITH_D3D8
    MeshMatDescClass *matdesc = m_defMatDesc;

    if (m_defMatDesc->Get_DCG_Source(context->m_curPass) != VertexMaterialClass::MATERIAL) {
        matdesc = &(context->m_alternateMatDesc);
    }

    if (matdesc->Has_Color_Array(0) == false) {
        W3dRGBAStruct color;
        unsigned int *dcg = matdesc->Get_Color_Array(0);

        for (int i = 0; i < Get_Vertex_Count(); i++) {
            cload.Read(&color, sizeof(color));
            Vector4 col;
            W3dUtilityClass::Convert_Color(color, &col);
            dcg[i] = DX8Wrapper::Convert_Color(col);
        }
    } else if (context->m_prelitChunkID == W3D_CHUNK_PRELIT_VERTEX) {

        W3dRGBAStruct color;
        unsigned int *dcg = matdesc->Get_Color_Array(0);

        for (int i = 0; i < Get_Vertex_Count(); i++) {
            cload.Read(&color, sizeof(color));
            Vector4 col;
            col = DX8Wrapper::Convert_Color(dcg[i]);
            col.W = float(color.A) / 255.0f;
            dcg[i] = DX8Wrapper::Convert_Color(col);
        }
    }

    matdesc->Set_DCG_Source(context->m_curPass, VertexMaterialClass::COLOR1);
#endif
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_DIG(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
#ifdef BUILD_WITH_D3D8
    MeshMatDescClass *matdesc = m_defMatDesc;

    if (context->Already_Loaded_DIG()) {
        matdesc = &(context->m_alternateMatDesc);
    }

    context->Notify_Loaded_DIG_Chunk(true);
    W3dRGBAStruct color;

    if (matdesc->Has_Color_Array(0) == false) {
        unsigned *dcg = matdesc->Get_Color_Array(0);

        for (int i = 0; i < Get_Vertex_Count(); i++) {
            cload.Read(&color, sizeof(color));
            Vector4 col;
            col.X = float(color.R) / 255.0f;
            col.Y = float(color.G) / 255.0f;
            col.Z = float(color.B) / 255.0f;
            col.W = 1.0f;
            dcg[i] = DX8Wrapper::Convert_Color(col);
        }
    } else {
        unsigned *dcg = matdesc->Get_Color_Array(0);

        for (int i = 0; i < Get_Vertex_Count(); i++) {
            cload.Read(&color, sizeof(color));
            Vector4 col = DX8Wrapper::Convert_Color(dcg[i]);
            col.X *= float(color.R) / 255.0f;
            col.Y *= float(color.G) / 255.0f;
            col.Z *= float(color.B) / 255.0f;
            dcg[i] = DX8Wrapper::Convert_Color(col);
        }
    }

    matdesc->Set_DCG_Source(context->m_curPass, VertexMaterialClass::COLOR1);
#endif
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_SCG(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Texture_Stage(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    while (cload.Open_Chunk()) {

        W3DErrorType error = W3D_ERROR_OK;

        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_TEXTURE_IDS:
                error = Read_Texture_Ids(cload, context);
                break;
            case W3D_CHUNK_STAGE_TEXCOORDS:
            case W3D_CHUNK_TEXCOORDS:
                error = Read_Stage_Texcoords(cload, context);
                break;

            case W3D_CHUNK_PER_FACE_TEXCOORD_IDS:
                error = Read_Per_Face_Texcoord_Ids(cload, context);
                break;
        }

        if (error != W3D_ERROR_OK) {
            return error;
        }

        cload.Close_Chunk();
    }

    context->m_curTexStage++;
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Texture_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    uint32_t texid;
    int pass = context->m_curPass;
    int stage = context->m_curTexStage;
    MeshMatDescClass *matdesc = m_defMatDesc;

    if (m_defMatDesc->Has_Texture_Data(pass, stage)) {
        matdesc = &(context->m_alternateMatDesc);
    }

    if (cload.Cur_Chunk_Length() == 1 * sizeof(uint32_t)) {
        cload.Read(&texid, sizeof(texid));
        matdesc->Set_Single_Texture(context->Peek_Texture(texid), pass, stage);
    } else {
        for (int i = 0; i < Get_Polygon_Count(); i++) {
            cload.Read(&texid, sizeof(uint32_t));

            if (texid != 0xffffffff) {
                matdesc->Set_Texture(i, context->Peek_Texture(texid), pass, stage);
            }
        }
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Stage_Texcoords(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    unsigned elementcount;
    Vector2 *uvs;
    W3dTexCoordStruct texcoord;
    MeshMatDescClass *matdesc = m_defMatDesc;

    if (m_defMatDesc->Has_UV(context->m_curPass, context->m_curTexStage)) {
        matdesc = &(context->m_alternateMatDesc);
    }

    elementcount = cload.Cur_Chunk_Length() / sizeof(W3dTexCoordStruct);
    uvs = context->Get_Temporary_UV_Array(elementcount);

    if (uvs != nullptr) {
        for (unsigned i = 0; i < elementcount; i++) {
            cload.Read(&texcoord, sizeof(texcoord));
            uvs[i].X = texcoord.U;
            uvs[i].Y = 1.0f - texcoord.V;
        }
    }

    matdesc->Install_UV_Array(context->m_curPass, context->m_curTexStage, uvs, elementcount);
    return W3D_ERROR_OK;
}

W3DErrorType MeshModelClass::Read_Per_Face_Texcoord_Ids(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    unsigned int size;
    size = sizeof(Vector3i) * Get_Polygon_Count();

    if (cload.Cur_Chunk_Length() == size) {
        return W3D_ERROR_OK;
    } else {
        return W3D_ERROR_LOAD_FAILED;
    }
}

W3DErrorType MeshModelClass::Read_Prelit_Material(ChunkLoadClass &cload, MeshLoadContextClass *context)
{
    if (cload.Cur_Chunk_ID() == context->m_prelitChunkID) {
        while (cload.Open_Chunk()) {
            W3DErrorType error = W3D_ERROR_OK;

            switch (cload.Cur_Chunk_ID()) {
                case W3D_CHUNK_MATERIAL_INFO:
                    error = Read_Material_Info(cload, context);
                    break;
                case W3D_CHUNK_VERTEX_MATERIALS:
                    error = Read_Vertex_Materials(cload, context);
                    break;
                case W3D_CHUNK_SHADERS:
                    error = Read_Shaders(cload, context);
                    break;
                case W3D_CHUNK_TEXTURES:
                    error = Read_Textures(cload, context);
                    break;
                case W3D_CHUNK_MATERIAL_PASS:
                    error = Read_Material_Pass(cload, context);
                    break;
                default:
                    break;
            }

            cload.Close_Chunk();

            if (error != W3D_ERROR_OK)
                return error;
        }
    }

    return W3D_ERROR_OK;
}

void MeshModelClass::Post_Process()
{
    if (Get_Flag(MeshGeometryClass::SKIN)) {
        Ref_Ptr_Release(m_cullTree);
    }

    if (Get_Flag(MeshGeometryClass::TWO_SIDED)) {

        m_defMatDesc->Disable_Backface_Culling();

        if (m_alternateMatDesc != nullptr) {
            m_alternateMatDesc->Disable_Backface_Culling();
        }
    }

    if (W3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
        Post_Process_Fog();
    }

    if (Get_Flag(SORT) && m_sortLevel == SORT_LEVEL_NONE && W3D::Is_Munge_Sort_On_Load_Enabled()) {
        Compute_Static_Sort_Levels();
    }

    if (W3D::Is_Overbright_Modify_On_Load_Enabled()) {
        Modify_For_Overbright();
    }
}

void MeshModelClass::Post_Process_Fog()
{
    if (m_defMatDesc->Get_Pass_Count() == 2) {
        if (!m_defMatDesc->m_shaderArray[0] && !m_defMatDesc->m_shaderArray[1]) {
            ShaderClass &shader0 = m_defMatDesc->m_shader[0];
            ShaderClass &shader1 = m_defMatDesc->m_shader[1];

            bool emissive_map_effect = m_defMatDesc->m_passCount == 2
                && shader0.Get_Texturing() == ShaderClass::TEXTURING_DISABLE
                && shader0.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE
                && shader0.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO
                && shader0.Get_Primary_Gradient() == ShaderClass::GRADIENT_MODULATE
                && shader0.Get_Secondary_Gradient() == ShaderClass::SECONDARY_GRADIENT_DISABLE
                && shader1.Get_Texturing() == ShaderClass::TEXTURING_ENABLE
                && shader1.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_SRC_ALPHA
                && shader1.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_COLOR;

            if (emissive_map_effect) {
                shader0.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
                shader1.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
                shader0.Set_Fog_Func(ShaderClass::FOG_ENABLE);
                shader1.Set_Fog_Func(ShaderClass::FOG_SCALE_FRAGMENT);

                Ref_Ptr_Set(m_defMatDesc->m_texture[0][0], m_defMatDesc->m_texture[1][0]);

                if (m_defMatDesc->m_textureArray[1][0]) {
                    if (!m_defMatDesc->m_textureArray[0][0]) {
                        m_defMatDesc->m_textureArray[0][0] =
                            New_Tex_Buffer(m_polyCount, "MeshModelClass::DefMatDesc::TextureArray");

                        for (int i = 0; i < m_polyCount; i++) {
                            m_defMatDesc->m_textureArray[0][0]->Set_Element(
                                i, m_defMatDesc->m_textureArray[1][0]->Peek_Element(i));
                        }
                    }
                }

                int uv_source = 0;

                if (m_defMatDesc->m_materialArray[1]) {
                    uv_source = m_defMatDesc->m_materialArray[1]->Peek_Element(0)->Get_UV_Source(0);
                } else {
                    m_defMatDesc->m_material[1]->Get_UV_Source(0);
                }

                if (m_defMatDesc->m_materialArray[0]) {
                    for (int i = 0; i < m_vertexCount; i++) {
                        m_defMatDesc->m_materialArray[0]->Peek_Element(i)->Set_UV_Source(0, uv_source);
                    }
                } else {
                    m_defMatDesc->m_material[0]->Set_UV_Source(0, uv_source);
                }

                return;
            }

            bool shiny_mask_effect = shader0.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE
                && shader0.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO
                && shader1.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE
                && (shader1.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_ALPHA
                    || shader1.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);

            if (shiny_mask_effect) {
                shader0.Set_Fog_Func(ShaderClass::FOG_SCALE_FRAGMENT);
                shader1.Set_Fog_Func(ShaderClass::FOG_ENABLE);
                return;
            }
        }
    }

    for (int pass = 0; pass < m_defMatDesc->m_passCount; pass++) {
        m_defMatDesc->m_shader[pass].Enable_Fog(Get_Name());

        if (m_defMatDesc->m_shaderArray[pass]) {
            for (int i = 0; i < m_defMatDesc->m_shaderArray[pass]->Get_Count(); i++) {
                m_defMatDesc->m_shaderArray[pass]->Get_Element(i).Enable_Fog(Get_Name());
            }
        }
    }
}

unsigned int MeshModelClass::Get_Sort_Flags(int pass) const
{
    unsigned int flags = 0;
    ShaderClass::StaticSortCategoryType scat;

    if (Has_Shader_Array(pass)) {
        for (int tri = 0; tri < m_curMatDesc->m_shaderArray[pass]->Get_Count(); tri++) {
            scat = m_curMatDesc->m_shaderArray[pass]->Get_Element(tri).Get_Static_Sort_Category();
            flags |= (1 << scat);
        }
    } else {
        scat = Get_Single_Shader(pass).Get_Static_Sort_Category();
        flags |= (1 << scat);
    }

    return flags;
}

unsigned int MeshModelClass::Get_Sort_Flags() const
{
    unsigned int flags = 0;

    for (int pass = 0; pass < Get_Pass_Count(); pass++) {
        flags |= Get_Sort_Flags(pass);
    }

    return flags;
}

void MeshModelClass::Compute_Static_Sort_Levels()
{
    enum StaticSortCategoryBitFieldType
    {
        SSCAT_OPAQUE_BF = (1 << ShaderClass::SSCAT_OPAQUE),
        SSCAT_ALPHA_TEST_BF = (1 << ShaderClass::SSCAT_ALPHA_TEST),
        SSCAT_ADDITIVE_BF = (1 << ShaderClass::SSCAT_ADDITIVE),
        SSCAT_OTHER_BF = (1 << ShaderClass::SSCAT_OTHER)
    };

    if (Get_Sort_Flags(0) == SSCAT_OPAQUE_BF) {
        m_sortLevel = SORT_LEVEL_NONE;
        return;
    }

    switch (Get_Sort_Flags()) {
        case (SSCAT_OPAQUE_BF | SSCAT_ALPHA_TEST_BF):
            m_sortLevel = SORT_LEVEL_NONE;
            break;
        case SSCAT_ADDITIVE_BF:
            m_sortLevel = SORT_LEVEL_BIN3;
            break;
        case (SSCAT_ADDITIVE_BF | SSCAT_ALPHA_TEST_BF):
            m_sortLevel = SORT_LEVEL_BIN2;
            break;
        default:
            m_sortLevel = SORT_LEVEL_BIN1;
            break;
    };
}

void MeshModelClass::Install_Materials(MeshLoadContextClass *context)
{
    int i;
    Install_Alternate_Material_Desc(context);
    bool lighting_enabled = true;

    if (Get_Flag(MeshGeometryClass::PRELIT_VERTEX)) {
        lighting_enabled = false;
    }

    m_defMatDesc->Post_Load_Process(lighting_enabled, this);

    if (m_alternateMatDesc != nullptr) {
        m_alternateMatDesc->Post_Load_Process(lighting_enabled, this);
    }

    for (i = 0; i < context->Texture_Count(); i++) {
        m_matInfo->Add_Texture(context->Peek_Texture(i));
    }

    for (i = 0; i < context->Vertex_Material_Count(); i++) {
        m_matInfo->Add_Vertex_Material(context->Peek_Vertex_Material(i));
    }
}

void MeshModelClass::Clone_Materials(const MeshModelClass &srcmesh)
{
    Ref_Ptr_Release(m_matInfo);
    m_matInfo = new MaterialInfoClass(*(srcmesh.m_matInfo));

    MaterialRemapperClass remapper(srcmesh.m_matInfo, m_matInfo);
    remapper.Remap_Mesh(srcmesh.m_curMatDesc, m_curMatDesc);
}

void MeshModelClass::Install_Alternate_Material_Desc(MeshLoadContextClass *context)
{
    if (context->m_alternateMatDesc.Is_Empty() == false) {
        captainslog_assert(m_alternateMatDesc == nullptr);
        m_alternateMatDesc = new MeshMatDescClass;
        m_alternateMatDesc->Init_Alternate(*m_defMatDesc, context->m_alternateMatDesc);
    }
}

MeshLoadContextClass::MeshLoadContextClass()
{
    memset(&m_header, 0, sizeof(m_header));
    memset(&m_matInfo, 0, sizeof(m_matInfo));
    m_prelitChunkID = 0xffffffff;
    m_curPass = 0;
    m_curTexStage = 0;
    m_texCoords = nullptr;
    m_loadedDIG = false;
}

MeshLoadContextClass::~MeshLoadContextClass(void)
{
    int i;

    if (m_texCoords != nullptr) {
        delete m_texCoords;
        m_texCoords = nullptr;
    }

    for (i = 0; i < m_textures.Count(); i++) {
        m_textures[i]->Release_Ref();
    }

    for (i = 0; i < m_vertexMaterials.Count(); i++) {
        m_vertexMaterials[i]->Release_Ref();
    }

    for (i = 0; i < m_legacyMaterials.Count(); i++) {
        delete m_legacyMaterials[i];
    }
}

W3dTexCoordStruct *MeshLoadContextClass::Get_Texcoord_Array()
{
    if (m_texCoords == nullptr) {
        m_texCoords = new W3dTexCoordStruct[m_header.NumVertices];
    }

    return m_texCoords;
}

int MeshLoadContextClass::Add_Shader(ShaderClass shader)
{
    int index = m_shaders.Count();
    m_shaders.Add(shader);
    return index;
}

int MeshLoadContextClass::Add_Vertex_Material(VertexMaterialClass *vmat)
{
    captainslog_assert(vmat != nullptr);
    vmat->Add_Ref();
    int index = m_vertexMaterials.Count();
    m_vertexMaterials.Add(vmat);
    return index;
}

int MeshLoadContextClass::Add_Texture(TextureClass *tex)
{
    captainslog_assert(tex != nullptr);
    tex->Add_Ref();
    int index = m_textures.Count();
    m_textures.Add(tex);
    return index;
}

ShaderClass MeshLoadContextClass::Peek_Legacy_Shader(int legacy_material_index)
{
    captainslog_assert(legacy_material_index >= 0);
    captainslog_assert(legacy_material_index < m_legacyMaterials.Count());
    int si = m_legacyMaterials[legacy_material_index]->m_shaderIdx;
    return Peek_Shader(si);
}

VertexMaterialClass *MeshLoadContextClass::Peek_Legacy_Vertex_Material(int legacy_material_index)
{
    captainslog_assert(legacy_material_index >= 0);
    captainslog_assert(legacy_material_index < m_legacyMaterials.Count());
    int vi = m_legacyMaterials[legacy_material_index]->m_vertexMaterialIdx;

    if (vi != -1) {
        return Peek_Vertex_Material(vi);
    } else {
        return nullptr;
    }
}

TextureClass *MeshLoadContextClass::Peek_Legacy_Texture(int legacy_material_index)
{
    captainslog_assert(legacy_material_index >= 0);
    captainslog_assert(legacy_material_index < m_legacyMaterials.Count());
    int ti = m_legacyMaterials[legacy_material_index]->m_textureIdx;

    if (ti != -1) {
        return Peek_Texture(ti);
    } else {
        return nullptr;
    }
}

Vector2 *MeshLoadContextClass::Get_Temporary_UV_Array(int elementcount)
{
    m_tempUVArray.Uninitialised_Grow(elementcount);
    return &(m_tempUVArray[0]);
}

void MeshModelClass::Modify_For_Overbright()
{
    for (int i = 0; i < m_curMatDesc->Get_Pass_Count(); i++) {
        ShaderClass shader = m_curMatDesc->Get_Single_Shader(i);

        if (shader.Get_Primary_Gradient() == ShaderClass::GRADIENT_MODULATE) {
            shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE2X);
            m_curMatDesc->Set_Single_Shader(shader, i);
        }

        if (m_curMatDesc->Get_Shader_Array(i)) {
            for (int j = 0; j < m_polyCount; j++) {
                shader = m_curMatDesc->Get_Shader(j, i);

                if (shader.Get_Primary_Gradient() == ShaderClass::GRADIENT_MODULATE) {
                    shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE2X);
                    m_curMatDesc->Set_Shader(j, shader, i);
                }
            }
        }
    }
}

DX8FVFCategoryContainer *MeshModelClass::Peek_FVF_Category_Container()
{
    if (m_polygonRendererList.Is_Empty()) {
        return nullptr;
    }

    DX8PolygonRendererClass *polygon_renderer = m_polygonRendererList.Get_Head();
    captainslog_assert(polygon_renderer);

    DX8TextureCategoryClass *texture_category = polygon_renderer->Get_Texture_Category();
    captainslog_assert(texture_category);

    DX8FVFCategoryContainer *fvf_category = texture_category->Get_Container();
    captainslog_assert(fvf_category);

    return fvf_category;
}
