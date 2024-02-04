/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tree Buffer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtreebuffer.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "camera.h"
#include "colorspace.h"
#include "dx8wrapper.h"
#include "filesystem.h"
#include "gamelogic.h"
#include "matinfo.h"
#include "mesh.h"
#include "meshmdl.h"
#include "partitionmanager.h"
#include "playerlist.h"
#include "randomvalue.h"
#include "scriptengine.h"
#include "shadermanager.h"
#include "tiledata.h"
#include "w3dbridgebuffer.h"
#include "w3dprojectedshadow.h"
#include "w3dshroud.h"
#include "w3dtreedraw.h"
#include "worldheightmap.h"

ShaderClass s_detailAlphaShader2(0x5441B);

static float ANGULAR_LIMIT = 1.521709f;
static float SCALE_EPSILON = 0.0099999998f;
static float LOGIC_FRAMES_PER_MSEC_REAL = 0.029999999f;

class GDIFileStream2 : public InputStream
{
public:
    GDIFileStream2(File *file) : m_file(file) {}
    int Read(void *dst, int size) override
    {
        if (m_file != nullptr) {
            return m_file->Read(dst, size);
        } else {
            return 0;
        }
    }

private:
    File *m_file;
};

void Do_FX_Pos(FXList const *list,
    const Coord3D *primary,
    const Matrix3D *primary_mtx,
    float primary_speed,
    const Coord3D *secondary,
    float radius);

int W3DTreeBuffer::W3DTreeTextureClass::Update(W3DTreeBuffer *buffer)
{
#ifdef BUILD_WITH_D3D8
    Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    w3dsurface_t surf = nullptr;
    DX8Wrapper::Handle_DX8_ErrorCode(
        reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())->GetSurfaceLevel(0, &surf));
    D3DSURFACE_DESC desc;
    DX8Wrapper::Handle_DX8_ErrorCode(surf->GetDesc(&desc));
    D3DLOCKED_RECT rect;
    DX8Wrapper::Handle_DX8_ErrorCode(surf->LockRect(&rect, nullptr, 0));

    int width = 64;

    if (desc.Format == D3DFMT_A8R8G8B8) {
        for (int i = 0; i < buffer->Get_Num_Tiles(); i++) {
            TileData *tile = buffer->Get_Tile_Data(i);

            if (tile) {
                int32_t x = tile->m_tileLocationInTexture.x;
                int32_t y = tile->m_tileLocationInTexture.y;

                if (x >= 0) {
                    for (int j = 0; j < width; j++) {
                        unsigned char *data = tile->Get_RGB_Data_For_Width(width);
                        unsigned char *data2 = &data[width * 4 * (width - 1 - j)];
                        unsigned int *data3 = (unsigned int *)((char *)rect.pBits + 4 * desc.Width * (j + y) + 4 * x);

                        for (int k = 0; k < width; k++) {
                            *data3++ = *data2 + (data2[1] << 8) + (data2[2] << 16) + (data2[3] << 24);
                            data2 += 4;
                        }
                    }
                }
            }
        }
    }

    surf->UnlockRect();
    surf->Release();
    DX8Wrapper::Handle_DX8_ErrorCode(
        D3DXFilterTexture(reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture()), nullptr, 0, D3DX_FILTER_BOX));

    if (g_theWriteableGlobalData->m_textureReductionFactor) {
        DX8Wrapper::Handle_DX8_ErrorCode(reinterpret_cast<w3dtexture_t>(Peek_Platform_Base_Texture())
                                             ->SetLOD(g_theWriteableGlobalData->m_textureReductionFactor));
    }

    return desc.Height;
#else
    return 0;
#endif
}

void W3DTreeBuffer::W3DTreeTextureClass::Set_Lod(int LOD) const
{
#ifdef BUILD_WITH_D3D8
    if (Peek_Platform_Base_Texture() != nullptr) {
        DX8Wrapper::Handle_DX8_ErrorCode(Peek_Platform_Base_Texture()->SetLOD(LOD));
    }
#endif
}

void W3DTreeBuffer::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    int num_trees = m_numTrees;
    xfer->xferInt(&num_trees);

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_numTrees = 0;

        for (int i = 0; i < MAX_PARTITON_INDICES; i++) {
            m_partitionIndices[i] = -1;
        }
    }

    for (int i = 0; i < num_trees; i++) {
        TTree tree;
        memset(&tree, 0, sizeof(tree));
        Utf8String model_name;
        Utf8String texture_name;
        int tree_type = -2;

        if (xfer->Get_Mode() != XFER_LOAD) {
            tree = m_trees[i];
            tree_type = m_trees[i].tree_type;

            if (tree_type != -2) {
                model_name = m_treeTypes[tree_type].module->m_modelName;
                texture_name = m_treeTypes[tree_type].module->m_textureName;
            }
        }

        xfer->xferAsciiString(&model_name);
        xfer->xferAsciiString(&texture_name);

        if (xfer->Get_Mode() == XFER_LOAD) {
            for (int j = 0; j < m_numTreeTypes; j++) {
                if (m_treeTypes[j].module->m_modelName.Compare_No_Case(model_name) == 0
                    && m_treeTypes[j].module->m_textureName.Compare_No_Case(texture_name) == 0) {
                    tree_type = j;
                    break;
                }
            }
        }

        xfer->xferReal(&tree.location.X);
        xfer->xferReal(&tree.location.Y);
        xfer->xferReal(&tree.location.Z);
        xfer->xferReal(&tree.scale);
        xfer->xferReal(&tree.sin);
        xfer->xferReal(&tree.cos);
        xfer->xferDrawableID(&tree.drawable_id);
        xfer->xferReal(&tree.angular_velocity);
        xfer->xferReal(&tree.angular_acceleration);
        xfer->xferCoord3D(&tree.topple_direction);
        xfer->xferUser(&tree.topple_state, sizeof(tree.topple_state));
        xfer->xferReal(&tree.angular_accumulation);
        xfer->xferUnsignedInt(&tree.options);
        xfer->xferMatrix3D(&tree.topple_transform);
        xfer->xferUnsignedInt(&tree.topple_sink_countdown);

        if (xfer->Get_Mode() == XFER_LOAD && tree_type != -2 && tree_type < m_numTreeTypes) {
            Coord3D location;
            location.Set(tree.location.X, tree.location.Y, tree.location.Z);
            Add_Tree(tree.drawable_id, location, tree.scale, 0.0f, 0.0f, m_treeTypes[tree_type].module);

            if (m_numTrees != 0) {
                TTree *tree2 = &m_trees[m_numTrees - 1];
                tree2->angular_acceleration = tree.angular_acceleration;
                tree2->angular_velocity = tree.angular_velocity;
                tree2->topple_direction = tree.topple_direction;
                tree2->topple_state = tree.topple_state;
                tree2->options = tree.options;
                tree2->topple_transform = tree.topple_transform;
                tree2->topple_sink_countdown = tree.topple_sink_countdown;
            }
        }
    }
}

void W3DTreeBuffer::Add_Tree(
    DrawableID drawable, Coord3D loc, float scale, float angle, float random, const W3DTreeDrawModuleData *module)
{
    if (m_numTrees < MAX_TREES && m_initialized) {
        int tree_type = -2;

        for (int i = 0; i < m_numTreeTypes; i++) {
            if (m_treeTypes[i].module->m_modelName.Compare_No_Case(module->m_modelName) == 0
                && m_treeTypes[i].module->m_textureName.Compare_No_Case(module->m_textureName) == 0) {
                tree_type = i;
                break;
            }
        }

        if (tree_type < 0) {
            tree_type = Add_Tree_Type(module);

            if (tree_type < 0) {
                return;
            }

            m_textureDirty = true;
        }

        if (module->m_moveOutwardTime > 2 || module->m_doTopple) {
            short bucket = Get_Partition_Bucket(&loc);
            m_trees[m_numTrees].partition_bucket_index = m_partitionIndices[bucket];
            m_partitionIndices[bucket] = m_numTrees;
        } else {
            m_trees[m_numTrees].partition_bucket_index = -1;
        }

        float random_high = 1.0f + random;
        float random_low = 1.0f - random;
        float random_scale = Get_Client_Random_Value_Real(random_low, random_high);
        m_trees[m_numTrees].sin = GameMath::Sin(angle);
        m_trees[m_numTrees].cos = GameMath::Cos(angle);

        if (random > 0.0f) {
            m_trees[m_numTrees].scale = scale * random_scale;
        } else {
            m_trees[m_numTrees].scale = scale;
        }

        m_trees[m_numTrees].location = Vector3(loc.x, loc.y, loc.z);
        m_trees[m_numTrees].tree_type = tree_type;
        m_trees[m_numTrees].bounds = m_treeTypes[tree_type].bounds;
        m_trees[m_numTrees].bounds.Center *= m_trees[m_numTrees].scale;
        m_trees[m_numTrees].bounds.Radius *= m_trees[m_numTrees].scale;
        m_trees[m_numTrees].bounds.Center += m_trees[m_numTrees].location;
        m_trees[m_numTrees].visible = false;
        m_trees[m_numTrees].drawable_id = drawable;
        m_trees[m_numTrees].vertex_array_index = 0;
        m_trees[m_numTrees].vb_index = -1;
        m_trees[m_numTrees].sway_rand = Get_Client_Random_Value(0, 9);
        m_trees[m_numTrees].push_aside_speed = 0.0f;
        m_trees[m_numTrees].push_aside_frame = 0;
        m_trees[m_numTrees].push_aside_obj_id = INVALID_OBJECT_ID;
        m_trees[m_numTrees].push_aside_move_time = 0.0f;
        m_trees[m_numTrees].push_aside_location.y = 1.0f;
        m_trees[m_numTrees].push_aside_location.x = 1.0f;
        m_trees[m_numTrees].topple_state = TTree::TOPPLE_UPRIGHT;
        m_numTrees++;
    }
}

int W3DTreeBuffer::Get_Partition_Bucket(const Coord3D *loc) const
{
    float x = loc->x;
    float y = loc->y;

    if (loc->x < m_partitionRegion.lo.x) {
        x = m_partitionRegion.lo.x;
    }

    if (y < m_partitionRegion.lo.y) {
        y = m_partitionRegion.lo.y;
    }

    if (x > m_partitionRegion.hi.x) {
        x = m_partitionRegion.hi.x;
    }

    if (y > m_partitionRegion.hi.y) {
        y = m_partitionRegion.hi.y;
    }

    int x_index = GameMath::Fast_To_Int_Floor(x / (m_partitionRegion.hi.x - m_partitionRegion.lo.x) * 99.9f);
    int y_index = GameMath::Fast_To_Int_Floor(y / (m_partitionRegion.hi.y - m_partitionRegion.lo.y) * 99.9f);
    captainslog_dbgassert(
        x_index >= 0 && y_index >= 0 && x_index < PARTITION_WIDTH_HEIGHT && y_index < PARTITION_WIDTH_HEIGHT,
        "Invalid range.");
    return x_index + PARTITION_WIDTH_HEIGHT * y_index;
}

int W3DTreeBuffer::Add_Tree_Type(const W3DTreeDrawModuleData *module)
{
    if (m_numTreeTypes >= MAX_TREE_TYPES) {
        captainslog_dbgassert(false, "Too many kinds of trees in map.  Reduce kinds of trees, or raise tree limit.");
        return 0;
    } else {
        m_textureDirty = true;
        m_treeTypes[m_numTreeTypes].mesh = nullptr;
        RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(module->m_modelName.Str());

        if (robj == nullptr) {
            captainslog_dbgassert(false, "Unable to find model for tree %s", module->m_modelName.Str());
            return 0;
        } else {
            AABoxClass bounding_box;
            robj->Get_Obj_Space_Bounding_Box(bounding_box);
            Vector3 pos(0.0f, 0.0f, 0.0f);

            if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                RenderObjClass *robj2 = robj;
                robj = robj->Get_Sub_Object(0);
                pos = robj->Get_Bone_Transform(0).Get_Translation();
                Ref_Ptr_Release(robj2);
            }

            if (robj->Class_ID() == RenderObjClass::CLASSID_MESH) {
                m_treeTypes[m_numTreeTypes].mesh = static_cast<MeshClass *>(robj);
            }

            if (m_treeTypes[m_numTreeTypes].mesh == nullptr) {
                captainslog_dbgassert(false,
                    "Tree %s is not simple mesh. Tell artist to re-export. Don't Ignore!!!",
                    module->m_modelName.Str());
                return 0;
            }

            int vert_count = m_treeTypes[m_numTreeTypes].mesh->Peek_Model()->Get_Vertex_Count();
            Vector3 *vert_array = m_treeTypes[m_numTreeTypes].mesh->Peek_Model()->Get_Vertex_Array();
            Matrix3D tm = m_treeTypes[m_numTreeTypes].mesh->Get_Transform();
            SphereClass bounds(vert_array, vert_count);
            bounds.Center += pos;
            m_treeTypes[m_numTreeTypes].bounds = bounds;
            m_treeTypes[m_numTreeTypes].texture_width = 0;
            m_treeTypes[m_numTreeTypes].texture_height = 0;
            m_treeTypes[m_numTreeTypes].module = module;
            m_treeTypes[m_numTreeTypes].pos = pos;
            m_treeTypes[m_numTreeTypes].uv_offset = bounding_box.m_extent.X + bounding_box.m_extent.Y;
            m_treeTypes[m_numTreeTypes].do_shadow = module->m_doShadow;
            m_numTreeTypes++;
            return m_numTreeTypes - 1;
        }
    }
}

W3DTreeBuffer::W3DTreeBuffer() :
    m_dwTreePixelShader(0),
    m_dwTreeVertexShader(0),
    m_treeTexture(nullptr),
    m_initialized(false),
    m_swayVersion(-1),
    m_decalShadow(nullptr)
{
    for (int i = 0; i < 1; ++i) {
        m_vertexTree[i] = 0;
        m_indexTree[i] = 0;
        m_curNumTreeVertices[i] = 0;
        m_curNumTreeIndices[i] = 0;
    }

    Clear_All_Trees();
    Allocate_Tree_Buffers();
    m_initialized = true;
}

W3DTreeBuffer::~W3DTreeBuffer()
{
    Free_Tree_Buffers();
    Ref_Ptr_Release(m_treeTexture);

    for (int i = 0; i < MAX_TREE_TYPES; i++) {
        Ref_Ptr_Release(m_treeTypes[i].mesh);
    }

    delete m_decalShadow;
    m_decalShadow = nullptr;
}

void W3DTreeBuffer::Clear_All_Trees()
{
    m_numTrees = 0;
    m_partitionRegion.lo.y = 0.0f;
    m_partitionRegion.lo.x = 0.0f;
    m_partitionRegion.hi.y = 1.0f;
    m_partitionRegion.hi.x = 1.0f;
    Ref_Ptr_Release(m_treeTexture);
    m_curNumTreeIndices[0] = 0;
    m_anythingChanged = true;

    for (int i = 0; i < MAX_TREE_TYPES; i++) {
        Ref_Ptr_Release(m_treeTypes[i].mesh);
    }

    for (int i = 0; i < MAX_PARTITON_INDICES; i++) {
        m_partitionIndices[i] = -1;
    }

    m_numTreeTypes = 0;
}

void W3DTreeBuffer::Free_Tree_Buffers()
{
    for (int i = 0; i < 1; i++) {
        Ref_Ptr_Release(m_vertexTree[i]);
        Ref_Ptr_Release(m_indexTree[i]);
    }

#ifdef BUILD_WITH_D3D8
    if (m_dwTreePixelShader != 0) {
        DX8Wrapper::Get_D3D_Device8()->DeletePixelShader(m_dwTreePixelShader);
    }

    m_dwTreePixelShader = 0;

    if (m_dwTreeVertexShader != 0) {
        DX8Wrapper::Get_D3D_Device8()->DeleteVertexShader(m_dwTreeVertexShader);
    }

    m_dwTreeVertexShader = 0;
#endif
}

void W3DTreeBuffer::Allocate_Tree_Buffers()
{
    for (int i = 0; i < 1; i++) {
        m_vertexTree[i] =
            new DX8VertexBufferClass(DX8_FVF_XYZNDUV1, MAX_TREE_VERTEX + 4, DX8VertexBufferClass::USAGE_DEFAULT, 0);
        m_indexTree[i] = new DX8IndexBufferClass(MAX_TREE_INDEX + 4, DX8IndexBufferClass::USAGE_DEFAULT);
        m_curNumTreeVertices[i] = 0;
        m_curNumTreeIndices[i] = 0;
    }

#ifdef BUILD_WITH_D3D8
    DWORD decl[] = { D3DVSD_STREAM(0),
        D3DVSD_REG(0, D3DVSDT_FLOAT3),
        D3DVSD_REG(1, D3DVSDT_FLOAT3),
        D3DVSD_REG(2, D3DVSDT_D3DCOLOR),
        D3DVSD_REG(7, D3DVSDT_FLOAT2),
        D3DVSD_END() };

    int res = W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\trees.vso", decl, 0, true, &m_dwTreeVertexShader);

    if (FAILED(res)) {
        return;
    }

    W3DShaderManager::Load_And_Create_D3D_Shader("shaders\\trees.pso", decl, 0, false, &m_dwTreePixelShader);
#endif
}

void W3DTreeBuffer::Set_Texture_LOD(int LOD)
{
    if (m_treeTexture != nullptr) {
        m_treeTexture->Set_Lod(LOD);
    }
}

void W3DTreeBuffer::Remove_Tree(DrawableID drawable)
{
    for (int i = 0; i < m_numTrees; i++) {
        if (m_trees[i].drawable_id == drawable) {
            m_trees[i].location = Vector3(0.0f, 0.0f, 0.0f);
            m_trees[i].tree_type = -2;
            m_trees[i].bounds.Center = Vector3(0.0f, 0.0f, 0.0f);
            m_trees[i].bounds.Radius = 1.0f;
            m_anythingChanged = true;
        }
    }
}

void W3DTreeBuffer::Remove_Trees_For_Construction(const Coord3D *pos, const GeometryInfo &geom, float angle)
{
    for (int i = 0; i < m_numTrees; i++) {
        if (m_trees[i].tree_type >= 0) {
            GeometryInfo geom2(GEOMETRY_CYLINDER, false, 35.0f, 14.0f, 14.0f);
            Coord3D pos2;
            pos2.Set(m_trees[i].location.X, m_trees[i].location.Y, m_trees[i].location.Z);

            if (g_thePartitionManager->Geom_Collides_With_Geom(pos, geom, angle, &pos2, geom2, 0.0f)) {
                m_trees[i].tree_type = -2;
                m_anythingChanged = true;
            }
        }
    }
}

bool W3DTreeBuffer::Update_Tree_Position(DrawableID drawable, Coord3D pos, float angle)
{
    for (int i = 0; i < m_numTrees; i++) {
        if (m_trees[i].drawable_id == drawable) {
            m_trees[i].location = Vector3(pos.x, pos.y, pos.z);
            m_trees[i].sin = GameMath::Sin(angle);
            m_trees[i].cos = GameMath::Cos(angle);
            m_trees[i].bounds = m_treeTypes[m_trees[i].tree_type].bounds;
            m_trees[i].bounds.Center *= m_trees[i].scale;
            m_trees[i].bounds.Radius *= m_trees[i].scale;
            m_trees[i].bounds.Center += m_trees[i].location;
            m_anythingChanged = true;
            return true;
        }
    }

    return false;
}

void W3DTreeBuffer::Update_Sway(const BreezeInfo &info)
{
    for (int i = 0; i < 100; i++) {
        float angle = GameMath::Cos((i + i) * GAMEMATH_PI / 101.0f);
        float angle2 = angle * info.intensity + info.lean;
        float sin = GameMath::Sin(angle2);
        float cos = GameMath::Cos(angle2);
        m_swayVector[i].X = sin * info.sway_direction.x;
        m_swayVector[i].Y = sin * info.sway_direction.y;
        m_swayVector[i].Z = cos - 1.0f;
    }

    float random = info.randomness * 0.5f;

    for (int i = 0; i < m_numTrees; i++) {
        m_trees[i].sway_rand = Get_Client_Random_Value(0, 9) + 1;
    }

    for (int i = 0; i < 10; i++) {
        m_swayPeriods[i] = 100.0f / info.period;
        float random_high = random + 1.0f;
        float random_low = 1.0f - random;
        m_swayPeriods[i] *= Get_Client_Random_Value_Real(random_low, random_high);

        if (m_swayPeriods[i] < 0.0f) {
            m_swayPeriods[i] = 0.0f;
        }

        m_swayCurrent[i] = 0.0f;
        random_high = random + 1.0f;
        random_low = 1.0f - random;
        m_swayLeanAngles[i] = Get_Client_Random_Value_Real(random_low, random_high);
    }

    m_swayVersion = info.version;
}

int W3DTreeBuffer::Do_Lighting(
    const Vector3 *loc, const GlobalData::TerrainLighting *light, const Vector3 *emissive, unsigned int color, float factor)
{
    float red = light->ambient.red + emissive->X;
    float green = light->ambient.green + emissive->Y;
    float blue = light->ambient.blue + emissive->Z;

    for (int i = 0; i < 3; i++) {
        Vector3 light_pos(light[i].lightPos.x, light[i].lightPos.y, light[i].lightPos.z);
        light_pos.Normalize();
        Vector3 neg_light_pos(-light_pos.X, -light_pos.Y, -light_pos.Z);
        float f1 = neg_light_pos * *loc;

        if (f1 > 1.0f) {
            f1 = 1.0f;
        }

        if (f1 < 0.0f) {
            f1 = 0.0f;
        }

        red = f1 * light[i].diffuse.red + red;
        green = f1 * light[i].diffuse.green + green;
        blue = f1 * light[i].diffuse.blue + blue;
    }

    red = red * factor;
    green = green * factor;
    blue = blue * factor;

    if (red > 1.0f) {
        red = 1.0f;
    }

    if (red < 0.0f) {
        red = 0.0f;
    }

    if (green > 1.0f) {
        green = 1.0f;
    }

    if (green < 0.0f) {
        green = 0.0f;
    }

    if (blue > 1.0f) {
        blue = 1.0f;
    }

    if (blue < 0.0f) {
        blue = 0.0f;
    }

    if (color != -1) {
        blue = (color & 0xFF) / 255.0f * blue;
        green = ((color >> 8) & 0xFF) / 255.0f * green;
        red = ((color >> 0x10) & 0xFF) / 255.0f * red;
    }

    red = red * 255.0f;
    green = green * 255.0f;
    blue = blue * 255.0f;

    return Make_Color(GameMath::Fast_To_Int_Truncate(red),
        GameMath::Fast_To_Int_Truncate(green),
        GameMath::Fast_To_Int_Truncate(blue),
        255);
}

void W3DTreeBuffer::Load_Trees_In_Index_And_Vertex_Buffers(
    RefMultiListIterator<class RenderObjClass> *pDynamicLightsIterator)
{
    if (m_indexTree[0] != nullptr && m_vertexTree[0] != nullptr && m_initialized && m_anythingChanged) {
        if (m_decalShadow == nullptr && g_theW3DProjectedShadowManager != nullptr) {
            Shadow::ShadowTypeInfo shadow;
            shadow.m_shadowName[0] = 0;
            shadow.m_allowUpdates = false;
            shadow.m_allowWorldAlign = true;
            shadow.m_type = SHADOW_DECAL;
            shadow.m_sizeX = 20.0f;
            shadow.m_sizeY = 20.0f;
            shadow.m_offsetX = 0.0f;
            shadow.m_offsetY = 0.0f;
            m_decalShadow = g_theW3DProjectedShadowManager->Create_Decal_Shadow(&shadow);
        }

        m_anythingChanged = false;
        int count = 0;
        GlobalData::TerrainLighting *lighting =
            g_theWriteableGlobalData->m_terrainObjectLighting[g_theWriteableGlobalData->m_timeOfDay];

        for (int i = 0; i < 1; i++) {
            m_curNumTreeVertices[i] = 0;
            m_curNumTreeIndices[i] = 0;

            if (count >= m_numTrees) {
                break;
            }

            IndexBufferClass::WriteLockClass indexlock(m_indexTree[i], 0);
            VertexBufferClass::WriteLockClass vertexlock(m_vertexTree[i], 0);
            VertexFormatXYZNDUV1 *vertexes = static_cast<VertexFormatXYZNDUV1 *>(vertexlock.Get_Vertex_Array());
            unsigned short *indexes = indexlock.Get_Index_Array();
            Vector2 camera(m_cameraLookAtVector.X, m_cameraLookAtVector.Y);
            camera.Normalize();

            while (count < m_numTrees) {
                int tree_type = m_trees[count].tree_type;

                if (tree_type >= 0) {
                    if (m_trees[count].visible) {
                        float scale = m_trees[count].scale;
                        Vector3 location(m_trees[count].location);
                        float sin = m_trees[count].sin;
                        float cos = m_trees[count].cos;

                        if (tree_type >= 0) {
                            if (m_treeTypes[tree_type].mesh != nullptr) {
                                bool has_normals = true;
                                Vector3 emissive(0.0f, 0.0f, 0.0f);
                                MaterialInfoClass *info = m_treeTypes[tree_type].mesh->Get_Material_Info();

                                if (info != nullptr) {
                                    VertexMaterialClass *material = info->Peek_Vertex_Material(0);

                                    if (material != nullptr) {
                                        material->Get_Emissive(&emissive);
                                    }
                                }

                                Ref_Ptr_Release(info);
                                int index = m_curNumTreeVertices[i];
                                m_trees[count].vertex_array_index = index;
                                m_trees[count].vb_index = i;
                                int vert_count = m_treeTypes[tree_type].mesh->Peek_Model()->Get_Vertex_Count();
                                Vector3 *vert_array = m_treeTypes[tree_type].mesh->Peek_Model()->Get_Vertex_Array();

                                if (m_curNumTreeVertices[i] + vert_count + 2 >= MAX_TREE_VERTEX) {
                                    break;
                                }

                                int poly_count = m_treeTypes[tree_type].mesh->Peek_Model()->Get_Polygon_Count();
                                const Vector3i16 *poly_array =
                                    m_treeTypes[tree_type].mesh->Peek_Model()->Get_Polygon_Array();

                                if (m_curNumTreeIndices[i] + 3 * poly_count + 6 >= MAX_TREE_INDEX) {
                                    break;
                                }

                                const Vector2 *uv_array =
                                    m_treeTypes[tree_type].mesh->Peek_Model()->Get_UV_Array_By_Index(0);
                                const Vector3 *normal_array =
                                    m_treeTypes[tree_type].mesh->Peek_Model()->Get_Vertex_Normal_Array();
                                unsigned int *color_array =
                                    m_treeTypes[tree_type].mesh->Peek_Model()->Get_Color_Array(0, false);
                                int diffuse = 0;

                                if (normal_array == nullptr) {
                                    has_normals = false;
                                    Vector3 pos(0.0f, 0.0f, 1.0f);
                                    diffuse = Do_Lighting(&pos, lighting, &emissive, 0xFFFFFFFF, 1.0f);
                                }

                                float u1 = (float)m_treeTypes[tree_type].width * 64.0f / (float)m_textureWidth;
                                float v1 = (float)m_treeTypes[tree_type].width * 64.0f / (float)m_textureHeight;
                                float u2 = (float)m_treeTypes[tree_type].texture_width / (float)m_textureWidth;
                                float v2 = (float)m_treeTypes[tree_type].texture_height / (float)m_textureHeight;

                                if (m_treeTypes[tree_type].valid_tile) {
                                    u1 = u1 * 0.5f;
                                    v1 = v1 * 0.5f;
                                    v2 = 32.0f / (float)m_textureHeight + v2;
                                }

                                for (int j = 0; j < vert_count && m_curNumTreeVertices[i] < MAX_TREE_VERTEX; j++) {
                                    float u = uv_array[j].X;
                                    float v = uv_array[j].Y;

                                    if (u > 1.0f) {
                                        u = 1.0f;
                                    }

                                    if (u < 0.0f) {
                                        u = 0.0f;
                                    }

                                    if (v > 1.0f) {
                                        v = 1.0f;
                                    }

                                    if (v < 0.0f) {
                                        v = 0.0f;
                                    }

                                    vertexes->u1 = u * u1 + u2;
                                    vertexes->v1 = v * v1 + v2;

                                    float vx = vert_array[j].X;
                                    float vy = vert_array[j].Y;
                                    Vector3 vertex;
                                    vx = vx + m_treeTypes[tree_type].pos.X;
                                    vy = vy + m_treeTypes[tree_type].pos.Y;
                                    vertex.X = vx * scale * cos - vy * scale * sin;
                                    vertex.Y = vx * scale * sin + vy * scale * cos;
                                    vertex.Z = scale * vert_array[j].Z;
                                    vertex.Z = vertex.Z + m_treeTypes[tree_type].pos.Z;

                                    if (m_trees[count].topple_state != TTree::TOPPLE_UPRIGHT) {
                                        Matrix3D::Transform_Vector(m_trees[count].topple_transform, vertex, &vertex);
                                    } else {
                                        if (m_trees[count].push_aside_speed > 0.0f) {
                                            vertex.X = vert_array[j].Z * m_trees[count].push_aside_speed
                                                    * m_trees[count].push_aside_location.y
                                                    * m_treeTypes[tree_type].module->m_moveOutwardDistanceFactor
                                                + vertex.X;
                                            vertex.Y = vert_array[j].Z * m_trees[count].push_aside_speed
                                                    * m_trees[count].push_aside_location.x
                                                    * m_treeTypes[tree_type].module->m_moveOutwardDistanceFactor
                                                + vertex.Y;
                                        }

                                        vertex.X = vertex.X + location.X;
                                        vertex.Y = vertex.Y + location.Y;
                                        vertex.Z = vertex.Z + location.Z;
                                    }

                                    vertexes->x = vertex.X;
                                    vertexes->y = vertex.Y;
                                    vertexes->z = vertex.Z;

                                    vertexes->nx = (float)m_trees[count].sway_rand;
                                    vertexes->ny = 1.0f
                                        - m_treeTypes[tree_type].module->m_darkeningFactor
                                            * m_trees[count].push_aside_speed; // BUG m_darkeningFactor should probably be
                                                                               // m_moveOutwardDistanceFactor
                                    vertexes->nz = location.Z;

                                    if (has_normals) {
                                        Vector3 loc(0.0f, 0.0f, 1.0f);

                                        if (normal_array != nullptr) {
                                            loc.X = cos * normal_array[j].X - sin * normal_array[j].Y;
                                            loc.Y = sin * normal_array[j].X + cos * normal_array[j].Y;
                                            loc.Z = normal_array[j].Z;
                                        }

                                        unsigned int color;

                                        if (color_array != nullptr) {
                                            color = color_array[j];
                                        } else {
                                            color = 0xFFFFFFFF;
                                        }

                                        vertexes->diffuse = Do_Lighting(&loc, lighting, &emissive, color, 1.0f);
                                    } else {
                                        vertexes->diffuse = diffuse;
                                    }

                                    vertexes++;
                                    m_curNumTreeVertices[i]++;
                                }

                                for (int j = 0; j < poly_count && m_curNumTreeIndices[i] + 4 <= MAX_TREE_INDEX; j++) {
                                    *indexes++ = poly_array[j].I + index;
                                    *indexes++ = poly_array[j].J + index;
                                    *indexes++ = poly_array[j].K + index;
                                    m_curNumTreeIndices[i] += 3;
                                }
                            }
                        }
                    }
                }

                count++;
            }
        }
    }
}

void W3DTreeBuffer::Cull(const CameraClass *camera)
{
    Matrix3D tm(camera->Get_Transform());
    float x = -1.0f * tm[0][2];
    float y = -1.0f * tm[1][2];
    float z = -1.0f * tm[2][2];
    m_cameraLookAtVector.Set(x, y, z);

    for (int i = 0; i < m_numTrees; i++) {
        bool sort = false;
        bool visible = !camera->Cull_Sphere(m_trees[i].bounds);

        if (visible != m_trees[i].visible) {
            m_trees[i].visible = visible;
            m_anythingChanged = true;

            if (visible) {
                sort = true;
            }
        }

        if (sort || (visible && m_updateAllKeys)) {
            m_trees[i].sort_key = m_trees[i].location * m_cameraLookAtVector;
        }
    }

    m_updateAllKeys = false;
}

void W3DTreeBuffer::Apply_Toppling_Force(TTree *tree, const Coord3D *pos, float speed, int options)
{
    if (tree->topple_state == TTree::TOPPLE_UPRIGHT) {
        const W3DTreeDrawModuleData *module = m_treeTypes[tree->tree_type].module;

        if (speed < module->m_minimumToppleSpeed) {
            speed = module->m_minimumToppleSpeed;
        }

        tree->topple_direction = *pos;
        tree->topple_direction.Normalize();
        tree->angular_accumulation = 0.0f;
        tree->angular_velocity = speed * module->m_initialVelocityPercent;
        tree->angular_acceleration = speed * module->m_initialAccelPercent;
        tree->topple_state = TTree::TOPPLE_WAITING_FOR_START;
        tree->options = options;
        Coord3D primary;
        primary.Set(tree->location.X, tree->location.Y, tree->location.Z);
        Do_FX_Pos(module->m_toppleFX, &primary, nullptr, 0.0f, nullptr, 0.0f);
        m_vertexesDirty = true;
        tree->topple_transform.Make_Identity();
        tree->topple_transform.Set_Translation(tree->location);
    }
}

void W3DTreeBuffer::Update_Toppling_Tree(TTree *tree)
{
    captainslog_dbgassert(tree->topple_state != TTree::TOPPLE_UPRIGHT, "bad topple state");
    if (tree->topple_state != TTree::TOPPLE_UPRIGHT && tree->topple_state != TTree::TOPPLE_DOWN) {
        const W3DTreeDrawModuleData *module = m_treeTypes[tree->tree_type].module;
        int id;

        if (g_thePlayerList != nullptr) {
            id = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        } else {
            id = 0;
        }

        Coord3D loc;
        loc.Set(tree->location.X, tree->location.Y, tree->location.Z);

        if (g_thePartitionManager->Get_Shroud_Status_For_Player(id, &loc) == SHROUDED_SEEN) {
            tree->topple_state = TTree::TOPPLE_TOPPLING;
        } else if (tree->topple_state == TTree::TOPPLE_TOPPLING) {
            tree->angular_velocity = 0.0f;
            tree->topple_state = TTree::TOPPLE_DOWN;
            tree->topple_transform.In_Place_Pre_Rotate_X(-ANGULAR_LIMIT * tree->topple_direction.y);
            tree->topple_transform.In_Place_Pre_Rotate_Y(ANGULAR_LIMIT * tree->topple_direction.x);

            if (module->m_killWhenFinishedToppling) {
                tree->topple_sink_countdown = 0;
            }
        } else {
            float angular_velocity = tree->angular_velocity;

            if (angular_velocity + tree->angular_accumulation > ANGULAR_LIMIT) {
                angular_velocity = ANGULAR_LIMIT - tree->angular_accumulation;
            }

            tree->topple_transform.In_Place_Pre_Rotate_X(-angular_velocity * tree->topple_direction.y);
            tree->topple_transform.In_Place_Pre_Rotate_Y(angular_velocity * tree->topple_direction.x);
            tree->angular_accumulation = angular_velocity + tree->angular_accumulation;

            if (tree->angular_accumulation >= ANGULAR_LIMIT && tree->angular_velocity > 0.0f) {
                tree->angular_velocity = -module->m_bounceVelocityPercent * tree->angular_velocity;

                if ((tree->options & TTree::TOPPLE_OPTIONS_NO_BOUNCE) == 0
                    && (SCALE_EPSILON <= GameMath::Fabs(tree->angular_velocity))) {
                    if (LOGIC_FRAMES_PER_MSEC_REAL < GameMath::Fabs(tree->angular_velocity)
                        && (tree->options & TTree::TOPPLE_OPTIONS_NO_FX) == 0) {
                        Vector3 in(0.0f, 0.0f, 21.0f);
                        Vector3 out;
                        Matrix3D::Transform_Vector(tree->topple_transform, in, &out);
                        Coord3D pos;
                        pos.Set(out.X, out.Y, out.Z);
                        Do_FX_Pos(module->m_bounceFX, &pos, nullptr, 0.0f, nullptr, 0.0f);
                    }
                } else {
                    tree->angular_velocity = 0.0f;
                    tree->topple_state = TTree::TOPPLE_DOWN;

                    if (module->m_killWhenFinishedToppling) {
                        tree->topple_sink_countdown = module->m_sinkTime;
                    }
                }
            } else {
                tree->angular_velocity = tree->angular_velocity + tree->angular_acceleration;
            }
        }
    }
}

void W3DTreeBuffer::Push_Aside_Tree(DrawableID drawable, const Coord3D *pos, const Coord3D *dir, ObjectID object)
{
    for (int i = 0; i < m_numTrees; i++) {
        if (m_trees[i].drawable_id == drawable) {
            unsigned int frame = m_trees[i].push_aside_frame;
            m_trees[i].push_aside_frame = g_theGameLogic->Get_Frame();

            if ((m_trees[i].push_aside_obj_id == object && m_trees[i].push_aside_frame - frame < 3)
                || m_trees[i].push_aside_speed != 0.0f) {
                return;
            }

            m_trees[i].push_aside_obj_id = object;
            Coord3D c;
            c.Set(m_trees[i].location.X, m_trees[i].location.Y, m_trees[i].location.Z);
            c.Sub(pos);

            if (c.y * dir->x - c.x * dir->y <= 0.0f) {
                m_trees[i].push_aside_location.y = dir->y;
                m_trees[i].push_aside_location.x = -dir->x;
            } else {
                m_trees[i].push_aside_location.y = -dir->y;
                m_trees[i].push_aside_location.x = dir->x;
            }

            m_vertexesDirty = true;
            m_trees[i].push_aside_move_time = 1.0f / m_treeTypes[m_trees[i].tree_type].module->m_moveOutwardTime;
        }
    }
}

void W3DTreeBuffer::Update_Texture()
{
    int texture_width = 0;
    Ref_Ptr_Release(m_treeTexture);
    bool unk1[32][32];

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            unk1[i][j] = true;
        }
    }

    for (int m = 0; m < m_numTiles; m++) {
        Ref_Ptr_Release(m_tileData[m]);
    }

    m_numTiles = 0;
    File *file = nullptr;

    for (int m = 0; m < m_numTreeTypes; m++) {
        m_treeTypes[m].num_tiles = 0;
        char filename[PATH_MAX];
        sprintf(filename, "%s%s", "Art/Terrain/", m_treeTypes[m].module->m_textureName.Str());
        file = g_theFileSystem->Open_File(filename, File::BINARY | File::READ);

        if (file == nullptr) {
            sprintf(filename, "%s%s", "Art/Textures/", m_treeTypes[m].module->m_textureName.Str());
            file = g_theFileSystem->Open_File(filename, File::BINARY | File::READ);
        }

        if (file != nullptr) {
            GDIFileStream2 stream(file);
            bool valid;
            int count = WorldHeightMap::Count_Tiles(&stream, &valid);
            int j;

            for (j = 10; j >= 1; j--) {
                if (count >= j * j) {
                    count = j * j;
                    break;
                }
            }

            bool texture_found = false;

            for (int k = 0; k < m; k++) {
                if (m_treeTypes[k].module->m_textureName.Compare_No_Case(m_treeTypes[m].module->m_textureName) == 0) {
                    m_treeTypes[m].first_tile = 0;
                    m_treeTypes[m].width = j;
                    m_treeTypes[m].num_tiles = 0;
                    texture_found = true;
                    break;
                }
            }

            if (!texture_found) {
                if (count + m_numTiles > MAX_TILE_SIZE) {
                    m_treeTypes[m].first_tile = 0;
                    m_treeTypes[m].width = 0;
                    m_treeTypes[m].num_tiles = 0;
                } else {
                    file->Seek(0, File::START);
                    m_treeTypes[m].first_tile = m_numTiles;
                    m_treeTypes[m].width = j;
                    m_treeTypes[m].num_tiles = count;
                    m_treeTypes[m].valid_tile = valid;
                    WorldHeightMap::Read_Tiles(&stream, &m_tileData[m_treeTypes[m].first_tile], j);
                    m_numTiles += count;
                }
            }

            file->Close();
        } else {
            captainslog_dbgassert(false, "Could not find texture %s", m_treeTypes[m].module->m_textureName.Str());
            m_treeTypes[m].first_tile = 0;
            m_treeTypes[m].width = 0;
            m_treeTypes[m].num_tiles = 0;
        }
    }

    int ii;

    for (ii = 8; ii * ii < m_numTiles; ii *= 2) {
    }

    m_textureWidth = ii << 6;

    if (m_textureWidth > 2048) {
        m_textureWidth = 64;
        m_textureHeight = 64;

        if (m_treeTexture == nullptr) {
            m_treeTexture = static_cast<W3DTreeTextureClass *>(
                new TextureClass("missing.tga", nullptr, MIP_LEVELS_ALL, WW3D_FORMAT_UNKNOWN, true, true));
        }

        captainslog_dbgassert(false, "Too many trees in a scene.");
    } else {
        for (int m = 0; m < m_numTiles; m++) {
            if (m_tileData[m] != nullptr) {
                m_tileData[m]->m_tileLocationInTexture.x = -1;
                m_tileData[m]->m_tileLocationInTexture.y = -1;
            }
        }

        for (int jj = ii; jj > 0; jj--) {
            for (int kk = 0; kk < m_numTreeTypes; kk++) {
                int width = m_treeTypes[kk].width;

                if (width == jj) {
                    bool texture_found = false;
                    for (int m = 0; m < kk; m++) {
                        if (m_treeTypes[m].module->m_textureName.Compare_No_Case(m_treeTypes[kk].module->m_textureName)
                            == 0) {
                            m_treeTypes[kk].texture_width = m_treeTypes[m].texture_width;
                            m_treeTypes[kk].texture_height = m_treeTypes[m].texture_height;
                            texture_found = true;
                            break;
                        }
                    }

                    if (!texture_found) {
                        bool b1 = false;
                        int i;
                        int k;

                        for (i = 0; i < ii - width + 1; i++) {
                            for (k = 0; k < ii - width + 1; k++) {
                                if (unk1[i][k]) {
                                    bool b2 = true;

                                    for (int m = 0; m < width && b2; m++) {
                                        for (int n = 0; n < width && b2; n++) {
                                            if (!unk1[i + n][m + k]) {
                                                b2 = false;
                                            }
                                        }
                                    }

                                    if (b2) {
                                        b1 = true;
                                    }

                                    break;
                                }
                            }

                            if (b1) {
                                break;
                            }
                        }

                        if (!b1) {
                            m_treeTypes[kk].texture_width = 0;
                            m_treeTypes[kk].texture_height = 0;
                        } else {
                            int x = k << 6;
                            int y = i << 6;
                            m_treeTypes[kk].texture_width = x;
                            m_treeTypes[kk].texture_height = y;

                            if (texture_width < (width << 6) + y) {
                                texture_width = (width << 6) + y;
                            }

                            for (int m = 0; m < width; m++) {
                                for (int n = 0; n < width; n++) {
                                    unk1[i + n][m + k] = false;
                                    int index = width * n + m + m_treeTypes[kk].first_tile;
                                    m_tileData[index]->m_tileLocationInTexture.x = (m << 6) + x;
                                    m_tileData[index]->m_tileLocationInTexture.y = ((width - n - 1) << 6) + y;
                                }
                            }
                        }
                    }
                }
            }
        }

        captainslog_dbgassert(texture_width <= m_textureWidth, "Bad max height.");
        W3DTreeTextureClass *texture = new W3DTreeTextureClass(m_textureWidth, m_textureWidth);
        m_textureHeight = texture->Update(this);
        m_treeTexture = texture;

        for (int m = 0; m < m_numTiles; m++) {
            Ref_Ptr_Release(m_tileData[m]);
        }
    }
}

void W3DTreeBuffer::Update_Vertex_Buffer()
{
    if (m_indexTree[0] != nullptr && m_vertexTree[0] != nullptr && m_initialized) {
        for (int i = 0; i < 1 && m_curNumTreeIndices[i] != 0; i++) {
            VertexBufferClass::WriteLockClass lock(m_vertexTree[i], 0);
            VertexFormatXYZNDUV1 *vertexes = static_cast<VertexFormatXYZNDUV1 *>(lock.Get_Vertex_Array());

            for (int tree = 0; tree < m_numTrees; tree++) {
                if (m_trees[tree].vb_index == i) {
                    int tree_type = m_trees[tree].tree_type;

                    if (tree_type >= 0
                        && (m_trees[tree].push_aside_move_time != 0.0f
                            || m_trees[tree].topple_state != TTree::TOPPLE_UPRIGHT)) {
                        m_vertexesDirty = true;

                        if (m_trees[tree].visible) {
                            float scale = m_trees[tree].scale;
                            Vector3 location(m_trees[tree].location);
                            float sin = m_trees[tree].sin;
                            float cos = m_trees[tree].cos;

                            if (tree_type < 0 || m_treeTypes[tree_type].mesh == nullptr) {
                                tree_type = 0;
                            }

                            VertexFormatXYZNDUV1 *current_vertex = &vertexes[m_trees[tree].vertex_array_index];
                            int vert_count = m_treeTypes[tree_type].mesh->Peek_Model()->Get_Vertex_Count();
                            Vector3 *vert_array = m_treeTypes[tree_type].mesh->Peek_Model()->Get_Vertex_Array();

                            for (int k = 0; k < vert_count; k++) {
                                float vx = vert_array[k].X;
                                float vy = vert_array[k].Y;
                                Vector3 vertex;
                                vx = vx + m_treeTypes[tree_type].pos.X;
                                vy = vy + m_treeTypes[tree_type].pos.Y;
                                vertex.X = vx * scale * cos - vy * scale * sin;
                                vertex.Y = vy * scale * cos + vx * scale * sin;
                                vertex.Z = scale * vert_array[k].Z;
                                vertex.Z = vertex.Z + m_treeTypes[tree_type].pos.Z;

                                if (m_trees[tree].topple_state != TTree::TOPPLE_UPRIGHT) {
                                    Matrix3D::Transform_Vector(m_trees[tree].topple_transform, vertex, &vertex);
                                } else {
                                    if (m_trees[tree].push_aside_speed > 0.0f) {
                                        vertex.X = vert_array[k].Z * m_trees[tree].push_aside_speed
                                                * m_trees[tree].push_aside_location.y
                                                * m_treeTypes[tree_type].module->m_moveOutwardDistanceFactor
                                            + vertex.X;
                                        vertex.Y = vert_array[k].Z * m_trees[tree].push_aside_speed
                                                * m_trees[tree].push_aside_location.x
                                                * m_treeTypes[tree_type].module->m_moveOutwardDistanceFactor
                                            + vertex.Y;
                                    }

                                    vertex.X = vertex.X + location.X;
                                    vertex.Y = vertex.Y + location.Y;
                                    vertex.Z = vertex.Z + location.Z;
                                }

                                current_vertex->x = vertex.X;
                                current_vertex->y = vertex.Y;
                                current_vertex->z = vertex.Z;
                                current_vertex->ny = 1.0f
                                    - m_treeTypes[tree_type].module->m_darkeningFactor
                                        * m_trees[tree].push_aside_speed; // BUG m_darkeningFactor should probably be
                                                                          // m_moveOutwardDistanceFactor
                                current_vertex++;
                            }
                        }
                    }
                }
            }
        }
    }
}

void W3DTreeBuffer::Unit_Moved(Object *unit)
{
    if (!unit->Is_KindOf(KINDOF_IMMOBILE)) {
        float radius = unit->Get_Geometry_Info().Get_Major_Radius();

        if (unit->Get_Geometry_Info().Get_Type() == GEOMETRY_BOX) {
            if (unit->Get_Geometry_Info().Get_Minor_Radius() < radius) {
                radius = unit->Get_Geometry_Info().Get_Minor_Radius();
            }
        }

        radius = radius + 7.0f;
        Coord3D pos = *unit->Get_Position();
        float x = pos.x - radius;
        float y = pos.y - radius;

        if (x < m_partitionRegion.lo.x) {
            x = m_partitionRegion.lo.x;
        }

        if (y < m_partitionRegion.lo.y) {
            y = m_partitionRegion.lo.y;
        }

        if (x > m_partitionRegion.hi.x) {
            x = m_partitionRegion.hi.x;
        }

        if (y > m_partitionRegion.hi.y) {
            y = m_partitionRegion.hi.y;
        }

        int min_x_index = GameMath::Fast_To_Int_Floor(x / (m_partitionRegion.hi.x - m_partitionRegion.lo.x) * 99.9f);
        int min_y_index = GameMath::Fast_To_Int_Floor(y / (m_partitionRegion.hi.y - m_partitionRegion.lo.y) * 99.9f);
        captainslog_dbgassert(min_x_index >= 0 && min_y_index >= 0 && min_x_index < PARTITION_WIDTH_HEIGHT
                && min_y_index < PARTITION_WIDTH_HEIGHT,
            "Invalid range.");

        x = pos.x + radius;
        y = pos.y + radius;

        if (x < m_partitionRegion.lo.x) {
            x = m_partitionRegion.lo.x;
        }

        if (y < m_partitionRegion.lo.y) {
            y = m_partitionRegion.lo.y;
        }

        if (x > m_partitionRegion.hi.x) {
            x = m_partitionRegion.hi.x;
        }

        if (y > m_partitionRegion.hi.y) {
            y = m_partitionRegion.hi.y;
        }

        int max_x_index = GameMath::Fast_To_Int_Ceil(x / (m_partitionRegion.hi.x - m_partitionRegion.lo.x) * 99.9f);
        int max_y_index = GameMath::Fast_To_Int_Ceil(y / (m_partitionRegion.hi.y - m_partitionRegion.lo.y) * 99.9f);
        captainslog_dbgassert(max_x_index >= 0 && max_y_index >= 0 && max_x_index <= PARTITION_WIDTH_HEIGHT
                && max_y_index <= PARTITION_WIDTH_HEIGHT,
            "Invalid range.");

        for (int px = min_x_index; px < max_x_index; px++) {
            for (int py = min_y_index; py < max_y_index; py++) {
                int index = m_partitionIndices[100 * py + px];

                while (index != -1) {
                    if (index < 0 || index >= m_numTrees) {
                        captainslog_dbgassert(false, "Invalid index.");
                        break;
                    }

                    if (m_trees[index].tree_type < 0) {
                        index = m_trees[index].partition_bucket_index;
                    } else {
                        Coord3D loc;
                        loc.Set(m_trees[index].location.X, m_trees[index].location.Y, m_trees[index].location.Z);
                        loc.Sub(&pos);

                        if (loc.Length2() < radius * radius) {
                            if (unit->Get_Crusher_Level() > 1 && m_treeTypes[m_trees[index].tree_type].module->m_doTopple) {
                                Coord3D topple_dir;
                                topple_dir.Set(m_trees[index].location.X, m_trees[index].location.Y, 0.0f);
                                topple_dir.x = topple_dir.x - unit->Get_Position()->x;
                                topple_dir.y = topple_dir.y - unit->Get_Position()->y;
                                Apply_Toppling_Force(&m_trees[index], &topple_dir, 0.0f, 0);
                            } else if (m_treeTypes[m_trees[index].tree_type].module->m_moveOutwardTime > 1) {
                                Push_Aside_Tree(
                                    m_trees[index].drawable_id, &pos, unit->Get_Unit_Dir_Vector2D(), unit->Get_ID());
                            }
                        }

                        index = m_trees[index].partition_bucket_index;
                    }
                }
            }
        }
    }
}

void W3DTreeBuffer::Draw_Trees(CameraClass *camera, RefMultiListIterator<class RenderObjClass> *pDynamicLightsIterator)
{
    if (m_isTerrainPass) {
        BreezeInfo *breeze = g_theScriptEngine->Get_Breeze_Info();
        bool frozen = g_theScriptEngine->Is_Time_Frozen_Script() || g_theScriptEngine->Is_Time_Frozen_Debug();

        if (g_theGameLogic && g_theGameLogic->Is_Game_Paused()) {
            frozen = true;
        }

        if (!frozen && breeze->version != m_swayVersion) {
            Update_Sway(*breeze);
        }

        Vector3 sway_vector[10];

        for (int i = 0; i < 10; i++) {
            if (!frozen) {
                m_swayCurrent[i] = m_swayCurrent[i] + m_swayPeriods[i];
                if (m_swayCurrent[i] > 99.0f) {
                    m_swayCurrent[i] = m_swayCurrent[i] - 99.0f;
                }
            }

            int sway_current = GameMath::Fast_To_Int_Floor(m_swayCurrent[i]);

            if (sway_current >= 0 && sway_current + 1 < 100) {
                float f1 = m_swayCurrent[i] - sway_current;
                float f2 = 1.0f - f1;
                sway_vector[i] = f2 * m_swayVector[sway_current] + f1 * m_swayVector[sway_current + 1];
                sway_vector[i] *= m_swayLeanAngles[i];
            }
        }

        m_isTerrainPass = false;

        if (m_textureDirty) {
            m_textureDirty = false;
            Update_Texture();
        }

        if (m_treeTexture != nullptr) {
            if (m_updateAllKeys) {
                Cull(camera);
            }

            if (m_decalShadow != nullptr && g_theW3DProjectedShadowManager != nullptr
                && g_theWriteableGlobalData->m_shadowDecals) {
                for (int j = 0; j < m_numTrees; j++) {
                    int tree_type = m_trees[j].tree_type;

                    if (tree_type >= 0) {
                        if (m_trees[j].visible) {
                            if (m_treeTypes[tree_type].do_shadow) {
                                if (m_trees[j].topple_state != TTree::TOPPLE_WAITING_FOR_START
                                    && m_trees[j].topple_state != TTree::TOPPLE_DOWN) {
                                    m_decalShadow->Set_Size(
                                        m_treeTypes[tree_type].uv_offset, -m_treeTypes[tree_type].uv_offset * 1.0f);
                                    m_decalShadow->Set_Position(
                                        m_trees[j].location.X, m_trees[j].location.Y, m_trees[j].location.Z);
                                    g_theW3DProjectedShadowManager->Queue_Decal(m_decalShadow);
                                }
                            }
                        }
                    }
                }

                g_theW3DProjectedShadowManager->Flush_Decals(m_decalShadow->Get_Texture(0), SHADOW_DECAL);
            }

            for (int j = 0; j < m_numTrees && !frozen; j++) {
                int tree_type = m_trees[j].tree_type;

                if (tree_type >= 0) {
                    if (m_trees[j].topple_state == TTree::TOPPLE_WAITING_FOR_START
                        || m_trees[j].topple_state == TTree::TOPPLE_TOPPLING) {
                        Update_Toppling_Tree(&m_trees[j]);
                    } else {
                        if (m_trees[j].topple_state == TTree::TOPPLE_DOWN) {
                            if (m_treeTypes[tree_type].module->m_killWhenFinishedToppling) {
                                if (m_trees[j].topple_sink_countdown == 0) {
                                    m_trees[j].tree_type = -2;
                                    m_anythingChanged = true;
                                }

                                m_trees[j].topple_sink_countdown--;
                                m_trees[j].location.Z = m_trees[j].location.Z
                                    - m_treeTypes[tree_type].module->m_sinkDistance
                                        / m_treeTypes[tree_type].module->m_sinkTime;
                                m_trees[j].topple_transform.Set_Translation(m_trees[j].location);
                            }
                        } else if (m_trees[j].push_aside_move_time != 0.0f) {
                            m_trees[j].push_aside_speed = m_trees[j].push_aside_speed + m_trees[j].push_aside_move_time;

                            if (m_trees[j].push_aside_speed >= 1.0f) {
                                m_trees[j].push_aside_move_time = -1.0f / m_treeTypes[tree_type].module->m_moveInwardTime;
                            } else if (m_trees[j].push_aside_speed <= 0.0f) {
                                m_trees[j].push_aside_move_time = 0.0f;
                                m_trees[j].push_aside_speed = 0.0f;
                            }
                        }
                    }
                }
            }

            if (m_anythingChanged) {
                Load_Trees_In_Index_And_Vertex_Buffers(pDynamicLightsIterator);
                m_anythingChanged = false;
            } else if (m_vertexesDirty) {
                m_vertexesDirty = false;
                Update_Vertex_Buffer();
            }

            if (m_curNumTreeIndices[0] != 0) {
#ifdef BUILD_WITH_D3D8
                DX8Wrapper::Set_Shader(s_detailAlphaShader2);
                DX8Wrapper::Set_Texture(0, m_treeTexture);
                DX8Wrapper::Set_Texture(1, nullptr);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
                DX8Wrapper::Apply_Render_State_Changes();
                W3DShaderManager::Set_Shroud_Tex(1);
                DX8Wrapper::Apply_Render_State_Changes();

                if (m_dwTreeVertexShader != 0) {
                    D3DXMATRIX projection;
                    D3DXMATRIX view;
                    D3DXMATRIX world;
                    DX8Wrapper::Get_DX8_Transform(D3DTS_WORLD, world);
                    DX8Wrapper::Get_DX8_Transform(D3DTS_VIEW, view);
                    DX8Wrapper::Get_DX8_Transform(D3DTS_PROJECTION, projection);

                    D3DXMATRIX wvp;
                    D3DXMatrixMultiply(&wvp, &view, &projection);
                    D3DXMatrixMultiply(&wvp, &world, &wvp);
                    D3DXMatrixTranspose(&wvp, &wvp);
                    DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(4, static_cast<const void *>(&wvp), 4);
                    Vector4 v(0.0f, 0.0f, 0.0f, 0.0f);
                    DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(8, static_cast<const void *>(&v), 1);

                    for (int i = 0; i < 10; i++) {
                        Vector4 sway(sway_vector[i].X, sway_vector[i].Y, sway_vector[i].Z, 0.0f);
                        DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(i + 9, static_cast<const void *>(&sway), 1);
                    }

                    W3DShroud *shroud = g_theTerrainRenderObject->Get_Shroud();

                    if (shroud != nullptr) {
                        float x = 0.0f;
                        float y = 0.0f;
                        float width = shroud->Get_Cell_Width();
                        float height = shroud->Get_Cell_Height();
                        x = width - shroud->Get_Draw_Origin_X();
                        y = height - shroud->Get_Draw_Origin_Y();
                        Vector4 shroud_vec(x, y, 0.0f, 0.0f);
                        DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(
                            32, static_cast<const void *>(&shroud_vec), 1);

                        width = 1.0f / (shroud->Get_Texture_Width() * width);
                        height = 1.0f / (shroud->Get_Texture_Height() * height);
                        shroud_vec.Set(width, height, 1.0f, 1.0f);
                        DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(
                            33, static_cast<const void *>(&shroud_vec), 1);
                    } else {
                        Vector4 shroud_vec(0.0f, 0.0f, 0.0f, 0.0f);
                        DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(
                            32, static_cast<const void *>(&shroud_vec), 1);
                        DX8Wrapper::Get_D3D_Device8()->SetVertexShaderConstant(
                            33, static_cast<const void *>(&shroud_vec), 1);
                    }

                    DX8Wrapper::Set_Vertex_Shader(m_dwTreeVertexShader);
                } else {
                    DX8Wrapper::Set_Vertex_Shader(DX8_FVF_XYZNDUV1);
                }

                for (int k = 0; k < 1 && m_curNumTreeIndices[k] != 0; k++) {
                    DX8Wrapper::Set_Index_Buffer(m_indexTree[k], 0);
                    DX8Wrapper::Set_Vertex_Buffer(m_vertexTree[k], 0);
                    DX8Wrapper::Apply_Render_State_Changes();

                    if (m_dwTreeVertexShader != 0) {
                        DX8Wrapper::Get_D3D_Device8()->SetVertexShader(m_dwTreeVertexShader);
                        DX8Wrapper::Get_D3D_Device8()->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
                        DX8Wrapper::Get_D3D_Device8()->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
                        DX8Wrapper::Get_D3D_Device8()->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT1);
                    }

                    DX8Wrapper::Draw_Triangles(0, m_curNumTreeIndices[k] / 3, 0, m_curNumTreeVertices[k]);
                }

                DX8Wrapper::Set_Vertex_Shader(DX8_FVF_XYZNDUV1);
                DX8Wrapper::Set_Pixel_Shader(0);
                DX8Wrapper::Invalidate_Cached_Render_States();
#endif
            }
        }
    }
}
