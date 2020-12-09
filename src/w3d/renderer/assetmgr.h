/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Hierarchy Tree Manager
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
#include "asciistring.h"
#include "hanimmgr.h"
#include "htreemgr.h"
#include "nullrobj.h"
#include "simplevec.h"
#include "texture.h"
#include "vector.h"

class RenderObjClass;
class RenderObjIterator;

class AssetIterator
{
protected:
    uint32_t m_index;

public:
    virtual ~AssetIterator(){};
    virtual void First() { m_index = 0; }
    virtual void Next() { ++m_index; }
    virtual bool Is_Done() const = 0;
    virtual const char *Current_Item_Name() const = 0;
};

class MetalMapManagerClass;
class Font3DInstanceClass;
class Font3DDataClass;
class FontCharsClass;
class PrototypeLoaderClass;
class PrototypeClass;
class TextureFileCache;

#ifdef GAME_DLL
extern NullPrototypeClass &s_nullPrototype;
#else
extern NullPrototypeClass s_nullPrototype;
#endif

// W3DAssetManager is the renamed WW3DAssetManager
class W3DAssetManager
{
public:
    W3DAssetManager();
    virtual ~W3DAssetManager();
    virtual bool Load_3D_Assets(const char *filename);
    virtual bool Load_3D_Assets(FileClass &asset_file);
    virtual void Free_Assets();
    // 0x00814850
    virtual void Release_Unused_Assets() { Release_Unused_Textures(); }
    virtual void Free_Assets_With_Exclusion_List(DynamicVectorClass<StringClass> const &list);
    virtual void Create_Asset_List(DynamicVectorClass<StringClass> &list);
    virtual RenderObjClass *Create_Render_Obj(const char *name);
    virtual bool Render_Obj_Exists(const char *name);
    virtual RenderObjIterator *Create_Render_Obj_Iterator();
    virtual void Release_Render_Obj_Iterator(RenderObjIterator *it);
    virtual AssetIterator *Create_HAnim_Iterator(void);
    virtual HAnimClass *Get_HAnim(const char *name);
    virtual bool Add_Anim(HAnimClass *new_anim);
    virtual TextureClass *Get_Texture(const char *filename,
        MipCountType mip_level_count = MIP_LEVELS_ALL,
        WW3DFormat texture_format = WW3D_FORMAT_UNKNOWN,
        bool allow_compression = true,
        TexAssetType asset_type = ASSET_STANDARD,
        bool allow_reduction = true);
    virtual void Release_All_Textures();
    virtual void Release_Unused_Textures();
    virtual void Release_Texture(TextureClass *tex);
    virtual void Load_Procedural_Textures();
    virtual MetalMapManagerClass *Peek_Metal_Map_Manager();
    virtual Font3DInstanceClass *Get_Font3DInstance(const char *name);
    virtual FontCharsClass *Get_FontChars(const char *name, int point_size, bool is_bold = false);
    virtual AssetIterator *Create_HTree_Iterator();
    virtual HTreeClass *Get_HTree(const char *name);
    // 0x00816550
    virtual void Register_Prototype_Loader(PrototypeLoaderClass *loader) { m_prototypeLoaders.Add(loader); }
    static W3DAssetManager *Get_Instance(void) { return s_theInstance; }

protected:
    virtual AssetIterator *Create_Font3DData_Iterator();
    virtual void Add_Font3DData(Font3DDataClass *font);
    virtual void Remove_Font3DData(Font3DDataClass *font);
    virtual Font3DDataClass *Get_Font3DData(const char *name);
    virtual void Release_All_Font3DDatas();
    virtual void Release_Unused_Font3DDatas();
    virtual void Release_All_FontChars();

    PrototypeClass *Find_Prototype(const char *name);

    static constexpr auto s_prototypeHashTableSize = 0x1000;
    void Prototype_Hash_Table_Add(PrototypeClass *entry);
    PrototypeClass *Prototype_Hash_Table_Find(char const *key);
    int32_t Prototype_Hash_Table_Hash(char const *key);

protected:
    DynamicVectorClass<PrototypeLoaderClass *> m_prototypeLoaders;
    DynamicVectorClass<PrototypeClass *> m_prototypes;
    PrototypeClass **m_prototypeHashTable;
    HTreeManagerClass m_hTreeManager;
    HAnimManagerClass m_hAnimManager;
    TextureFileCache *m_textureCache;
    uint32_t m_font3DDatasPad[3]; // This was not used and is now just padding.
    SimpleDynVecClass<FontCharsClass *> m_fontCharsList;
    bool m_loadOnDemand;
    bool m_activateFogOnLoad;
    uint32_t m_metalManagerPad; // Not used
    HashTemplateClass<StringClass, TextureClass *> m_textureHash;
#ifdef GAME_DLL
    static W3DAssetManager *&s_theInstance;
#else
    static W3DAssetManager *s_theInstance;
#endif
};

// GameAssetManager is the renamed W3DAssetManager
class GameAssetManager final : public W3DAssetManager
{
public:
    GameAssetManager(){};
    virtual ~GameAssetManager();
    virtual bool Load_3D_Assets(const char *filename);
    virtual RenderObjClass *Create_Render_Obj(const char *name);
    virtual HAnimClass *Get_HAnim(const char *name);
    virtual TextureClass *Get_Texture(const char *filename,
        MipCountType mip_level_count = MIP_LEVELS_ALL,
        WW3DFormat texture_format = WW3D_FORMAT_UNKNOWN,
        bool allow_compression = true,
        TexAssetType asset_type = ASSET_STANDARD,
        bool allow_reduction = true);
    virtual RenderObjClass *Create_Render_Obj(
        const char *name, float scale, uint32_t colour, const char *old_texture, const char *new_texture);

    // 0x00763D70
    GameAssetManager *Hook_Ctor() { return new (this) GameAssetManager; }

private:
    void Make_Mesh_Unique(RenderObjClass *robj, bool geometry, bool colors);
    void Make_Unique(RenderObjClass *robj, bool geometry, bool colors);
    bool Recolor_Mesh(RenderObjClass *robj, uint32_t colour);
    bool Recolor_HLOD(RenderObjClass *robj, uint32_t colour);
    bool Replace_HLOD_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture);
    bool Replace_Mesh_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture);
    uint32_t m_grannyAnimManager; // Not used, only here to match original size
};

class W3DPrototypeClass final : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(W3DPrototypeClass);

public:
    W3DPrototypeClass(RenderObjClass *proto, const char *name);
    virtual const char *Get_Name() override { return m_name; }
    virtual int32_t Get_Class_ID() override;
    virtual RenderObjClass *Create() override;
    virtual void Delete_Self() override { delete this; };
    virtual ~W3DPrototypeClass() override;

private:
    RenderObjClass *m_proto;
    Utf8String m_name;
};
