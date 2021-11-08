/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Utillity class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3d_util.h"
#include "shader.h"
#include "vector3.h"
#include "vector4.h"
#include "w3d_file.h"

void W3dUtilityClass::Convert_Vector(const IOVector3Struct &v, Vector3 *set)
{
    set->X = v.x;
    set->Y = v.y;
    set->Z = v.z;
}

void W3dUtilityClass::Convert_Color(const W3dRGBStruct &rgb, Vector3 *set)
{
    set->X = (float)rgb.r / 255;
    set->Y = (float)rgb.g / 255;
    set->Z = (float)rgb.b / 255;
}

void W3dUtilityClass::Convert_Color(const W3dRGBAStruct &rgb, Vector4 *set)
{
    set->X = (float)rgb.R / 255;
    set->Y = (float)rgb.G / 255;
    set->Z = (float)rgb.B / 255;
    set->W = (float)rgb.A / 255;
}

void W3dUtilityClass::Convert_Shader(const W3dShaderStruct &shader, ShaderClass *set)
{
    set->Set_Depth_Compare((ShaderClass::DepthCompareType)W3d_Shader_Get_Depth_Compare(&shader));
    set->Set_Depth_Mask((ShaderClass::DepthMaskType)W3d_Shader_Get_Depth_Mask(&shader));
    set->Set_Color_Mask(ShaderClass::COLOR_WRITE_ENABLE);
    set->Set_Dst_Blend_Func((ShaderClass::DstBlendFuncType)W3d_Shader_Get_Dest_Blend_Func(&shader));
    set->Set_Fog_Func(ShaderClass::FOG_DISABLE);
    set->Set_Primary_Gradient((ShaderClass::PriGradientType)W3d_Shader_Get_Pri_Gradient(&shader));
    set->Set_Secondary_Gradient((ShaderClass::SecGradientType)W3d_Shader_Get_Sec_Gradient(&shader));
    set->Set_Src_Blend_Func((ShaderClass::SrcBlendFuncType)W3d_Shader_Get_Src_Blend_Func(&shader));
    set->Set_Texturing((ShaderClass::TexturingType)W3d_Shader_Get_Texturing(&shader));
    set->Set_Alpha_Test((ShaderClass::AlphaTestType)W3d_Shader_Get_Alpha_Test(&shader));
    set->Set_Post_Detail_Color_Func((ShaderClass::DetailColorFuncType)W3d_Shader_Get_Detail_Color_Func(&shader));
    set->Set_Post_Detail_Alpha_Func((ShaderClass::DetailAlphaFuncType)W3d_Shader_Get_Detail_Alpha_Func(&shader));
}

void W3dUtilityClass::Convert_Shader(const ShaderClass &shader, W3dShaderStruct *set)
{
    W3d_Shader_Reset(set);
    W3d_Shader_Set_Depth_Compare(set, shader.Get_Depth_Compare());
    W3d_Shader_Set_Depth_Mask(set, shader.Get_Depth_Mask());
    W3d_Shader_Set_Dest_Blend_Func(set, shader.Get_Dst_Blend_Func());
    W3d_Shader_Set_Pri_Gradient(set, shader.Get_Primary_Gradient());
    W3d_Shader_Set_Sec_Gradient(set, shader.Get_Secondary_Gradient());
    W3d_Shader_Set_Texturing(set, shader.Get_Texturing());
    W3d_Shader_Set_Alpha_Test(set, shader.Get_Alpha_Test());
    W3d_Shader_Set_Post_Detail_Color_Func(set, shader.Get_Post_Detail_Color_Func());
    W3d_Shader_Set_Post_Detail_Alpha_Func(set, shader.Get_Post_Detail_Alpha_Func());
}
