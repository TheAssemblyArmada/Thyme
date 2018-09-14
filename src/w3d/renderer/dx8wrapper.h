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
#include "vector4.h"

#ifdef PLATFORM_WINDOWS
#include <d3d8.h>
#endif

//#define o_d3dInterface Make_Global<IDirect3D8 *>(0x00A47EEC);


// This class is going to be very much a WIP until we have a better idea
// of the way it interacts with the rest of the program and what its structure
// is.

class DX8Wrapper
{
    enum
    {
        MAX_TEXTURE_STAGES = 8,
    };

public:
    static void Init(void *hwnd, bool lite = false);
    static void Shutdown();

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
private:
#ifndef THYME_STANDALONE
    static IDirect3D8 *(__stdcall *&s_d3dCreateFunction)(unsigned);
    static HMODULE &s_d3dLib;
    static IDirect3D8 *&s_d3dInterface;
    static IDirect3DBaseTexture8 **s_textures;
    static unsigned *s_renderStates;
    static unsigned *s_textureStageStates;
    static Vector4 *s_vertexShaderConstants;
    static unsigned *s_pixelShaderConstants;
    static bool &s_isInitialised;
    static bool &s_isWindowed;
#else
#ifdef PLATFORM_WINDOWS
    static IDirect3D8 *(__stdcall *s_d3dCreateFunction)(unsigned);
    static HMODULE s_d3dLib;
    static IDirect3D8 *s_d3dInterface;
    static IDirect3DBaseTexture8 *s_textures[MAX_TEXTURE_STAGES];
#endif
    static unsigned s_renderStates[256];
    static unsigned s_textureStageStates[MAX_TEXTURE_STAGES][32];
    static Vector4 s_vertexShaderConstants[96]; // Not 100% sure this is a Vector4 array
    static unsigned s_pixelShaderConstants[32]; // Not 100% on type, seems unused.
    static bool s_isInitialised;
    static bool s_isWindowed;
#endif
};

#ifdef PLATFORM_WINDOWS
//Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Texture_Op_Name(unsigned value)
{
    switch ( value ) {
        case D3DTOP_DISABLE: return "D3DTOP_DISABLE";
        case D3DTOP_SELECTARG1: return "D3DTOP_SELECTARG1";
        case D3DTOP_SELECTARG2: return "D3DTOP_SELECTARG2";
        case D3DTOP_MODULATE: return "D3DTOP_MODULATE";
        case D3DTOP_MODULATE2X: return "D3DTOP_MODULATE2X";
        case D3DTOP_MODULATE4X: return "D3DTOP_MODULATE4X";
        case D3DTOP_ADD: return "D3DTOP_ADD";
        case D3DTOP_ADDSIGNED: return "D3DTOP_ADDSIGNED";
        case D3DTOP_ADDSIGNED2X: return "D3DTOP_ADDSIGNED2X";
        case D3DTOP_SUBTRACT: return "D3DTOP_SUBTRACT";
        case D3DTOP_ADDSMOOTH: return "D3DTOP_ADDSMOOTH";
        case D3DTOP_BLENDDIFFUSEALPHA: return "D3DTOP_BLENDDIFFUSEALPHA";
        case D3DTOP_BLENDTEXTUREALPHA: return "D3DTOP_BLENDTEXTUREALPHA";
        case D3DTOP_BLENDFACTORALPHA: return "D3DTOP_BLENDFACTORALPHA";
        case D3DTOP_BLENDTEXTUREALPHAPM: return "D3DTOP_BLENDTEXTUREALPHAPM";
        case D3DTOP_BLENDCURRENTALPHA: return "D3DTOP_BLENDCURRENTALPHA";
        case D3DTOP_PREMODULATE: return "D3DTOP_PREMODULATE";
        case D3DTOP_MODULATEALPHA_ADDCOLOR: return "D3DTOP_MODULATEALPHA_ADDCOLOR";
        case D3DTOP_MODULATECOLOR_ADDALPHA: return "D3DTOP_MODULATECOLOR_ADDALPHA";
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR: return "D3DTOP_MODULATEINVALPHA_ADDCOLOR";
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA: return "D3DTOP_MODULATEINVCOLOR_ADDALPHA";
        case D3DTOP_BUMPENVMAP: return "D3DTOP_BUMPENVMAP";
        case D3DTOP_BUMPENVMAPLUMINANCE: return "D3DTOP_BUMPENVMAPLUMINANCE";
        case D3DTOP_DOTPRODUCT3: return "D3DTOP_DOTPRODUCT3";
        case D3DTOP_MULTIPLYADD: return "D3DTOP_MULTIPLYADD";
        case D3DTOP_LERP: return "D3DTOP_LERP";
        default: return "UNKNOWN";
    }
}


//Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Texture_Arg_Name(unsigned value)
{
    switch ( value ) {
        case D3DTA_CURRENT: return "D3DTA_CURRENT";
        case D3DTA_DIFFUSE: return "D3DTA_DIFFUSE";
        case D3DTA_SELECTMASK: return "D3DTA_SELECTMASK";
        case D3DTA_SPECULAR: return "D3DTA_SPECULAR";
        case D3DTA_TEMP: return "D3DTA_TEMP";
        case D3DTA_TEXTURE: return "D3DTA_TEXTURE";
        case D3DTA_TFACTOR: return "D3DTA_TFACTOR";
        case D3DTA_ALPHAREPLICATE: return "D3DTA_ALPHAREPLICATE";
        case D3DTA_COMPLEMENT: return "D3DTA_COMPLEMENT";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Texture_Filter_Name(unsigned value)
{
    switch ( value ) {
        case D3DTEXF_NONE: return "D3DTEXF_NONE";
        case D3DTEXF_POINT: return "D3DTEXF_POINT";
        case D3DTEXF_LINEAR: return "D3DTEXF_LINEAR";
        case D3DTEXF_ANISOTROPIC: return "D3DTEXF_ANISOTROPIC";
        case D3DTEXF_FLATCUBIC: return "D3DTEXF_FLATCUBIC";
        case D3DTEXF_GAUSSIANCUBIC: return "D3DTEXF_GAUSSIANCUBIC";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Texture_Address_Name(unsigned value)
{
    switch ( value ) {
        case D3DTADDRESS_WRAP: return "D3DTADDRESS_WRAP";
        case D3DTADDRESS_MIRROR: return "D3DTADDRESS_MIRROR";
        case D3DTADDRESS_CLAMP: return "D3DTADDRESS_CLAMP";
        case D3DTADDRESS_BORDER: return "D3DTADDRESS_BORDER";
        case D3DTADDRESS_MIRRORONCE: return "D3DTADDRESS_MIRRORONCE";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Texture_Transform_Flag_Name(unsigned value)
{
    switch ( value ) {
        case D3DTTFF_DISABLE: return "D3DTTFF_DISABLE";
        case D3DTTFF_COUNT1: return "D3DTTFF_COUNT1";
        case D3DTTFF_COUNT2: return "D3DTTFF_COUNT2";
        case D3DTTFF_COUNT3: return "D3DTTFF_COUNT3";
        case D3DTTFF_COUNT4: return "D3DTTFF_COUNT4";
        case D3DTTFF_PROJECTED: return "D3DTTFF_PROJECTED";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_ZBuffer_Type_Name(unsigned value)
{
    switch ( value ) {
        case D3DZB_FALSE: return "D3DZB_FALSE";
        case D3DZB_TRUE: return "D3DZB_TRUE";
        case D3DZB_USEW: return "D3DZB_USEW";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Fill_Mode_Name(unsigned value)
{
    switch ( value ) {
        case D3DFILL_POINT: return "D3DFILL_POINT";
        case D3DFILL_WIREFRAME: return "D3DFILL_WIREFRAME";
        case D3DFILL_SOLID: return "D3DFILL_SOLID";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Shade_Mode_Name(unsigned value)
{
    switch ( value ) {
        case D3DSHADE_FLAT: return "D3DSHADE_FLAT";
        case D3DSHADE_GOURAUD: return "D3DSHADE_GOURAUD";
        case D3DSHADE_PHONG: return "D3DSHADE_PHONG";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Blend_Name(unsigned value)
{
    switch ( value ) {
        case D3DBLEND_ZERO: return "D3DBLEND_ZERO";
        case D3DBLEND_ONE: return "D3DBLEND_ONE";
        case D3DBLEND_SRCCOLOR: return "D3DBLEND_SRCCOLOR";
        case D3DBLEND_INVSRCCOLOR: return "D3DBLEND_INVSRCCOLOR";
        case D3DBLEND_SRCALPHA: return "D3DBLEND_SRCALPHA";
        case D3DBLEND_INVSRCALPHA: return "D3DBLEND_INVSRCALPHA";
        case D3DBLEND_DESTALPHA: return "D3DBLEND_DESTALPHA";
        case D3DBLEND_INVDESTALPHA: return "D3DBLEND_INVDESTALPHA";
        case D3DBLEND_DESTCOLOR: return "D3DBLEND_DESTCOLOR";
        case D3DBLEND_INVDESTCOLOR: return "D3DBLEND_INVDESTCOLOR";
        case D3DBLEND_SRCALPHASAT: return "D3DBLEND_SRCALPHASAT";
        case D3DBLEND_BOTHSRCALPHA: return "D3DBLEND_BOTHSRCALPHA";
        case D3DBLEND_BOTHINVSRCALPHA: return "D3DBLEND_BOTHINVSRCALPHA";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Cull_Mode_Name(unsigned value)
{
    switch ( value ) {
        case D3DCULL_NONE: return "D3DCULL_NONE";
        case D3DCULL_CW: return "D3DCULL_CW";
        case D3DCULL_CCW: return "D3DCULL_CCW";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Cmp_Func_Name(unsigned value)
{
    switch ( value ) {
        case D3DCMP_NEVER: return "D3DCMP_NEVER";
        case D3DCMP_LESS: return "D3DCMP_LESS";
        case D3DCMP_EQUAL: return "D3DCMP_EQUAL";
        case D3DCMP_LESSEQUAL: return "D3DCMP_LESSEQUAL";
        case D3DCMP_GREATER: return "D3DCMP_GREATER";
        case D3DCMP_NOTEQUAL: return "D3DCMP_NOTEQUAL";
        case D3DCMP_GREATEREQUAL: return "D3DCMP_GREATEREQUAL";
        case D3DCMP_ALWAYS: return "D3DCMP_ALWAYS";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Fog_Mode_Name(unsigned value)
{
    switch ( value ) {
        case D3DFOG_NONE: return "D3DFOG_NONE";
        case D3DFOG_EXP: return "D3DFOG_EXP";
        case D3DFOG_EXP2: return "D3DFOG_EXP2";
        case D3DFOG_LINEAR: return "D3DFOG_LINEAR";
        default: return "UNKNOWN";
    }
}

//Inlined in DX8Wrapper::Get_DX8_Render_State_Value_Name in ZH
inline const char *DX8Wrapper::Get_DX8_Stencil_Op_Name(unsigned value)
{
    switch ( value ) {
        case D3DSTENCILOP_KEEP: return "D3DSTENCILOP_KEEP";
        case D3DSTENCILOP_ZERO: return "D3DSTENCILOP_ZERO";
        case D3DSTENCILOP_REPLACE: return "D3DSTENCILOP_REPLACE";
        case D3DSTENCILOP_INCRSAT: return "D3DSTENCILOP_INCRSAT";
        case D3DSTENCILOP_DECRSAT: return "D3DSTENCILOP_DECRSAT";
        case D3DSTENCILOP_INVERT: return "D3DSTENCILOP_INVERT";
        case D3DSTENCILOP_INCR: return "D3DSTENCILOP_INCR";
        case D3DSTENCILOP_DECR: return "D3DSTENCILOP_DECR";
        default: return "UNKNOWN";
    }
}

inline const char *DX8Wrapper::Get_DX8_Material_Source_Name(unsigned value)
{
    switch ( value ) {
        case D3DMCS_MATERIAL: return "D3DMCS_MATERIAL";
        case D3DMCS_COLOR1: return "D3DMCS_COLOR1";
        case D3DMCS_COLOR2: return "D3DMCS_COLOR2";
        default: return "UNKNOWN";
    }
}

inline const char *DX8Wrapper::Get_DX8_Vertex_Blend_Flag_Name(unsigned value)
{
    switch ( value ) {
        case D3DVBF_DISABLE: return "D3DVBF_DISABLE";
        case D3DVBF_1WEIGHTS: return "D3DVBF_1WEIGHTS";
        case D3DVBF_2WEIGHTS: return "D3DVBF_2WEIGHTS";
        case D3DVBF_3WEIGHTS: return "D3DVBF_3WEIGHTS";
        case D3DVBF_TWEENING: return "D3DVBF_TWEENING";
        case D3DVBF_0WEIGHTS: return "D3DVBF_0WEIGHTS";
        default: return "UNKNOWN";
    }
}

inline const char *DX8Wrapper::Get_DX8_Patch_Edge_Style_Name(unsigned value)
{
    switch ( value ) {
        case D3DPATCHEDGE_DISCRETE: return "D3DPATCHEDGE_DISCRETE";
        case D3DPATCHEDGE_CONTINUOUS:return "D3DPATCHEDGE_CONTINUOUS";
        default: return "UNKNOWN";
    }
}

inline const char *DX8Wrapper::Get_DX8_Debug_Monitor_Token_Name(unsigned value)
{
    switch ( value ) {
        case D3DDMT_ENABLE: return "D3DDMT_ENABLE";
        case D3DDMT_DISABLE: return "D3DDMT_DISABLE";
        default: return "UNKNOWN";
    }
}

inline const char *DX8Wrapper::Get_DX8_Blend_Op_Name(unsigned value)
{
    switch ( value ) {
        case D3DBLENDOP_ADD: return "D3DBLENDOP_ADD";
        case D3DBLENDOP_SUBTRACT: return "D3DBLENDOP_SUBTRACT";
        case D3DBLENDOP_REVSUBTRACT: return "D3DBLENDOP_REVSUBTRACT";
        case D3DBLENDOP_MIN: return "D3DBLENDOP_MIN";
        case D3DBLENDOP_MAX: return "D3DBLENDOP_MAX";
        default: return "UNKNOWN";
    }
}
#endif

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif
