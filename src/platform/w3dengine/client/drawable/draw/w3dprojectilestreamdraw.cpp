/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Projectile Stream Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dprojectilestreamdraw.h"
#include "assetmgr.h"
#include "drawable.h"
#include "object.h"
#include "segline.h"
#include "texture.h"
#include "w3ddisplay.h"
#include "w3dscene.h"

W3DProjectileStreamDrawModuleData::W3DProjectileStreamDrawModuleData() :
    m_width(0.0f), m_tileFactor(0.0f), m_scrollRate(0.0f), m_maxSegments(0)
{
}

void W3DProjectileStreamDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(W3DProjectileStreamDrawModuleData, m_texture) },
        { "Width", &INI::Parse_Real, nullptr, offsetof(W3DProjectileStreamDrawModuleData, m_width) },
        { "TileFactor", &INI::Parse_Real, nullptr, offsetof(W3DProjectileStreamDrawModuleData, m_tileFactor) },
        { "ScrollRate", &INI::Parse_Real, nullptr, offsetof(W3DProjectileStreamDrawModuleData, m_scrollRate) },
        { "MaxSegments", &INI::Parse_Int, nullptr, offsetof(W3DProjectileStreamDrawModuleData, m_maxSegments) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DProjectileStreamDraw::W3DProjectileStreamDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data)
{
    m_texture = W3DAssetManager::Get_Instance()->Get_Texture(Get_W3D_Projectile_Stream_Draw_Module_Data()->m_texture.Str());

    for (int i = 0; i < MAX_SEGMENT_COUNT; i++) {
        m_allLines[i] = nullptr;
    }

    m_linesValid = 0;
}

W3DProjectileStreamDraw::~W3DProjectileStreamDraw()
{
    for (int i = 0; i < m_linesValid; i++) {
        SegmentedLineClass *line = m_allLines[i];

        if (line != nullptr) {
            if (line->Peek_Scene() != nullptr) {
                W3DDisplay::s_3DScene->Remove_Render_Object(line);
            }

            line->Release_Ref();
        }
    }

    Ref_Ptr_Release(m_texture);
}

NameKeyType W3DProjectileStreamDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DProjectileStreamDraw");
    return _key;
}

void W3DProjectileStreamDraw::Do_Draw_Module(const Matrix3D *transform)
{
    Object *object = Get_Drawable()->Get_Object();
    if (object != nullptr) {
        static const NameKeyType _projectileStreamUpdateKey = g_theNameKeyGenerator->Name_To_Key("ProjectileStreamUpdate");
        ProjectileStreamUpdate *update = (ProjectileStreamUpdate *)object->Find_Update_Module(_projectileStreamUpdateKey);
        const W3DProjectileStreamDrawModuleData *data = Get_W3D_Projectile_Stream_Draw_Module_Data();

        Vector3 all_points[MAX_SEGMENT_COUNT];
        int points_used;
        update->Get_All_Points(all_points, &points_used);

        Vector3 staging_points[MAX_SEGMENT_COUNT];
        int lines_made = 0;
        int num = 0;
        unsigned int point_count = 0;
        Vector3 zero(0.0f, 0.0f, 0.0f);

        if (data->m_maxSegments != 0) {
            num = std::max(0, points_used - data->m_maxSegments);
        }

        while (num < points_used) {
            while (num < points_used && all_points[num] != zero) {
                staging_points[point_count++] = all_points[num++];
            }

            if (point_count > 1) {
                Make_Or_Update_Line(staging_points, point_count, lines_made++);
            }

            num++;
            point_count = 0;
        }

        int lines = m_linesValid;

        for (int i = lines_made; i < lines; i++) {
            if (m_allLines[i]->Peek_Scene() != nullptr) {
                W3DDisplay::s_3DScene->Remove_Render_Object(m_allLines[i]);
            }

            Ref_Ptr_Release(m_allLines[i]);
            m_linesValid--;
        }
    }
}

void W3DProjectileStreamDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (obscured) {
        for (int i = 0; i < m_linesValid; i++) {
            SegmentedLineClass *line = m_allLines[i];

            if (line != nullptr) {
                if (line->Peek_Scene() != nullptr) {
                    line->Remove();
                }
            }
        }
    } else {
        for (int i = 0; i < m_linesValid; i++) {
            SegmentedLineClass *line = m_allLines[i];

            if (line != nullptr && line->Peek_Scene() == nullptr) {
                W3DDisplay::s_3DScene->Add_Render_Object(line);
            }
        }
    }
}

void W3DProjectileStreamDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DProjectileStreamDraw::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char ver = 1;
    xfer->xferVersion(&ver, 1);
    DrawModule::Xfer_Snapshot(xfer);
}

void W3DProjectileStreamDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

void W3DProjectileStreamDraw::Make_Or_Update_Line(Vector3 *points, unsigned int point_count, int line_index)
{
    bool new_line = false;

    if (m_allLines[line_index] == nullptr) {
        m_allLines[line_index] = new SegmentedLineClass();
        m_linesValid++;
        new_line = true;
    }

    SegmentedLineClass *line = m_allLines[line_index];
    line->Set_Points(point_count, points);

    if (new_line) {
        const W3DProjectileStreamDrawModuleData *data = Get_W3D_Projectile_Stream_Draw_Module_Data();
        line->Set_Texture(m_texture);
        line->Set_Shader(ShaderClass::s_presetAdditiveSpriteShader);
        line->Set_Width(data->m_width);
        line->Set_Texture_Mapping_Mode(SegLineRendererClass::TILED_TEXTURE_MAP);
        line->Set_Texture_Tile_Factor(data->m_tileFactor);
        line->Set_UV_Offset_Rate(Vector2(0.0f, data->m_scrollRate));
        W3DDisplay::s_3DScene->Add_Render_Object(line);
    }
}

ModuleData *W3DProjectileStreamDraw::Friend_New_Module_Data(INI *ini)
{
    W3DProjectileStreamDrawModuleData *data = new W3DProjectileStreamDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DProjectileStreamDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DProjectileStreamDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DProjectileStreamDraw, thing, module_data);
}
