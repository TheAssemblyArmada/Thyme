/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Vertex Materials
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
#include "mapper.h"
#include "refcount.h"
#include "vector3.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "w3dmpo.h"
#include "w3dtypes.h"
#include "wwstring.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class ChunkLoadClass;
class ChunkSaveClass;

#ifdef BUILD_WITH_D3D8
struct DynD3DMATERIAL8 : public W3DMPO, public D3DMATERIAL8
{
    IMPLEMENT_W3D_POOL(DynD3DMATERIAL8);
};
#endif
class DX8Wrapper;
class TextureMapperClass;
class VertexMaterialClass : public W3DMPO, public RefCountClass
{
    IMPLEMENT_W3D_POOL(VertexMaterialClass);
    friend DX8Wrapper;

public:
    enum
    {
        MAX_STAGES = 8
    };

    enum FlagsType
    {
        DEPTH_CUE = 0,
        DEPTH_CUE_TO_ALPHA,
        COPY_SPECULAR_TO_DIFFUSE,
    };

    enum ColorSourceType
    {
        MATERIAL = 0,
        COLOR1,
        COLOR2,
    };

    enum PresetType
    {
        PRELIT_DIFFUSE = 0,
        PRELIT_NODIFFUSE,
        PRESET_COUNT
    };

    VertexMaterialClass();
    VertexMaterialClass(const VertexMaterialClass &src);
    virtual ~VertexMaterialClass();

    VertexMaterialClass &operator=(const VertexMaterialClass &src);
    VertexMaterialClass *Clone()
    {
        VertexMaterialClass *mat = new VertexMaterialClass();
        *mat = *this;
        return mat;
    }

    void Set_Name(const char *name) { m_name = name; }
    const char *Get_Name() const { return m_name; }

    void Set_Flag(FlagsType flag, bool onoff)
    {
        m_CRCDirty = true;

        if (onoff) {
            m_flags |= (1 << flag);
        } else {
            m_flags &= ~(1 << flag);
        }
    }

    int Get_Flag(FlagsType flag) { return (m_flags >> flag) & 0x1; }

    unsigned long Get_CRC() const
    {
        if (m_CRCDirty) {
            m_CRC = Compute_CRC();
            m_CRCDirty = false;
        }

        return m_CRC;
    }

    float Get_Shininess() const;
    void Set_Shininess(float shin);

    float Get_Opacity() const;
    void Set_Opacity(float o);

    void Get_Ambient(Vector3 *set_color) const;
    void Set_Ambient(const Vector3 &color);
    void Set_Ambient(float r, float g, float b);

    void Get_Diffuse(Vector3 *set_color) const;
    void Set_Diffuse(const Vector3 &color);
    void Set_Diffuse(float r, float g, float b);

    void Get_Specular(Vector3 *set_color) const;
    void Set_Specular(const Vector3 &color);
    void Set_Specular(float r, float g, float b);

    void Get_Emissive(Vector3 *set_color) const;
    void Set_Emissive(const Vector3 &color);
    void Set_Emissive(float r, float g, float b);

    void Set_Lighting(bool lighting)
    {
        m_CRCDirty = true;
        m_useLighting = lighting;
    };
    bool Get_Lighting() const { return m_useLighting; };

    void Set_Ambient_Color_Source(ColorSourceType src);
    ColorSourceType Get_Ambient_Color_Source();

    void Set_Emissive_Color_Source(ColorSourceType src);
    ColorSourceType Get_Emissive_Color_Source();

    void Set_Diffuse_Color_Source(ColorSourceType src);
    ColorSourceType Get_Diffuse_Color_Source();

    void Set_UV_Source(int stage, int array_index);
    int Get_UV_Source(int stage);

    void Set_Mapper(TextureMapperClass *mapper, int stage = 0);
    TextureMapperClass *Get_Mapper(int stage = 0);
    TextureMapperClass *Peek_Mapper(int stage = 0);
    void Reset_Mappers();

    W3DErrorType Load_W3D(ChunkLoadClass &cload);
    W3DErrorType Save_W3D(ChunkSaveClass &csave);

    void Init_From_Material3(const W3dMaterial3Struct &mat3);

    void Parse_Mapping_Args(W3dVertexMaterialStruct const &vmat, char *mapping0_arg_buffer, char *mapping1_arg_buffer);
    void Parse_W3dVertexMaterialStruct(W3dVertexMaterialStruct const &vmat);

    bool Do_Mappers_Need_Normals() const;
    bool Are_Mappers_Time_Variant() const;

    static void Init();
    static void Shutdown();

    static VertexMaterialClass *Get_Preset(PresetType type);

    void Make_Unique();

#ifdef GAME_DLL
    VertexMaterialClass *Hook_Ctor() { return new (this) VertexMaterialClass; }
    VertexMaterialClass *Hook_Ctor2(const VertexMaterialClass &src) { return new (this) VertexMaterialClass(src); }
#endif

protected:
#ifdef BUILD_WITH_D3D8
    DynD3DMATERIAL8 *m_material;
#else
    w3dmat_t *m_material;
#endif
    unsigned int m_flags;
    unsigned int m_ambientColorSource;
    unsigned int m_emissiveColorSource;
    unsigned int m_diffuseColorSource;
    StringClass m_name;
    TextureMapperClass *m_mapper[MAX_STAGES];
    unsigned int m_UVSource[MAX_STAGES];
    unsigned int m_uniqueID;
    mutable unsigned long m_CRC;
    mutable bool m_CRCDirty;
    bool m_useLighting;

private:
    void Apply() const;
    static void Apply_Null();
    unsigned long Compute_CRC() const;

#ifdef GAME_DLL
    static ARRAY_DEC(VertexMaterialClass *, s_presets, PRESET_COUNT);
#else
    static VertexMaterialClass *s_presets[PRESET_COUNT];
#endif
};

inline void VertexMaterialClass::Set_Mapper(TextureMapperClass *mapper, int stage)
{
    m_CRCDirty = true;
    Ref_Ptr_Set(m_mapper[stage], mapper);
}

inline TextureMapperClass *VertexMaterialClass::Get_Mapper(int stage)
{
    if (m_mapper[stage]) {
        m_mapper[stage]->Add_Ref();
    }

    return m_mapper[stage];
}

inline TextureMapperClass *VertexMaterialClass::Peek_Mapper(int stage)
{
    return m_mapper[stage];
}

inline void VertexMaterialClass::Reset_Mappers()
{
    for (int stage = 0; stage < 8; stage++) {
        if (m_mapper[stage]) {
            m_mapper[stage]->Reset();
        }
    }
}

inline bool VertexMaterialClass::Do_Mappers_Need_Normals() const
{
    for (int stage = 0; stage < 8; stage++) {
        if (m_mapper[stage] && (m_mapper[stage]->Needs_Normals()))
            return true;
    }
    return false;
}

inline bool VertexMaterialClass::Are_Mappers_Time_Variant() const
{
    for (int stage = 0; stage < 8; stage++) {
        if (m_mapper[stage] && (m_mapper[stage]->Is_Time_Variant()))
            return true;
    }
    return false;
}
