/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Asset Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dassetmanager.h"
#include "boxrobj.h"
#include "chunkio.h"
#include "colorspace.h"
#include "dazzle.h"
#include "dx8renderer.h"
#include "ffactory.h"
#include "hanim.h"
#include "hlod.h"
#include "loaders.h"
#include "matinfo.h"
#include "mesh.h"
#include "meshmdl.h"
#include "part_ldr.h"
#include "proto.h"
#include "realcrc.h"
#include "render2dsentence.h"
#include "string.h"
#include "textureloader.h"
#include "vertmaterial.h"
#include "w3d_file.h"
#include "w3dexclusionlist.h"
#include "wwfile.h"
#include <cmath>
#include <stdio.h>

GameAssetManager::~GameAssetManager() {}

// 0x00765CD0
bool GameAssetManager::Load_3D_Assets(const char *filename)
{
    // file_base will be the filename up until the first '.' i.e. TANK.GUN would be TANK
    char file_base[512];
    strncpy(file_base, filename, sizeof(file_base));
    file_base[sizeof(file_base) - 1] = '\0';
    auto *period = strrchr(file_base, '.');
    if (period != nullptr) {
        *period = '\0';
    }

    if (Find_Prototype(file_base) != nullptr) {
        return true;
    }

    return W3DAssetManager::Load_3D_Assets(filename);
}

// 0x00763E10
RenderObjClass *GameAssetManager::Create_Render_Obj(const char *name)
{
    return W3DAssetManager::Create_Render_Obj(name);
}

// 0x00765D60
HAnimClass *GameAssetManager::Get_HAnim(const char *name)
{
    return W3DAssetManager::Get_HAnim(name);
}

// 0x00763DC0
TextureClass *GameAssetManager::Get_Texture(const char *filename,
    MipCountType mip_level_count,
    WW3DFormat texture_format,
    bool allow_compression,
    TexAssetType asset_type,
    bool allow_reduction)
{
    if (filename != nullptr) {
        if (*filename != '\0') {
            if (strncasecmp(filename, "ZHC", 3) == 0) {
                allow_reduction = false;
            }
        }
    }
    return W3DAssetManager::Get_Texture(
        filename, mip_level_count, texture_format, allow_compression, asset_type, allow_reduction);
}

static void Create_Mangled_Name(char *mangled_name, const char *name, float scale, uint32_t colour, const char *new_texture)
{
    char new_name[256]{};
    strcpy(new_name, name);
    strlwr(new_name);
    const char *tex = "";
    if (new_texture != nullptr) {
        tex = new_texture;
    }

    snprintf(mangled_name, 512, "#%d!%g!%s#%s", colour, scale, tex, new_name);
}

// 0x00765180
RenderObjClass *GameAssetManager::Create_Render_Obj(
    const char *name, float scale, uint32_t colour, const char *old_texture, const char *new_texture)
{
    bool has_scaling = GameMath::Fabs(scale - 1.0f) >= 0.001f;
    bool has_colour = (colour & 0xFFFFFF) != 0;
    bool has_texture = old_texture != nullptr && new_texture != nullptr;

    if (!has_scaling && !has_colour && !has_texture) {
        return W3DAssetManager::Create_Render_Obj(name);
    }

    char mangled_name[512]{};
    Create_Mangled_Name(mangled_name, name, scale, colour, new_texture);

    Set_W3D_Load_On_Demand(false);
    auto *robj = W3DAssetManager::Create_Render_Obj(mangled_name);
    if (robj != nullptr) {
        robj->Set_House_Color(colour);
        Set_W3D_Load_On_Demand(true);
        return robj;
    }

    auto *proto = Find_Prototype(name);
    Set_W3D_Load_On_Demand(true);
    if (m_loadOnDemand == true && proto == nullptr) {
        char asset_filename[256]{};
        auto *period = strchr(name, '.');
        if (period == nullptr) {
            snprintf(asset_filename, 256, "%s.w3d", name);
        } else {
            snprintf(asset_filename, 256, "%s.w3d", period + 1);
        }
        if (Load_3D_Assets(asset_filename) == false) {
            StringClass new_filename = StringClass{ "..\\", true } + asset_filename;
            Load_3D_Assets(new_filename);
        }
        proto = Find_Prototype(name);
    }

    if (proto == nullptr) {
        captainslog_debug("Warning: Failed to create Render Object: %s\r\n", name);
        return nullptr;
    }

    robj = proto->Create();
    if (robj == nullptr) {
        return nullptr;
    }

    if (has_scaling) {
        robj->Scale(scale);
    }

    Make_Unique(robj, has_scaling, has_colour);

    if (has_texture) {
        auto *old_tex = Get_Texture(old_texture);
        auto *new_tex = Get_Texture(new_texture);

        Replace_Asset_Texture(robj, old_tex, new_tex);

        if (old_tex != nullptr) {
            old_tex->Release_Ref();
        }
        if (new_tex != nullptr) {
            new_tex->Release_Ref();
        }
    }
    if (has_colour) {
        Recolour_Asset(robj, colour);
    }

    auto *w3d_proto = NEW_POOL_OBJ(W3DPrototypeClass, robj, mangled_name);
    robj->Release_Ref();
    Add_Prototype(w3d_proto);
    robj = w3d_proto->Create();
    robj->Set_House_Color(colour);
    return robj;
}

bool Get_Mesh_Color_Methods(MeshClass *mesh, bool *housecolor, bool *zhc)
{
    *housecolor = false;
    *zhc = false;
    MaterialInfoClass *matinfo = mesh->Get_Material_Info();

    if (matinfo != nullptr) {
        for (int i = 0; i < matinfo->Texture_Count(); i++) {

            if (!strncasecmp(matinfo->Peek_Texture(i)->Get_Name(), "ZHC", 3)) {
                *zhc = true;
                break;
            }
        }

        matinfo->Release_Ref();
    }

    captainslog_assert(mesh->Get_Name() != nullptr);
    const char *word = strchr(mesh->Get_Name(), '.');
    word = (word != nullptr) ? word + 1 : mesh->Get_Name();

    if (!strncasecmp(word, "HOUSECOLOR", 10)) {
        *housecolor = true;
    }

    return *housecolor || *zhc;
}

// 0x00765DF0
void GameAssetManager::Make_Mesh_Unique(RenderObjClass *robj, bool geometry, bool colors)
{
    MeshClass *mesh = static_cast<MeshClass *>(robj);
    bool housecolor;
    bool zhc;

    if ((colors && Get_Mesh_Color_Methods(mesh, &housecolor, &zhc)) || geometry) {
        if (!geometry) {
            mesh->Make_Unique(false);
        }

        MeshModelClass *model = mesh->Get_Model();

        if (colors && housecolor) {
            MaterialInfoClass *matinfo = mesh->Get_Material_Info();

            for (int i = 0; i < matinfo->Vertex_Material_Count(); i++) {
                matinfo->Peek_Vertex_Material(i)->Make_Unique();
            }

            Ref_Ptr_Release(matinfo);
        }

        Ref_Ptr_Release(model);
    }
}

void GameAssetManager::Make_HLOD_Unique(RenderObjClass *robj, bool geometry, bool colors)
{
    for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
        auto *sub_robj = robj->Get_Sub_Object(i);
        if (sub_robj != nullptr) {
            // #BUGFIX Guard pointer by check always.
            Make_Unique(sub_robj, geometry, colors);
            sub_robj->Release_Ref();
        }
    }
}

// 0x00765D70
void GameAssetManager::Make_Unique(RenderObjClass *robj, bool geometry, bool colors)
{
    switch (robj->Class_ID()) {
        case RenderObjClass::CLASSID_MESH:
            Make_Mesh_Unique(robj, geometry, colors);
            break;
        case RenderObjClass::CLASSID_HLOD:
            Make_HLOD_Unique(robj, geometry, colors);
            break;

        default:
            break;
    }
}

// 0x00765B27
void GameAssetManager::Recolor_Vertex_Material(VertexMaterialClass *vmat, uint32_t colour)
{
    Vector3 rgb_color;
    Color_To_RGB(rgb_color, colour);
    vmat->Set_Ambient(rgb_color);
    vmat->Set_Diffuse(rgb_color);
}

// 0x00765A40
bool GameAssetManager::Recolor_Mesh(RenderObjClass *robj, uint32_t colour)
{
    bool recolored = false;
    MeshClass *mesh = static_cast<MeshClass *>(robj);
    MeshModelClass *model = mesh->Get_Model();
    MaterialInfoClass *matinfo = mesh->Get_Material_Info();

    captainslog_assert(mesh->Get_Name() != nullptr);
    const char *word = strchr(mesh->Get_Name(), '.');
    word = (word != nullptr) ? word + 1 : mesh->Get_Name();

    if (!strncasecmp(word, "HOUSECOLOR", 10)) {
        for (int i = 0; i < matinfo->Vertex_Material_Count(); i++) {
            Recolor_Vertex_Material(matinfo->Peek_Vertex_Material(i), colour);
        }

        recolored = true;
    }

    for (int i = 0; i < matinfo->Texture_Count(); i++) {
        TextureClass *texture = matinfo->Peek_Texture(i);
        if (!strncasecmp(texture->Get_Name(), "ZHC", 3)) {
            TextureClass *new_texture = Recolor_Texture(texture, colour);

            if (new_texture != nullptr) {
                model->Replace_Texture(texture, new_texture);
                matinfo->Replace_Texture(i, new_texture);
                new_texture->Release_Ref();
                recolored = true;
            }
        }
    }

    Ref_Ptr_Release(matinfo);
    Ref_Ptr_Release(model);
    return recolored;
}

// 0x007659B0
bool GameAssetManager::Recolour_Asset(RenderObjClass *robj, uint32_t colour)
{
    switch (robj->Class_ID()) {
        case RenderObjClass::CLASSID_MESH:
            return Recolor_Mesh(robj, colour);
        case RenderObjClass::CLASSID_HLOD:
            return Recolor_HLOD(robj, colour);
        default:
            return false;
    }
}

TextureClass *GameAssetManager::Recolor_Texture(TextureClass *texture, int color)
{
    TextureClass *new_texture = Find_Texture(texture->Get_Name(), color);

    if (new_texture != nullptr) {
        return new_texture;
    } else {
        return Recolor_Texture_One_Time(texture, color);
    }
}

void Create_Color_Texture_Name(char *buffer, const char *name, int color)
{
    char new_name[256];
    strcpy(new_name, name);
    strlwr(new_name);
    sprintf(buffer, "#%d#%s", color, new_name);
}

TextureClass *GameAssetManager::Find_Texture(const char *name, int color)
{
    char buffer[512];
    Create_Color_Texture_Name(buffer, name, color);
    TextureClass *texture = m_textureHash.Get(buffer);

    if (texture) {
        texture->Add_Ref();
    }

    return texture;
}

TextureClass *GameAssetManager::Recolor_Texture_One_Time(TextureClass *texture, int color)
{
    const char *name = texture->Get_Name();

    // #BUGFIX Return if name is null too
    if (name == nullptr || name[0] == '!') {
        return nullptr;
    }

    if (!texture->Is_Initialized()) {
        TextureLoader::Request_Foreground_Loading(texture);
    }

    SurfaceClass::SurfaceDescription desc;
    texture->Get_Level_Description(desc, 0);
    captainslog_dbgassert(
        SurfaceClass::Pixel_Size(desc) == 2 || SurfaceClass::Pixel_Size(desc) == 4, "Can't Recolor Texture %s", name);
    SurfaceClass *surface = texture->Get_Surface_Level(0);
    SurfaceClass *new_surface = new SurfaceClass(desc.width, desc.height, desc.format);
    new_surface->Copy(0, 0, 0, 0, desc.width, desc.height, surface);

    if (name[3] == 'D' || name[3] == 'd') {
        // Do Palette Only
        Remap_Palette(new_surface, color, true, false);
    } else if (name[3] == 'A' || name[3] == 'a') {
        // Use Alpha
        Remap_Palette(new_surface, color, false, true);
    }

    TextureClass *new_texture = new TextureClass(new_surface, texture->Get_Mip_Level_Count());
    new_texture->Get_Texture_Filter()->Set_Mag_Filter(texture->Get_Texture_Filter()->Get_Mag_Filter());
    new_texture->Get_Texture_Filter()->Set_Min_Filter(texture->Get_Texture_Filter()->Get_Min_Filter());
    new_texture->Get_Texture_Filter()->Set_Mip_Mapping(texture->Get_Texture_Filter()->Get_Mip_Mapping());
    new_texture->Get_Texture_Filter()->Set_U_Address_Mode(texture->Get_Texture_Filter()->Get_U_Address_Mode());
    new_texture->Get_Texture_Filter()->Set_V_Address_Mode(texture->Get_Texture_Filter()->Get_V_Address_Mode());

    char buffer[512];
    Create_Color_Texture_Name(buffer, name, color);
    new_texture->Set_Texture_Name(buffer);
    captainslog_dbgassert(
        !m_textureHash.Exists(new_texture->Get_Name()), "Texture hash collision occurred"); // Thyme specific
    m_textureHash.Insert(new_texture->Get_Name(), new_texture);
    new_texture->Add_Ref();
    Ref_Ptr_Release(surface);
    Ref_Ptr_Release(new_surface);
    return new_texture;
}

void GameAssetManager::Remap_Palette(SurfaceClass *surface, int color, bool do_palette_only, bool use_alpha)
{
#ifdef GAME_DLL
    Call_Method<void, GameAssetManager, SurfaceClass *, int, bool, bool>(
        PICK_ADDRESS(0x007642F0, 0x0061675A), this, surface, color, do_palette_only, use_alpha);
#endif
}

bool GameAssetManager::Recolor_HLOD(RenderObjClass *robj, uint32_t colour)
{
    bool recolored = false;
    for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
        auto *sub_robj = robj->Get_Sub_Object(i);
        if (sub_robj != nullptr) {
            // #BUGFIX Guard pointer by check always.
            recolored |= Recolour_Asset(sub_robj, colour);
            sub_robj->Release_Ref();
        }
    }
    return recolored;
}

bool GameAssetManager::Replace_Asset_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture)
{
    switch (robj->Class_ID()) {
        case RenderObjClass::CLASSID_MESH:
            return Replace_Mesh_Texture(robj, old_texture, new_texture);
        case RenderObjClass::CLASSID_HLOD:
            return Replace_HLOD_Texture(robj, old_texture, new_texture);
        default:
            return false;
    }
}

bool GameAssetManager::Replace_Prototype_Texture(RenderObjClass *robj, const char *old_name, const char *new_name)
{
    TextureClass *old_texture = Get_Texture(old_name);
    TextureClass *new_texture = Get_Texture(new_name);
    bool ret = Replace_Asset_Texture(robj, old_texture, new_texture);

    if (old_texture != nullptr) {
        old_texture->Release_Ref();
    }

    if (new_texture != nullptr) {
        new_texture->Release_Ref();
    }

    return ret;
}

// 0x00763EC0
bool GameAssetManager::Replace_HLOD_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture)
{
    bool replaced = false;
    for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
        auto *sub_robj = robj->Get_Sub_Object(i);
        if (sub_robj != nullptr) {
            // #BUGFIX Guard pointer by check always.
            replaced |= Replace_Asset_Texture(sub_robj, old_texture, new_texture);
            sub_robj->Release_Ref();
        }
    }
    return replaced;
}

// 0x00763F70
bool GameAssetManager::Replace_Mesh_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture)
{
    bool replaced = false;
    MeshClass *mesh = static_cast<MeshClass *>(robj);
    MeshModelClass *model = mesh->Get_Model();
    MaterialInfoClass *matinfo = mesh->Get_Material_Info();

    for (int i = 0; i < matinfo->Texture_Count(); i++) {
        if (matinfo->Peek_Texture(i) == old_texture) {
            model->Replace_Texture(old_texture, new_texture);
            matinfo->Replace_Texture(i, new_texture);
            replaced = true;
        }
    }

    Ref_Ptr_Release(matinfo);
    Ref_Ptr_Release(model);
    return replaced;
}

W3DPrototypeClass::W3DPrototypeClass(RenderObjClass *proto, const char *name) : m_proto(proto), m_name(name)
{
    m_proto->Add_Ref();
}

int32_t W3DPrototypeClass::Get_Class_ID() const
{
    return m_proto->Class_ID();
}

RenderObjClass *W3DPrototypeClass::Create()
{
    return m_proto->Clone();
}

W3DPrototypeClass::~W3DPrototypeClass()
{
    m_proto->Release_Ref();
    m_proto = nullptr;
}
