/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Laser Draw
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dlaserdraw.h"
#include "assetmgr.h"
#include "color.h"
#include "colorspace.h"
#include "drawable.h"
#include "laserupdate.h"
#include "segline.h"
#include "terrainlogic.h"
#include "texture.h"
#include "w3ddisplay.h"
#include "w3dscene.h"

W3DLaserDrawModuleData::W3DLaserDrawModuleData() :
    // BUGFIX initalize all members
    m_innerColor(0),
    m_outerColor(0),
    m_innerBeamWidth(0.0f),
    m_outerBeamWidth(1.0f),
    m_scrollRate(0.0f),
    m_tile(false),
    m_numBeams(1),
    m_maxIntensityFrames(0),
    m_fadeFrames(0),
    m_segments(1),
    m_arcHeight(1.0f),
    m_segmentOverlapRatio(0.0f),
    m_tilingScalar(1.0f)
{
}

void W3DLaserDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    static const FieldParse dataFieldParse[] = {
        { "NumBeams", &INI::Parse_Unsigned_Int, nullptr, offsetof(W3DLaserDrawModuleData, m_numBeams) },
        { "InnerBeamWidth", &INI::Parse_Real, nullptr, offsetof(W3DLaserDrawModuleData, m_innerBeamWidth) },
        { "OuterBeamWidth", &INI::Parse_Real, nullptr, offsetof(W3DLaserDrawModuleData, m_outerBeamWidth) },
        { "InnerColor", &INI::Parse_Color_Int, nullptr, offsetof(W3DLaserDrawModuleData, m_innerColor) },
        { "OuterColor", &INI::Parse_Color_Int, nullptr, offsetof(W3DLaserDrawModuleData, m_outerColor) },
        { "MaxIntensityLifetime",
            &INI::Parse_Duration_Unsigned_Int,
            nullptr,
            offsetof(W3DLaserDrawModuleData, m_maxIntensityFrames) },
        { "FadeLifetime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(W3DLaserDrawModuleData, m_fadeFrames) },
        { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(W3DLaserDrawModuleData, m_textureName) },
        { "ScrollRate", &INI::Parse_Real, nullptr, offsetof(W3DLaserDrawModuleData, m_scrollRate) },
        { "Tile", &INI::Parse_Bool, nullptr, offsetof(W3DLaserDrawModuleData, m_tile) },
        { "Segments", &INI::Parse_Unsigned_Int, nullptr, offsetof(W3DLaserDrawModuleData, m_segments) },
        { "ArcHeight", &INI::Parse_Real, nullptr, offsetof(W3DLaserDrawModuleData, m_arcHeight) },
        { "SegmentOverlapRatio", &INI::Parse_Real, nullptr, offsetof(W3DLaserDrawModuleData, m_segmentOverlapRatio) },
        { "TilingScalar", &INI::Parse_Real, nullptr, offsetof(W3DLaserDrawModuleData, m_tilingScalar) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

W3DLaserDraw::W3DLaserDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data),
    m_line3D(nullptr),
    m_texture(nullptr),
    m_textureAspectRatio(1.0f),
    m_setLaserPosition(true)
{
    const W3DLaserDrawModuleData *data = Get_W3D_Laser_Draw_Module_Data();
    m_texture = W3DAssetManager::Get_Instance()->Get_Texture(data->m_textureName.Str());

    if (m_texture) {
        if (!m_texture->Is_Initialized()) {
            m_texture->Init();
        }

        SurfaceClass::SurfaceDescription desc;
        m_texture->Get_Level_Description(desc, 0);
        m_textureAspectRatio = (float)desc.width / (float)desc.height;
    }

    float innerred;
    float innergreen;
    float innerblue;
    float inneralpha;
    float outerred;
    float outergreen;
    float outerblue;
    float outeralpha;
    Get_Color_Components_Real(data->m_innerColor, &innerred, &innergreen, &innerblue, &inneralpha);
    Get_Color_Components_Real(data->m_outerColor, &outerred, &outergreen, &outerblue, &outeralpha);
    m_line3D = new SegmentedLineClass *[data->m_segments * data->m_numBeams];

    for (unsigned int i = 0; i < data->m_segments; i++) {
        for (int j = (int)data->m_numBeams - 1; j >= 0; j--) {
            int index = j + data->m_numBeams * i;

            float beamwidth;
            float red;
            float green;
            float blue;
            float alpha;

            if (data->m_numBeams == 1) {
                beamwidth = data->m_innerBeamWidth;
                alpha = inneralpha;
                red = innerred * inneralpha;
                green = innergreen * inneralpha;
                blue = innerblue * inneralpha;
            } else {
                float f1 = (float)j / ((float)data->m_numBeams - 1.0f);
                beamwidth = (data->m_outerBeamWidth - data->m_innerBeamWidth) * f1 + data->m_innerBeamWidth;
                alpha = (outeralpha - inneralpha) * f1 + inneralpha;
                red = (outerred - innerred) * f1 * inneralpha + innerred;
                green = (outergreen - innergreen) * f1 * inneralpha + innergreen;
                blue = (outerblue - innerblue) * f1 * inneralpha + innerblue;
            }

            m_line3D[index] = new SegmentedLineClass();
            SegmentedLineClass *line = m_line3D[index];

            if (line) {
                line->Set_Texture(m_texture);
                line->Set_Shader(ShaderClass::s_presetAdditiveShader);
                line->Set_Width(beamwidth);
                line->Set_Color(Vector3(red, green, blue));
                line->Set_UV_Offset_Rate(Vector2(0.0f, data->m_scrollRate));

                if (m_texture) {
                    line->Set_Texture_Mapping_Mode(SegLineRendererClass::TILED_TEXTURE_MAP);
                }

                W3DDisplay::s_3DScene->Add_Render_Object(line);
                line->Set_Visible(0);
            }
        }
    }
}

W3DLaserDraw::~W3DLaserDraw()
{
    const W3DLaserDrawModuleData *data = Get_W3D_Laser_Draw_Module_Data();

    for (unsigned int i = 0; i < data->m_segments * data->m_numBeams; i++) {
        W3DDisplay::s_3DScene->Remove_Render_Object(m_line3D[i]);
        Ref_Ptr_Release(m_line3D[i]);
    }

    delete[] m_line3D;
}

float W3DLaserDraw::Get_Laser_Template_Width() const
{
    return Get_W3D_Laser_Draw_Module_Data()->m_outerBeamWidth * 0.5f;
}

ModuleData *W3DLaserDraw::Friend_New_Module_Data(INI *ini)
{
    W3DLaserDrawModuleData *data = new W3DLaserDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DLaserDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DLaserDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DLaserDraw, thing, module_data);
}

NameKeyType W3DLaserDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DLaserDraw");
    return _key;
}

void W3DLaserDraw::Do_Draw_Module(const Matrix3D *transform)
{
    const W3DLaserDrawModuleData *data = Get_W3D_Laser_Draw_Module_Data();
    Drawable *drawable = Get_Drawable();
    static NameKeyType key_LaserUpdate = g_theNameKeyGenerator->Name_To_Key("LaserUpdate");
    LaserUpdate *update = static_cast<LaserUpdate *>(drawable->Find_Client_Update_Module(key_LaserUpdate));

    if (!update) {
        captainslog_debug(
            "W3DLaserDraw::doDrawModule() expects its owner drawable %s to have a ClientUpdate = LaserUpdate module.",
            drawable->Get_Template()->Get_Name().Str());
        return;
    }

    if (update->Is_Dirty() || m_setLaserPosition) {
        update->Set_Dirty(false);
        m_setLaserPosition = false;
        Vector3 points[2];

        for (unsigned int i = 0; i < data->m_segments; i++) {
            if (data->m_arcHeight > 0.0f && data->m_segments > 1) {
                Coord3D start;
                start.Set(&update->Get_Start_Pos());

                Coord3D end;
                end.Set(&update->Get_End_Pos());

                Coord3D start_to_end;
                start_to_end.Set(&end);
                start_to_end.Sub(&start);
                float start_to_end_length = start_to_end.Length();

                Coord3D midpoint;
                midpoint.Set(&start);
                midpoint.Add(&end);
                midpoint.Scale(0.5f);
                float mid_length = start_to_end_length * 0.5f;
                float f3 = (float)i / (float)data->m_segments;
                float f4 = ((float)i + 1.0f) / (float)data->m_segments;

                if (i > 0) {
                    f3 = f3 - data->m_segmentOverlapRatio;
                }

                if (i < data->m_segments - 1) {
                    f4 = f4 + data->m_segmentOverlapRatio;
                }

                Coord3D c3;
                c3.Set(&start_to_end);
                c3.Scale(f3);

                Coord3D c4;
                c4.Set(&start);
                c4.Add(&c3);

                c3.Set(&start_to_end);
                c3.Scale(f4);

                Coord3D c5;
                c5.Set(&start);
                c5.Add(&c3);

                c3.Set(&midpoint);
                c3.Sub(&c4);

                float f5 = c3.Length();
                float f6 = f5 / mid_length * GAMEMATH_PI * 0.5f;
                float f7 = GameMath::Cos(f6);
                f7 = f7 * data->m_arcHeight;
                c4.z = c4.z + f7;

                c3.Set(&midpoint);
                c3.Sub(&c5);
                float f8 = c3.Length();
                float f9 = f8 / mid_length * GAMEMATH_PI * 0.5f;
                f7 = GameMath::Cos(f9);
                f7 = f7 * data->m_arcHeight;
                c5.z = c5.z + f7;

                if (g_theTerrainLogic->Get_Ground_Height(c4.x, c4.y, nullptr) + 2.0f < c4.z) {
                    points[0].Set(c4.x, c4.y, c4.z);
                } else {
                    points[0].Set(c4.x, c4.y, g_theTerrainLogic->Get_Ground_Height(c4.x, c4.y, nullptr) + 2.0f);
                }

                if (g_theTerrainLogic->Get_Ground_Height(c5.x, c5.y, nullptr) + 2.0f < c5.z) {
                    points[1].Set(c5.x, c5.y, c5.z);
                } else {
                    points[1].Set(c5.x, c5.y, g_theTerrainLogic->Get_Ground_Height(c4.x, c4.y, nullptr) + 2.0f);
                }
            } else {
                points[0].Set(Vector3(update->Get_Start_Pos().x, update->Get_Start_Pos().y, update->Get_Start_Pos().z));
                points[1].Set(Vector3(update->Get_End_Pos().x, update->Get_End_Pos().y, update->Get_End_Pos().z));
            }

            float innerred;
            float innergreen;
            float innerblue;
            float inneralpha;
            float outerred;
            float outergreen;
            float outerblue;
            float outeralpha;
            Get_Color_Components_Real(data->m_innerColor, &innerred, &innergreen, &innerblue, &inneralpha);
            Get_Color_Components_Real(data->m_outerColor, &outerred, &outergreen, &outerblue, &outeralpha);

            for (int j = (int)data->m_numBeams - 1; j >= 0; j--) {
                int index = j + data->m_numBeams * i;

                float width;
                float alpha;

                if (data->m_numBeams == 1) {
                    width = update->Get_Width() * data->m_innerBeamWidth;
                    alpha = inneralpha;
                } else {
                    float f = (float)j / ((float)data->m_numBeams - 1.0f);
                    width = ((data->m_outerBeamWidth - data->m_innerBeamWidth) * f + data->m_innerBeamWidth)
                        * update->Get_Width();
                    alpha = (outeralpha - inneralpha) * f + inneralpha;
                }

                if (m_texture && data->m_tile) {
                    Vector3 v1;
                    Vector3::Subtract(points[1], points[0], &v1);
                    m_line3D[index]->Set_Texture_Tile_Factor(
                        v1.Length() / width * m_textureAspectRatio * data->m_tilingScalar);
                }

                m_line3D[index]->Set_Width(width);
                m_line3D[index]->Set_Points(2, points);
            }
        }
    }
}
