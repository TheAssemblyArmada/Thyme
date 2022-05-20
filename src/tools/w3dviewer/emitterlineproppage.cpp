/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter line prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterlineproppage.h"
#include "emitterinstancelist.h"
#include "resource.h"
#include "utils.h"

IMPLEMENT_DYNCREATE(EmitterLinePropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterLinePropPageClass, CPropertyPage)
END_MESSAGE_MAP()
// clang-format on

EmitterLinePropPageClass::EmitterLinePropPageClass() :
    CPropertyPage(IDD_EMITTERLINE),
    m_instanceList(nullptr),
    m_isValid(true),
    m_textureMappingMode(W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP),
    m_mergeIntersections(false),
    m_endCaps(false),
    m_disableSorting(false),
    m_subdivisionLevel(0),
    m_noiseAmplitude(0.0f),
    m_mergeAbortFactor(0.0f),
    m_textureTileFactor(0.0f),
    m_uPerSec(0.0f),
    m_vPerSec(0.0f)
{
}

BOOL EmitterLinePropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam)) {
        case IDC_MERGEABORT:
        case IDC_UVTILE:
        case IDC_UPERSEC:
        case IDC_VPERSEC:
        case IDC_SUBDIVISION:
        case IDC_AMPLITUDE:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            break;
        case IDC_MERGE:
        case IDC_ENDCAPS:
        case IDC_SORTING:
            if (HIWORD(wParam) == BN_CLICKED) {
                SetModified();
            }

            break;
        case IDC_MAPPING:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                SetModified();
            }
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterLinePropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

void EmitterLinePropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_MAPPING, m_mapping);
    DDX_Control(pDX, IDC_MERGEABORTSPIN, m_mergeAbortSpin);
    DDX_Control(pDX, IDC_VPERSECSPIN, m_vPerSecSpin);
    DDX_Control(pDX, IDC_UVTILESPIN, m_uvTileSpin);
    DDX_Control(pDX, IDC_UPERSECSPIN, m_uPerSecSpin);
    DDX_Control(pDX, IDC_AMPLITUDESPIN, m_amplitudeSpin);
    DDX_Control(pDX, IDC_SUBDIVISIONSPIN, m_subdivisionSpin);
}

BOOL EmitterLinePropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_subdivisionSpin.SetRange(0, 8);
    m_subdivisionSpin.SetPos(m_subdivisionLevel);
    InitializeSpinButton(&m_amplitudeSpin, m_noiseAmplitude, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_mergeAbortSpin, m_mergeAbortFactor, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_uvTileSpin, m_textureTileFactor, 0.0f, 8.0f);
    InitializeSpinButton(&m_uPerSecSpin, m_uPerSec, 0.0f, 32.0f);
    InitializeSpinButton(&m_vPerSecSpin, m_vPerSec, 0.0f, 32.0f);
    m_mapping.SetCurSel(m_textureMappingMode);
    SendDlgItemMessage(IDC_MERGE, BM_SETCHECK, m_mergeIntersections, 0);
    SendDlgItemMessage(IDC_ENDCAPS, BM_SETCHECK, m_endCaps, 0);
    SendDlgItemMessage(IDC_SORTING, BM_SETCHECK, m_disableSorting, 0);
    DisableWindows(m_hWnd, m_instanceList->Get_Render_Mode() == W3D_EMITTER_RENDER_MODE_LINE);
    return TRUE;
}

BOOL EmitterLinePropPageClass::OnApply()
{
    m_subdivisionLevel = GetDlgItemInt(IDC_SUBDIVISION);
    m_noiseAmplitude = GetDlgItemFloat(m_hWnd, IDC_AMPLITUDE);
    m_mergeAbortFactor = GetDlgItemFloat(m_hWnd, IDC_MERGEABORT);
    m_textureTileFactor = GetDlgItemFloat(m_hWnd, IDC_UVTILE);
    m_uPerSec = GetDlgItemFloat(m_hWnd, IDC_UPERSEC);
    m_vPerSec = GetDlgItemFloat(m_hWnd, IDC_VPERSEC);
    m_textureMappingMode = SendDlgItemMessage(IDC_MAPPING, CB_GETCURSEL, 0, 0);
    m_mergeIntersections = SendDlgItemMessage(IDC_MERGE, BM_GETCHECK, 0, 0) != 0;
    m_endCaps = SendDlgItemMessage(IDC_ENDCAPS, BM_GETCHECK, 0, 0) != 0;
    m_disableSorting = SendDlgItemMessage(IDC_SORTING, BM_GETCHECK, 0, 0) != 0;
    m_instanceList->Set_Subdivision_Level(m_subdivisionLevel);
    m_instanceList->Set_Noise_Amplitude(m_noiseAmplitude);
    m_instanceList->Set_Merge_Abort_Factor(m_mergeAbortFactor);
    m_instanceList->Set_Texture_Tile_Factor(m_textureTileFactor);
    m_instanceList->Set_UV_Offset_Rate(Vector2(m_uPerSec, m_vPerSec));
    m_instanceList->Set_Line_Texture_Mapping_Mode(m_textureMappingMode);
    m_instanceList->Set_Merge_Intersections(m_mergeIntersections);
    m_instanceList->Set_Disable_Sorting(m_disableSorting);
    m_instanceList->Set_End_Caps(m_endCaps);
    return CPropertyPage::OnApply();
}

void EmitterLinePropPageClass::Initialize()
{
    if (m_instanceList != nullptr) {
        m_textureMappingMode = m_instanceList->Get_Line_Texture_Mapping_Mode();
        m_mergeIntersections = m_instanceList->Is_Merge_Intersections() != 0;
        m_disableSorting = m_instanceList->Is_Sorting_Disabled() != 0;
        m_endCaps = m_instanceList->Are_End_Caps_Enabled() != 0;
        m_subdivisionLevel = m_instanceList->Get_Subdivision_Level();
        m_noiseAmplitude = m_instanceList->Get_Noise_Amplitude();
        m_mergeAbortFactor = m_instanceList->Get_Merge_Abort_Factor();
        m_textureTileFactor = m_instanceList->Get_Texture_Tile_Factor();
        m_uPerSec = m_instanceList->Get_UV_Offset_Rate().U;
        m_vPerSec = m_instanceList->Get_UV_Offset_Rate().V;
    }
}
