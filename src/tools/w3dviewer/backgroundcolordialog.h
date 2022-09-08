/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Background Color Dialog
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

class CBackgroundColorDialog : public CDialog
{
public:
    CBackgroundColorDialog(CWnd *pParentWnd);
    virtual ~CBackgroundColorDialog() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnCancel() override;

private:
    DECLARE_MESSAGE_MAP();

    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
    afx_msg void OnGreyscale();

    CSliderCtrl m_blueSlider;
    CSliderCtrl m_greenSlider;
    CSliderCtrl m_redSlider;
    int m_currentRed;
    int m_currentGreen;
    int m_currentBlue;
};
