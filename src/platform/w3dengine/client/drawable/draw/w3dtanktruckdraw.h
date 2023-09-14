/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tank Truck Draw Module
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
#include "rendobj.h"
#include "w3dmodeldraw.h"

class W3DTankTruckDrawModuleData : public W3DModelDrawModuleData
{
private:
    Utf8String m_dustEffectName;
    Utf8String m_dirtEffectName;
    Utf8String m_powerslideEffectName;
    Utf8String m_frontLeftTireBoneName;
    Utf8String m_frontRightTireBoneName;
    Utf8String m_rearLeftTireBoneName;
    Utf8String m_rearRightTireBoneName;
    Utf8String m_midFrontLeftTireBoneName;
    Utf8String m_midFrontRightTireBoneName;
    Utf8String m_midRearLeftTireBoneName;
    Utf8String m_midRearRightTireBoneName;
    float m_rotationSpeedMultiplier;
    float m_powerslideRotationAddition;
    Utf8String m_treadDebrisNameLeft;
    Utf8String m_treadDebrisNameRight;
    float m_treadAnimationRate;
    float m_treadPivotSpeedFraction;
    float m_treadDriveSpeedFraction;

public:
    W3DTankTruckDrawModuleData();
    virtual ~W3DTankTruckDrawModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);
    friend class W3DTankTruckDraw;
};

class W3DTankTruckDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DTankTruckDraw);

public:
    enum
    {
        MAX_TREADS_PER_TANK = 4,
    };

    enum TreadType
    {
        TREAD_LEFT,
        TREAD_RIGHT,
        TREAD_MIDDLE,
    };

    struct TreadObjectInfo
    {
        RenderObjClass *m_robj;
        TreadType m_type;
        RenderObjClass::Material_Override m_materialSettings;
    };

    W3DTankTruckDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DTankTruckDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    virtual void On_Render_Obj_Recreated() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Set_Hidden(bool hidden) override;

    void Toss_Emitters();
    void Create_Emitters();
    void Enable_Emitters(bool enable);
    void Update_Bones();
    void Start_Move_Debris();
    void Stop_Move_Debris();
    void Update_Tread_Positions(float uv_delta);
    void Update_Tread_Objects();

    const W3DTankTruckDrawModuleData *Get_W3D_Tank_Truck_Draw_Module_Data() const
    {
        return static_cast<const W3DTankTruckDrawModuleData *>(Module::Get_Module_Data());
    }

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);

private:
    bool m_effectsInitialized;
    bool m_wasAirborne;
    bool m_isPowersliding;
    ParticleSystem *m_dustEffect;
    ParticleSystem *m_dirtEffect;
    ParticleSystem *m_powerslideEffect;
    float m_frontWheelRotation;
    float m_rearWheelRotation;
    float m_midFrontWheelRotation;
    float m_midRearWheelRotation;
    int m_frontLeftTireBone;
    int m_frontRightTireBone;
    int m_rearLeftTireBone;
    int m_rearRightTireBone;
    int m_midFrontLeftTireBone;
    int m_midFrontRightTireBone;
    int m_midRearLeftTireBone;
    int m_midRearRightTireBone;
    AudioEventRTS m_powerslideSound;
    AudioEventRTS m_landingSound;
    ParticleSystem *m_treadDebrisLeft;
    ParticleSystem *m_treadDebrisRight;
    TreadObjectInfo m_treads[MAX_TREADS_PER_TANK];
    int m_treadCount;
    RenderObjClass *m_prevRenderObj;
};
