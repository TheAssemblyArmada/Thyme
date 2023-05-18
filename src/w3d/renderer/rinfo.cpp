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
#include "rinfo.h"
#include "matpass.h"
#include <captainslog.h>

RenderInfoClass::RenderInfoClass(CameraClass &camera) :
    m_camera(camera),
    field_4(0),
    field_8(0),
    field_C(0),
    m_alphaOverride(1.0f),
    m_opacityOverride(1.0f),
    m_emissiveScale(1.0f),
    m_lightEnvironment(nullptr),
    m_texProject(nullptr),
    m_additionalMaterialPassCount(0),
    field_A8(0),
    m_overrideFlagLevel(0)
{
    for (int i = 0; i < ADDITIONAL_MATERIAL_PASSES; ++i) {
        m_additionalMaterialPassArray[i] = nullptr;
    }

    for (int i = 0; i < MAX_OVERRIDE_FLAG_LEVEL; ++i) {
        m_overrideFlag[i] = RINFO_OVERRIDE_DEFAULT;
    }
}

RenderInfoClass::~RenderInfoClass()
{
    while (m_additionalMaterialPassCount) {
        Pop_Material_Pass();
    }
}

void RenderInfoClass::Push_Material_Pass(MaterialPassClass *matpass)
{
    if (m_additionalMaterialPassCount < ADDITIONAL_MATERIAL_PASSES - 1) {
        if (matpass != nullptr) {
            matpass->Add_Ref();
        }
        m_additionalMaterialPassArray[m_additionalMaterialPassCount++] = matpass;
    } else {
        ++field_A8;
    }
}

void RenderInfoClass::Pop_Material_Pass()
{
    if (field_A8) {
        --field_A8;
    } else {
        captainslog_assert(m_additionalMaterialPassCount > 0);
        MaterialPassClass *matpass = m_additionalMaterialPassArray[--m_additionalMaterialPassCount];
        if (matpass != nullptr) {
            matpass->Release_Ref();
        }
    }
}

MaterialPassClass *RenderInfoClass::Peek_Additional_Pass(int i) const
{
    captainslog_assert(i < ADDITIONAL_MATERIAL_PASSES);
    return m_additionalMaterialPassArray[i];
}

void RenderInfoClass::Push_Override_Flags(RINFO_OVERRIDE_FLAGS flags)
{
    captainslog_assert(m_overrideFlagLevel < MAX_OVERRIDE_FLAG_LEVEL);
    m_overrideFlag[++m_overrideFlagLevel] = flags;
}

void RenderInfoClass::Pop_Override_Flags()
{
    captainslog_assert(m_overrideFlagLevel > 0);
    --m_overrideFlagLevel;
}

RenderInfoClass::RINFO_OVERRIDE_FLAGS &RenderInfoClass::Current_Override_Flags()
{
    return m_overrideFlag[m_overrideFlagLevel];
}

SpecialRenderInfoClass::SpecialRenderInfoClass(CameraClass &camera, RenderType render_type) :
    RenderInfoClass(camera), m_renderType(render_type), m_visRasterizer(nullptr), m_bwRenderer(nullptr)
{
}
