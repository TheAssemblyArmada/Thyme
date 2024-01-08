/**
 * @file
 *
 * @author Jonathan Wilson
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
#include "parabolicease.h"
#include "vector3.h"
#include "view.h"

class CameraClass;
class Matrix3D;

struct TRotateCameraInfo
{
    int num_frames;
    int cur_frame;
    int start_time_multiplier;
    int end_time_multiplier;
    int num_hold_frames;
    ParabolicEase ease;
    bool track_object;
    union
    {
        struct
        {
            float start_angle;
            float end_angle;
        };
        struct
        {
            ObjectID target_object_id;
            Coord3D target_object_pos;
        };
    };

    TRotateCameraInfo() : ease(0.0f, 0.0f) {}
};

struct TPitchCameraInfo
{
    int num_frames;
    int cur_frame;
    int angle;
    float start_pitch;
    float end_pitch;
    int start_time_multiplier;
    int end_time_multiplier;
    ParabolicEase ease;

    TPitchCameraInfo() : ease(0.0f, 0.0f) {}
};

struct TZoomCameraInfo
{
    int num_frames;
    int cur_frame;
    float start_zoom;
    float end_zoom;
    int start_time_multiplier;
    int end_time_multiplier;
    ParabolicEase ease;

    TZoomCameraInfo() : ease(0.0f, 0.0f) {}
};

struct TMoveAlongWaypointPathInfo
{
    int num_waypoints;
    Coord3D waypoints[27];
    float way_seg_length[27];
    float camera_angle[27];
    int time_multiplier[27];
    float ground_height[26];
    int total_time_milliseconds;
    int elapsed_time_milliseconds;
    float total_distance;
    float cur_seg_distance;
    int shutter;
    int cur_segment;
    int cur_shutter;
    int rolling_average_frames;
    ParabolicEase ease;

    TMoveAlongWaypointPathInfo() : ease(0.0f, 0.0f) {}
};

class W3DView : public View, public SubsystemInterface
{
public:
    W3DView();
    virtual ~W3DView() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual Drawable *Pick_Drawable(const ICoord2D *screen, bool force_attack, PickType type) override;
    virtual int Iterate_Drawables_In_Region(
        IRegion2D *screen_region, bool (*callback)(Drawable *, void *), void *user_data) override;
    virtual void Set_Width(int32_t width) override;
    virtual void Set_Height(int32_t height) override;
    virtual void Set_Origin(int32_t x, int32_t y) override;
    virtual void Force_Redraw() override;
    virtual void Look_At(const Coord3D *pos) override;
    virtual void Init_Height_For_Map() override;
    virtual void Scroll_By(Coord2D *pos) override;
    virtual void Move_Camera_To(const Coord3D *o, int frames, int shutter, bool orient, float in, float out) override;
    virtual void Move_Camera_Along_Waypoint_Path(
        Waypoint *way, int frames, int shutter, bool orient, float in, float out) override;
    virtual bool Is_Camera_Movement_Finished() override;
    virtual void Camera_Mod_Final_Zoom(float final_zoom, float in, float out) override;
    virtual void Camera_Mod_Rolling_Average(int frames_to_average) override;
    virtual void Camera_Mod_Final_Time_Multiplier(int final_multiplier) override;
    virtual void Camera_Mod_Final_Pitch(float final_pitch, float in, float out) override;
    virtual void Camera_Mod_Freeze_Time() override;
    virtual void Camera_Mod_Freeze_Angle() override;
    virtual void Camera_Mod_Look_Toward(Coord3D *p_loc) override;
    virtual void Camera_Mod_Final_Look_Toward(Coord3D *p_loc) override;
    virtual void Camera_Mod_Final_Move_To(Coord3D *p_loc) override;
    virtual void Camera_Enable_Slave_Mode(const Utf8String &thing, const Utf8String &bone) override;
    virtual void Camera_Disable_Slave_Mode() override;
    virtual void Add_Camera_Shake(const Coord3D &position, float radius, float duration, float amplitude) override;
    virtual FilterModes Get_View_Filter_Mode() override;
    virtual FilterTypes Get_View_Filter_Type() override;
    virtual bool Set_View_Filter_Mode(FilterModes mode) override;
    virtual void Set_View_Filter_Pos(const Coord3D *pos) override;
    virtual bool Set_View_Filter(FilterTypes filter) override;
    virtual void Set_Fade_Parameters(int fade_frames, int direction) override;
    virtual void Set_3D_Wireframe_Mode(bool on) override;
    virtual void Reset_Camera(const Coord3D *location, int frames, float in, float out) override;
    virtual void Rotate_Camera(float rotations, int frames, float in, float out) override;
    virtual void Rotate_Camera_Toward_Object(
        ObjectID id, int milliseconds, int hold_milliseconds, float in, float out) override;
    virtual void Rotate_Camera_Toward_Position(const Coord3D *p_loc, int milliseconds, float in, float out, bool b) override;
    virtual bool Is_Time_Frozen() override;
    virtual int Get_Time_Multiplier() override;
    virtual void Set_Time_Multiplier(int multiple) override;
    virtual void Set_Default_View(float pitch, float angle, float max_height) override;
    virtual void Zoom_Camera(float final_zoom, int milliseconds, float in, float out) override;
    virtual void Pitch_Camera(float final_pitch, int milliseconds, float in, float out) override;
    virtual void Set_Angle(float angle) override;
    virtual void Set_Pitch(float pitch) override;
    virtual void Set_Angle_And_Pitch_To_Default() override;
    virtual const Coord3D &Get_3D_Camera_Position() override;
    virtual void Set_Zoom(float z) override;
    virtual void Set_Height_Above_Ground(float z) override;
    virtual void Set_Zoom_To_Default() override;
    virtual void Set_Field_Of_View(float angle) override;
    virtual int World_To_Screen_Tri_Return(const Coord3D *w, ICoord2D *s) override;
    virtual void Screen_To_World(const ICoord2D *s, Coord3D *w) override;
    virtual void Screen_To_Terrain(const ICoord2D *screen, Coord3D *world) override;
    virtual void Screen_To_World_At_Z(const ICoord2D *s, Coord3D *w, float z) override;
    virtual void Draw_View() override;
    virtual void Update_View() override;
    virtual void Set_Camera_Lock(ObjectID id) override;
    virtual void Set_Snap_Mode(CameraLockType lock_type, float lock_dist) override;
    virtual void Shake(const Coord3D *epicenter, CameraShakeType shake_type) override;
    virtual float Get_FX_Pitch() override;
    virtual void Force_Camera_Constraint_Recalc() override;
    virtual void Set_Guard_Band_Bias(Coord2D *bias) override;
    virtual bool Is_Camera_Movement_At_Waypoint_Along_Path();
    virtual void Camera_Enable_Real_Zoom_Mode();
    virtual void Camera_Disable_Real_Zoom_Mode();
    virtual void Update() override;
    virtual void Draw() override;

    bool Update_Camera_Movements();
    CameraClass *Get_3D_Camera() { return m_3DCamera; }

private:
    void Build_Camera_Transform(Matrix3D *transform);
    void Calc_Camera_Constraints();
    void Get_Pick_Ray(const ICoord2D *screen, Vector3 *ray_start, Vector3 *ray_end);
    void Set_Camera_Transform();
    void Get_Axis_Aligned_View_Region(Region3D &axis_aligned_region);
    void Calc_Delta_Scroll(Coord2D &screen_delta);
    void Setup_Waypoint_Path(bool orient);
    void Rotate_Camera_One_Frame();
    void Zoom_Camera_One_Frame();
    void Pitch_Camera_One_Frame();
    void Move_Along_Waypoint_Path(int milliseconds);

    CameraClass *m_3DCamera;
    CameraClass *m_2DCamera;
    FilterModes m_viewFilterMode;
    FilterTypes m_viewFilter;
    bool m_extraPass;
    bool m_wireframeMode;
    Coord2D m_shakeOffset;
    float m_shakeAngleCos;
    float m_shakeAngleSin;
    float m_shakeIntensity;
    Vector3 m_cameraShakeAngles;
    TRotateCameraInfo m_rcInfo;
    bool m_doingRotateCamera;
    TPitchCameraInfo m_pcInfo;
    bool m_doingPitchCamera;
    TZoomCameraInfo m_zcInfo;
    bool m_doingZoomCamera;
    bool m_doingScriptedCameraLock;
    float m_FXPitch;
    TMoveAlongWaypointPathInfo m_mcwpInfo;
    bool m_doingMoveCameraOnWaypointPath;
    bool m_doingMoveCameraAlongWaypointPath;
    bool m_freezeTimeForCameraMovement;
    int m_timeMultiplier;
    bool m_cameraHasMovedSinceRequest;
    std::vector<std::pair<ICoord2D, Coord3D>> m_locationRequests;
    Coord3D m_cameraOffset;
    Coord3D m_previousLookAtPosition;
    Coord2D m_scrollAmount;
    float m_scrollAmountCutoff;
    float m_groundLevel;
    Region2D m_cameraConstraint;
    bool m_cameraConstraintValid;
    bool m_cameraSlaveMode;
    bool m_realZoom;
    Utf8String m_cameraSlaveThing;
    Utf8String m_cameraSlaveBone;
};
