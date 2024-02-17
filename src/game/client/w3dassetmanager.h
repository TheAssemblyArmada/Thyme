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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "assetmgr.h"
#include "mempoolobj.h"

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
    bool Replace_Prototype_Texture(RenderObjClass *robj, const char *old_name, const char *new_name);

    // 0x00763D70
    GameAssetManager *Hook_Ctor() { return new (this) GameAssetManager; }

private:
    void Make_Mesh_Unique(RenderObjClass *robj, bool geometry, bool colors);
    void Make_HLOD_Unique(RenderObjClass *robj, bool geometry, bool colors);
    void Make_Unique(RenderObjClass *robj, bool geometry, bool colors);
    void Recolor_Vertex_Material(VertexMaterialClass *vmat, uint32_t colour);
    bool Recolor_Mesh(RenderObjClass *robj, uint32_t colour);
    bool Recolor_HLOD(RenderObjClass *robj, uint32_t colour);
    bool Recolour_Asset(RenderObjClass *robj, uint32_t colour);
    bool Replace_HLOD_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture);
    bool Replace_Mesh_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture);
    bool Replace_Asset_Texture(RenderObjClass *robj, TextureClass *old_texture, TextureClass *new_texture);
    TextureClass *Recolor_Texture(TextureClass *texture, int color);
    TextureClass *Find_Texture(const char *name, int color);
    TextureClass *Recolor_Texture_One_Time(TextureClass *texture, int color);
    void Remap_Palette(SurfaceClass *surface, int color, bool do_palette_only, bool use_alpha);

    uint32_t m_grannyAnimManager = 0; // Not used, only here to match original size
};

class W3DPrototypeClass final : public MemoryPoolObject, public PrototypeClass
{
    IMPLEMENT_POOL(W3DPrototypeClass)

public:
    W3DPrototypeClass(RenderObjClass *proto, const char *name);
    virtual const char *Get_Name() const override { return m_name.Str(); }
    virtual int32_t Get_Class_ID() const override;
    virtual RenderObjClass *Create() override;
    virtual void Delete_Self() override { this->Delete_Instance(); };
    virtual ~W3DPrototypeClass() override;

private:
    RenderObjClass *m_proto;
    Utf8String m_name;
};
