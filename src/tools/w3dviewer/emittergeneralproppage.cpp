/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter general prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emittergeneralproppage.h"
#include "colorbar.h"
#include "emitterinstancelist.h"
#include "emitterpropertysheet.h"
#include "resource.h"
#include "utils.h"

IMPLEMENT_DYNCREATE(EmitterGeneralPropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterGeneralPropPageClass, CPropertyPage)
    ON_COMMAND(IDC_BROWSE, OnBrowse)
    ON_EN_CHANGE(IDC_TEXTURE, OnTextureChanged)
    ON_EN_CHANGE(IDC_NAME, OnNameChanged)
    ON_EN_CHANGE(IDC_LIFETIME, OnLifetimeChanged)
    ON_CBN_SELCHANGE(IDC_SHADER, OnShaderChanged)
    ON_COMMAND(IDC_LIFETIMECHECK, OnLifetime)
    ON_EN_CHANGE(IDC_FUTURESTART, OnFutureStartTimeChanged)
END_MESSAGE_MAP()
// clang-format on

EmitterGeneralPropPageClass::EmitterGeneralPropPageClass() :
    CPropertyPage(IDD_EMITTER),
    m_instanceList(nullptr),
    m_sheet(nullptr),
    m_lifetime(0.0f),
    m_futureStartTime(0.0f),
    m_isValid(true)
{
    Initialize();
}

BOOL EmitterGeneralPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_TEXTURE:
        case IDC_LIFETIME:
        case IDC_NAME:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            break;
        case IDC_LIFETIMECHECK:
            if (HIWORD(wParam) == BN_CLICKED) {
                SetModified();
            }

            break;
        case IDC_RENDERMODE:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                SetModified(true);

                if (m_sheet != nullptr) {
                    m_sheet->UpdateRenderingMode(::SendMessage(reinterpret_cast<HWND>(lParam), CB_GETCURSEL, 0, 0));
                }
            }

            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterGeneralPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    if (hdr != nullptr && hdr->code == UDN_DELTAPOS) {
        NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);
        UpdateEditCtrl(hdr->hwndFrom, nm->iDelta);
    }

    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void EmitterGeneralPropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_FUTURESTARTSPIN, m_startTimeSpin);
    DDX_Control(pDX, IDC_RENDERMODE, m_renderMode);
    DDX_Control(pDX, IDC_LIFETIMESPIN, m_lifetimeSpin);
}

BOOL EmitterGeneralPropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    AddShader(&ShaderClass::s_presetAdditiveSpriteShader, "Additive");
    AddShader(&ShaderClass::s_presetAlphaSpriteShader, "Alpha");
    AddShader(&ShaderClass::s_presetATestSpriteShader, "Alpha-Test");
    AddShader(&ShaderClass::s_presetATestBlendSpriteShader, "Alpha-Test-Blend");
    AddShader(&ShaderClass::s_presetScreenSpriteShader, "Screen");
    AddShader(&ShaderClass::s_presetMultiplicativeSpriteShader, "Multiplicative");
    AddShader(&ShaderClass::s_presetOpaqueSpriteShader, "Opaque");
    SetDlgItemText(IDC_NAME, m_name);
    SetDlgItemText(IDC_TEXTURE, m_textureName);
    SendDlgItemMessage(IDC_LIFETIMECHECK, BM_SETCHECK, m_lifetime < 100.0f, 0);

    if (m_lifetime > 100.0f) {
        m_lifetime = 0.0f;
    }

    InitializeSpinButton(&m_lifetimeSpin, m_lifetime, 0.0f, 1000.0f);
    InitializeSpinButton(&m_startTimeSpin, m_futureStartTime, 0.0f, 1000.0f);
    OnLifetime();
    m_renderMode.SetCurSel(0);
    return TRUE;
}

BOOL EmitterGeneralPropPageClass::OnApply()
{
    GetDlgItemText(IDC_NAME, m_name);
    GetDlgItemText(IDC_TEXTURE, m_textureName);
    m_lifetime = GetDlgItemFloat(m_hWnd, IDC_LIFETIME);

    if (!SendDlgItemMessage(IDC_LIFETIMECHECK, BM_GETCHECK, 0, 0)) {
        m_lifetime = 5000000.0f;
    }

    m_futureStartTime = GetDlgItemFloat(m_hWnd, IDC_FUTURESTART);

    int index = SendDlgItemMessage(IDC_SHADER, CB_GETCURSEL, 0, 0);

    if (index != -1) {
        ShaderClass *shader = reinterpret_cast<ShaderClass *>(SendDlgItemMessage(IDC_SHADER, CB_GETITEMDATA, index, 0));

        if (shader != nullptr) {
            m_shader = *shader;
        }
    }

    if (m_name.GetLength() != 0) {
        m_instanceList->Set_Lifetime(m_lifetime);
        m_instanceList->Set_Future_Start_Time(m_futureStartTime);
        m_instanceList->Set_Texture_Filename(m_textureName);
        m_instanceList->Set_Name(m_name);
        m_instanceList->Set_Shader(m_shader);
        m_instanceList->Set_Render_Mode(m_renderMode.GetCurSel());
        m_isValid = TRUE;
        return CPropertyPage::OnApply();
    } else {
        MessageBoxA("Invalid emitter name.  Please enter a new name.", "Invalid settings", MB_ICONWARNING);
        m_isValid = FALSE;
        return FALSE;
    }
}

void EmitterGeneralPropPageClass::OnBrowse()
{
    CFileDialog dlg(TRUE,
        ".tga",
        0,
        OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Textures files (*.tga)|*.tga||",
        AfxGetMainWnd());

    if (dlg.DoModal() == IDOK) {
        SetDlgItemText(IDC_TEXTURE, dlg.GetPathName());
        SetModified();
    }
}

void EmitterGeneralPropPageClass::OnTextureChanged()
{
    SetModified();
}

void EmitterGeneralPropPageClass::OnNameChanged()
{
    SetModified();
}

void EmitterGeneralPropPageClass::OnLifetimeChanged()
{
    SetModified();
}

void EmitterGeneralPropPageClass::OnFutureStartTimeChanged()
{
    SetModified();
}

void EmitterGeneralPropPageClass::OnShaderChanged()
{
    SetModified();
}

void EmitterGeneralPropPageClass::OnLifetime()
{
    bool lifetime = SendDlgItemMessage(IDC_LIFETIMECHECK, BM_GETCHECK, 0, 0) == 1;
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_LIFETIME), lifetime);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_LIFETIMESPIN), lifetime);

    if (!lifetime) {
        m_lifetime = 0.0f;
        SetDlgItemFloat(m_hWnd, IDC_LIFETIME, 0.0f);
    }

    SetModified();
}

void EmitterGeneralPropPageClass::Initialize()
{
    if (m_instanceList != nullptr) {
        m_textureName = m_instanceList->Get_Texture_Filename();
        m_lifetime = m_instanceList->Get_Lifetime();
        m_futureStartTime = m_instanceList->Get_Future_Start_Time();
        m_name = m_instanceList->Get_Name();
        m_instanceList->Get_Shader(m_shader);
    }
}

void EmitterGeneralPropPageClass::AddShader(ShaderClass *shader, const char *name)
{
    int index = SendDlgItemMessage(IDC_SHADER, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name));

    if (index != -1) {
        SendDlgItemMessage(IDC_SHADER, CB_SETITEMDATA, index, reinterpret_cast<LPARAM>(shader));
        uint32_t bits = shader->Get_Bits() ^ m_shader.Get_Bits();

        if ((bits & (ShaderClass::MASK_ALPHATEST | ShaderClass::MASK_SRCBLEND)) == 0
            && (bits & ShaderClass::MASK_DSTBLEND) == 0) {
            SendDlgItemMessage(IDC_SHADER, CB_SETCURSEL, index, 0);
        }
    }
}
