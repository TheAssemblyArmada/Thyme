/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Debris Draw Module
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
#include "w3dmodeldraw.h"
#include "w3dshadow.h"

class DebrisDrawInterface
{
public:
    virtual void Set_Model_Name(Utf8String name, unsigned int color, ShadowType t) = 0;
    virtual void Set_Anim_Names(Utf8String initial, Utf8String flying, Utf8String final, const FXList *final_fx) = 0;
};

class W3DDebrisDraw : public DrawModule, public DebrisDrawInterface
{
    IMPLEMENT_POOL(W3DDebrisDraw)

public:
    W3DDebrisDraw(Thing *thing, ModuleData const *module_data);
    ~W3DDebrisDraw() override;
    NameKeyType Get_Module_Name_Key() const override;
    void Do_Draw_Module(const Matrix3D *transform) override;
    void Set_Shadows_Enabled(bool enable) override;
    void Release_Shadows() override {}
    void Allocate_Shadows() override {}
    void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    void React_To_Geometry_Change() override {}
    DebrisDrawInterface *Get_Debris_Draw_Interface() override { return this; }
    const DebrisDrawInterface *Get_Debris_Draw_Interface() const override { return this; }

    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    void Set_Model_Name(Utf8String name, unsigned int color, ShadowType type) override;
    void Set_Anim_Names(Utf8String initial, Utf8String flying, Utf8String final, const FXList *final_fx) override;

    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    enum AnimStateType
    {
        INITIAL,
        FLYING,
        FINAL,
        STATECOUNT,
    };

    Utf8String m_modelName;
    int m_color;
    Utf8String m_initialAnimName;
    Utf8String m_flyingAnimName;
    Utf8String m_finalAnimName;
    RenderObjClass *m_renderObject;
    HAnimClass *m_anims[STATECOUNT];
    const FXList *m_fxFinal;
    int m_state;
    int m_frames;
    bool m_finalStop;
    Shadow *m_shadow;
};