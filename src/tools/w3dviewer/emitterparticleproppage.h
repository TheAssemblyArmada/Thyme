/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter particle prop page
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

class EmitterInstanceList;
class Vector3Randomizer;

class EmitterParticlePropPageClass : public CPropertyPage
{
public:
    EmitterParticlePropPageClass();
    virtual ~EmitterParticlePropPageClass() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnApply() override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

    void Initialize();

    bool IsValid() const { return m_isValid; }
    void SetInstanceList(EmitterInstanceList *list) { m_instanceList = list; }

private:
    DECLARE_DYNCREATE(EmitterParticlePropPageClass)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnSpecify();
    afx_msg void OnMaxParticles();

    CSpinButtonCtrl m_particlesSpin;
    CSpinButtonCtrl m_rateSpin;
    CSpinButtonCtrl m_maxPartSpin;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    float m_emissionRate;
    unsigned int m_burstSize;
    int m_maxEmissions;
    Vector3Randomizer *m_creationVolume;
};
