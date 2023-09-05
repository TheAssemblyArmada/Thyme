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
#include "view.h"
class CameraClass;

class W3DView : public View, public SubsystemInterface
{
public:
    ~W3DView() override;
    void Init() override;
    void Reset() override;
    Drawable *Pick_Drawable(const ICoord2D *o, bool b, PickType type) override;
    int Iterate_Drawables_In_Region(IRegion2D *r, bool (*func)(Drawable *, void *), void *) override;
    void Set_Width(int32_t width) override;
    void Set_Height(int32_t height) override;
    void Set_Origin(int32_t x, int32_t y) override;
    void Force_Redraw() override;
    void Look_At(const Coord3D *pos) override;
    void Init_Height_For_Map() override;
    void Scroll_By(Coord2D *pos) override;
    void Move_Camera_To(const Coord3D *o, int i1, int i2, bool b, float f1, float f2) override;
    void Move_Camera_Along_Waypoint_Path(Waypoint *w, int i1, int i2, bool b, float f1, float f2) override;
    bool Is_Camera_Movement_Finished() override;
    void Camera_Mod_Final_Zoom(float f1, float f2, float f3) override;
    void Camera_Mod_Rolling_Average(int i) override;
    void Camera_Mod_Final_Time_Multiplier(int i) override;
    void Camera_Mod_Final_Pitch(float f1, float f2, float f3) override;
    void Camera_Mod_Freeze_Time() override;
    void Camera_Mod_Freeze_Angle() override;
    void Camera_Mod_Look_Toward(Coord3D *o) override;
    void Camera_Mod_Final_Look_Toward(Coord3D *o) override;
    void Camera_Mod_Final_Move_To(Coord3D *o) override;
    void Camera_Enable_Slave_Mode(const Utf8String &s1, const Utf8String &s2) override;
    void Camera_Disable_Slave_Mode() override;
    void Add_Camera_Shake(const Coord3D &o, float f1, float f2, float f3) override;
    FilterModes Get_View_Filter_Mode() override;
    FilterTypes Get_View_Filter_Type() override;
    bool Set_View_Filter_Mode(FilterModes mode) override;
    void Set_View_Filter_Pos(const Coord3D *pos) override;
    bool Set_View_Filter(FilterTypes filter) override;
    void Set_Fade_Parameters(int frames, int direction) override;
    void Set_3D_Wireframe_Mode(bool on) override;
    void Reset_Camera(const Coord3D *o, int i, float f1, float f2) override;
    void Rotate_Camera(float f1, int i, float f2, float f3) override;
    void Rotate_Camera_Toward_Object(ObjectID id, int i1, int i2, float f1, float f2) override;
    void Rotate_Camera_Toward_Position(const Coord3D *o, int i, float f1, float f2, bool b) override;
    bool Is_Time_Frozen() override;
    int Get_Time_Multiplier() override;
    void Set_Time_Multiplier(int multiplier) override;
    void Set_Default_View(float f1, float f2, float f3) override;
    void Zoom_Camera(float f1, int i, float f2, float f3) override;
    void Pitch_Camera(float f1, int i, float f2, float f3) override;
    void Set_Angle(float angle) override;
    void Set_Pitch(float pitch) override;
    void Set_Angle_And_Pitch_To_Default() override;
    const Coord3D &Get_3D_Camera_Position() override;
    void Set_Zoom(float zoom) override;
    void Set_Height_Above_Ground(float height) override;
    void Set_Zoom_To_Default() override;
    void Set_Field_Of_View(float fov) override;
    int World_To_Screen_Tri_Return(const Coord3D *o1, ICoord2D *o2) override;
    void Screen_To_World(const ICoord2D *o1, Coord3D *o2) override;
    void Screen_To_Terrain(const ICoord2D *o1, Coord3D *o2) override;
    void Screen_To_World_At_Z(const ICoord2D *o1, Coord3D *o2, float f) override;
    void Draw_View() override;
    void Update_View() override;
    void Set_Camera_Lock(ObjectID id) override;
    void Set_Snap_Mode(CameraLockType type, float dist) override;
    void Shake(const Coord3D *o, CameraShakeType type) override;
    float Get_FX_Pitch() override;
    void Force_Camera_Constraint_Recalc() override;
    void Set_Guard_Band_Bias(Coord2D *) override;
    virtual bool Is_Camera_Movement_At_Waypoint_Along_Path();
    virtual void Camera_Enable_Real_Zoom_Mode();
    virtual void Camera_Disable_Real_Zoom_Mode();
    bool Update_Camera_Movements();
    CameraClass *Get_3D_Camera() { return m_3DCamera; }

private:
    CameraClass *m_3DCamera;
};
