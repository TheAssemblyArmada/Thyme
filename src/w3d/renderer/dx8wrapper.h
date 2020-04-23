/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper around platform 3D graphics library.
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
#include "dx8caps.h"
#include "matrix4.h"
#include "refcount.h"
#include "renderdevicedesc.h"
#include "shader.h"
#include "texturebase.h"
#include "vector.h"
#include "vector4.h"
#include "w3dtypes.h"
#include "wwstring.h"
#include <captainslog.h>
class VertexMaterialClass;
class VertexBufferClass;
class IndexBufferClass;

// Some constants to control numbers of things.
enum
{
    MAX_TEXTURE_STAGES = 8,
    VERTEX_BUFFERS = 2,
    GFX_LIGHT_COUNT = 4,
};

#ifdef GAME_DLL
extern uint32_t &g_numberOfDx8Calls;
#else
extern uint32_t g_numberOfDx8Calls;
#endif

#ifdef BUILD_WITH_D3D8
#define DX8CALL(x) \
    DX8Wrapper::Get_D3D_Device8()->x; \
    ++g_numberOfDx8Calls;
#endif

struct RenderStateStruct
{
    ShaderClass shader;
    VertexMaterialClass *material;
    TextureClass *Textures[MAX_TEXTURE_STAGES];
#ifdef BUILD_WITH_D3D8
    D3DLIGHT8 Lights[GFX_LIGHT_COUNT];
#endif
    bool LightEnable[GFX_LIGHT_COUNT];
    Matrix4 world;
    Matrix4 view;
    unsigned vertex_buffer_types[VERTEX_BUFFERS];
    unsigned index_buffer_type;
    unsigned short vba_offset;
    unsigned short vba_count;
    unsigned short iba_offset;
    VertexBufferClass *vertex_buffers[VERTEX_BUFFERS];
    IndexBufferClass *index_buffer;
    unsigned short index_base_offset;

    RenderStateStruct();
    ~RenderStateStruct();

    RenderStateStruct &operator=(const RenderStateStruct &src);
};

class DX8Wrapper
{
    ALLOW_HOOKING
    enum ChangedStates
    {
        WORLD_CHANGED = 1 << 0,
        VIEW_CHANGED = 1 << 1,
        LIGHT0_CHANGED = 1 << 2,
        LIGHT1_CHANGED = 1 << 3,
        LIGHT2_CHANGED = 1 << 4,
        LIGHT3_CHANGED = 1 << 5,
        TEXTURE0_CHANGED = 1 << 6,
        TEXTURE1_CHANGED = 1 << 7,
        TEXTURE2_CHANGED = 1 << 8,
        TEXTURE3_CHANGED = 1 << 9,
        MATERIAL_CHANGED = 1 << 14,
        SHADER_CHANGED = 1 << 15,
        VERTEX_BUFFER_CHANGED = 1 << 16,
        INDEX_BUFFER_CHANGED = 1 << 17,
        WORLD_IDENTITY = 1 << 18,
        VIEW_IDENTITY = 1 << 19,

        TEXTURES_CHANGED = TEXTURE0_CHANGED | TEXTURE1_CHANGED | TEXTURE2_CHANGED | TEXTURE3_CHANGED,
        LIGHTS_CHANGED = LIGHT0_CHANGED | LIGHT1_CHANGED | LIGHT2_CHANGED | LIGHT3_CHANGED,
    };

public:
    static bool Init(void *hwnd, bool lite = false);
    static void Shutdown();
    static void Enumerate_Devices();
#ifdef BUILD_WITH_D3D8
    static void Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4 &m);
    static void Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4 &m);
    static void Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value);
    static void Set_DX8_Texture(unsigned stage, w3dbasetexture_t texture);
    static IDirect3DDevice8 *Get_D3D_Device8() { return s_d3dDevice; }
#endif
    static const char *Get_DX8_Texture_Address_Name(unsigned value);
    static const char *Get_DX8_Texture_Filter_Name(unsigned value);
    static const char *Get_DX8_Texture_Arg_Name(unsigned value);
    static const char *Get_DX8_Texture_Op_Name(unsigned value);
    static const char *Get_DX8_Texture_Transform_Flag_Name(unsigned value);
    static const char *Get_DX8_ZBuffer_Type_Name(unsigned value);
    static const char *Get_DX8_Fill_Mode_Name(unsigned value);
    static const char *Get_DX8_Shade_Mode_Name(unsigned value);
    static const char *Get_DX8_Blend_Name(unsigned value);
    static const char *Get_DX8_Cull_Mode_Name(unsigned value);
    static const char *Get_DX8_Cmp_Func_Name(unsigned value);
    static const char *Get_DX8_Fog_Mode_Name(unsigned value);
    static const char *Get_DX8_Stencil_Op_Name(unsigned value);
    static const char *Get_DX8_Material_Source_Name(unsigned value);
    static const char *Get_DX8_Vertex_Blend_Flag_Name(unsigned value);
    static const char *Get_DX8_Patch_Edge_Style_Name(unsigned value);
    static const char *Get_DX8_Debug_Monitor_Token_Name(unsigned value);
    static const char *Get_DX8_Blend_Op_Name(unsigned value);
    static void Log_DX8_ErrorCode(unsigned error);
    static void Handle_DX8_ErrorCode(unsigned error);
    static int Get_Texture_Bit_Depth() { return s_textureBitDepth; }
    static void Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed);
    static w3dtexture_t Create_Texture(
        unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, w3dpool_t pool, bool rendertarget);
    static w3dtexture_t Create_Texture(w3dsurface_t surface, MipCountType mip_level_count);
    static w3dsurface_t Create_Surface(unsigned width, unsigned height, WW3DFormat format);
    static w3dsurface_t Create_Surface(const char *name);

    static int Get_Main_Thread_ID() { return s_mainThreadID; }
    static const DX8Caps *Get_Caps()
    {
        captainslog_assert(s_currentCaps != nullptr);
        return s_currentCaps;
    }
    static bool Supports_DXTC() { return s_currentCaps->Supports_DXTC(); }

private:
    static void Reset_Statistics();
    static void Invalidate_Cached_Render_States();

private:
#ifdef GAME_DLL
    static IDirect3D8 *(__stdcall *&s_d3dCreateFunction)(unsigned);
    static HMODULE &s_d3dLib;
    static IDirect3D8 *&s_d3dInterface;
    static IDirect3DDevice8 *&s_d3dDevice;
    static ARRAY_DEC(w3dbasetexture_t, s_textures, MAX_TEXTURE_STAGES);
    static void *&s_hwnd; // Actually a hwnd, but we only care for building the dll.
    static void *&s_shadowMap;
    static ARRAY_DEC(unsigned, s_renderStates, 256);
    static ARRAY2D_DEC(unsigned, s_textureStageStates, MAX_TEXTURE_STAGES, 32);
    static ARRAY_DEC(Vector4, s_vertexShaderConstants, 96);
    static ARRAY_DEC(unsigned, s_pixelShaderConstants, 32);
    static bool &s_isInitialised;
    static bool &s_isWindowed;
    static bool &s_debugIsWindowed;
    static RenderStateStruct &s_renderState;
    static unsigned &s_renderStateChanged;
    static float &s_zNear;
    static float &s_zFar;
    static Matrix4 &s_projectionMatrix;
    static int &s_mainThreadID;
    static int &s_currentRenderDevice;
    static DX8Caps *&s_currentCaps;
    static int &s_resolutionWidth;
    static int &s_resolutionHeight;
    static int &s_bitDepth;
    static int &s_textureBitDepth;
    static ARRAY_DEC(bool, s_currentLightEnables, GFX_LIGHT_COUNT);
    static unsigned &s_matrixChanges;
    static unsigned &s_materialChanges;
    static unsigned &s_vertexBufferChanges;
    static unsigned &s_indexBufferChanges;
    static unsigned &s_lightChanges;
    static unsigned &s_textureChanges;
    static unsigned &s_renderStateChanges;
    static unsigned &s_textureStageStateChanges;
    static unsigned &s_drawCalls;
    static unsigned &s_lastFrameMatrixChanges;
    static unsigned &s_lastFrameMaterialChanges;
    static unsigned &s_lastFrameVertexBufferChanges;
    static unsigned &s_lastFrameIndexBufferChanges;
    static unsigned &s_lastFrameLightChanges;
    static unsigned &s_lastFrameTextureChanges;
    static unsigned &s_lastFrameRenderStateChanges;
    static unsigned &s_lastFrameTextureStageStateChanges;
    static unsigned &s_lastFrameNumberDX8Calls;
    static unsigned &s_lastFrameDrawCalls;
    static DynamicVectorClass<StringClass> &s_renderDeviceNameTable;
    static DynamicVectorClass<StringClass> &s_renderDeviceShortNameTable;
    static DynamicVectorClass<RenderDeviceDescClass> &s_renderDeviceDescriptionTable;
#else
#ifdef BUILD_WITH_D3D8
    static IDirect3D8 *(__stdcall *s_d3dCreateFunction)(unsigned);
    static HMODULE s_d3dLib;
    static IDirect3D8 *s_d3dInterface;
    static IDirect3DDevice8 *s_d3dDevice;
#endif
    static void *s_hwnd;
    static void *s_shadowMap; // Not sure what type this actually is for now.
    static unsigned s_renderStates[256];
    static w3dbasetexture_t s_textures[MAX_TEXTURE_STAGES];
    static unsigned s_textureStageStates[MAX_TEXTURE_STAGES][32];
    static Vector4 s_vertexShaderConstants[96]; // Not 100% sure this is a Vector4 array
    static unsigned s_pixelShaderConstants[32]; // Not 100% on type, seems unused.
    static bool s_isInitialised;
    static bool s_isWindowed;
    static bool s_debugIsWindowed;
    static RenderStateStruct s_renderState;
    static unsigned s_renderStateChanged;
    static float s_zNear;
    static float s_zFar;
    static Matrix4 s_projectionMatrix;
    static int s_mainThreadID;
    static int s_currentRenderDevice;
    static DX8Caps *s_currentCaps;
    static int s_resolutionWidth;
    static int s_resolutionHeight;
    static int s_bitDepth;
    static int s_textureBitDepth;
    static bool s_currentLightEnables[GFX_LIGHT_COUNT];
    static unsigned s_matrixChanges;
    static unsigned s_materialChanges;
    static unsigned s_vertexBufferChanges;
    static unsigned s_indexBufferChanges;
    static unsigned s_lightChanges;
    static unsigned s_textureChanges;
    static unsigned s_renderStateChanges;
    static unsigned s_textureStageStateChanges;
    static unsigned s_drawCalls;
    static unsigned s_lastFrameMatrixChanges;
    static unsigned s_lastFrameMaterialChanges;
    static unsigned s_lastFrameVertexBufferChanges;
    static unsigned s_lastFrameIndexBufferChanges;
    static unsigned s_lastFrameLightChanges;
    static unsigned s_lastFrameTextureChanges;
    static unsigned s_lastFrameRenderStateChanges;
    static unsigned s_lastFrameTextureStageStateChanges;
    static unsigned s_lastFrameNumberDX8Calls;
    static unsigned s_lastFrameDrawCalls;
    static DynamicVectorClass<StringClass> s_renderDeviceNameTable;
    static DynamicVectorClass<StringClass> s_renderDeviceShortNameTable;
    static DynamicVectorClass<RenderDeviceDescClass> s_renderDeviceDescriptionTable;
#endif
};

inline RenderStateStruct::RenderStateStruct() : material(0), index_buffer(0)
{
    for (unsigned i = 0; i < MAX_TEXTURE_STAGES; ++i)
        Textures[i] = 0;
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        vertex_buffers[i] = 0;
    }
}

inline RenderStateStruct::~RenderStateStruct()
{
    // Ref_Ptr_Release(material);
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        // Ref_Ptr_Release(vertex_buffers[i]);
    }
    // Ref_Ptr_Release(index_buffer);
    for (unsigned i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        // Ref_Ptr_Release(Textures[i]);
    }
}

inline RenderStateStruct &RenderStateStruct::operator=(const RenderStateStruct &src)
{
    // Ref_Ptr_Set(material, src.material);
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        // Ref_Ptr_Set(vertex_buffers[i], src.vertex_buffers[i]);
    }

    // Ref_Ptr_Set(index_buffer, src.index_buffer);

    for (unsigned i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        // Ref_Ptr_Set(Textures[i], src.Textures[i]);
    }

    LightEnable[0] = src.LightEnable[0];
    LightEnable[1] = src.LightEnable[1];
    LightEnable[2] = src.LightEnable[2];
    LightEnable[3] = src.LightEnable[3];
#ifdef BUILD_WITH_D3D8
    if (LightEnable[0]) {
        Lights[0] = src.Lights[0];

        if (LightEnable[1]) {
            Lights[1] = src.Lights[1];

            if (LightEnable[2]) {
                Lights[2] = src.Lights[2];

                if (LightEnable[3]) {
                    Lights[3] = src.Lights[3];
                }
            }
        }
    }
#endif
    shader = src.shader;
    world = src.world;
    view = src.view;

    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        vertex_buffer_types[i] = src.vertex_buffer_types[i];
    }

    index_buffer_type = src.index_buffer_type;
    vba_offset = src.vba_offset;
    vba_count = src.vba_count;
    iba_offset = src.iba_offset;
    index_base_offset = src.index_base_offset;

    return *this;
}

#ifdef BUILD_WITH_D3D8
inline void DX8Wrapper::Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value)
{
    if (s_textureStageStates[stage][state] == value) {
        return;
    }

    s_textureStageStates[stage][state] = value;
    DX8CALL(SetTextureStageState(stage, state, value));
    ++s_textureStageStateChanges;
}

inline void DX8Wrapper::Set_DX8_Texture(unsigned stage, w3dbasetexture_t texture)
{
    if (stage >= MAX_TEXTURE_STAGES) {
        DX8CALL(SetTexture(stage, nullptr));
        return;
    }

    if (s_textures[stage] == texture) {
        return;
    }

    if (s_textures[stage] != W3D_TYPE_INVALID_TEXTURE) {
        s_textures[stage]->Release();
    }

    s_textures[stage] = texture;
    s_textures[stage]->AddRef();
    DX8CALL(SetTexture(stage, texture));
    ++s_textureChanges;
}

inline void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4 &m)
{
    switch (transform) {
        case D3DTS_WORLD:
            s_renderState.world = m.Transpose();
            s_renderStateChanged |= WORLD_CHANGED;
            s_renderStateChanged &= ~WORLD_IDENTITY;
            break;
        case D3DTS_VIEW:
            s_renderState.view = m.Transpose();
            s_renderStateChanged |= VIEW_CHANGED;
            s_renderStateChanged &= ~VIEW_IDENTITY;
            break;
        case D3DTS_PROJECTION: {
            Matrix4 ProjectionMatrix = m.Transpose();
            s_zFar = 0.0f;
            s_zNear = 0.0f;
            DX8CALL(SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&ProjectionMatrix));
        } break;
        default:
            s_matrixChanges++;
            Matrix4 m2 = m.Transpose();
            DX8CALL(SetTransform(transform, (D3DMATRIX *)&m2));
            break;
    }
}

inline void DX8Wrapper::Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4 &m)
{
    D3DMATRIX mat;

    switch (transform) {
        case D3DTS_WORLD:
            if (s_renderStateChanged & WORLD_IDENTITY)
                m.Make_Identity();
            else
                m = s_renderState.world.Transpose();
            break;
        case D3DTS_VIEW:
            if (s_renderStateChanged & VIEW_IDENTITY)
                m.Make_Identity();
            else
                m = s_renderState.view.Transpose();
            break;
        default:
            DX8CALL(GetTransform(transform, &mat));
            m = *(Matrix4 *)&mat;
            m = m.Transpose();
            break;
    }
}
#endif

inline void DX8Wrapper::Handle_DX8_ErrorCode(unsigned error)
{
#ifdef BUILD_WITH_D3D8
    if (error != D3D_OK) {
        DX8Wrapper::Log_DX8_ErrorCode(error);
    }
#endif
}
