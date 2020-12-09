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

class W3DView : public View, public SubsystemInterface
{
public:
    virtual ~W3DView() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual Drawable *Pick_Drawable(const ICoord2D *o, bool b, PickType type) override;
    virtual int Iterate_Drawables_In_Region(IRegion2D *r, bool (*func)(Drawable *, void *), void *) override;
    virtual void Set_Width(int32_t width) override;
    virtual void Set_Height(int32_t height) override;
    virtual void Set_Origin(int32_t x, int32_t y) override;
    virtual void Force_Redraw() override;
    virtual void Look_At(const Coord3D *pos) override;
    virtual void Init_Height_For_Map() override;
    virtual void Scroll_By(Coord2D *pos) override;
    virtual void Move_Camera_To(const Coord3D *o, int i1, int i2, bool b, float f1, float f2) override;
    virtual void Move_Camera_Along_Waypoint_Path(Waypoint *w, int i1, int i2, bool b, float f1, float f2) override;
    virtual bool Is_Camera_Movement_Finished() override;
    virtual void Camera_Mod_Final_Zoom(float f1, float f2, float f3) override;
    virtual void Camera_Mod_Rolling_Average(int i) override;
    virtual void Camera_Mod_Final_Time_Multiplier(int i) override;
    virtual void Camera_Mod_Final_Pitch(float f1, float f2, float f3) override;
    virtual void Camera_Mod_Freeze_Time() override;
    virtual void Camera_Mod_Freeze_Angle() override;
    virtual void Camera_Mod_Look_Toward(Coord3D *o) override;
    virtual void Camera_Mod_Final_Look_Toward(Coord3D *o) override;
    virtual void Camera_Mod_Final_Move_To(Coord3D *o) override;
    virtual void Camera_Enable_Slave_Mode(const Utf8String &s1, const Utf8String &s2) override;
    virtual void Camera_Disable_Slave_Mode() override;
    virtual void Add_Camera_Shake(const Coord3D &o, float f1, float f2, float f3) override;
    virtual FilterModes Get_View_Filter_Mode() override;
    virtual FilterTypes Get_View_Filter_Type() override;
    virtual bool Set_View_Filter_Mode(FilterModes mode) override;
    virtual void Set_View_Filter_Pos(const Coord3D *pos) override;
    virtual bool Set_View_Filter(FilterTypes filter) override;
    virtual void Set_Fade_Parameters(int frames, int direction) override;
    virtual void Set_3D_Wireframe_Mode(bool on) override;
    virtual void Reset_Camera(const Coord3D *o, int i, float f1, float f2) override;
    virtual void Rotate_Camera(float f1, int i, float f2, float f3) override;
    virtual void Rotate_Camera_Toward_Object(ObjectID id, int i1, int i2, float f1, float f2) override;
    virtual void Rotate_Camera_Toward_Position(const Coord3D *o, int i, float f1, float f2, bool b) override;
    virtual bool Is_Time_Frozen() override;
    virtual int Get_Time_Multiplier() override;
    virtual void Set_Time_Multiplier(int multiplier) override;
    virtual void Set_Default_View(float f1, float f2, float f3) override;
    virtual void Zoom_Camera(float f1, int i, float f2, float f3) override;
    virtual void Pitch_Camera(float f1, int i, float f2, float f3) override;
    virtual void Set_Angle(float angle) override;
    virtual void Set_Pitch(float pitch) override;
    virtual void Set_Angle_And_Pitch_To_Default() override;
    virtual const Coord3D &Get_3D_Camera_Position() override;
    virtual void Set_Zoom(float zoom) override;
    virtual void Set_Height_Above_Ground(float height) override;
    virtual void Set_Zoom_To_Default() override;
    virtual void Set_Field_Of_View(float fov) override;
    virtual void World_To_Screen_Tri_Return(const Coord3D *o1, ICoord2D *o2) override;
    virtual void Screen_To_World(const ICoord2D *o1, Coord3D *o2) override;
    virtual void Screen_To_Terrain(const ICoord2D *o1, Coord3D *o2) override;
    virtual void Screen_To_World_At_Z(const ICoord2D *o1, Coord3D *o2, float f) override;
    virtual void Draw_View() override;
    virtual void Update_View() override;
    virtual void Set_Camera_Lock(ObjectID id) override;
    virtual void Set_Snap_Mode(CameraLockType type, float dist) override;
    virtual void Shake(const Coord3D *o, CameraShakeType type) override;
    virtual float Get_FX_Pitch() override;
    virtual void Force_Camera_Constraint_Recalc() override;
    virtual void Set_Guard_Band_Bias(Coord2D *) override;
    virtual bool Is_Camera_Movement_At_Waypoint_Along_Path();
    virtual void Camera_Enable_Real_Zoom_Mode();
    virtual void Camera_Disable_Real_Zoom_Mode();
};
