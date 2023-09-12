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
#include "vertmaterial.h"
#include "chunkio.h"
#include "dx8wrapper.h"
#include "iniclass.h"
#include "realcrc.h"
#include "w3d_util.h"
#include "xstraw.h"
#include <cstdio>

#ifndef GAME_DLL
VertexMaterialClass *VertexMaterialClass::s_presets[VertexMaterialClass::PRESET_COUNT];
#endif

void VertexMaterialClass::Init()
{
    for (int i = 0; i < PRESET_COUNT; i++) {
        s_presets[i] = new VertexMaterialClass();
    }

    s_presets[0]->Set_Diffuse_Color_Source(COLOR1);
    s_presets[0]->Set_Lighting(false);
    s_presets[1]->Set_Lighting(false);
}

void VertexMaterialClass::Shutdown()
{
    for (int i = 0; i < PRESET_COUNT; i++) {
        delete s_presets[i];
    }
}

void VertexMaterialClass::Apply() const
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Material(m_material);

    if (W3D::Is_Coloring_Enabled()) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, 0);
    } else {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, m_useLighting);
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENTMATERIALSOURCE, m_ambientColorSource);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_DIFFUSEMATERIALSOURCE, m_diffuseColorSource);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_EMISSIVEMATERIALSOURCE, m_emissiveColorSource);

    for (int i = 0; i < MAX_STAGES; i++) {
        if (m_mapper[i]) {
            m_mapper[i]->Apply(m_UVSource[i]);
        } else {
            DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, m_UVSource[0]);
            DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
        }
    }
#endif
}

void VertexMaterialClass::Apply_Null()
{
#ifdef BUILD_WITH_D3D8
    static D3DMATERIAL8 default_settings = { { { 1.0f }, { 1.0f }, { 1.0f }, { 1.0f } },
        { { 1.0f }, { 1.0f }, { 1.0f }, { 1.0f } },
        { { 0.0f }, { 0.0f }, { 0.0f }, { 0.0f } },
        { { 0.0f }, { 0.0f }, { 0.0f }, { 0.0f } },
        1.0f };

    DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, 0);
    DX8Wrapper::Set_DX8_Material(&default_settings);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENTMATERIALSOURCE, 0);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_DIFFUSEMATERIALSOURCE, 0);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_EMISSIVEMATERIALSOURCE, 0);

    for (int i = 0; i < MAX_STAGES; i++) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, i);
        DX8Wrapper::Set_DX8_Texture_Stage_State(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
    }
#endif
}

float VertexMaterialClass::Get_Opacity() const
{
#ifdef BUILD_WITH_D3D8
    return m_material->Diffuse.a;
#else
    return 0;
#endif
}

void VertexMaterialClass::Get_Diffuse(Vector3 *set_color) const
{
#ifdef BUILD_WITH_D3D8
    set_color->X = m_material->Diffuse.r;
    set_color->Y = m_material->Diffuse.g;
    set_color->Z = m_material->Diffuse.b;
#endif
}

unsigned long VertexMaterialClass::Compute_CRC() const
{
    unsigned int crc = 0;
#ifdef BUILD_WITH_D3D8
    D3DMATERIAL8 *mat = m_material;
    crc = CRC_Memory(mat, sizeof(*mat), crc);
#endif
    crc = CRC_Memory(&m_flags, sizeof(m_flags), crc);
    crc = CRC_Memory(&m_diffuseColorSource, sizeof(m_diffuseColorSource), crc);
    crc = CRC_Memory(&m_ambientColorSource, sizeof(m_ambientColorSource), crc);
    crc = CRC_Memory(&m_emissiveColorSource, sizeof(m_emissiveColorSource), crc);
    crc = CRC_Memory(&m_UVSource, sizeof(m_UVSource), crc);
    crc = CRC_Memory(&m_useLighting, sizeof(m_useLighting), crc);
    crc = CRC_Memory(&m_uniqueID, sizeof(m_uniqueID), crc);
    for (int i = 0; i < MAX_STAGES; i++) {
        crc = CRC_Memory(&m_mapper[i], sizeof(*m_mapper[i]), crc);
    }
    return crc;
}

void VertexMaterialClass::Set_Diffuse(float r, float g, float b)
{
#ifdef BUILD_WITH_D3D8
    m_CRCDirty = true;
    m_material->Diffuse.r = r;
    m_material->Diffuse.g = g;
    m_material->Diffuse.b = b;
#endif
}

void VertexMaterialClass::Set_Opacity(float o)
{
#ifdef BUILD_WITH_D3D8
    m_CRCDirty = true;
    m_material->Diffuse.a = o;
#endif
}

void VertexMaterialClass::Parse_Mapping_Args(
    W3dVertexMaterialStruct const &vmat, char *mapping0_arg_buffer, char *mapping1_arg_buffer)
{
    INIClass mapping0_arg_ini;
    if (mapping0_arg_buffer) {
        unsigned int mapping0_arg_len = strlen(mapping0_arg_buffer);
        char *c = new char[mapping0_arg_len + 10];
        snprintf(c, mapping0_arg_len + 10, "[Args]\n%s", mapping0_arg_buffer);
        BufferStraw map_arg_buf_straw(c, (int)strlen(c));
        mapping0_arg_ini.Load(map_arg_buf_straw);
        delete[] c;
    }
    INIClass mapping1_arg_ini;
    if (mapping1_arg_buffer) {
        unsigned int mapping1_arg_len = strlen(mapping1_arg_buffer);
        char *c = new char[mapping1_arg_len + 20];
        snprintf(c, mapping1_arg_len + 20, "[Args]\n%s", mapping1_arg_buffer);
        BufferStraw map_arg_buf_straw(c, (int)strlen(c));
        mapping1_arg_ini.Load(map_arg_buf_straw);
        delete[] c;
    }

    switch (vmat.Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) {
        case W3DVERTMAT_STAGE0_MAPPING_UV:
            Set_Mapper(nullptr, 0);
            break;
        case W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET:
            Set_Mapper(new StepLinearOffsetTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET:
            Set_Mapper(new LinearOffsetTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT:
            Set_Mapper(new EnvironmentMapperClass(0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT:
            Set_Mapper(new ClassicEnvironmentMapperClass(0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_SCREEN:
            Set_Mapper(new ScreenMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_SCALE:
            Set_Mapper(new ScaleTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_GRID:
            Set_Mapper(new GridTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_ROTATE:
            Set_Mapper(new RotateTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET:
            Set_Mapper(new SineLinearOffsetTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET:
            Set_Mapper(new ZigZagLinearOffsetTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV:
            Set_Mapper(new WSClassicEnvironmentMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT:
            Set_Mapper(new WSEnvironmentMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV:
            Set_Mapper(new GridClassicEnvironmentMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT:
            Set_Mapper(new GridEnvironmentMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_RANDOM:
            Set_Mapper(new RandomTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_EDGE:
            Set_Mapper(new EdgeMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_BUMPENV:
            Set_Mapper(new BumpEnvTextureMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV:
            Set_Mapper(new GridWSClassicEnvironmentMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT:
            Set_Mapper(new GridWSEnvironmentMapperClass(mapping0_arg_ini, "Args", 0), 0);
            m_mapper[0]->Release_Ref();
            break;
        default:
            captainslog_error("Invalid Texture Mapper Specified.");
            break;
    }
    switch (vmat.Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) {
        case W3DVERTMAT_STAGE1_MAPPING_UV:
            Set_Mapper(nullptr, 1);
            break;
        case W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET:
            Set_Mapper(new StepLinearOffsetTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET:
            Set_Mapper(new LinearOffsetTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT:
            Set_Mapper(new EnvironmentMapperClass(1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT:
            Set_Mapper(new ClassicEnvironmentMapperClass(1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_SCREEN:
            Set_Mapper(new ScreenMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_SCALE:
            Set_Mapper(new ScaleTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_GRID:
            Set_Mapper(new GridTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_ROTATE:
            Set_Mapper(new RotateTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET:
            Set_Mapper(new SineLinearOffsetTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET:
            Set_Mapper(new ZigZagLinearOffsetTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV:
            Set_Mapper(new WSClassicEnvironmentMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT:
            Set_Mapper(new WSEnvironmentMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV:
            Set_Mapper(new GridClassicEnvironmentMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT:
            Set_Mapper(new GridEnvironmentMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_RANDOM:
            Set_Mapper(new RandomTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_EDGE:
            Set_Mapper(new EdgeMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_BUMPENV:
            Set_Mapper(new BumpEnvTextureMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV:
            Set_Mapper(new GridWSClassicEnvironmentMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        case W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT:
            Set_Mapper(new GridWSEnvironmentMapperClass(mapping1_arg_ini, "Args", 1), 1);
            m_mapper[1]->Release_Ref();
            break;
        default:
            captainslog_error("Invalid Texture Mapper Specified.");
            break;
    }
}

VertexMaterialClass::VertexMaterialClass() :
    m_material(nullptr),
    m_flags(0),
    m_ambientColorSource(MATERIAL),
    m_emissiveColorSource(MATERIAL),
    m_diffuseColorSource(MATERIAL),
    m_uniqueID(0),
    m_CRCDirty(true),
    m_useLighting(false)
{
    for (int i = 0; i < MAX_STAGES; i++) {
        m_mapper[i] = nullptr;
        m_UVSource[i] = 0;
    }

#ifdef BUILD_WITH_D3D8
    m_material = new DynD3DMATERIAL8;
    memset(static_cast<D3DMATERIAL8 *>(m_material), 0, sizeof(D3DMATERIAL8));
    Set_Ambient(1.0f, 1.0f, 1.0f);
    Set_Diffuse(1.0f, 1.0f, 1.0f);
    Set_Opacity(1.0f);
#endif
}

VertexMaterialClass::VertexMaterialClass(const VertexMaterialClass &src) :
    m_material(nullptr),
    m_flags(src.m_flags),
    m_ambientColorSource(src.m_ambientColorSource),
    m_emissiveColorSource(src.m_emissiveColorSource),
    m_diffuseColorSource(src.m_diffuseColorSource),
    m_name(src.m_name),
    m_uniqueID(src.m_uniqueID),
    m_CRCDirty(true),
    m_useLighting(src.m_useLighting)
{
    for (int i = 0; i < MAX_STAGES; i++) {
        m_mapper[i] = 0;

        if (src.m_mapper[i]) {
            TextureMapperClass *mapper = src.m_mapper[i]->Clone();
            Set_Mapper(mapper, i);
            mapper->Release_Ref();
        }

        m_UVSource[i] = src.m_UVSource[i];
    }

#ifdef BUILD_WITH_D3D8
    m_material = new DynD3DMATERIAL8;
    memcpy(static_cast<D3DMATERIAL8 *>(m_material), static_cast<D3DMATERIAL8 *>(src.m_material), sizeof(D3DMATERIAL8));
#endif
}

VertexMaterialClass::~VertexMaterialClass()
{
    for (int i = 0; i < MAX_STAGES; i++) {
        Ref_Ptr_Release(m_mapper[i]);
    }

#ifdef BUILD_WITH_D3D8
    if (m_material) {
        delete m_material;
    }
#endif
}

VertexMaterialClass &VertexMaterialClass::operator=(const VertexMaterialClass &src)
{
    if (this != &src) {
        m_name = src.m_name;
        m_flags = src.m_flags;
        m_ambientColorSource = src.m_ambientColorSource;
        m_emissiveColorSource = src.m_emissiveColorSource;
        m_diffuseColorSource = src.m_diffuseColorSource;
        m_useLighting = src.m_useLighting;
        m_uniqueID = src.m_uniqueID;
        m_CRCDirty = src.m_CRCDirty;

        for (int i = 0; i < MAX_STAGES; i++) {
            Ref_Ptr_Release(m_mapper[i]);

            if (src.m_mapper[i]) {
                TextureMapperClass *mapper = src.m_mapper[i]->Clone();
                Set_Mapper(mapper, i);
                mapper->Release_Ref();
            }

            m_UVSource[i] = src.m_UVSource[i];
        }

#ifdef BUILD_WITH_D3D8
        memcpy(static_cast<D3DMATERIAL8 *>(m_material), static_cast<D3DMATERIAL8 *>(src.m_material), sizeof(D3DMATERIAL8));
#endif
    }
    return *this;
}

float VertexMaterialClass::Get_Shininess() const
{
#ifdef BUILD_WITH_D3D8
    return m_material->Power;
#else
    return 0;
#endif
}

void VertexMaterialClass::Set_Shininess(float shin)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Power = shin;
#endif
}

void VertexMaterialClass::Get_Ambient(Vector3 *set_color) const
{
#ifdef BUILD_WITH_D3D8
    set_color->X = m_material->Ambient.r;
    set_color->Y = m_material->Ambient.g;
    set_color->Z = m_material->Ambient.b;
#endif
}

void VertexMaterialClass::Set_Ambient(const Vector3 &color)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Ambient.r = color.X;
    m_material->Ambient.g = color.Y;
    m_material->Ambient.b = color.Z;
#endif
}

void VertexMaterialClass::Set_Ambient(float r, float g, float b)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Ambient.r = r;
    m_material->Ambient.g = g;
    m_material->Ambient.b = b;
#endif
}

void VertexMaterialClass::Set_Diffuse(const Vector3 &color)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Diffuse.r = color.X;
    m_material->Diffuse.g = color.Y;
    m_material->Diffuse.b = color.Z;
#endif
}

void VertexMaterialClass::Get_Specular(Vector3 *set_color) const
{
#ifdef BUILD_WITH_D3D8
    set_color->X = m_material->Specular.r;
    set_color->Y = m_material->Specular.g;
    set_color->Z = m_material->Specular.b;
#endif
}

void VertexMaterialClass::Set_Specular(const Vector3 &color)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Specular.r = color.X;
    m_material->Specular.g = color.Y;
    m_material->Specular.b = color.Z;
#endif
}

void VertexMaterialClass::Set_Specular(float r, float g, float b)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Specular.r = r;
    m_material->Specular.g = g;
    m_material->Specular.b = b;
#endif
}

void VertexMaterialClass::Get_Emissive(Vector3 *set_color) const
{
#ifdef BUILD_WITH_D3D8
    set_color->X = m_material->Emissive.r;
    set_color->Y = m_material->Emissive.g;
    set_color->Z = m_material->Emissive.b;
#endif
}

void VertexMaterialClass::Set_Emissive(const Vector3 &color)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Emissive.r = color.X;
    m_material->Emissive.g = color.Y;
    m_material->Emissive.b = color.Z;
#endif
}

void VertexMaterialClass::Set_Emissive(float r, float g, float b)
{
    m_CRCDirty = true;
#ifdef BUILD_WITH_D3D8
    m_material->Emissive.r = r;
    m_material->Emissive.g = g;
    m_material->Emissive.b = b;
#endif
}

void VertexMaterialClass::Set_Ambient_Color_Source(ColorSourceType src)
{
    m_CRCDirty = true;

    if (src == COLOR1) {
        m_ambientColorSource = COLOR1;
    } else if (src == COLOR2) {
        m_ambientColorSource = COLOR2;
    } else {
        m_ambientColorSource = MATERIAL;
    }
}

VertexMaterialClass::ColorSourceType VertexMaterialClass::Get_Ambient_Color_Source()
{
    if (m_ambientColorSource == COLOR1) {
        return COLOR1;
    } else if (m_ambientColorSource == COLOR2) {
        return COLOR2;
    } else {
        return MATERIAL;
    }
}

void VertexMaterialClass::Set_Emissive_Color_Source(ColorSourceType src)
{
    m_CRCDirty = true;

    if (src == COLOR1) {
        m_emissiveColorSource = COLOR1;
    } else if (src == COLOR2) {
        m_emissiveColorSource = COLOR2;
    } else {
        m_emissiveColorSource = MATERIAL;
    }
}

VertexMaterialClass::ColorSourceType VertexMaterialClass::Get_Emissive_Color_Source()
{
    if (m_emissiveColorSource == COLOR1) {
        return COLOR1;
    } else if (m_emissiveColorSource == COLOR2) {
        return COLOR2;
    } else {
        return MATERIAL;
    }
}

void VertexMaterialClass::Set_Diffuse_Color_Source(ColorSourceType src)
{
    m_CRCDirty = true;

    if (src == COLOR1) {
        m_diffuseColorSource = COLOR1;
    } else if (src == COLOR2) {
        m_diffuseColorSource = COLOR2;
    } else {
        m_diffuseColorSource = MATERIAL;
    }
}

VertexMaterialClass::ColorSourceType VertexMaterialClass::Get_Diffuse_Color_Source()
{
    if (m_diffuseColorSource == COLOR1) {
        return COLOR1;
    } else if (m_diffuseColorSource == COLOR2) {
        return COLOR2;
    } else {
        return MATERIAL;
    }
}

void VertexMaterialClass::Set_UV_Source(int stage, int array_index)
{
    captainslog_assert(stage >= 0);
    captainslog_assert(stage < MAX_STAGES);
    captainslog_assert(array_index >= 0);
    captainslog_assert(array_index < 8);
    m_CRCDirty = true;
    m_UVSource[stage] = array_index;
}

int VertexMaterialClass::Get_UV_Source(int stage)
{
    captainslog_assert(stage >= 0);
    captainslog_assert(stage < MAX_STAGES);
    return m_UVSource[stage];
}

W3DErrorType VertexMaterialClass::Load_W3D(ChunkLoadClass &cload)
{
    char name[256];
    bool hasname = false;
    char *mapping0_arg_buffer = nullptr;
    char *mapping1_arg_buffer = nullptr;
    W3dVertexMaterialStruct vmat;
    unsigned int mapping0_arg_len = 0;
    unsigned int mapping1_arg_len = 0;
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_VERTEX_MATERIAL_NAME:
                cload.Read(name, cload.Cur_Chunk_Length());
                hasname = true;
                break;
            case W3D_CHUNK_VERTEX_MATERIAL_INFO:
                if (cload.Read(&vmat, sizeof(vmat)) != sizeof(vmat)) {
                    return W3D_ERROR_LOAD_FAILED;
                }
                break;
            case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
                mapping0_arg_len = cload.Cur_Chunk_Length();
                mapping0_arg_buffer = new char[mapping0_arg_len];
                if (cload.Read(mapping0_arg_buffer, mapping0_arg_len) != mapping0_arg_len) {
                    return W3D_ERROR_LOAD_FAILED;
                }
                break;
            case W3D_CHUNK_VERTEX_MAPPER_ARGS1:
                mapping1_arg_len = cload.Cur_Chunk_Length();
                mapping1_arg_buffer = new char[mapping1_arg_len];
                if (cload.Read(mapping1_arg_buffer, mapping1_arg_len) != mapping1_arg_len) {
                    return W3D_ERROR_LOAD_FAILED;
                }
                break;
        }
        cload.Close_Chunk();
    }

    if ((hasname) && (name)) {
        m_name = name;
    }

    Parse_W3dVertexMaterialStruct(vmat);
    Parse_Mapping_Args(vmat, mapping0_arg_buffer, mapping1_arg_buffer);

    delete[] mapping0_arg_buffer;
    delete[] mapping1_arg_buffer;
    return W3D_ERROR_OK;
}

W3DErrorType VertexMaterialClass::Save_W3D(ChunkSaveClass &csave)
{
    captainslog_assert(0);
    return W3D_ERROR_OK;
}

void VertexMaterialClass::Init_From_Material3(const W3dMaterial3Struct &mat3)
{
    Vector3 v1;
    Vector3 v2;
    Vector3 v3;
    W3dUtilityClass::Convert_Color(mat3.diffuse_color, &v1);
    W3dUtilityClass::Convert_Color(mat3.diffuse_coeffs, &v2);
    v3.X = v2.X * v1.X;
    v3.Y = v2.Y * v1.Y;
    v3.Z = v2.Z * v1.Z;
    Set_Diffuse(v3);

    W3dUtilityClass::Convert_Color(mat3.specular_color, &v1);
    W3dUtilityClass::Convert_Color(mat3.specular_coeffs, &v2);
    v3.X = v2.X * v1.X;
    v3.Y = v2.Y * v1.Y;
    v3.Z = v2.Z * v1.Z;
    Set_Specular(v3);

    W3dUtilityClass::Convert_Color(mat3.emissive_coeffs, &v1);
    Set_Emissive(v1);

    W3dUtilityClass::Convert_Color(mat3.ambient_coeffs, &v1);
    Set_Ambient(v1);

    Set_Shininess(mat3.shininess);
    Set_Opacity(mat3.opacity);
}

void VertexMaterialClass::Parse_W3dVertexMaterialStruct(W3dVertexMaterialStruct const &vmat)
{
    Vector3 v1;
    W3dUtilityClass::Convert_Color(vmat.Ambient, &v1);
    Set_Ambient(v1);

    W3dUtilityClass::Convert_Color(vmat.Diffuse, &v1);
    Set_Diffuse(v1);

    W3dUtilityClass::Convert_Color(vmat.Specular, &v1);
    Set_Specular(v1);

    W3dUtilityClass::Convert_Color(vmat.Emissive, &v1);
    Set_Emissive(v1);

    Set_Shininess(vmat.Shininess);
    Set_Opacity(vmat.Opacity);

    if (vmat.Attributes & W3DVERTMAT_USE_DEPTH_CUE) {
        Set_Flag(DEPTH_CUE, true);
    }

    if (vmat.Attributes & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE) {
        Set_Flag(COPY_SPECULAR_TO_DIFFUSE, true);
    }
}

VertexMaterialClass *VertexMaterialClass::Get_Preset(PresetType type)
{
    captainslog_assert(type < PRESET_COUNT);
    s_presets[type]->Add_Ref();
    return s_presets[type];
}

unsigned int g_unique = 1;
void VertexMaterialClass::Make_Unique()
{
    m_CRCDirty = true;
    m_uniqueID = g_unique;
    g_unique++;
}
