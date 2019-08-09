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
#pragma once

#include "always.h"
#include "coord.h"
#include "gametype.h"
#include "snapshot.h"
#include "subsysteminterface.h"

class Drawable;
class Waypoint;

// need to find these
class ViewLocation
{
public:
    ViewLocation() {} // needs to be found, likely source BFME2 WB
    void Init(float, float, float, float, float, float) {} // needs to be found, likely source BFME2 WB

private:
    int32_t m_valid;
    Coord3D m_pos;
    float m_angle;
    float m_pitch;
    float m_zoom;
};

enum FilterModes
{
    FM_NULL_MODE,
    FM_VIEW_BW_BLACK_AND_WHITE,
    FM_VIEW_BW_RED_AND_WHITE,
    FM_VIEW_BW_GREEN_AND_WHITE,
    FM_VIEW_MB_IN_AND_OUT_ALPHA,
    FM_VIEW_MB_IN_AND_OUT_STATURATE,
    FM_VIEW_MB_IN_ALPHA,
    FM_VIEW_MB_OUT_ALPHA,
    FM_VIEW_MB_IN_STATURATE,
    FM_VIEW_MB_OUT_STATURATE,
    FM_VIEW_MB_END_PAN_ALPHA,
    FM_VIEW_MB_PAN_ALPHA,
};

enum FilterTypes
{
    FT_NULL_FILTER,
    FT_VIEW_BW_FILTER,
    FT_VIEW_MOTION_BLUR_FILTER,
    FT_MAX,
};

// dunno
enum PickType
{
    PICKTYPE0,
};

// wip
class View : public SnapShot
{
    enum CameraLockType
    {
        LOCK_FOLLOW = 0x0,
        LOCK_TETHER = 0x1,
    };
    enum CameraShakeType
    {
        SHAKE_SUBTLE = 0x0,
        SHAKE_NORMAL = 0x1,
        SHAKE_STRONG = 0x2,
        SHAKE_SEVERE = 0x3,
    };

public:
    View();
    virtual ~View() {}

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Init();
    virtual void Reset() { m_zoomLimited = 1; };
    virtual uint32_t Get_ID();
    virtual void Set_Zoom_Limited(bool);
    virtual bool Is_Zoom_Limited();
    virtual Drawable *Pick_Drawable(ICoord2D const *, bool, PickType) = 0;
    virtual int Iterate_Drawables_In_Region(IRegion2D *, bool (*)(Drawable *, void *), void *) = 0;
    virtual void Get_Screen_Corner_World_Points_At_Z(Coord3D *, Coord3D *, Coord3D *, Coord3D *, float);
    virtual void Set_Width(uint32_t width) { m_width = width; }
    virtual uint32_t Get_Width() { return m_width; }
    virtual void Set_Height(uint32_t height) { m_height = height; }
    virtual uint32_t Get_Height() { return m_height; }
    virtual void Set_Origin(int, int);
    virtual void Get_Origin(int32_t *, int32_t *);
    virtual void Force_Redraw() = 0;
    virtual void Look_At(const Coord3D *);
    virtual void Init_Height_For_Map();
    virtual void Scroll_By(Coord2D *);
    virtual void Move_Camera_To(
        const Coord3D *, int, int, bool, float, float); // in mac, might be (const Coord3D *,int,int,bool) in pc
    virtual void Move_Camera_Along_Waypoint_Path(
        Waypoint *, int, int, char, float, float); // in mac, might be (Waypoint32_t*,int,int,bool)
    virtual bool Is_Camera_Movement_Finished();
    virtual void Camera_Mod_Final_Zoom(float, float, float); // in mac, might be (float)
    virtual void Camera_Mod_Rolling_Average(int);
    virtual void Camera_Mod_Final_Time_Multiplier(int);
    virtual void Camera_Mod_Final_Pitch(float, float, float); // in mac, might be (float)
    virtual void Camera_Mod_Freeze_Time();
    virtual void Camera_Mod_Freeze_Angle();
    virtual void Camera_Mod_Look_Toward(Coord3D *);
    virtual void Camera_Mod_Final_Look_Toward(Coord3D *);
    virtual void Camera_Mod_Final_Move_To(Coord3D *);
    virtual void Camera_Enable_Slave_Mode(const Utf8String &, const Utf8String &); // return needs to be confirmed
    virtual void Camera_Disable_Slave_Mode(); // return needs to be confirmed
    virtual void Add_Camera_Shake(const Coord3D &, float, float, float); // return needs to be confirmed
    virtual FilterModes Get_View_Filter_Mode();
    virtual FilterTypes Get_View_Filter_Type();
    virtual bool Set_View_Filter_Mode(FilterModes mode);
    virtual void Set_View_Filter_Pos(const Coord3D *pos);
    virtual bool Set_View_Filter(FilterTypes filter);
    virtual void Set_Fade_Parameters(int, int);
    virtual void Set_3D_Wire_Frame_Mode(bool on);
    virtual void Reset_Camera(const Coord3D *, int, float, float); // in mac, might be (const Coord3D *,int)
    virtual void Rotate_Camera(float, int, float, float); // in mac, might be (float,int)
    virtual void Rotate_Camera_Toward_Object(ObjectID, int, int, float, float); // in mac, might be (ObjectID,int,int)
    virtual void Rotate_CameraTowardPosition(
        const Coord3D *, int, float, float, char); // in mac, might be (const Coord3D *,int)
    virtual bool Is_Time_Frozen();
    virtual int Get_Time_Multiplier();
    virtual void Set_Time_Multiplier(int multiplier);
    virtual void Set_Default_View(float, float, float);
    virtual void Zoom_Camera(float, int, float, float); // in mac, might be (float,int)
    virtual void Pitch_Camera(float, int, float, float); // in mac, might be (float,int)
    virtual void Set_Angle(float angle);
    virtual float Get_Angle();
    virtual void Set_Pitch(float pitch);
    virtual float Get_Pitch();
    virtual void Set_Angle_And_Pitch_To_Default();
    virtual void Get_Position(Coord3D *pos);
    virtual const Coord3D &Get_3D_Camera_Position() = 0;
    virtual float Get_Zoom();
    virtual void Set_Zoom(float zoom);
    virtual float Get_Height_Above_Ground();
    virtual void Set_Height_Above_Ground(float height);
    virtual void Zoom_In();
    virtual void Zoom_Out();
    virtual void Set_Zoom_To_Default() = 0;
    virtual float Get_Max_Zoom() { return m_maxZoom; }
    virtual void Set_Ok_To_Adjust_Height(bool ok) { m_okToAdjustHeight = ok; }
    virtual float Get_Terrain_Height_Under_Camera() { return m_terrainHeightUnderCamera; }
    virtual void Set_Terrain_Height_Under_Camera(float height) { m_terrainHeightUnderCamera = height; }
    virtual float Get_Current_Height_Above_Ground() { return m_currentHeightAboveGround; }
    virtual void Set_Current_Height_Above_Ground(float height) { m_currentHeightAboveGround = height; }
    virtual void Set_Field_Of_View(float fov) { m_FOV = fov; }
    virtual float Get_Field_Of_View() { return m_FOV; }
    virtual void World_To_Screen_Tri_Return(const Coord3D *, ICoord2D *) = 0;
    virtual void Screen_To_World(ICoord2D const *, Coord3D *) = 0;
    virtual void Screen_To_Terrain(ICoord2D const *, Coord3D *) = 0;
    virtual void Screen_To_World_At_Z(ICoord2D const *, Coord3D *, float) = 0;
    virtual void Get_Location(ViewLocation *location); // need help with these
    virtual void Set_Location(ViewLocation *location); // need help with these
    virtual void Draw_View() = 0;
    virtual void Update_View() = 0;
    virtual ObjectID Get_Camera_Lock() { return m_cameraLock; };
    virtual void Set_Camera_Lock(ObjectID id)
    {
        m_cameraLock = id;
        m_lockDist = 0;
        m_lockType = LOCK_FOLLOW;
    };
    virtual void Snap_To_Camera_Lock() { m_snapImmediate = true; }
    virtual void Set_Snap_Mode(CameraLockType type, float dist)
    {
        m_lockType = type;
        m_lockDist = dist;
    };
    virtual Drawable *Get_Camera_Lock_Drawable() { return m_cameraLockDrawable; }
    virtual void Set_Camera_Lock_Drawable(Drawable *drawable)
    {
        m_cameraLockDrawable = drawable;
        m_lockDist = 0;
    }
    virtual void Set_Mouse_Lock(bool locked) { m_mouseLocked = locked; }
    virtual bool Is_Mouse_Locked() { return m_mouseLocked; }
    virtual void Shake(const Coord3D *, CameraShakeType) {}
    virtual float Get_FX_Pitch() = 0;
    virtual void Force_Camera_Constraint_Recalc() {}
    virtual void Set_Guard_Band_Bias(Coord2D *) = 0;
    virtual View *Prepend_View_To_List(View *view)
    {
        m_next = view;
        return this;
    };
    virtual View *Get_Next_View() { return m_next; };

protected:
    View *m_next;
    uint32_t m_id;
    Coord3D m_pos;
    uint32_t m_width;
    uint32_t m_height;
    int32_t m_originX;
    int32_t m_originY;
    float m_angle;
    float m_pitchAngle;
    float m_maxZoom;
    float m_minZoom;
    float m_maxHeightAboveGround;
    float m_minHeightAboveGround;
    float m_zoom;
    float m_heightAboveGround;
    bool m_zoomLimited;
    float m_defaultAngle;
    float m_defaultPitchAngle;
    float m_currentHeightAboveGround;
    float m_terrainHeightUnderCamera;
    ObjectID m_cameraLock;
    Drawable *m_cameraLockDrawable;
    CameraLockType m_lockType;
    float m_lockDist;
    float m_FOV;
    bool m_mouseLocked;
    bool m_okToAdjustHeight;
    bool m_snapImmediate;
    Coord2D m_guardBandBias;

#ifdef GAME_DLL
    static uint32_t &s_idNext;
#else
    static uint32_t s_idNext;
#endif
};
