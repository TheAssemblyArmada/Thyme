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
#pragma once
#include "always.h"

struct IOVector3Struct;
class ShaderClass;
class Vector4;
class Vector3;
struct W3dRGBAStruct;
struct W3dRGBStruct;
struct W3dShaderStruct;

class W3dUtilityClass
{
public:
    static void Convert_Vector(const IOVector3Struct &v, Vector3 *set);
    static void Convert_Color(const W3dRGBStruct &rgb, Vector3 *set);
    static void Convert_Color(const W3dRGBAStruct &rgb, Vector4 *set);
    static void Convert_Shader(const ShaderClass &shader, W3dShaderStruct *set);
    static void Convert_Shader(const W3dShaderStruct &shader, ShaderClass *set);
};
