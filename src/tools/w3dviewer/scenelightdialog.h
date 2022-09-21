/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Scene Light Dialog
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

class CSceneLightDialog : public CDialog
{
public:
    CSceneLightDialog(CWnd *pParentWnd);
    virtual ~CSceneLightDialog() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnCancel() override;

private:
    DECLARE_MESSAGE_MAP();

    enum flags
    {
        LIGHTING_DIFFUSE = 2,
        LIGHTING_SPECULAR = 4,
    };

    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
    afx_msg void OnGreyscale();
    afx_msg void OnBoth();
    afx_msg void OnDiffuse();
    afx_msg void OnSpecular();
    afx_msg void OnAttenuation();

    void SetDistance(float distance);

    CSpinButtonCtrl m_startSpin;
    CSpinButtonCtrl m_endSpin;
    CSpinButtonCtrl m_distanceSpin;
    CSliderCtrl m_intensitySlider;
    CSliderCtrl m_blueSlider;
    CSliderCtrl m_greenSlider;
    CSliderCtrl m_redSlider;
    int m_diffuseRed;
    int m_diffuseGreen;
    int m_diffuseBlue;
    int m_specularRed;
    int m_specularGreen;
    int m_specularBlue;
    int m_flags;
    float m_farAttenStart;
    float m_farAttenEnd;
    float m_distance;
    float m_intensity;
    int m_attenuation;
};
