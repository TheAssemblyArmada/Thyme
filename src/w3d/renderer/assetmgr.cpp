/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shader Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "assetmgr.h"
#include "chunkio.h"
#include "crc.h"
#include "ffactory.h"
#include "fileclass.h"
#include "mesh.h"
#include "proto.h"
#include "render2dsentence.h"
#include "string.h"
#include "w3d_file.h"
#include <cmath>
#include <stdio.h>

#ifndef GAME_DLL
W3DAssetManager * ::W3DAssetManager::s_theInstance;
NullPrototypeClass s_nullPrototype;
#endif

// 0x00814090
W3DAssetManager::W3DAssetManager()
{
#ifdef GAME_DLL
    Call_Method<void, W3DAssetManager>(0x00814090, this);
#endif
}

// 0x008143C0
W3DAssetManager::~W3DAssetManager()
{
    Free_Assets();
    s_theInstance = nullptr;
    delete[] m_prototypeHashTable;
}

// 0x00814C30
bool W3DAssetManager::Load_3D_Assets(const char *filename)
{
    auto file = g_theFileFactory->Get_File(filename);
    if (file == nullptr) {
        return false;
    }

    if (file->Is_Available() == false) {
        g_theFileFactory->Return_File(file);
        return false;
    }

    Load_3D_Assets(*file);
    g_theFileFactory->Return_File(file);
    return true;
}

// 0x00814E10
bool W3DAssetManager::Load_3D_Assets(FileClass &asset_file)
{
    if (asset_file.Open() == false) {
        return false;
    }

    for (ChunkLoadClass chunk(&asset_file); chunk.Open_Chunk(); chunk.Close_Chunk()) {
        auto chunk_id = chunk.Cur_Chunk_ID();
        switch (chunk_id) {
            case W3D_CHUNK_COMPRESSED_ANIMATION:
            case W3D_CHUNK_ANIMATION:
            case W3D_CHUNK_MORPH_ANIMATION:
                m_hAnimManager.Load_Anim(chunk);
                break;
            case W3D_CHUNK_HIERARCHY:
                m_hTreeManager.Load_Tree(chunk);
                break;
            default: {
                for (auto i = 0; i < m_prototypeLoaders.Count(); ++i) {
                    auto *loader = m_prototypeLoaders[i];

                    if (loader == nullptr) {
                        continue;
                    }

                    if (loader->Chunk_Type() != chunk_id) {
                        continue;
                    }

                    auto *prototype = loader->Load_W3D(chunk);
                    if (prototype == nullptr) {
                        continue;
                    }

                    const auto *name = prototype->Get_Name();
                    if (Render_Obj_Exists(name)) {
                        prototype->Delete_Self();
                    } else {
                        Prototype_Hash_Table_Add(prototype);
                        m_prototypes.Add(prototype);
                    }
                }
                break;
            }
        }
    }

    asset_file.Close();
    return true;
}

// 0x008147C0
void W3DAssetManager::Free_Assets()
{
    for (auto i = m_prototypes.Count() - 1; i >= 0; --i) {
        auto *prototype = m_prototypes[i];
        m_prototypes.Delete(i);
        if (prototype != nullptr) {
            prototype->Delete_Self();
        }
    }
    memset(m_prototypeHashTable, 0, s_prototypeHashTableSize * sizeof(uintptr_t));
    m_hAnimManager.Free_All_Anims();
    m_hTreeManager.Free_All_Trees();
    Release_All_Textures();
    Release_All_FontChars();
}

// 0x00814870
void W3DAssetManager::Free_Assets_With_Exclusion_List(DynamicVectorClass<StringClass> const &list)
{
#ifdef GAME_DLL
    Call_Method<void, W3DAssetManager, DynamicVectorClass<StringClass> const &>(0x00814870, this, list);
#endif
}

// 0x00814A60
void W3DAssetManager::Create_Asset_List(DynamicVectorClass<StringClass> &list)
{
    captainslog_dbgassert(false, "Create_Asset_List is not used");
    // Not used
    for (auto i = 0; i < m_prototypes.Count(); ++i) {
        auto *proto = m_prototypes[i];
        if (proto == nullptr) {
            continue;
        }

        const auto *name = proto->Get_Name();
        if (strchr(name, '#') != nullptr) {
            continue;
        }
        const auto *period = strchr(name, '.');
        if (period == nullptr) {
            continue;
        }
        list.Add(name);
    }
    m_hAnimManager.Create_Asset_List(list);
}

// 0x00814FC0
RenderObjClass *W3DAssetManager::Create_Render_Obj(const char *name)
{
    auto *proto = Find_Prototype(name);

    if (m_loadOnDemand && proto == nullptr) {
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
        return nullptr;
    }
    return proto->Create();
}

// 0x008152C0
bool W3DAssetManager::Render_Obj_Exists(const char *name)
{
    if (strcasecmp(name, "NULL") == 0) {
        return true;
    }

    auto proto = Prototype_Hash_Table_Find(name);
    return proto != nullptr;
}

// 0x00815340
RenderObjIterator *W3DAssetManager::Create_Render_Obj_Iterator()
{
    captainslog_dbgassert(false, "RenderObjIterator class is not used");
    return nullptr;
}

// 0x00815370
void W3DAssetManager::Release_Render_Obj_Iterator(RenderObjIterator *it)
{
    captainslog_dbgassert(false, "RenderObjIterator class is not used");
}

// 0x00815390
AssetIterator *W3DAssetManager::Create_HAnim_Iterator(void)
{
    captainslog_dbgassert(false, "HAnim Iterator class is not used");
    return nullptr;
}

// 0x008154F0
HAnimClass *W3DAssetManager::Get_HAnim(const char *name)
{
    auto *anim = m_hAnimManager.Get_Anim(name);
    if (m_loadOnDemand == false || anim != nullptr) {
        return anim;
    }

    if (m_hAnimManager.Is_Missing(name)) {
        return anim;
    }

    const char *asset = strchr(name, '.');
    if (asset == nullptr) {
        return nullptr;
    }
    // Skip over the seperator
    asset++;

    char asset_filename[256]{};
    snprintf(asset_filename, 256, "%s.w3d", asset);

    if (Load_3D_Assets(asset_filename) == false) {
        StringClass new_filename = StringClass{ "..\\", true } + asset_filename;
        Load_3D_Assets(new_filename);
    }
    anim = m_hAnimManager.Get_Anim(name);
    if (anim == nullptr) {
        m_hAnimManager.Register_Missing(name);
    }

    return anim;
}

// 0x00765FF0
bool W3DAssetManager::Add_Anim(HAnimClass *new_anim)
{
    // TODO: Not used?
    return m_hAnimManager.Add_Anim(new_anim);
}

// 0x00815920
TextureClass *W3DAssetManager::Get_Texture(const char *filename,
    MipCountType mip_level_count,
    WW3DFormat texture_format,
    bool allow_compression,
    TexAssetType asset_type,
    bool allow_reduction)
{
    if (texture_format == WW3DFormat::WW3D_FORMAT_U8V8) {
        mip_level_count = MipCountType::MIP_LEVELS_1;
    }

    if (filename == nullptr || strlen(filename) == 0) {
        return nullptr;
    }

    StringClass name = { filename };
    name.To_Lower();
    auto *texture = m_textureHash.Get(name);
    if (texture != nullptr) {
        (*texture)->Add_Ref();
        return *texture;
    }

    TextureClass *new_texture = nullptr;
    switch (asset_type) {
        case TexAssetType::ASSET_STANDARD:
            new_texture =
                new TextureClass{ name, nullptr, mip_level_count, texture_format, allow_compression, allow_reduction };
            break;
        case TexAssetType::ASSET_CUBE:
            captainslog_dbgassert(false, "CubeTextureClass is not used");
            break;
        case TexAssetType::ASSET_VOLUME:
            captainslog_dbgassert(false, "VolumeTextureClass is not used");
            break;
        default:
            break;
    }
    if (new_texture == nullptr) {
        return nullptr;
    }
    m_textureHash.Insert(new_texture->Get_Name(), new_texture);

    new_texture->Add_Ref();
    return new_texture;
}

// 0x00815C90
void W3DAssetManager::Release_All_Textures()
{
    for (HashTemplateIterator<StringClass, TextureClass *> texture(m_textureHash); texture; texture.Reset()) {
        texture.getValue()->Release_Ref();
        texture.Remove();
    }
    m_textureHash.Remove_All();
}

// 0x00815D90
void W3DAssetManager::Release_Unused_Textures()
{
    constexpr auto unused_textures_size = 256;
    TextureClass *unused_textures[unused_textures_size]{};
    auto unused_textures_count = 0;
    for (HashTemplateIterator<StringClass, TextureClass *> texture(m_textureHash); texture; ++texture) {
        if (texture.getValue()->Num_Refs() != 1) {
            continue;
        }
        unused_textures[unused_textures_count++] = texture.getValue();

        if (unused_textures_count < unused_textures_size) {
            continue;
        }

        for (auto i = 0; i < unused_textures_count; ++i) {
            auto *unused_texture = unused_textures[i];
            m_textureHash.Remove(unused_texture->Get_Name());
            unused_texture->Release_Ref();
            unused_textures[i] = nullptr;
        }
        unused_textures_count = 0;
        texture.Reset();
    }

    if (unused_textures_count != 0) {
        for (auto i = 0; i < unused_textures_count; ++i) {
            auto *unused_texture = unused_textures[i];
            m_textureHash.Remove(unused_texture->Get_Name());
            unused_texture->Release_Ref();
            unused_textures[i] = nullptr;
        }
    }
}

// 0x00816090
void W3DAssetManager::Release_Texture(TextureClass *tex)
{
    m_textureHash.Remove(tex->Get_Name());
    tex->Release_Ref();
}

// 0x008145A0
void W3DAssetManager::Load_Procedural_Textures()
{
    captainslog_dbgassert(false, "MetalMapManagerClass not used");
}

// 0x00766010
MetalMapManagerClass *W3DAssetManager::Peek_Metal_Map_Manager()
{
    captainslog_dbgassert(false, "MetalMapManagerClass not used");
    return nullptr;
}

// 0x008161A0
Font3DInstanceClass *W3DAssetManager::Get_Font3DInstance(const char *name)
{
    captainslog_dbgassert(false, "Font3D classes are not used");
    return nullptr;
}

// 0x00816370
FontCharsClass *W3DAssetManager::Get_FontChars(const char *name, int point_size, bool is_bold)
{
    for (auto i = 0; i < m_fontCharsList.Count(); ++i) {
        auto *font = m_fontCharsList[i];
        if (font->Is_Font(name, point_size, is_bold)) {
            font->Add_Ref();
            return font;
        }
    }

    // Font not found so will create a new font
    auto *font = new FontCharsClass;
    font->Initialize_GDI_Font(name, point_size, is_bold);
    font->Add_Ref();
    m_fontCharsList.Add(font);
    return font;
}

// 0x00815440
AssetIterator *W3DAssetManager::Create_HTree_Iterator()
{
    captainslog_dbgassert(false, "HTree Iterator class is not used");
    return nullptr;
}

// 0x00815720
HTreeClass *W3DAssetManager::Get_HTree(const char *name)
{
    auto *hTree = m_hTreeManager.Get_Tree(name);
    if (m_loadOnDemand == false || hTree != nullptr) {
        return hTree;
    }

    char asset_filename[256]{};
    snprintf(asset_filename, 256, "%s.w3d", name);

    if (Load_3D_Assets(asset_filename) == false) {
        StringClass new_filename = StringClass{ "..\\", true } + asset_filename;
        Load_3D_Assets(new_filename);
    }
    hTree = m_hTreeManager.Get_Tree(name);
    return hTree;
}

// 0x00815460
AssetIterator *W3DAssetManager::Create_Font3DData_Iterator()
{
    captainslog_dbgassert(false, "Font3D classes are not used");
    return nullptr;
}

// 0x008162B0
void W3DAssetManager::Add_Font3DData(Font3DDataClass *font)
{
    captainslog_dbgassert(false, "Font3D classes are not used");
}

// 0x008162D0
void W3DAssetManager::Remove_Font3DData(Font3DDataClass *font)
{
    captainslog_dbgassert(false, "Font3D classes are not used");
}

// 0x00816210
Font3DDataClass *W3DAssetManager::Get_Font3DData(const char *name)
{
    captainslog_dbgassert(false, "Font3D classes are not used");
    return nullptr;
}

// 0x00816300
void W3DAssetManager::Release_All_Font3DDatas()
{
    captainslog_dbgassert(false, "Font3D classes are not used");
}

// 0x00816330
void W3DAssetManager::Release_Unused_Font3DDatas()
{
    captainslog_dbgassert(false, "Font3D classes are not used");
}

// 0x008164C0
void W3DAssetManager::Release_All_FontChars()
{
    for (auto i = 0; i < m_fontCharsList.Count(); ++i) {
        m_fontCharsList[i]->Release_Ref();
    }
    m_fontCharsList.Delete_All();
}

// 0x00816610
PrototypeClass *W3DAssetManager::Find_Prototype(const char *name)
{
    PrototypeClass *proto = &s_nullPrototype;
    if (strcasecmp(name, "NULL") != 0) {
        proto = Prototype_Hash_Table_Find(name);
    }
    return proto;
}

void W3DAssetManager::Prototype_Hash_Table_Add(PrototypeClass *entry)
{
    unsigned int hash = Prototype_Hash_Table_Hash(entry->Get_Name());
    entry->Set_Next_Hash(m_prototypeHashTable[hash]);
    m_prototypeHashTable[hash] = entry;
}

PrototypeClass *W3DAssetManager::Prototype_Hash_Table_Find(char const *key)
{
    for (PrototypeClass *i = m_prototypeHashTable[Prototype_Hash_Table_Hash(key)]; i; i = i->Get_Next_Hash()) {
        if (!strcasecmp(i->Get_Name(), key)) {
            return i;
        }
    }
    return nullptr;
}

int32_t W3DAssetManager::Prototype_Hash_Table_Hash(char const *key)
{
    return (s_prototypeHashTableSize - 1) & CRC::Stringi(key, 0);
}

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

// 0x00765180
RenderObjClass *GameAssetManager::Create_Render_Obj(
    const char *name, float scale, uint32_t colour, const char *old_texture, const char *new_texture)
{
    bool has_scaling = fabs(scale - 1.0) >= 0.001f;
    bool has_colour = (colour & 0xFFFFFF) != 0;
    bool has_texture = old_texture != nullptr && new_texture != nullptr;

    if (has_scaling || has_colour || has_texture) {
        char new_name[256]{};
        strcpy(new_name, name);
        strlwr(new_name);
        const char *tex = "";
        if (new_texture != nullptr) {
            tex = new_texture;
        }

        char buffer[512]{};
        snprintf(buffer, 512, "#%d!%g!%s#%s", colour, scale, tex, new_name);
        m_loadOnDemand = false;
        auto *robj = W3DAssetManager::Create_Render_Obj(buffer);
        if (robj != nullptr) {
            robj->Set_House_Color(colour);
            m_loadOnDemand = true;
            return robj;
        }

        auto *proto = Find_Prototype(name);
        m_loadOnDemand = true;
        if (proto == nullptr) {
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
            return nullptr;
        }

        robj = proto->Create();
        if (robj == nullptr) {
            return nullptr;
        }

        if (has_scaling) {
            robj->Set_ObjectScale(scale);
        }

        switch (robj->Class_ID()) {
            case RenderObjClass::CLASSID_MESH:
                Make_Mesh_Unique(robj, has_scaling, has_colour);
                break;
            case RenderObjClass::CLASSID_HLOD:
                Make_Unique(robj, has_scaling, has_colour);
                break;
            default:
                break;
        }
        if (has_texture) {
            auto *old_tex = Get_Texture(old_texture);
            auto *new_tex = Get_Texture(new_texture);
            switch (robj->Class_ID()) {
                case RenderObjClass::CLASSID_MESH:
                    Replace_Mesh_Texture(robj, old_tex, new_tex);
                    break;
                case RenderObjClass::CLASSID_HLOD:
                    Replace_HLOD_Texture(robj, old_tex, new_tex);
                    break;
                default:
                    break;
            }
            if (old_tex != nullptr) {
                old_tex->Release_Ref();
            }
            if (new_tex != nullptr) {
                new_tex->Release_Ref();
            }
        }
        if (has_colour) {
            switch (robj->Class_ID()) {
                case RenderObjClass::CLASSID_MESH:
                    Recolor_Mesh(robj, colour);
                    break;
                case RenderObjClass::CLASSID_HLOD: {
                    bool recolored = false;
                    for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
                        auto *sobj = robj->Get_Sub_Object(i);
                        recolored |= Recolor_HLOD(sobj, colour);
                        if (sobj != nullptr) {
                            sobj->Release_Ref();
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        auto *w3d_proto = new W3DPrototypeClass(robj, buffer);
        robj->Release_Ref();
        Prototype_Hash_Table_Add(w3d_proto);
        robj = w3d_proto->Create();
        robj->Set_House_Color(colour);
        return robj;
    } else {
        return W3DAssetManager::Create_Render_Obj(name);
    }
}

// 0x00765DF0
void GameAssetManager::Make_Mesh_Unique(RenderObjClass *robj, bool geometry, bool colors)
{
    // Uses MeshClass requires MaterialInfoClass
#ifdef GAME_DLL
    Call_Method<void, W3DAssetManager, RenderObjClass *, bool, bool>(0x00765DF0, this, robj, geometry, colors);
#endif
}

// 0x00765D70
void GameAssetManager::Make_Unique(RenderObjClass *robj, bool geometry, bool colors)
{
    for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
        auto *sub_robj = robj->Get_Sub_Object(i);

        switch (sub_robj->Class_ID()) {
            case RenderObjClass::CLASSID_MESH:
                Make_Mesh_Unique(sub_robj, geometry, colors);
                break;
            case RenderObjClass::CLASSID_HLOD:
                Make_Unique(sub_robj, geometry, colors);
                break;

            default:
                break;
        }

        if (sub_robj != nullptr) {
            sub_robj->Release_Ref();
        }
    }
}

// 0x00765A40
bool GameAssetManager::Recolor_Mesh(RenderObjClass *robj, uint32_t colour)
{
    // Uses MeshClass requires MaterialInfoClass
#ifdef GAME_DLL
    return Call_Method<bool, W3DAssetManager, RenderObjClass *, uint32_t>(0x00765A40, this, robj, colour);
#else
    return false;
#endif
}

// 0x007659B0 maybe rename to Recolor_Asset
bool GameAssetManager::Recolor_HLOD(RenderObjClass *robj, uint32_t colour)
{
    switch (robj->Class_ID()) {
        case RenderObjClass::CLASSID_MESH:
            return Recolor_Mesh(robj, colour);
        case RenderObjClass::CLASSID_HLOD: {
            bool recolored = false;
            for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
                auto *sub_robj = robj->Get_Sub_Object(i);
                recolored |= Recolor_HLOD(sub_robj, colour);
                if (sub_robj != nullptr) {
                    sub_robj->Release_Ref();
                }
            }
            return recolored;
        }
        default:
            return false;
    }
}

// 0x00763EC0
bool GameAssetManager::Replace_HLOD_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture)
{
    bool replaced = false;
    for (auto i = 0; i < robj->Get_Num_Sub_Objects(); ++i) {
        auto *sub_robj = robj->Get_Sub_Object(i);

        switch (sub_robj->Class_ID()) {
            case RenderObjClass::CLASSID_MESH:
                replaced |= Replace_Mesh_Texture(sub_robj, old_texture, new_texture);
                break;
            case RenderObjClass::CLASSID_HLOD:
                replaced |= Replace_HLOD_Texture(sub_robj, old_texture, new_texture);
                break;

            default:
                break;
        }

        if (sub_robj != nullptr) {
            sub_robj->Release_Ref();
        }
    }
    return replaced;
}

// 0x00763F70
bool GameAssetManager::Replace_Mesh_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture)
{
    // Uses MeshClass requires MaterialInfoClass
#ifdef GAME_DLL
    return Call_Method<bool, W3DAssetManager, RenderObjClass *, TextureClass *, TextureClass *>(
        0x00763F70, this, robj, old_texture, new_texture);
#else
    return false;
#endif
}

W3DPrototypeClass::W3DPrototypeClass(RenderObjClass *proto, const char *name) : m_proto(proto), m_name(name)
{
    m_proto->Add_Ref();
}

int32_t W3DPrototypeClass::Get_Class_ID()
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
