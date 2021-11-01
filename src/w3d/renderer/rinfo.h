/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
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

class CameraClass;
class MaterialPassClass;
class LightEnvironmentClass;
class VisRasterizerClass;
class BWRenderClass;
class TexProjectClass;

class RenderInfoClass
{
public:
    enum
    {
        ADDITIONAL_MATERIAL_PASSES = 32,
        MAX_OVERRIDE_FLAG_LEVEL = 32,
    };

    enum RINFO_OVERRIDE_FLAGS
    {
        RINFO_OVERRIDE_DEFAULT = 0,
        RINFO_OVERRIDE_FORCE_TWO_SIDED = 1,
        RINFO_OVERRIDE_FORCE_SORTING = 2,
        RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY = 4,
        RINFO_OVERRIDE_SHADOW_RENDERING = 8,
    };

    RenderInfoClass(CameraClass &camera);
    ~RenderInfoClass();

    void Push_Material_Pass(MaterialPassClass *matpass);
    void Pop_Material_Pass();

    int Additional_Pass_Count() { return m_additionalMaterialPassCount; }
    MaterialPassClass *Peek_Additional_Pass(int i) const;

    void Push_Override_Flags(RINFO_OVERRIDE_FLAGS flags);
    void Pop_Override_Flags();
    RINFO_OVERRIDE_FLAGS &Current_Override_Flags();

public:
    CameraClass &m_camera;
    int field_4; // possibly float - fog_scale, fog_start, fog_end or according to BFME2 Vector3 FogColor
    int field_8;
    int field_C;
    float m_alphaOverride;
    float m_opacityOverride;
    float m_emissiveScale;
    LightEnvironmentClass *m_lightEnvironment;
    TexProjectClass *m_texProject;

protected:
    MaterialPassClass *m_additionalMaterialPassArray[ADDITIONAL_MATERIAL_PASSES];
    unsigned int m_additionalMaterialPassCount;
    int field_A8; // some delay for pushes
    RINFO_OVERRIDE_FLAGS m_overrideFlag[MAX_OVERRIDE_FLAG_LEVEL];
    unsigned int m_overrideFlagLevel;
};

class SpecialRenderInfoClass : public RenderInfoClass
{
public:
    enum RenderType
    {
        RENDER_VIS = 0x0,
        RENDER_SHADOW = 0x1,
    };

    SpecialRenderInfoClass(CameraClass &camera, RenderType render_type);
    ~SpecialRenderInfoClass() {}

public:
    RenderType m_renderType;
    VisRasterizerClass *m_visRasterizer;
    BWRenderClass *m_bwRenderer;
};
