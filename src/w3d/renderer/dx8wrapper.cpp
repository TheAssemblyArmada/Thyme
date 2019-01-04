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
#include "dx8wrapper.h"
#include "thread.h"
#include <cstring>

using std::memcpy;
using std::memset;

#ifndef THYME_STANDALONE
uint32_t &g_numberOfDx8Calls = Make_Global<uint32_t>(0x00A47F40);
#else
uint32_t g_numberOfDx8Calls = 0;
#endif

#ifndef THYME_STANDALONE
IDirect3D8 *(__stdcall *&DX8Wrapper::s_d3dCreateFunction)(unsigned) = Make_Global<IDirect3D8 *(__stdcall *)(unsigned)>(
    0x00A47F6C);
HMODULE &DX8Wrapper::s_d3dLib = Make_Global<HMODULE>(0x00A47F70);
IDirect3D8 *&DX8Wrapper::s_d3dInterface = Make_Global<IDirect3D8 *>(0x00A47EEC);
IDirect3DDevice8 *&DX8Wrapper::s_d3dDevice = Make_Global<IDirect3DDevice8 *>(0x00A47EF0);
IDirect3DBaseTexture8 **DX8Wrapper::s_textures = Make_Pointer<w3dbasetexture_t>(0x00A42784);
void *&DX8Wrapper::s_shadowMap = Make_Global<void *>(0x00A47EBC);
void *&DX8Wrapper::s_hwnd = Make_Global<void *>(0x00A47EBC);
unsigned *DX8Wrapper::s_renderStates = Make_Pointer<unsigned>(0x00A46CC0);
unsigned *DX8Wrapper::s_textureStageStates = Make_Pointer<unsigned>(0x00A46CC0);
Vector4 *DX8Wrapper::s_vertexShaderConstants = Make_Pointer<Vector4>(0x00A47778);
unsigned *DX8Wrapper::s_pixelShaderConstants = Make_Pointer<unsigned>(0x00A427C0);
bool &DX8Wrapper::s_isInitialised = Make_Global<bool>(0x00A47EC8);
bool &DX8Wrapper::s_isWindowed = Make_Global<bool>(0x00A47EC9);
RenderStateStruct &DX8Wrapper::s_renderState = Make_Global<RenderStateStruct>(0x00A47508);
unsigned &DX8Wrapper::s_renderStateChanged = Make_Global<unsigned>(0x00A42778);
float &DX8Wrapper::s_zNear = Make_Global<float>(0x00A47E38);
float &DX8Wrapper::s_zFar = Make_Global<float>(0x00A47EB8);
Matrix4 &DX8Wrapper::s_projectionMatrix = Make_Global<Matrix4>(0x00A47DF8);
int &DX8Wrapper::s_mainThreadID = Make_Global<int>(0x00A47F2C);
int &DX8Wrapper::s_currentRenderDevice = Make_Global<int>(0x00898BC4);
DX8Caps *&DX8Wrapper::s_currentCaps = Make_Global<DX8Caps *>(0x007F1C6C);
#else
#ifdef BUILD_WITH_D3D8
IDirect3D8 *(__stdcall *DX8Wrapper::s_d3dCreateFunction)(unsigned) = nullptr;
HMODULE DX8Wrapper::s_d3dLib = nullptr;
IDirect3D8 *DX8Wrapper::s_d3dInterface;
IDirect3DDevice8 *DX8Wrapper::s_d3dDevice;
#endif
void *DX8Wrapper::s_shadowMap;
void *DX8Wrapper::s_hwnd;
unsigned DX8Wrapper::s_renderStates[256];
unsigned DX8Wrapper::s_textureStageStates[MAX_TEXTURE_STAGES][32];
w3dbasetexture_t DX8Wrapper::s_textures[MAX_TEXTURE_STAGES];
Vector4 DX8Wrapper::s_vertexShaderConstants[96]; // Not 100% sure this is a Vector4 array
unsigned DX8Wrapper::s_pixelShaderConstants[32]; // Not 100% on type, seems unused.
bool DX8Wrapper::s_isInitialised;
bool DX8Wrapper::s_isWindowed;
RenderStateStruct DX8Wrapper::s_renderState;
unsigned DX8Wrapper::s_renderStateChanged;
float DX8Wrapper::s_zNear;
float DX8Wrapper::s_zFar;
Matrix4 DX8Wrapper::s_projectionMatrix;
int DX8Wrapper::s_mainThreadID;
int DX8Wrapper::s_currentRenderDevice = -1;
DX8Caps *DX8Wrapper::s_currentCaps;
#endif

void DX8Wrapper::Init(void *hwnd, bool lite)
{
#ifndef THYME_STANDALONE
    Call_Function<void, void *, bool>(0x00800670, hwnd, lite);
#else
#ifndef THYME_STANDALONE
    memset(s_textures, 0, sizeof(*s_textures) * MAX_TEXTURE_STAGES);
    memset(s_renderStates, 0, sizeof(*s_renderStates) * 256);
    memset(s_textureStageStates, 0, (sizeof(*s_textureStageStates) * 32) * MAX_TEXTURE_STAGES);
    memset(s_vertexShaderConstants, 0, sizeof(*s_vertexShaderConstants) * 96);
    memset(s_pixelShaderConstants, 0, sizeof(*s_pixelShaderConstants) * 32);
#else
    memset(s_textures, 0, sizeof(s_textures));
    memset(s_renderStates, 0, sizeof(s_renderStates));
    memset(s_textureStageStates, 0, sizeof(s_textureStageStates));
    memset(s_vertexShaderConstants, 0, sizeof(s_vertexShaderConstants));
    memset(s_pixelShaderConstants, 0, sizeof(s_pixelShaderConstants));
#endif
    memset(&s_renderState, 0, sizeof(s_renderState));
    s_shadowMap = nullptr;
    s_hwnd = hwnd;
    s_mainThreadID = ThreadClass::Get_Current_Thread_ID(); // Init only called from main thread so this is fine.
    s_currentRenderDevice = -1;
#endif
}

void DX8Wrapper::Shutdown()
{
#ifndef THYME_STANDALONE
    Call_Function<void>(0x00800860);
#endif
}

void DX8Wrapper::Log_DX8_ErrorCode(unsigned error)
{
    // This made use the d3d8x part of the sdk found in the DirectX 8.1 SDK which is hard to find.
    DEBUG_LOG("Direct3D8 generated error %x.\n", error);
}

w3dtexture_t DX8Wrapper::Create_Texture(
    unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, w3dpool_t pool, bool rendertarget)
{
#ifndef THYME_STANDALONE
    return Call_Function<w3dtexture_t, unsigned, unsigned, WW3DFormat, MipCountType, w3dpool_t, bool>(
        0x008036F0, width, height, format, mip_level_count, pool, rendertarget);
#else
    return w3dtexture_t();
#endif
}

w3dsurface_t DX8Wrapper::Create_Surface(unsigned width, unsigned height, WW3DFormat format)
{
#ifdef BUILD_WITH_D3D8
    w3dsurface_t surf = W3D_TYPE_INVALID_SURFACE;
    DX8CALL(CreateImageSurface(width, height, (D3DFORMAT)WW3DFormat_To_D3DFormat(format), &surf));

    return surf;
#else
    return w3dsurface_t();
#endif
}

#ifdef BUILD_WITH_D3D8
// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Op_Name(unsigned value)
{
    switch (value) {
        case D3DTOP_DISABLE:
            return "D3DTOP_DISABLE";
        case D3DTOP_SELECTARG1:
            return "D3DTOP_SELECTARG1";
        case D3DTOP_SELECTARG2:
            return "D3DTOP_SELECTARG2";
        case D3DTOP_MODULATE:
            return "D3DTOP_MODULATE";
        case D3DTOP_MODULATE2X:
            return "D3DTOP_MODULATE2X";
        case D3DTOP_MODULATE4X:
            return "D3DTOP_MODULATE4X";
        case D3DTOP_ADD:
            return "D3DTOP_ADD";
        case D3DTOP_ADDSIGNED:
            return "D3DTOP_ADDSIGNED";
        case D3DTOP_ADDSIGNED2X:
            return "D3DTOP_ADDSIGNED2X";
        case D3DTOP_SUBTRACT:
            return "D3DTOP_SUBTRACT";
        case D3DTOP_ADDSMOOTH:
            return "D3DTOP_ADDSMOOTH";
        case D3DTOP_BLENDDIFFUSEALPHA:
            return "D3DTOP_BLENDDIFFUSEALPHA";
        case D3DTOP_BLENDTEXTUREALPHA:
            return "D3DTOP_BLENDTEXTUREALPHA";
        case D3DTOP_BLENDFACTORALPHA:
            return "D3DTOP_BLENDFACTORALPHA";
        case D3DTOP_BLENDTEXTUREALPHAPM:
            return "D3DTOP_BLENDTEXTUREALPHAPM";
        case D3DTOP_BLENDCURRENTALPHA:
            return "D3DTOP_BLENDCURRENTALPHA";
        case D3DTOP_PREMODULATE:
            return "D3DTOP_PREMODULATE";
        case D3DTOP_MODULATEALPHA_ADDCOLOR:
            return "D3DTOP_MODULATEALPHA_ADDCOLOR";
        case D3DTOP_MODULATECOLOR_ADDALPHA:
            return "D3DTOP_MODULATECOLOR_ADDALPHA";
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
            return "D3DTOP_MODULATEINVALPHA_ADDCOLOR";
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
            return "D3DTOP_MODULATEINVCOLOR_ADDALPHA";
        case D3DTOP_BUMPENVMAP:
            return "D3DTOP_BUMPENVMAP";
        case D3DTOP_BUMPENVMAPLUMINANCE:
            return "D3DTOP_BUMPENVMAPLUMINANCE";
        case D3DTOP_DOTPRODUCT3:
            return "D3DTOP_DOTPRODUCT3";
        case D3DTOP_MULTIPLYADD:
            return "D3DTOP_MULTIPLYADD";
        case D3DTOP_LERP:
            return "D3DTOP_LERP";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Arg_Name(unsigned value)
{
    switch (value) {
        case D3DTA_CURRENT:
            return "D3DTA_CURRENT";
        case D3DTA_DIFFUSE:
            return "D3DTA_DIFFUSE";
        case D3DTA_SELECTMASK:
            return "D3DTA_SELECTMASK";
        case D3DTA_SPECULAR:
            return "D3DTA_SPECULAR";
        case D3DTA_TEMP:
            return "D3DTA_TEMP";
        case D3DTA_TEXTURE:
            return "D3DTA_TEXTURE";
        case D3DTA_TFACTOR:
            return "D3DTA_TFACTOR";
        case D3DTA_ALPHAREPLICATE:
            return "D3DTA_ALPHAREPLICATE";
        case D3DTA_COMPLEMENT:
            return "D3DTA_COMPLEMENT";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Filter_Name(unsigned value)
{
    switch (value) {
        case D3DTEXF_NONE:
            return "D3DTEXF_NONE";
        case D3DTEXF_POINT:
            return "D3DTEXF_POINT";
        case D3DTEXF_LINEAR:
            return "D3DTEXF_LINEAR";
        case D3DTEXF_ANISOTROPIC:
            return "D3DTEXF_ANISOTROPIC";
        case D3DTEXF_FLATCUBIC:
            return "D3DTEXF_FLATCUBIC";
        case D3DTEXF_GAUSSIANCUBIC:
            return "D3DTEXF_GAUSSIANCUBIC";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Address_Name(unsigned value)
{
    switch (value) {
        case D3DTADDRESS_WRAP:
            return "D3DTADDRESS_WRAP";
        case D3DTADDRESS_MIRROR:
            return "D3DTADDRESS_MIRROR";
        case D3DTADDRESS_CLAMP:
            return "D3DTADDRESS_CLAMP";
        case D3DTADDRESS_BORDER:
            return "D3DTADDRESS_BORDER";
        case D3DTADDRESS_MIRRORONCE:
            return "D3DTADDRESS_MIRRORONCE";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Texture_Transform_Flag_Name(unsigned value)
{
    switch (value) {
        case D3DTTFF_DISABLE:
            return "D3DTTFF_DISABLE";
        case D3DTTFF_COUNT1:
            return "D3DTTFF_COUNT1";
        case D3DTTFF_COUNT2:
            return "D3DTTFF_COUNT2";
        case D3DTTFF_COUNT3:
            return "D3DTTFF_COUNT3";
        case D3DTTFF_COUNT4:
            return "D3DTTFF_COUNT4";
        case D3DTTFF_PROJECTED:
            return "D3DTTFF_PROJECTED";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_ZBuffer_Type_Name(unsigned value)
{
    switch (value) {
        case D3DZB_FALSE:
            return "D3DZB_FALSE";
        case D3DZB_TRUE:
            return "D3DZB_TRUE";
        case D3DZB_USEW:
            return "D3DZB_USEW";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Fill_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DFILL_POINT:
            return "D3DFILL_POINT";
        case D3DFILL_WIREFRAME:
            return "D3DFILL_WIREFRAME";
        case D3DFILL_SOLID:
            return "D3DFILL_SOLID";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Shade_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DSHADE_FLAT:
            return "D3DSHADE_FLAT";
        case D3DSHADE_GOURAUD:
            return "D3DSHADE_GOURAUD";
        case D3DSHADE_PHONG:
            return "D3DSHADE_PHONG";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Blend_Name(unsigned value)
{
    switch (value) {
        case D3DBLEND_ZERO:
            return "D3DBLEND_ZERO";
        case D3DBLEND_ONE:
            return "D3DBLEND_ONE";
        case D3DBLEND_SRCCOLOR:
            return "D3DBLEND_SRCCOLOR";
        case D3DBLEND_INVSRCCOLOR:
            return "D3DBLEND_INVSRCCOLOR";
        case D3DBLEND_SRCALPHA:
            return "D3DBLEND_SRCALPHA";
        case D3DBLEND_INVSRCALPHA:
            return "D3DBLEND_INVSRCALPHA";
        case D3DBLEND_DESTALPHA:
            return "D3DBLEND_DESTALPHA";
        case D3DBLEND_INVDESTALPHA:
            return "D3DBLEND_INVDESTALPHA";
        case D3DBLEND_DESTCOLOR:
            return "D3DBLEND_DESTCOLOR";
        case D3DBLEND_INVDESTCOLOR:
            return "D3DBLEND_INVDESTCOLOR";
        case D3DBLEND_SRCALPHASAT:
            return "D3DBLEND_SRCALPHASAT";
        case D3DBLEND_BOTHSRCALPHA:
            return "D3DBLEND_BOTHSRCALPHA";
        case D3DBLEND_BOTHINVSRCALPHA:
            return "D3DBLEND_BOTHINVSRCALPHA";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Cull_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DCULL_NONE:
            return "D3DCULL_NONE";
        case D3DCULL_CW:
            return "D3DCULL_CW";
        case D3DCULL_CCW:
            return "D3DCULL_CCW";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Cmp_Func_Name(unsigned value)
{
    switch (value) {
        case D3DCMP_NEVER:
            return "D3DCMP_NEVER";
        case D3DCMP_LESS:
            return "D3DCMP_LESS";
        case D3DCMP_EQUAL:
            return "D3DCMP_EQUAL";
        case D3DCMP_LESSEQUAL:
            return "D3DCMP_LESSEQUAL";
        case D3DCMP_GREATER:
            return "D3DCMP_GREATER";
        case D3DCMP_NOTEQUAL:
            return "D3DCMP_NOTEQUAL";
        case D3DCMP_GREATEREQUAL:
            return "D3DCMP_GREATEREQUAL";
        case D3DCMP_ALWAYS:
            return "D3DCMP_ALWAYS";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Fog_Mode_Name(unsigned value)
{
    switch (value) {
        case D3DFOG_NONE:
            return "D3DFOG_NONE";
        case D3DFOG_EXP:
            return "D3DFOG_EXP";
        case D3DFOG_EXP2:
            return "D3DFOG_EXP2";
        case D3DFOG_LINEAR:
            return "D3DFOG_LINEAR";
        default:
            return "UNKNOWN";
    }
}

// Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
const char *DX8Wrapper::Get_DX8_Stencil_Op_Name(unsigned value)
{
    switch (value) {
        case D3DSTENCILOP_KEEP:
            return "D3DSTENCILOP_KEEP";
        case D3DSTENCILOP_ZERO:
            return "D3DSTENCILOP_ZERO";
        case D3DSTENCILOP_REPLACE:
            return "D3DSTENCILOP_REPLACE";
        case D3DSTENCILOP_INCRSAT:
            return "D3DSTENCILOP_INCRSAT";
        case D3DSTENCILOP_DECRSAT:
            return "D3DSTENCILOP_DECRSAT";
        case D3DSTENCILOP_INVERT:
            return "D3DSTENCILOP_INVERT";
        case D3DSTENCILOP_INCR:
            return "D3DSTENCILOP_INCR";
        case D3DSTENCILOP_DECR:
            return "D3DSTENCILOP_DECR";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Material_Source_Name(unsigned value)
{
    switch (value) {
        case D3DMCS_MATERIAL:
            return "D3DMCS_MATERIAL";
        case D3DMCS_COLOR1:
            return "D3DMCS_COLOR1";
        case D3DMCS_COLOR2:
            return "D3DMCS_COLOR2";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Vertex_Blend_Flag_Name(unsigned value)
{
    switch (value) {
        case D3DVBF_DISABLE:
            return "D3DVBF_DISABLE";
        case D3DVBF_1WEIGHTS:
            return "D3DVBF_1WEIGHTS";
        case D3DVBF_2WEIGHTS:
            return "D3DVBF_2WEIGHTS";
        case D3DVBF_3WEIGHTS:
            return "D3DVBF_3WEIGHTS";
        case D3DVBF_TWEENING:
            return "D3DVBF_TWEENING";
        case D3DVBF_0WEIGHTS:
            return "D3DVBF_0WEIGHTS";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Patch_Edge_Style_Name(unsigned value)
{
    switch (value) {
        case D3DPATCHEDGE_DISCRETE:
            return "D3DPATCHEDGE_DISCRETE";
        case D3DPATCHEDGE_CONTINUOUS:
            return "D3DPATCHEDGE_CONTINUOUS";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Debug_Monitor_Token_Name(unsigned value)
{
    switch (value) {
        case D3DDMT_ENABLE:
            return "D3DDMT_ENABLE";
        case D3DDMT_DISABLE:
            return "D3DDMT_DISABLE";
        default:
            return "UNKNOWN";
    }
}

const char *DX8Wrapper::Get_DX8_Blend_Op_Name(unsigned value)
{
    switch (value) {
        case D3DBLENDOP_ADD:
            return "D3DBLENDOP_ADD";
        case D3DBLENDOP_SUBTRACT:
            return "D3DBLENDOP_SUBTRACT";
        case D3DBLENDOP_REVSUBTRACT:
            return "D3DBLENDOP_REVSUBTRACT";
        case D3DBLENDOP_MIN:
            return "D3DBLENDOP_MIN";
        case D3DBLENDOP_MAX:
            return "D3DBLENDOP_MAX";
        default:
            return "UNKNOWN";
    }
}
#endif
