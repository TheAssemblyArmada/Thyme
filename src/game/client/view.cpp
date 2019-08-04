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

#ifdef GAME_DLL
uint32_t &View::s_idNext = Make_Global<uint32_t>(0x009D4178);
#else
uint32_t View::s_idNext = 1;
#endif

View::View() : 
    m_next(nullptr),
    m_id(s_idNext++),
    m_pos(),
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
    m_cameraLock(OBJECT_UNK),
    m_cameraLockDrawable(nullptr),
    m_lockType(LOCK_FOLLOW),
    m_lockDist(0.0f),
    m_FOV(0.87266463f),
    m_mouseLocked(false),
    m_okToAdjustHeight(true),
    m_snapImmediate(false),
    m_guardBandBias()
{
}

void View::Init() {}

void View::CRC_Snapshot(Xfer *xfer) {}

void View::Xfer_Snapshot(Xfer *xfer) {}

void View::Load_Post_Process() {}

uint32_t View::Get_ID()
{
    return uint32_t();
}

void View::Set_Zoom_Limited(bool) {}

bool View::Is_Zoom_Limited()
{
    return false;
}

void View::Get_Screen_Corner_World_Points_At_Z(Coord3D *, Coord3D *, Coord3D *, Coord3D *, float) {}

void View::Set_Origin(int, int) {}

void View::Get_Origin(int32_t *, int32_t *) {}

void View::Look_At(const Coord3D *) {}

void View::Init_Height_For_Map() {}

void View::Scroll_By(Coord2D *) {}

void View::Move_Camera_To(const Coord3D *, int, int, bool, float, float) {}

void View::Move_Camera_Along_Waypoint_Path(Waypoint *, int, int, char, float, float) {}

bool View::Is_Camera_Movement_Finished()
{
    return false;
}

void View::Camera_Mod_Final_Zoom(float, float, float) {}

void View::Camera_Mod_Rolling_Average(int) {}

void View::Camera_Mod_Final_Time_Multiplier(int) {}

void View::Camera_Mod_Final_Pitch(float, float, float) {}

void View::Camera_Mod_Freeze_Time() {}

void View::Camera_Mod_Freeze_Angle() {}

void View::Camera_Mod_Look_Toward(Coord3D *) {}

void View::Camera_Mod_Final_Look_Toward(Coord3D *) {}

void View::Camera_Mod_Final_Move_To(Coord3D *) {}

void View::Camera_Enable_Slave_Mode(const Utf8String &, const Utf8String &) {}

void View::Camera_Disable_Slave_Mode() {}

void View::Add_Camera_Shake(const Coord3D &, float, float, float) {}

FilterModes View::Get_View_Filter_Mode()
{
    return FilterModes();
}

FilterTypes View::Get_View_Filter_Type()
{
    return FilterTypes();
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

void View::Set_Fade_Parameters(int, int) {}

void View::Set_3D_Wire_Frame_Mode(bool on) {}

void View::Reset_Camera(const Coord3D *, int, float, float) {}

void View::Rotate_Camera(float, int, float, float) {}

void View::Rotate_Camera_Toward_Object(ObjectID, int, int, float, float) {}

void View::Rotate_CameraTowardPosition(const Coord3D *, int, float, float, char) {}

bool View::Is_Time_Frozen()
{
    return false;
}

int View::Get_Time_Multiplier()
{
    return 0;
}

void View::Set_Time_Multiplier(int multiplier) {}

void View::Set_Default_View(float, float, float) {}

void View::Zoom_Camera(float, int, float, float) {}

void View::Pitch_Camera(float, int, float, float) {}

void View::Set_Angle(float angle) {}

float View::Get_Angle()
{
    return 0.0f;
}

void View::Set_Pitch(float pitch) {}

float View::Get_Pitch()
{
    return 0.0f;
}

void View::Set_Angle_And_Pitch_To_Default() {}

void View::Get_Position(Coord3D *pos) {}

float View::Get_Zoom()
{
    return 0.0f;
}

void View::Set_Zoom(float zoom) {}

float View::Get_Height_Above_Ground()
{
    return 0.0f;
}

void View::Set_Height_Above_Ground(float height) {}

void View::Zoom_In() {}

void View::Zoom_Out() {}

void View::Get_Location(ViewLocation *location) {}

void View::Set_Location(ViewLocation *location) {}
