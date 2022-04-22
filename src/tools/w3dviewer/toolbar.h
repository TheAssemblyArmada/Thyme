/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View fancy toolbar
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

class CFancyToolbar : public CControlBar
{
public:
    CFancyToolbar();
    virtual ~CFancyToolbar() override;
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz) override;
    virtual CSize CalcDynamicLayout(int nLength, DWORD nMode) override;
    virtual void OnUpdateCmdUI(CFrameWnd *pTarget, BOOL bDisableIfNoHndler) override {}
    virtual void Unk(int unk) {}

    BOOL Create(LPCTSTR lpszWindowName, CWnd *pParentWnd, UINT nID);
    void AddItem(USHORT inactiveBitmapID, USHORT activeBitmapID, UINT id, BOOL activateOnPress);
    void SetStatus(UINT id, BOOL isActive, BOOL invalidate);

protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
    DECLARE_MESSAGE_MAP()

    void DoPaint();

    struct FancyToolbarItem
    {
        HBITMAP m_inactiveBitmap;
        HBITMAP m_activeBitmap;
        UINT m_id;
        BOOL m_isActive;
        BOOL m_activateOnPress;
        BOOL m_isVisible;
    };

    FancyToolbarItem m_items[10];
    int m_itemCount;
    int m_activeItem;
};
