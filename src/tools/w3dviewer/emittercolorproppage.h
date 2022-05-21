/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter color prop page
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
#include "part_emt.h"

class EmitterInstanceList;
class ColorBarClass;

class EmitterColorPropPageClass : public CPropertyPage
{
public:
    EmitterColorPropPageClass();
    virtual ~EmitterColorPropPageClass() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnCancel() override;
    virtual BOOL OnApply() override;

    void Initialize();
    void UpdateLifetime(float lifetime);

    bool IsValid() const { return m_isValid; }
    void SetInstanceList(EmitterInstanceList *list) { m_instanceList = list; }

private:
    DECLARE_DYNCREATE(EmitterColorPropPageClass)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnDestroy();
    afx_msg void OnDeltaPos(NMHDR *pNMHDR, LRESULT *pResult);

    void UpdateOpacity();
    void UpdateColor();

    CSpinButtonCtrl m_opacitySpin;
    CSpinButtonCtrl m_redSpin;
    CSpinButtonCtrl m_greenSpin;
    CSpinButtonCtrl m_blueSpin;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    ColorBarClass *m_colorBar;
    ColorBarClass *m_opacityBar;
    ParticlePropertyStruct<Vector3> m_oldColorKeyFrames;
    ParticlePropertyStruct<float> m_oldOpacityKeyFrames;
    ParticlePropertyStruct<Vector3> m_colorKeyFrames;
    ParticlePropertyStruct<float> m_opacityKeyFrames;
    float m_lifetime;
};
