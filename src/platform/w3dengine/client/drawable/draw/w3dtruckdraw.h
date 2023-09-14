/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Truck Draw Module
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

class W3DTruckDrawModuleData : public W3DModelDrawModuleData
{
public:
    W3DTruckDrawModuleData() :
        // BUGFIX: init all members
        m_cabRotationMultiplier(0.0f),
        m_trailerRotationMultiplier(0.0f),
        m_rotationDamping(0.0f),
        m_rotationSpeedMultiplier(0.0f),
        m_powerslideRotationAddition(0.0f)
    {
    }
    virtual ~W3DTruckDrawModuleData() {}
    static void Build_Field_Parse(MultiIniFieldParse &p);

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
    Utf8String m_midMidLeftTireBoneName;
    Utf8String m_midMidRightTireBoneName;
    Utf8String m_cabBoneName;
    Utf8String m_trailerBoneName;
    float m_cabRotationMultiplier;
    float m_trailerRotationMultiplier;
    float m_rotationDamping;
    float m_rotationSpeedMultiplier;
    float m_powerslideRotationAddition;
    friend class W3DTruckDraw;
};

class W3DTruckDraw : public W3DModelDraw
{
    IMPLEMENT_POOL(W3DTruckDraw)

public:
    W3DTruckDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DTruckDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    virtual void React_To_Geometry_Change() override {}
    virtual void On_Render_Obj_Recreated() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual void Set_Hidden(bool hidden) override;

    void Toss_Emitters();
    void Create_Emitters();
    void Enable_Emitters(bool enable);
    void Update_Bones();

    const W3DTruckDrawModuleData *Get_W3D_Truck_Draw_Module_Data() const
    {
        return static_cast<const W3DTruckDrawModuleData *>(Module::Get_Module_Data());
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
    int m_midMidLeftTireBone;
    int m_midMidRightTireBone;
    int m_cabBone;
    float m_cabRotation;
    int m_trailerBone;
    float m_trailerRotation;
    int m_numBones;
    AudioEventRTS m_powerslideSound;
    AudioEventRTS m_landingSound;
    RenderObjClass *m_prevRenderObj;
};
