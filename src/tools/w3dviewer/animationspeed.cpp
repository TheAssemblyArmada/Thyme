/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Scale Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "animationspeed.h"
#include "graphicview.h"
#include "mainfrm.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(CAnimationSpeed, CDialog)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_BLENDFRAMES, OnBlendFrames)
END_MESSAGE_MAP()
// clang-format on

CAnimationSpeed::CAnimationSpeed(CWnd *pParentWnd) : CDialog(IDD_ANIMSETTINGS, pParentWnd), m_speed(0) {}

void CAnimationSpeed::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_FRAMESLIDER, m_speedSlider);
}

BOOL CAnimationSpeed::OnInitDialog()
{
    CDialog::OnInitDialog();
    PositionWindow(m_hWnd);
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        SendDlgItemMessage(IDC_BLENDFRAMES, BM_SETCHECK, doc->m_blendFrames);
    }

    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

        if (view != nullptr) {
            m_speed = view->m_animationSpeed * 100.0f;
        }
    }

    m_speedSlider.SetRange(1, 200);
    m_speedSlider.SetPos(m_speed);

    return TRUE;
}

void CAnimationSpeed::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    m_speed = m_speedSlider.GetPos();

    CMainFrame *frame = static_cast<CMainFrame *>(AfxGetMainWnd());

    if (frame != nullptr) {
        CGraphicView *view = static_cast<CGraphicView *>(frame->m_splitter.GetPane(0, 1));

        if (view != nullptr) {
            view->m_animationSpeed = m_speed / 100.0f;
        }
    }

    CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAnimationSpeed::OnDestroy()
{
    m_speed = m_speedSlider.GetPos();
    CWnd::OnDestroy();
}

void CAnimationSpeed::OnBlendFrames()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        doc->m_blendFrames = SendDlgItemMessage(IDC_BLENDFRAMES, BM_GETCHECK);
    }
}
