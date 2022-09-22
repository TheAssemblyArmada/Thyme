/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Animation Speed Dialog
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

class CAnimationSpeed : public CDialog
{
public:
    CAnimationSpeed(CWnd *pParentWnd);
    virtual ~CAnimationSpeed() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;

private:
    DECLARE_MESSAGE_MAP();

    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
    afx_msg void OnDestroy();
    afx_msg void OnBlendFrames();

    CSliderCtrl m_speedSlider;
    float m_speed;
};
