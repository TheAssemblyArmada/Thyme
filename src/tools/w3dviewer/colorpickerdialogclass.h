/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View color picker dialog
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

class ColorBarClass;
class ColorPickerClass;

class ColorPickerDialogClass : public CDialog
{
public:
    ColorPickerDialogClass(int red, int green, int blue, CWnd *pParentWnd, UINT nIDTemplate);
    virtual ~ColorPickerDialogClass() override {}
    virtual BOOL OnInitDialog() override;

    static int DoDialog(int *red, int *green, int *blue);

private:
    enum flags
    {
        SET_SLIDERS = 1,
        SET_SATURATION = 2,
        SET_COLOR = 4,
    };

    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
    virtual void PostNcDestroy() override;

    void SetColor(float red, float green, float blue, int flags);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnReset();

    CSpinButtonCtrl m_blueSpin;
    CSpinButtonCtrl m_greenSpin;
    CSpinButtonCtrl m_redSpin;
    float m_originalRed;
    float m_originalGreen;
    float m_originalBlue;
    float m_currentRed;
    float m_currentGreen;
    float m_currentBlue;
    ColorBarClass *m_currentColor;
    ColorBarClass *m_originalColor;
    ColorBarClass *m_red;
    ColorBarClass *m_green;
    ColorBarClass *m_blue;
    ColorBarClass *m_saturation;
    ColorPickerClass *m_colorPicker;
    bool m_created;
    void (*m_setColorCallback)(int red, int green, int blue, void *data);
    void *m_setColorCallbackData;
};
