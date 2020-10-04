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
#include "always.h"
#include "hanimmgr.h"
#include "htreemgr.h"
#include "simplevec.h"
#include "slist.h"
#include "texture.h"
#include "vector.h"

class RenderObjClass;
class RenderObjIterator;
class AssetIterator;
class MetalMapManagerClass;
class Font3DDataClass;
class Font3DInstanceClass;
class FontCharsClass;
class PrototypeLoaderClass;
class PrototypeClass;
class TextureFileCache;

class W3DAssetManager
{
public:
    virtual ~W3DAssetManager();
    virtual bool Load_3D_Assets(const char *filename);
    virtual bool Load_3D_Assets(FileClass &asset_file);
    virtual void Free_Assets();
    virtual void Release_Unused_Assets();
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
    virtual void Register_Prototype_Loader(PrototypeLoaderClass *loader);
    static W3DAssetManager *Get_Instance(void) { return s_theInstance; }

protected:
    virtual AssetIterator *Create_Font3DData_Iterator();
    virtual void Add_Font3DData(Font3DDataClass *font);
    virtual void Remove_Font3DData(Font3DDataClass *font);
    virtual Font3DDataClass *Get_Font3DData(const char *name);
    virtual void Release_All_Font3DDatas();
    virtual void Release_Unused_Font3DDatas();
    virtual void Release_All_FontChars();

    DynamicVectorClass<PrototypeLoaderClass *> m_prototypeLoaders;
    DynamicVectorClass<PrototypeClass *> m_prototypes;
    PrototypeClass **m_prototypeHashTable;
    HTreeManagerClass m_hTreeManager;
    HAnimManagerClass m_hAnimManager;
    TextureFileCache *m_textureCache;
    SList<Font3DDataClass> m_font3DDatas;
    SimpleDynVecClass<FontCharsClass *> m_fontCharsList;
    bool m_loadOnDemand;
    bool m_activateFogOnLoad;
    MetalMapManagerClass *m_metalManager;
    HashTemplateClass<StringClass, TextureClass *> m_textureHash;
#ifdef GAME_DLL
    static W3DAssetManager *&s_theInstance;
#else
    static W3DAssetManager *s_theInstance;
#endif
};
