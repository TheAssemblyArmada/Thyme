/**
 * @file
 *
 * @author tomsons26
 * @author OmniBlade
 *
 * @brief Base class for the display handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "view.h"
#include "globaldata.h"
#include "xfer.h"
#include <algorithm>

#ifndef GAME_DLL
uint32_t View::s_idNext = 1;
View *g_theTacticalView;
#endif

View::View() :
    m_next(nullptr),
    m_id(s_idNext++),
    m_width(0),
    m_height(0),
    m_originX(0),
    m_originY(0),
    m_angle(0.0f),
    m_pitchAngle(0.0f),
    m_maxZoom(0.0f),
    m_minZoom(0.0f),
    m_maxHeightAboveGround(0.0f),
    m_minHeightAboveGround(0.0f),
    m_zoom(0.0f),
    m_heightAboveGround(0.0f),
    m_zoomLimited(true),
    m_defaultAngle(0.0f),
    m_defaultPitchAngle(0.0f),
    m_currentHeightAboveGround(0.0f),
    m_terrainHeightUnderCamera(0.0f),
    m_cameraLock(INVALID_OBJECT_ID),
    m_cameraLockDrawable(nullptr),
    m_lockType(LOCK_FOLLOW),
    m_lockDist(0.0f),
    m_FOV(DEG_TO_RADF(50.f)),
    m_mouseLocked(false),
    m_okToAdjustHeight(true),
    m_snapImmediate(false)
{
    m_pos.x = 0.0f;
    m_pos.y = 0.0f;
    m_guardBandBias.x = 0.0f;
    m_guardBandBias.y = 0.0f;
}

void View::Init()
{
    m_width = 640;
    m_height = 480;
    m_originX = 0;
    m_originY = 0;
    m_pos.x = 0.0f;
    m_pos.y = 0.0f;
    m_angle = 0.0f;
    m_cameraLock = INVALID_OBJECT_ID;
    m_cameraLockDrawable = 0;
    m_zoomLimited = true;
    m_maxZoom = 1.3f;
    m_minZoom = 0.2f;
    m_zoom = 1.3f;
    m_maxHeightAboveGround = g_theWriteableGlobalData->m_maxCameraHeight;
    m_minHeightAboveGround = g_theWriteableGlobalData->m_minCameraHeight;
    m_okToAdjustHeight = false;
    m_defaultAngle = 0.0f;
    m_defaultPitchAngle = 0.0f;
}

void View::CRC_Snapshot(Xfer *xfer) {}

void View::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    float angle = Get_Angle();
    xfer->xferReal(&angle);
    Set_Angle(angle);
    Coord3D pos;
    Get_Position(&pos);
    xfer->xferReal(&pos.x);
    xfer->xferReal(&pos.y);
    xfer->xferReal(&pos.z);
    Look_At(&pos);
}

void View::Load_Post_Process() {}

uint32_t View::Get_ID()
{
    return m_id;
}

void View::Set_Zoom_Limited(bool b)
{
    m_zoomLimited = b;
}

bool View::Is_Zoom_Limited()
{
    return m_zoomLimited;
}

void View::Get_Screen_Corner_World_Points_At_Z(
    Coord3D *top_left, Coord3D *top_right, Coord3D *bottom_left, Coord3D *bottom_right, float z)
{
    uint32_t width = Get_Width();
    uint32_t height = Get_Height();

    if (top_left && top_right && bottom_left && bottom_right) {
        int32_t x;
        int32_t y;
        Get_Origin(&x, &y);
        ICoord2D tl;
        tl.x = x;
        tl.y = y;
        ICoord2D tr;
        tr.x = width + x;
        tr.y = y;
        ICoord2D bl;
        bl.x = width + x;
        bl.y = height + y;
        ICoord2D br;
        br.x = x;
        br.y = height + y;
        Screen_To_World_At_Z(&tl, top_left, z);
        Screen_To_World_At_Z(&tr, top_right, z);
        Screen_To_World_At_Z(&bl, bottom_left, z);
        Screen_To_World_At_Z(&br, bottom_right, z);
    }
}

void View::Set_Origin(int32_t x, int32_t y)
{
    m_originX = x;
    m_originY = y;
}

void View::Get_Origin(int32_t *x, int32_t *y)
{
    *x = m_originX;
    *y = m_originY;
}

void View::Look_At(const Coord3D *pos)
{
    Coord3D c = Get_Position();
    c.x = pos->x - (double)m_width * 0.5f;
    c.y = pos->y - (double)m_height * 0.5f;
    Set_Position(&c);
}

void View::Init_Height_For_Map() {}

void View::Scroll_By(Coord2D *pos)
{
    m_pos.x += pos->x;
    m_pos.y += pos->y;
}

void View::Move_Camera_To(const Coord3D *o, int frames, int shutter, bool orient, float in, float out)
{
    Look_At(o);
}

void View::Move_Camera_Along_Waypoint_Path(Waypoint *way, int frames, int stutter, bool orient, float in, float out) {}

bool View::Is_Camera_Movement_Finished()
{
    return true;
}

void View::Camera_Mod_Final_Zoom(float final_zoom, float in, float out) {}

void View::Camera_Mod_Rolling_Average(int frames_to_average) {}

void View::Camera_Mod_Final_Time_Multiplier(int final_multiplier) {}

void View::Camera_Mod_Final_Pitch(float final_pitch, float in, float out) {}

void View::Camera_Mod_Freeze_Time() {}

void View::Camera_Mod_Freeze_Angle() {}

void View::Camera_Mod_Look_Toward(Coord3D *p_loc) {}

void View::Camera_Mod_Final_Look_Toward(Coord3D *p_loc) {}

void View::Camera_Mod_Final_Move_To(Coord3D *p_loc) {}

void View::Camera_Enable_Slave_Mode(const Utf8String &thing, const Utf8String &bone) {}

void View::Camera_Disable_Slave_Mode() {}

void View::Add_Camera_Shake(const Coord3D &position, float radius, float duration, float amplitude) {}

FilterModes View::Get_View_Filter_Mode()
{
    return FM_NULL_MODE;
}

FilterTypes View::Get_View_Filter_Type()
{
    return FT_NULL_FILTER;
}

bool View::Set_View_Filter_Mode(FilterModes mode)
{
    return false;
}

void View::Set_View_Filter_Pos(const Coord3D *pos) {}

bool View::Set_View_Filter(FilterTypes filter)
{
    return false;
}

void View::Set_Fade_Parameters(int fade_frames, int direction) {}

void View::Set_3D_Wireframe_Mode(bool on) {}

void View::Reset_Camera(const Coord3D *location, int frames, float in, float out) {}

void View::Rotate_Camera(float rotations, int frames, float in, float out) {}

void View::Rotate_Camera_Toward_Object(ObjectID id, int milliseconds, int hold_milliseconds, float in, float out) {}

void View::Rotate_Camera_Toward_Position(const Coord3D *p_loc, int milliseconds, float in, float out, bool b) {}

bool View::Is_Time_Frozen()
{
    return false;
}

int View::Get_Time_Multiplier()
{
    return 1;
}

void View::Set_Time_Multiplier(int multiple) {}

void View::Set_Default_View(float pitch, float angle, float max_height) {}

void View::Zoom_Camera(float final_zoom, int milliseconds, float in, float out) {}

void View::Pitch_Camera(float final_pitch, int milliseconds, float in, float out) {}

void View::Set_Angle(float angle)
{
    m_angle = angle;
}

float View::Get_Angle()
{
    return m_angle;
}

void View::Set_Pitch(float angle)
{
    m_pitchAngle = std::clamp(angle, DEG_TO_RADF(-36.f), DEG_TO_RADF(36.f));
}

float View::Get_Pitch()
{
    return m_pitchAngle;
}

void View::Set_Angle_And_Pitch_To_Default()
{
    m_angle = m_defaultAngle;
    m_pitchAngle = m_defaultPitchAngle;
}

void View::Get_Position(Coord3D *pos)
{
    *pos = m_pos;
}

float View::Get_Zoom()
{
    return m_zoom;
}

void View::Set_Zoom(float z) {}

float View::Get_Height_Above_Ground()
{
    return m_heightAboveGround;
}

void View::Set_Height_Above_Ground(float z)
{
    m_heightAboveGround = z;
}

void View::Zoom_In()
{
    Set_Height_Above_Ground(Get_Height_Above_Ground() - 10.0f);
}

void View::Zoom_Out()
{
    Set_Height_Above_Ground(Get_Height_Above_Ground() + 10.0f);
}

void View::Get_Location(ViewLocation *location)
{
    Coord3D pos = Get_Position();
    location->Init(pos.x, pos.y, pos.z, Get_Angle(), Get_Pitch(), Get_Zoom());
}

void View::Set_Location(const ViewLocation *location)
{
    if (location->m_valid) {
        Set_Position(&location->m_pos);
        Set_Angle(location->m_angle);
        Set_Pitch(location->m_pitch);
        Set_Zoom(location->m_zoom);
        Force_Redraw();
    }
}
