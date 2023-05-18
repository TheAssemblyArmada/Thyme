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
#include "colorpickerdialogclass.h"
#include "colorbar.h"
#include "colorpicker.h"
#include "resource.h"

// clang-format off
BEGIN_MESSAGE_MAP(ColorPickerDialogClass, CDialog)
	ON_COMMAND(IDABORT, OnReset)
END_MESSAGE_MAP()
// clang-format on

ColorPickerDialogClass::ColorPickerDialogClass(int red, int green, int blue, CWnd *pParentWnd, UINT nIDTemplate) :
    CDialog(nIDTemplate, pParentWnd),
    m_originalRed(red),
    m_originalGreen(green),
    m_originalBlue(blue),
    m_currentRed(red),
    m_currentGreen(green),
    m_currentBlue(blue),
    m_currentColor(0),
    m_originalColor(0),
    m_red(0),
    m_green(0),
    m_blue(0),
    m_saturation(0),
    m_colorPicker(nullptr),
    m_created(false),
    m_setColorCallback(nullptr)
{
}

BOOL ColorPickerDialogClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_blueSpin.SetRange(0, 255);
    m_greenSpin.SetRange(0, 255);
    m_redSpin.SetRange(0, 255);
    m_blueSpin.SetPos(m_currentBlue);
    m_greenSpin.SetPos(m_currentGreen);
    m_redSpin.SetPos(m_currentRed);

    m_currentColor = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_CURRENTBAR), "CLASSPOINTER"));
    m_originalColor = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_ORIGINALBAR), "CLASSPOINTER"));
    m_red = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_REDBAR), "CLASSPOINTER"));
    m_green = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_GREENBAR), "CLASSPOINTER"));
    m_blue = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_BLUEBAR), "CLASSPOINTER"));
    m_saturation = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_SATURATIONBAR), "CLASSPOINTER"));
    m_colorPicker = static_cast<ColorPickerClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_COLORPICKER), "CLASSPOINTER"));

    m_originalColor->SetColor(0, 0.0f, m_originalRed, m_originalGreen, m_originalBlue, 3);
    m_colorPicker->SetColor(m_originalRed, m_originalGreen, m_originalBlue);

    m_red->SetMinMax(0.0f, 255.0f);
    m_green->SetMinMax(0.0f, 255.0f);
    m_blue->SetMinMax(0.0f, 255.0f);
    m_saturation->SetMinMax(0.0f, 255.0f);

    m_saturation->AddColor(1, 255.0f, 255.0f, 255.0f, 255.0f, 3);

    m_red->SetSliderPos(m_currentRed);
    m_red->SetColor(0, 0.0f, 0.0f, m_currentGreen, m_currentBlue, 3);
    m_red->SetColor(1, 255.0f, 255.0f, m_currentGreen, m_currentBlue, 3);

    m_green->SetSliderPos(m_currentGreen);
    m_green->SetColor(0, 0.0f, m_currentRed, 0.0f, m_currentBlue, 3);
    m_green->SetColor(1, 255.0f, m_currentRed, 255.0f, m_currentBlue, 3);

    m_blue->SetSliderPos(m_currentBlue);
    m_blue->SetColor(0, 0.0f, m_currentRed, m_currentGreen, 0.0f, 3);
    m_blue->SetColor(1, 255.0f, m_currentRed, m_currentGreen, 255.0f, 3);

    m_currentColor->SetColor(0, 0.0f, m_currentRed, m_currentGreen, m_currentBlue, 3);

    int cpred = 0;
    int cpgreen = 0;
    int cpblue = 0;
    m_colorPicker->GetColor(&cpred, &cpgreen, &cpblue);
    float color = m_currentRed;

    if (color >= m_currentGreen) {
        color = m_currentGreen;
    }

    if (color >= m_currentBlue) {
        color = m_currentBlue;
    }

    m_saturation->SetSliderPos(color);
    m_saturation->SetColor(0, 0.0f, cpred, cpgreen, cpblue, 3);
    return TRUE;
}

int ColorPickerDialogClass::DoDialog(int *red, int *green, int *blue)
{
    int ret = 0;
    ColorPickerDialogClass dlg(*red, *green, *blue, nullptr, IDD_COLORPICKER);

    if (dlg.DoModal() == 1) {
        *red = dlg.m_currentRed;
        *green = dlg.m_currentGreen;
        *blue = dlg.m_currentBlue;
        ret = 1;
    }

    return ret;
}

BOOL ColorPickerDialogClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (LOWORD(wParam) >= IDC_REDEDIT && LOWORD(wParam) <= IDC_BLUEEDIT && HIWORD(wParam) == EN_KILLFOCUS) {
        float red = GetDlgItemInt(IDC_REDEDIT);
        float green = GetDlgItemInt(IDC_GREENEDIT);
        float blue = GetDlgItemInt(IDC_BLUEEDIT);
        SetColor(red, green, blue, SET_SLIDERS | SET_SATURATION | SET_COLOR);
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL ColorPickerDialogClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    switch (hdr->idFrom) {
        case IDC_COLORPICKER:
            if (hdr->code == CLPN_COLORCHANGED) {
                ColorPickerNotify *cpn = reinterpret_cast<ColorPickerNotify *>(lParam);
                float saturation = m_saturation->GetSliderPos() / 255.0f;
                float blue = (255.0 - cpn->blue) * saturation + cpn->blue;
                float green = (255.0 - cpn->green) * saturation + cpn->green;
                float red = (255.0 - cpn->red) * saturation + cpn->red;
                SetColor(red, green, blue, SET_SLIDERS | SET_SATURATION);
            }
            break;
        case IDC_SATURATIONBAR:
            if (hdr->code = CLBN_SLIDERCHANGE) {
                ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);
                SetColor(cbn->red, cbn->green, cbn->blue, SET_SLIDERS);
            }
            break;
        case IDC_REDBAR:
            if (hdr->code = CLBN_SLIDERCHANGE) {
                ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);
                SetColor(cbn->red, m_currentGreen, m_currentBlue, SET_SLIDERS | SET_SATURATION | SET_COLOR);
            }
            break;
        case IDC_GREENBAR:
            if (hdr->code = CLBN_SLIDERCHANGE) {
                ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);
                SetColor(m_currentRed, cbn->green, m_currentBlue, SET_SLIDERS | SET_SATURATION | SET_COLOR);
            }
            break;
        case IDC_BLUEBAR:
            if (hdr->code = CLBN_SLIDERCHANGE) {
                ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);
                SetColor(m_currentRed, m_currentGreen, cbn->blue, SET_SLIDERS | SET_SATURATION | SET_COLOR);
            }
            break;
        case IDC_BLUESPIN:
        case IDC_GREENSPIN:
        case IDC_REDSPIN:
            if (hdr->code = UDN_DELTAPOS) {
                int red = m_redSpin.GetPos();
                int green = m_greenSpin.GetPos();
                int blue = m_blueSpin.GetPos();
                SetColor(red, green, blue, SET_SLIDERS | SET_SATURATION | SET_COLOR);
            }
            break;
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

void ColorPickerDialogClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_BLUESPIN, m_blueSpin);
    DDX_Control(pDX, IDC_GREENSPIN, m_greenSpin);
    DDX_Control(pDX, IDC_REDSPIN, m_redSpin);
}

LRESULT ColorPickerDialogClass::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    return CWnd::WindowProc(message, wParam, lParam);
}

void ColorPickerDialogClass::PostNcDestroy()
{
    CWnd::PostNcDestroy();

    if (m_created) {
        delete this;
    }
}

void ColorPickerDialogClass::SetColor(float red, float green, float blue, int flags)
{
    m_currentRed = red;
    m_currentGreen = green;
    m_currentBlue = blue;

    if (blue != m_blueSpin.GetPos()) {
        m_blueSpin.SetPos(blue);
    }

    if (green != m_greenSpin.GetPos()) {
        m_greenSpin.SetPos(green);
    }

    if (red != m_redSpin.GetPos()) {
        m_redSpin.SetPos(red);
    }

    ::UpdateWindow(::GetDlgItem(m_hWnd, IDC_REDEDIT));
    ::UpdateWindow(::GetDlgItem(m_hWnd, IDC_GREENEDIT));
    ::UpdateWindow(::GetDlgItem(m_hWnd, IDC_BLUEEDIT));

    if ((flags & SET_SLIDERS) != 0) {
        m_red->SetSliderPos(m_currentRed);
        m_red->SetColor(0, 0.0f, 0.0f, m_currentGreen, m_currentBlue, 3);
        m_red->SetColor(1, 255.0f, 255.0f, m_currentGreen, m_currentBlue, 3);

        m_green->SetSliderPos(m_currentGreen);
        m_green->SetColor(0, 0.0f, m_currentRed, 0.0f, m_currentBlue, 3);
        m_green->SetColor(1, 255.0f, m_currentRed, 255.0f, m_currentBlue, 3);

        m_blue->SetSliderPos(m_currentBlue);
        m_blue->SetColor(0, 0.0f, m_currentRed, m_currentGreen, 0.0f, 3);
        m_blue->SetColor(1, 255.0f, m_currentRed, m_currentGreen, 255.0f, 3);
    }

    if ((flags & SET_COLOR) != 0) {
        m_colorPicker->SetColor(red, green, blue);
    }

    if ((flags & SET_SATURATION) != 0) {
        int cpred = 0;
        int cpgreen = 0;
        int cpblue = 0;
        m_colorPicker->GetColor(&cpred, &cpgreen, &cpblue);
        float color = m_currentRed;

        if (color >= m_currentGreen) {
            color = m_currentGreen;
        }

        if (color >= m_currentBlue) {
            color = m_currentBlue;
        }

        m_saturation->SetSliderPos(color);
        m_saturation->SetColor(0, 0.0f, cpred, cpgreen, cpblue, 3);
    }

    m_currentColor->SetColor(0, 0.0f, m_currentRed, m_currentGreen, m_currentBlue, 3);

    if (m_setColorCallback != nullptr) {
        m_setColorCallback(m_currentRed, m_currentGreen, m_currentBlue, m_setColorCallbackData);
    }
}

void ColorPickerDialogClass::OnReset()
{
    SetColor(m_originalRed, m_originalGreen, m_originalBlue, SET_SLIDERS | SET_SATURATION | SET_COLOR);
}
