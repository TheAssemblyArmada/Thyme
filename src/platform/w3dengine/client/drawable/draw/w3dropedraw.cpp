/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Rope Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dropedraw.h"
#include "drawable.h"
#include "line3d.h"
#include "randomvalue.h"
#include "w3ddisplay.h"
#include "w3dscene.h"

W3DRopeDraw::W3DRopeDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data),
    m_ropeLength(0.0f),
    m_height(1.0f),
    m_width(0.5f),
    m_curDropSpeed(0.0f),
    m_maxDropSpeed(0.0f),
    m_gravity(0.0f),
    m_wobbleAmplitude(0.0f),
    m_wobbleRate(0.0f),
    m_wobbleAngle(0.0f),
    m_curDropHeight(0.0f)
{
    m_color.red = 0;
    m_color.green = 0;
    m_color.blue = 0;
    m_wobbleLen = m_height;
    m_segments.clear();
}

void W3DRopeDraw::Build_Segments()
{
    captainslog_dbgassert(m_segments.empty(), "Segments expected empty");
    m_segments.clear();
    int segment_count = (int)GameMath::Ceil(m_height / m_wobbleLen);
    float segment_height = m_height / segment_count;
    Coord3D pos = *Get_Drawable()->Get_Position();

    for (int i = 0; i < segment_count; i++) {
        SegInfo segment;
        float angle = Get_Client_Random_Value_Real(0.0f, DEG_TO_RADF(360.0f));
        segment.m_cosAngle = GameMath::Cos(angle);
        segment.m_sinAngle = GameMath::Sin(angle);

        segment.m_line1 = new Line3DClass(Vector3(pos.x, pos.y, pos.z),
            Vector3(pos.x, pos.y, pos.z + segment_height),
            m_width * 0.5f,
            m_color.red,
            m_color.green,
            m_color.blue,
            1.0f);

        segment.m_line2 = new Line3DClass(Vector3(pos.x, pos.y, pos.z),
            Vector3(pos.x, pos.y, pos.z + segment_height),
            m_width,
            m_color.red,
            m_color.green,
            m_color.blue,
            0.5f);

        W3DDisplay::s_3DScene->Add_Render_Object(segment.m_line1);
        W3DDisplay::s_3DScene->Add_Render_Object(segment.m_line2);
        m_segments.push_back(segment);
        pos.z += segment_height;
    }
}

void W3DRopeDraw::Toss_Segments()
{
    for (auto segment = m_segments.begin(); segment != m_segments.end(); segment++) {
        if (segment->m_line1 != nullptr) {
            W3DDisplay::s_3DScene->Remove_Render_Object(segment->m_line1);
            Ref_Ptr_Release(segment->m_line1);
        }

        if (segment->m_line2 != nullptr) {
            W3DDisplay::s_3DScene->Remove_Render_Object(segment->m_line2);
            Ref_Ptr_Release(segment->m_line2);
        }
    }

    m_segments.clear();
}

void W3DRopeDraw::Init_Rope_Params(
    float height, float width, const RGBColor &color, float wobble_len, float wobble_amplitude, float wobble_rate)
{
    m_height = std::max(1.0f, height);
    m_ropeLength = 0.0f;
    m_width = width;
    m_color = color;
    m_wobbleLen = std::min(m_height, wobble_len);
    m_wobbleAmplitude = wobble_amplitude;
    m_wobbleRate = wobble_rate;
    m_curDropHeight = 0.0f;
    Toss_Segments();
    Build_Segments();
}

void W3DRopeDraw::Set_Rope_Cur_Len(float length)
{
    m_ropeLength = length;
}

void W3DRopeDraw::Set_Rope_Speed(float initial_speed, float max_speed, float gravity)
{
    m_curDropSpeed = initial_speed;
    m_maxDropSpeed = max_speed;
    m_gravity = gravity;
}

W3DRopeDraw::~W3DRopeDraw()
{
    Toss_Segments();
}

void W3DRopeDraw::Do_Draw_Module(const Matrix3D *transform)
{
    if (m_segments.empty()) {
        Build_Segments();
    }

    if (!m_segments.empty()) {
        float wobble_sin_angle = GameMath::Sin(m_wobbleAngle) * m_wobbleAmplitude;
        const Coord3D *pos = Get_Drawable()->Get_Position();
        Vector3 start(pos->x, pos->y, pos->z + m_curDropHeight);
        float segment_length = m_ropeLength / m_segments.size();

        for (auto segment = m_segments.begin(); segment != m_segments.end(); segment++) {
            Vector3 end(wobble_sin_angle * segment->m_cosAngle + pos->x,
                wobble_sin_angle * segment->m_sinAngle + pos->y,
                start.Z - segment_length);

            if (segment->m_line1 != nullptr) {
                segment->m_line1->Reset(start, end);
            }

            if (segment->m_line2 != nullptr) {
                segment->m_line2->Reset(start, end);
            }

            start = end;
        }
    }

    m_wobbleAngle += m_wobbleRate;

    if (m_wobbleAngle > DEG_TO_RADF(360.0f)) {
        m_wobbleAngle -= DEG_TO_RADF(360.0f);
    }

    m_curDropHeight += m_curDropSpeed;
    m_curDropSpeed += m_gravity;

    if (m_curDropSpeed > m_maxDropSpeed) {
        m_curDropSpeed = m_maxDropSpeed;
    } else if (-m_maxDropSpeed > m_curDropSpeed) {
        m_curDropSpeed = -m_maxDropSpeed;
    }
}

void W3DRopeDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DRopeDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);
    DrawModule::Xfer_Snapshot(xfer);
    xfer->xferReal(&m_ropeLength);
    xfer->xferReal(&m_height);
    xfer->xferReal(&m_width);
    xfer->xferRGBColor(&m_color);
    xfer->xferReal(&m_curDropSpeed);
    xfer->xferReal(&m_maxDropSpeed);
    xfer->xferReal(&m_gravity);
    xfer->xferReal(&m_wobbleLen);
    xfer->xferReal(&m_wobbleAmplitude);
    xfer->xferReal(&m_wobbleRate);
    xfer->xferReal(&m_wobbleAngle);
    xfer->xferReal(&m_curDropHeight);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Toss_Segments();
    }
}

void W3DRopeDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

NameKeyType W3DRopeDraw::Get_Module_Name_Key() const
{
    static const NameKeyType nk = g_theNameKeyGenerator->Name_To_Key("W3DRopeDraw");
    return nk;
}

Module *W3DRopeDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DRopeDraw, thing, module_data);
}
