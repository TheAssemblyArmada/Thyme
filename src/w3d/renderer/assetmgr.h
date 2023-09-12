/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Asset Manager
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
#include "hanimmgr.h"
#include "htreemgr.h"
#include "nullrobj.h"
#include "simplevec.h"
#include "texture.h"
#include "vector.h"

class RenderObjClass;

class AssetIterator
{
protected:
    int32_t m_index;

public:
    AssetIterator() { m_index = 0; }

    virtual ~AssetIterator(){};
    virtual void First() { m_index = 0; }
    virtual void Next() { ++m_index; }
    virtual bool Is_Done() const = 0;
    virtual const char *Current_Item_Name() const = 0;
};

class RenderObjIterator : public AssetIterator
{
public:
    virtual int Current_Item_Class_ID() = 0;
};

class MetalMapManagerClass;
class Font3DInstanceClass;
class Font3DDataClass;
class FontCharsClass;
class PrototypeLoaderClass;
class PrototypeClass;
class TextureFileCache;
class VertexMaterialClass;

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
    virtual void Release_Unused_Assets()
    {
        Release_Unused_Textures();
        // Release_Unused_Font3DDatas(); Font3DData is not used
    }
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
    virtual void Register_Prototype_Loader(PrototypeLoaderClass *loader)
    {
        if (!Find_Prototype_Loader(loader->Chunk_Type())) {
            m_prototypeLoaders.Add(loader);
        }
    }
    static W3DAssetManager *Get_Instance() { return s_theInstance; }

    static void Delete_This()
    {
        if (s_theInstance)
            delete s_theInstance;
        s_theInstance = NULL;
    }

    void Add_Prototype(PrototypeClass *proto);
    PrototypeClass *Find_Prototype(const char *name);
    void Remove_Prototype(PrototypeClass *proto);
    void Remove_Prototype(const char *name);

    bool Get_W3D_Load_On_Demand() const { return m_loadOnDemand; }
    void Set_W3D_Load_On_Demand(bool state) { m_loadOnDemand = state; }
    bool Get_Activate_Fog_On_Load() { return m_activateFogOnLoad; }
    void Set_Activate_Fog_On_Load(bool state) { m_activateFogOnLoad = state; }
    HashTemplateClass<StringClass, TextureClass *> &Texture_Hash() { return m_textureHash; }

protected:
    enum
    {
        PROTOLOADERS_VECTOR_SIZE = 32,
        PROTOLOADERS_GROWTH_RATE = 16,

        PROTOTYPES_VECTOR_SIZE = 256,
        PROTOTYPES_GROWTH_RATE = 32,
    };

    enum
    {
        PROTOTYPE_HASH_TABLE_SIZE = 4096,
        PROTOTYPE_HASH_BITS = 12,
        PROTOTYPE_HASH_MASK = 0x00000FFF
    };

    virtual AssetIterator *Create_Font3DData_Iterator();
    virtual void Add_Font3DData(Font3DDataClass *font);
    virtual void Remove_Font3DData(Font3DDataClass *font);
    virtual Font3DDataClass *Get_Font3DData(const char *name);
    virtual void Release_All_Font3DDatas();
    virtual void Release_Unused_Font3DDatas();
    virtual void Release_All_FontChars();

    void Prototype_Hash_Table_Add(PrototypeClass *entry);
    PrototypeClass *Prototype_Hash_Table_Find(char const *key);
    int32_t Prototype_Hash_Table_Hash(char const *key);

    PrototypeLoaderClass *Find_Prototype_Loader(int chunk_id);
    bool Load_Prototype(ChunkLoadClass &cload);

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
    friend class RObjIterator;
    friend class HAnimIterator;
};
