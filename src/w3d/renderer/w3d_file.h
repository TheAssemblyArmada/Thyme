/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D data structures
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

struct W3dRGBStruct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t pad;
};

struct W3dMaterial3Struct
{
    uint32_t attributes;
    W3dRGBStruct diffuse_color;
    W3dRGBStruct specular_color;
    W3dRGBStruct emissive_coeffs;
    W3dRGBStruct ambient_coeffs;
    W3dRGBStruct diffuse_coeffs;
    W3dRGBStruct specular_coeffs;
    float shininess;
    float opacity;
    float translucency;
    float fog_coeff;
};

// clang-format off
#define W3DVERTMAT_USE_DEPTH_CUE                        0x00000001
#define W3DVERTMAT_ARGB_EMISSIVE_ONLY                   0x00000002
#define W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE             0x00000004
#define W3DVERTMAT_DEPTH_CUE_TO_ALPHA                   0x00000008

#define W3DVERTMAT_STAGE0_MAPPING_MASK                  0x00FF0000
#define W3DVERTMAT_STAGE0_MAPPING_UV                    0x00000000
#define W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT           0x00010000
#define W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT     0x00020000
#define W3DVERTMAT_STAGE0_MAPPING_SCREEN                0x00030000
#define W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET         0x00040000
#define W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE            0x00050000
#define W3DVERTMAT_STAGE0_MAPPING_SCALE                 0x00060000
#define W3DVERTMAT_STAGE0_MAPPING_GRID                  0x00070000
#define W3DVERTMAT_STAGE0_MAPPING_ROTATE                0x00080000
#define W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET    0x00090000
#define W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET    0x000A0000
#define W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET  0x000B0000
#define W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV        0x000C0000
#define W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT        0x000D0000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV      0x000E0000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT      0x000F0000
#define W3DVERTMAT_STAGE0_MAPPING_RANDOM                0x00100000
#define W3DVERTMAT_STAGE0_MAPPING_EDGE                  0x00110000
#define W3DVERTMAT_STAGE0_MAPPING_BUMPENV               0x00120000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV   0x00130000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT   0x00140000

#define W3DVERTMAT_STAGE1_MAPPING_MASK                  0x0000FF00
#define W3DVERTMAT_STAGE1_MAPPING_UV                    0x00000000
#define W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT           0x00000100
#define W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT     0x00000200
#define W3DVERTMAT_STAGE1_MAPPING_SCREEN                0x00000300
#define W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET         0x00000400
#define W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE            0x00000500
#define W3DVERTMAT_STAGE1_MAPPING_SCALE                 0x00000600
#define W3DVERTMAT_STAGE1_MAPPING_GRID                  0x00000700
#define W3DVERTMAT_STAGE1_MAPPING_ROTATE                0x00000800
#define W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET    0x00000900
#define W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET    0x00000A00
#define W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET  0x00000B00
#define W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV        0x00000C00
#define W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT        0x00000D00
#define W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV      0x00000E00
#define W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT      0x00000F00
#define W3DVERTMAT_STAGE1_MAPPING_RANDOM                0x00001000
#define W3DVERTMAT_STAGE1_MAPPING_EDGE                  0x00001100
#define W3DVERTMAT_STAGE1_MAPPING_BUMPENV               0x00001200
#define W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV   0x00001300
#define W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT   0x00001400
// clang-format on

struct W3dVertexMaterialStruct
{
    uint32_t Attributes;
    W3dRGBStruct Ambient;
    W3dRGBStruct Diffuse;
    W3dRGBStruct Specular;
    W3dRGBStruct Emissive;
    float Shininess;
    float Opacity;
    float Translucency;
};
