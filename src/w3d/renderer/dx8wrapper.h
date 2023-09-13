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
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "matrix4.h"
#include "rddesc.h"
#include "refcount.h"
#include "shader.h"
#include "texture.h"
#include "texturebase.h"
#include "vector.h"
#include "vector4.h"
#include "vertmaterial.h"
#include "w3dtypes.h"
#include "wwstring.h"
#include <algorithm>
#include <captainslog.h>
#ifdef BUILD_WITH_D3D8
#include <d3dx8math.h>
#endif

class LightClass;
class SurfaceClass;
class DynamicVBAccessClass;
class DynamicIBAccessClass;
class LightEnvironmentClass;
class ZTextureClass;
class DX8_CleanupHook
{
public:
    virtual void Release_Resources() = 0;
    virtual void Re_Acquire_Resources() = 0;
};

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
#define DX8CALL_HRES(x, res) \
    res = DX8Wrapper::Get_D3D_Device8()->x; \
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
#ifdef BUILD_WITH_D3D8
    D3DXMATRIX world;
    D3DXMATRIX view;
#else
    Matrix4 world;
    Matrix4 view;
#endif
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
#ifdef PLATFORM_WINDOWS
    static bool Init(HWND hwnd, bool lite = false);
#endif
    static void Shutdown();
    static void Do_Onetime_Device_Dependent_Inits();
    static void Do_Onetime_Device_Dependent_Shutdowns();
    static void Begin_Scene();
    static void End_Scene(bool flip_frames = true);
    static void Clear(
        bool clear_color, bool clear_z_stencil, const Vector3 &color, float alpha, float z = 1.0f, unsigned int stencil = 0);
    static void Set_Vertex_Buffer(const VertexBufferClass *vb, int number);
    static void Set_Vertex_Buffer(const DynamicVBAccessClass &vba);
    static void Set_Index_Buffer(const IndexBufferClass *ib, unsigned short index_base_offset);
    static void Set_Index_Buffer(const DynamicIBAccessClass &iba, unsigned short index_base_offset);
    static void Set_Index_Buffer_Index_Offset(unsigned offset);
    static void Set_Fog(bool enable, const Vector3 &color, float start, float end);
    static void Set_Gamma(float gamma, float bright, float contrast, bool calibrate = true, bool uselimit = true);
    static void Set_Light(unsigned index, const LightClass &light);
    static void Set_Light_Environment(LightEnvironmentClass *light_env);
    static void Apply_Render_State_Changes();
    static void Draw_Triangles(unsigned int buffer_type,
        unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index,
        unsigned short vertex_count);
    static void Draw_Triangles(unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index,
        unsigned short vertex_count);
    static void Draw_Strip(unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index,
        unsigned short vertex_count);
    static w3dtexture_t Create_Texture(
        unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, w3dpool_t pool, bool rendertarget);
    static w3dtexture_t Create_Texture(w3dsurface_t surface, MipCountType mip_level_count);
    static w3dsurface_t Create_Surface(unsigned width, unsigned height, WW3DFormat format);
    static w3dsurface_t Create_Surface(const char *name);
    static SurfaceClass *Get_DX8_Back_Buffer(unsigned int num = 0);
    static void Begin_Statistics();
    static void End_Statistics();
    static TextureClass *Create_Render_Target(int width, int height, WW3DFormat format);
    static void Set_Render_Target(w3dsurface_t render_target, bool use_default_depth_buffer = false);
    static void Set_Render_Target_With_Z(TextureClass *texture, ZTextureClass *z_texture);
    static void Set_Texture(unsigned stage, TextureClass *texture);
    static void Set_Material(const VertexMaterialClass *material);
    static void Set_Shader(const ShaderClass &shader);
    static void Get_Render_State(RenderStateStruct &state);
    static void Set_Render_State(const RenderStateStruct &state);
    static void Release_Render_State();

#ifdef BUILD_WITH_D3D8
    static const char *Get_DX8_Render_State_Name(D3DRENDERSTATETYPE state);
    static const char *Get_DX8_Texture_Stage_State_Name(D3DTEXTURESTAGESTATETYPE state);
    static void Get_DX8_Texture_Stage_State_Value_Name(StringClass &name, D3DTEXTURESTAGESTATETYPE state, unsigned value);
    static void Get_DX8_Render_State_Value_Name(StringClass &name, D3DRENDERSTATETYPE state, unsigned value);
    static void Set_Viewport(CONST D3DVIEWPORT8 *pViewport);
    static void Set_Light(unsigned int index, const D3DLIGHT8 *light);
    static void Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4 &m);
    static void Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix3D &m);
    static void Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4 &m);
    static void Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value);
    static void Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value);
    static void Set_DX8_Texture(unsigned stage, w3dbasetexture_t texture);
    static IDirect3DDevice8 *Get_D3D_Device8() { return s_d3dDevice; }
    static IDirect3DSurface8 *Get_DX8_Front_Buffer();
    static void Copy_DX8_Rects(IDirect3DSurface8 *pSourceSurface,
        CONST RECT *pSourceRectsArray,
        UINT cRects,
        IDirect3DSurface8 *pDestinationSurface,
        CONST POINT *pDestPointsArray);
    static Vector4 Convert_Color(unsigned color);
    static unsigned int Convert_Color(const Vector4 &color);
    static unsigned int Convert_Color(const Vector3 &color, const float alpha);
    static void Clamp_Color(Vector4 &color);
    static unsigned int Convert_Color_Clamp(const Vector4 &color);
    static void Set_DX8_Light(int index, D3DLIGHT8 *light);
    static void Get_DX8_Transform(D3DTRANSFORMSTATETYPE transform, D3DMATRIX &m);
    static void Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform, const D3DMATRIX &m);
    static void Set_Projection_Transform_With_Z_Bias(const Matrix4 &matrix, float znear, float zfar);
    static void Set_DX8_Material(const D3DMATERIAL8 *mat);
    static void Set_Vertex_Shader(unsigned long shader);
    static void Set_Pixel_Shader(unsigned long shader);
    static void Set_Ambient_Color(const Vector3 &color);
    static bool Get_Fog_Enable() { return s_fogEnable; }
    static D3DCOLOR Get_Fog_Color() { return s_fogColor; }
    static void Set_World_Identity();
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
#ifdef BUILD_WITH_D3D8
    static void Log_DX8_ErrorCode(HRESULT error);
    static void Handle_DX8_ErrorCode(HRESULT error);
#endif

    static int Get_Main_Thread_ID() { return s_mainThreadID; }
    static const DX8Caps *Get_Current_Caps()
    {
        captainslog_assert(s_currentCaps != nullptr);
        return s_currentCaps;
    }
    static bool Has_Stencil();
    static WW3DFormat Get_Back_Buffer_Format();
    static const w3dadapterid_t &Get_Current_Adapter_Identifier() { return s_currentAdapterIdentifier; }
    static bool Reset_Device(bool reacquire);
    static void Invalidate_Cached_Render_States();
    static void Enable_Triangle_Draw(bool enable) { s_EnableTriangleDraw = enable; }
    static bool Is_Triangle_Draw_Enabled() { return s_EnableTriangleDraw; }
    static bool Is_Initted() { return s_isInitialised; }
    static void Set_Texture_Bitdepth(int depth)
    {
        captainslog_assert(depth == 16 || depth == 32);
        s_textureBitDepth = depth;
    }

    static void Set_Cleanup_Hook(DX8_CleanupHook *hook) { s_cleanupHook = hook; }

private:
    static void Draw_Sorting_IB_VB(unsigned int primitive_type,
        unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index,
        unsigned short vertex_count);
    static void Draw(unsigned int primitive_type,
        unsigned short start_index,
        unsigned short polygon_count,
        unsigned short min_vertex_index = 0,
        unsigned short vertex_count = 0);

protected:
    static bool Create_Device();
    static void Release_Device();
    static void Reset_Statistics();
    static void Enumerate_Devices();
    static void Set_Default_Global_Render_States();
    static int Get_Render_Device();
    static int Get_Render_Device_Count();
    static const char *Get_Render_Device_Name(int device_index);
    static const RenderDeviceDescClass &Get_Render_Device_Desc(int deviceidx);
    static bool Set_Device_Resolution(
        int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false);
    static void Get_Device_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed);
    static void Get_Render_Target_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed);
    static bool Set_Render_Device(int dev = -1,
        int resx = -1,
        int resy = -1,
        int bits = -1,
        int windowed = -1,
        bool resize_window = false,
        bool reset_device = false,
        bool restore_assets = true);
#ifdef BUILD_WITH_D3D8
    static bool Find_Color_And_Z_Mode(
        int resx, int resy, int bitdepth, D3DFORMAT *set_colorbuffer, D3DFORMAT *set_backbuffer, D3DFORMAT *set_zmode);
    static bool Find_Color_Mode(D3DFORMAT colorbuffer, int resx, int resy, UINT *mode);
    static bool Find_Z_Mode(D3DFORMAT colorbuffer, D3DFORMAT backbuffer, D3DFORMAT *zmode);
    static bool Test_Z_Mode(D3DFORMAT colorbuffer, D3DFORMAT backbuffer, D3DFORMAT zmode);
    static void Compute_Caps(WW3DFormat display_format);
    static void Get_Format_Name(unsigned int format, StringClass *format_name);
#endif

#ifdef GAME_DLL
#ifdef BUILD_WITH_D3D8
    static IDirect3D8 *(__stdcall *&s_d3dCreateFunction)(unsigned);
    static HMODULE &s_d3dLib;
    static IDirect3D8 *&s_d3dInterface;
    static IDirect3DDevice8 *&s_d3dDevice;
    static D3DMATRIX &s_oldPrj;
    static D3DMATRIX &s_oldView;
    static D3DMATRIX &s_oldWorld;
    static D3DPRESENT_PARAMETERS &s_presentParameters;
    static D3DCOLOR &s_fogColor;
    static D3DFORMAT &s_displayFormat;
#endif
    static ARRAY_DEC(w3dbasetexture_t, s_textures, MAX_TEXTURE_STAGES);
#ifdef PLATFORM_WINDOWS
    static HWND &s_hwnd;
#endif
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
#ifdef BUILD_WITH_D3D8
    static D3DMATRIX &s_projectionMatrix;
#else
    static Matrix4 &s_projectionMatrix;
#endif
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
    static w3dadapterid_t &s_currentAdapterIdentifier;
#ifdef BUILD_WITH_D3D8
    static ARRAY_DEC(D3DMATRIX, s_DX8Transforms, D3DTS_WORLD1);
#else
    static ARRAY_DEC(Matrix4, s_DX8Transforms, 257);
#endif
    static bool &s_EnableTriangleDraw;
    static int &s_ZBias;
    static Vector3 &s_ambientColor;
    static bool &s_isDeviceLost;
    static int &s_FPUPreserve;
    static unsigned long &s_vertexShader;
    static unsigned long &s_pixelShader;
    static LightEnvironmentClass *&s_lightEnvironment;
    static unsigned long &s_vertexProcessingBehavior;
    static bool &s_fogEnable;
    static w3dsurface_t &s_currentRenderTarget;
    static w3dsurface_t &s_currentDepthBuffer;
    static w3dsurface_t &s_defaultRenderTarget;
    static w3dsurface_t &s_defaultDepthBuffer;
    static bool &s_isRenderToTexture;
    static unsigned int &s_drawPolygonLowBoundLimit;
    static unsigned long &s_frameCount;
    static bool &s_DX8SingleThreaded;
    static DX8_CleanupHook *&s_cleanupHook;
#else
#ifdef BUILD_WITH_D3D8
    static IDirect3D8 *(__stdcall *s_d3dCreateFunction)(unsigned);
    static HMODULE s_d3dLib;
    static IDirect3D8 *s_d3dInterface;
    static IDirect3DDevice8 *s_d3dDevice;
    static D3DMATRIX s_oldPrj;
    static D3DMATRIX s_oldView;
    static D3DMATRIX s_oldWorld;
    static D3DPRESENT_PARAMETERS s_presentParameters;
    static D3DCOLOR s_fogColor;
    static D3DFORMAT s_displayFormat;
#endif
#ifdef PLATFORM_WINDOWS
    static HWND s_hwnd;
#endif
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
#ifdef BUILD_WITH_D3D8
    static D3DMATRIX s_projectionMatrix;
#else
    static Matrix4 s_projectionMatrix;
#endif
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
    static w3dadapterid_t s_currentAdapterIdentifier;
#ifdef BUILD_WITH_D3D8
    static D3DMATRIX s_DX8Transforms[D3DTS_WORLD1];
#else
    static Matrix4 s_DX8Transforms[257];
#endif
    static bool s_EnableTriangleDraw;
    static int s_ZBias;
    static Vector3 s_ambientColor;
    static bool s_isDeviceLost;
    static int s_FPUPreserve;
    static unsigned long s_vertexShader;
    static unsigned long s_pixelShader;
    static LightEnvironmentClass *s_lightEnvironment;
    static unsigned long s_vertexProcessingBehavior;
    static bool s_fogEnable;
    static w3dsurface_t s_currentRenderTarget;
    static w3dsurface_t s_currentDepthBuffer;
    static w3dsurface_t s_defaultRenderTarget;
    static w3dsurface_t s_defaultDepthBuffer;
    static bool s_isRenderToTexture;
    static unsigned int s_drawPolygonLowBoundLimit;
    static unsigned long s_frameCount;
    static bool s_DX8SingleThreaded;
    static DX8_CleanupHook *s_cleanupHook;
#endif
    friend class W3D;
    friend class DX8IndexBufferClass;
    friend class DX8VertexBufferClass;
};

inline RenderStateStruct::RenderStateStruct() : shader(), material(nullptr), index_buffer(nullptr)
{
    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        Textures[i] = nullptr;
    }
    for (int i = 0; i < VERTEX_BUFFERS; ++i) {
        vertex_buffers[i] = nullptr;
    }
    // #BUGFIX Initialize all members
    for (int i = 0; i < GFX_LIGHT_COUNT; ++i) {
        LightEnable[i] = false;
    }
#ifdef BUILD_WITH_D3D8
    for (int i = 0; i < GFX_LIGHT_COUNT; ++i) {
        Lights[i] = D3DLIGHT8{};
    }
    D3DXMatrixIdentity(&world);
    D3DXMatrixIdentity(&view);
#else
    world.Make_Identity();
    view.Make_Identity();
#endif
    for (int i = 0; i < VERTEX_BUFFERS; ++i) {
        vertex_buffer_types[i] = 0;
    }
    index_buffer_type = 0;
    vba_offset = 0;
    vba_count = 0;
    iba_offset = 0;
    index_base_offset = 0;
}

inline RenderStateStruct::~RenderStateStruct()
{
    Ref_Ptr_Release(material);
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        Ref_Ptr_Release(vertex_buffers[i]);
    }
    Ref_Ptr_Release(index_buffer);
    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        Ref_Ptr_Release(Textures[i]);
    }
}

inline RenderStateStruct &RenderStateStruct::operator=(const RenderStateStruct &src)
{
    auto *mb = const_cast<VertexMaterialClass *>(src.material);
    Ref_Ptr_Set(material, mb);
    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        auto *vb = const_cast<VertexBufferClass *>(src.vertex_buffers[i]);
        Ref_Ptr_Set(vertex_buffers[i], vb);
    }

    auto *ib = const_cast<IndexBufferClass *>(src.index_buffer);
    Ref_Ptr_Set(index_buffer, ib);
    for (unsigned i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        auto *t = const_cast<TextureClass *>(src.Textures[i]);
        Ref_Ptr_Set(Textures[i], t);
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
inline void DX8Wrapper::Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value)
{
    if (s_renderStates[state] == value) {
        return;
    }

    s_renderStates[state] = value;
    DX8CALL(SetRenderState(state, value));
}

inline void DX8Wrapper::Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value)
{
    if (stage < 8) {
        if (s_textureStageStates[stage][state] == value) {
            return;
        }

        s_textureStageStates[stage][state] = value;
        DX8CALL(SetTextureStageState(stage, state, value));
        ++s_textureStageStateChanges;
    } else {
        DX8CALL(SetTextureStageState(stage, state, value));
    }
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

    if (s_textures[stage]) {
        s_textures[stage]->AddRef();
    }

    DX8CALL(SetTexture(stage, texture));
    ++s_textureChanges;
}

inline void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4 &m)
{
    switch (transform) {
        case D3DTS_WORLD:
            Thyme::To_D3DMATRIX(s_renderState.world, m);
            s_renderStateChanged |= WORLD_CHANGED;
            s_renderStateChanged &= ~WORLD_IDENTITY;
            break;
        case D3DTS_VIEW:
            Thyme::To_D3DMATRIX(s_renderState.view, m);
            s_renderStateChanged |= VIEW_CHANGED;
            s_renderStateChanged &= ~VIEW_IDENTITY;
            break;
        case D3DTS_PROJECTION: {
            D3DMATRIX dxm;
            Thyme::To_D3DMATRIX(dxm, m);
            s_zFar = 0.0f;
            s_zNear = 0.0f;
            DX8CALL(SetTransform(D3DTS_PROJECTION, &dxm));
        } break;
        default:
            s_matrixChanges++;
            D3DMATRIX dxm;
            Thyme::To_D3DMATRIX(dxm, m);
            DX8CALL(SetTransform(transform, &dxm));
            break;
    }
}

inline void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix3D &m)
{
    Matrix4 m4(m);
    switch (transform) {
        case D3DTS_WORLD:
            Thyme::To_D3DMATRIX(s_renderState.world, m4);
            s_renderStateChanged |= (unsigned)WORLD_CHANGED;
            s_renderStateChanged &= ~(unsigned)WORLD_IDENTITY;
            break;
        case D3DTS_VIEW:
            Thyme::To_D3DMATRIX(s_renderState.view, m4);
            s_renderStateChanged |= (unsigned)VIEW_CHANGED;
            s_renderStateChanged &= ~(unsigned)VIEW_IDENTITY;
            break;
        default:
            s_matrixChanges++;
            D3DMATRIX dxm;
            Thyme::To_D3DMATRIX(dxm, m4);
            DX8CALL(SetTransform(transform, &dxm));
            break;
    }
}

inline void DX8Wrapper::Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4 &m)
{
    switch (transform) {
        case D3DTS_WORLD:
            if (s_renderStateChanged & WORLD_IDENTITY)
                m.Make_Identity();
            else
                Thyme::To_Matrix4(m, s_renderState.world);
            break;
        case D3DTS_VIEW:
            if (s_renderStateChanged & VIEW_IDENTITY)
                m.Make_Identity();
            else
                Thyme::To_Matrix4(m, s_renderState.view);
            break;
        default:
            D3DMATRIX dxm;
            DX8CALL(GetTransform(transform, &dxm));
            Thyme::To_Matrix4(m, dxm);
            break;
    }
}

inline void DX8Wrapper::Set_World_Identity()
{
    if (s_renderStateChanged & (unsigned)WORLD_IDENTITY) {
        return;
    }

    D3DXMatrixIdentity(&s_renderState.world);
    s_renderStateChanged |= (unsigned)WORLD_CHANGED | (unsigned)WORLD_IDENTITY;
}

inline void DX8Wrapper::Handle_DX8_ErrorCode(HRESULT error)
{
    if (FAILED(error)) {
        DX8Wrapper::Log_DX8_ErrorCode(error);
    }
}
#endif

inline void DX8Wrapper::Set_Texture(unsigned stage, TextureClass *texture)
{
    if (texture == s_renderState.Textures[stage]) {
        return;
    }
    Ref_Ptr_Set(s_renderState.Textures[stage], texture);
    s_renderStateChanged |= (TEXTURE0_CHANGED << stage);
}

inline void DX8Wrapper::Set_Material(const VertexMaterialClass *material)
{
    if (material == s_renderState.material) {
        return;
    }
    VertexMaterialClass *v = const_cast<VertexMaterialClass *>(material);
    Ref_Ptr_Set(s_renderState.material, v);
    s_renderStateChanged |= MATERIAL_CHANGED;
}

inline void DX8Wrapper::Set_Shader(const ShaderClass &shader)
{
    if (!ShaderClass::s_shaderDirty && (shader == s_renderState.shader)) {
        return;
    }
    s_renderState.shader = shader;
    s_renderStateChanged |= SHADER_CHANGED;
}

#ifdef BUILD_WITH_D3D8
inline Vector4 DX8Wrapper::Convert_Color(unsigned color)
{
    Vector4 col;
    col[3] = ((color & 0xff000000) >> 24) / 255.0f;
    col[0] = ((color & 0xff0000) >> 16) / 255.0f;
    col[1] = ((color & 0xff00) >> 8) / 255.0f;
    col[2] = ((color & 0xff) >> 0) / 255.0f;
    return col;
}

inline unsigned int DX8Wrapper::Convert_Color(const Vector3 &color, const float alpha)
{
    return D3DCOLOR_COLORVALUE(color.X, color.Y, color.Z, alpha);
}

inline unsigned int DX8Wrapper::Convert_Color(const Vector4 &color)
{
    return D3DCOLOR_COLORVALUE(color.X, color.Y, color.Z, color.W);
}

inline void DX8Wrapper::Clamp_Color(Vector4 &color)
{
    color.X = std::clamp(color.X, 0.f, 1.f);
    color.Y = std::clamp(color.Y, 0.f, 1.f);
    color.Z = std::clamp(color.Z, 0.f, 1.f);
    color.W = std::clamp(color.W, 0.f, 1.f);
}

inline unsigned int DX8Wrapper::Convert_Color_Clamp(const Vector4 &color)
{
    Vector4 clamped_color = color;
    DX8Wrapper::Clamp_Color(clamped_color);
    return Convert_Color(clamped_color);
}

inline void DX8Wrapper::Set_Light(unsigned int index, const D3DLIGHT8 *light)
{
    if (light) {
        s_renderState.Lights[index] = *light;
        s_renderState.LightEnable[index] = true;
    } else {
        s_renderState.LightEnable[index] = false;
    }
    s_renderStateChanged |= (LIGHT0_CHANGED << index);
}

inline void DX8Wrapper::Set_DX8_Light(int index, D3DLIGHT8 *light)
{
    if (light) {
        s_lightChanges++;
        DX8CALL(SetLight(index, light));
        DX8CALL(LightEnable(index, TRUE));
        s_currentLightEnables[index] = true;
    } else if (s_currentLightEnables[index]) {
        s_lightChanges++;
        s_currentLightEnables[index] = false;
        DX8CALL(LightEnable(index, FALSE));
    }
}

inline void DX8Wrapper::Get_DX8_Transform(D3DTRANSFORMSTATETYPE transform, D3DMATRIX &m)
{
    DX8CALL(GetTransform(transform, &m));
}

inline void DX8Wrapper::Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform, const D3DMATRIX &m)
{
    captainslog_assert(transform <= D3DTS_WORLD);
    s_DX8Transforms[transform] = m;
    s_matrixChanges++;
    DX8CALL(SetTransform(transform, &m));
}

inline void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const Matrix4 &matrix, float znear, float zfar)
{
    s_zFar = zfar;
    s_zNear = znear;
    Thyme::To_D3DMATRIX(s_projectionMatrix, matrix);

    if (!Get_Current_Caps()->Supports_ZBias() && s_zNear != s_zFar) {
        D3DMATRIX tmp = s_projectionMatrix;
        float tmp_zbias = s_ZBias;
        tmp_zbias *= (1.0f / 16.0f);
        tmp_zbias *= 1.0f / (s_zFar - s_zNear);
        tmp.m[2][2] -= tmp_zbias * tmp.m[3][2];
        DX8CALL(SetTransform(D3DTS_PROJECTION, &tmp));
    } else {
        DX8CALL(SetTransform(D3DTS_PROJECTION, &s_projectionMatrix));
    }
}

inline void DX8Wrapper::Set_DX8_Material(const D3DMATERIAL8 *mat)
{
    s_materialChanges++;
    DX8CALL(SetMaterial(mat));
}

inline void DX8Wrapper::Set_Fog(bool enable, const Vector3 &color, float start, float end)
{
    s_fogEnable = enable;
    s_fogColor = Convert_Color(color, 0.0f);

    ShaderClass::Invalidate();

    Set_DX8_Render_State(D3DRS_FOGSTART, *reinterpret_cast<DWORD *>(&start));
    Set_DX8_Render_State(D3DRS_FOGEND, *reinterpret_cast<DWORD *>(&end));
}

inline void DX8Wrapper::Set_Ambient_Color(const Vector3 &color)
{
    s_ambientColor = color;
    Set_DX8_Render_State(D3DRS_AMBIENT, Convert_Color(color, 1.0f));
}

inline void DX8Wrapper::Copy_DX8_Rects(IDirect3DSurface8 *pSourceSurface,
    CONST RECT *pSourceRectsArray,
    UINT cRects,
    IDirect3DSurface8 *pDestinationSurface,
    CONST POINT *pDestPointsArray)
{
    DX8CALL(CopyRects(pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray));
}

inline void DX8Wrapper::Set_Vertex_Shader(unsigned long shader)
{
    s_vertexShader = shader;
    Get_D3D_Device8()->SetVertexShader(shader);
}

inline void DX8Wrapper::Set_Pixel_Shader(unsigned long shader)
{
    if (s_pixelShader != shader) {
        s_pixelShader = shader;
        Get_D3D_Device8()->SetPixelShader(shader);
    }
}

#endif

inline void DX8Wrapper::Set_Index_Buffer_Index_Offset(unsigned offset)
{
    if (s_renderState.index_base_offset == offset) {
        return;
    }

    s_renderState.index_base_offset = offset;
    s_renderStateChanged |= INDEX_BUFFER_CHANGED;
}

inline void DX8Wrapper::Get_Render_State(RenderStateStruct &state)
{
    state = s_renderState;
}

inline void DX8Wrapper::Set_Render_State(const RenderStateStruct &state)
{
    if (s_renderState.index_buffer) {
        s_renderState.index_buffer->Release_Engine_Ref();
    }

    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        if (s_renderState.vertex_buffers[i]) {
            s_renderState.vertex_buffers[i]->Release_Engine_Ref();
        }
    }

    s_renderState = state;
    s_renderStateChanged = 0xffffffff;

    if (s_renderState.index_buffer) {
        s_renderState.index_buffer->Add_Engine_Ref();
    }

    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        if (s_renderState.vertex_buffers[i]) {
            s_renderState.vertex_buffers[i]->Add_Engine_Ref();
        }
    }
}

inline void DX8Wrapper::Release_Render_State()
{
    if (s_renderState.index_buffer) {
        s_renderState.index_buffer->Release_Engine_Ref();
    }

    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        if (s_renderState.vertex_buffers[i]) {
            s_renderState.vertex_buffers[i]->Release_Engine_Ref();
        }
    }

    for (int i = 0; i < VERTEX_BUFFERS; i++) {
        if (s_renderState.vertex_buffers[i]) {
            Ref_Ptr_Release(s_renderState.vertex_buffers[i]);
        }
    }

    Ref_Ptr_Release(s_renderState.index_buffer);
    Ref_Ptr_Release(s_renderState.material);

    for (int i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        Ref_Ptr_Release(s_renderState.Textures[i]);
    }
}
