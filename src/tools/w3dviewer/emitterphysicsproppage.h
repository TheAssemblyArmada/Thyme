/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter physics prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "w3dafx.h"
#include "vector3.h"

class EmitterInstanceList;
class Vector3Randomizer;

class EmitterPhysicsPropPageClass : public CPropertyPage
{
public:
    EmitterPhysicsPropPageClass();
    virtual ~EmitterPhysicsPropPageClass() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnApply() override;

    void Initialize();

    bool IsValid() const { return m_isValid; }
    void SetInstanceList(EmitterInstanceList *list) { m_instanceList = list; }

private:
    DECLARE_DYNCREATE(EmitterPhysicsPropPageClass)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnRandomizer();

    void OnUpdateEditCtrl(int id);

    CSpinButtonCtrl m_outwardSpin;
    CSpinButtonCtrl m_inheritSpin;
    CSpinButtonCtrl m_zVelSpin;
    CSpinButtonCtrl m_yVelSpin;
    CSpinButtonCtrl m_xVelSpin;
    CSpinButtonCtrl m_zAccelSpin;
    CSpinButtonCtrl m_yAccelSpin;
    CSpinButtonCtrl m_xAccelSpin;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    Vector3 m_velocity;
    Vector3 m_acceleration;
    float m_outwardVel;
    float m_velInherit;
    Vector3Randomizer *m_randomizer;
};
