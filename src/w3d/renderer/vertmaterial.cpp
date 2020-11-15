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
#include "iniclass.h"
#include "xstraw.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void VertexMaterialClass::Init()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008194F0, 0x004E6590));
#endif
}

void VertexMaterialClass::Shutdown()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008195B0, 0x004E6650));
#endif
}

void VertexMaterialClass::Apply() const
{
#ifdef GAME_DLL
    Call_Method<void, const VertexMaterialClass>(PICK_ADDRESS(0x008193A0, 0x004E63A0), this);
#endif
}

void VertexMaterialClass::Apply_Null()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00819460, 0x004E64B0));
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
#ifdef GAME_DLL
    return Call_Method<unsigned long, const VertexMaterialClass>(PICK_ADDRESS(0x00817750, 0x004E4400), this);
#else
    return 0;
#endif
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
