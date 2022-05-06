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
#include "toolbar.h"

// clang-format off
BEGIN_MESSAGE_MAP(CFancyToolbar, CControlBar)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()
// clang-format on

CFancyToolbar::CFancyToolbar() : m_itemCount(0), m_activeItem(-1)
{
    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(wndcls));

    if (!GetClassInfo(AfxGetInstanceHandle(), "FANCYTOOLBAR", &wndcls)) {
        wndcls.style = WS_EX_TOOLWINDOW;
        wndcls.lpfnWndProc = ::DefWindowProc;
        wndcls.hInstance = AfxGetInstanceHandle();
        wndcls.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndcls.hbrBackground = (HBRUSH)COLOR_BTNFACE;
        wndcls.lpszClassName = "FANCYTOOLBAR";
        RegisterClass(&wndcls);
    }

    memset(&m_items, 0, sizeof(m_items));
}

CFancyToolbar::~CFancyToolbar()
{
    for (int i = 0; i < m_itemCount; i++) {
        if (m_items[i].m_inactiveBitmap != nullptr) {
            DeleteObject(m_items[i].m_inactiveBitmap);
            m_items[i].m_inactiveBitmap = nullptr;
        }

        if (m_items[i].m_activeBitmap != nullptr) {
            DeleteObject(m_items[i].m_activeBitmap);
            m_items[i].m_activeBitmap = nullptr;
        }
    }
}

BOOL CFancyToolbar::PreCreateWindow(CREATESTRUCT &cs)
{
    return CControlBar::PreCreateWindow(cs);
}

CSize CFancyToolbar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    return CSize(42 * m_itemCount + 16, 52);
}

CSize CFancyToolbar::CalcDynamicLayout(int nLength, DWORD nMode)
{
    return CSize(42 * m_itemCount + 16, 52);
}

BOOL CFancyToolbar::Create(LPCTSTR lpszWindowName, CWnd *pParentWnd, UINT nID)
{
    RECT rect;
    memset(&rect, 0, sizeof(rect));

    BOOL ret = CWnd::Create("FANCYTOOLBAR", lpszWindowName, WS_CHILD | WS_VISIBLE, rect, pParentWnd, nID);

    if (ret) {
        SetBarStyle(m_dwStyle | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
        EnableDocking(CBRS_ALIGN_ANY);
    }

    return ret;
}

void CFancyToolbar::AddItem(USHORT inactiveBitmapID, USHORT activeBitmapID, UINT id, BOOL activateOnPress)
{
    int count = m_itemCount;
    m_itemCount = count + 1;
    m_items[count].m_inactiveBitmap = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(inactiveBitmapID));
    m_items[count].m_activeBitmap = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(activeBitmapID));
    m_items[count].m_id = id;
    m_items[count].m_activateOnPress = activateOnPress;
    m_items[count].m_isActive = FALSE;
    m_items[count].m_isVisible = TRUE;
}

void CFancyToolbar::SetStatus(UINT id, BOOL isActive, BOOL invalidate)
{
    BOOL found = FALSE;

    for (int i = 0; i < m_itemCount; i++) {
        if (found) {
            break;
        }

        if (m_items[i].m_id == id) {
            m_items[i].m_isActive = isActive;

            if (invalidate) {
                Invalidate();
                UpdateWindow();
            }
        }
    }
}

void CFancyToolbar::OnPaint()
{
    DoPaint();
}

void CFancyToolbar::OnLButtonDown(UINT nFlags, CPoint pt)
{
    int id = -1;
    int pos = 8;

    for (int i = 0; i < m_itemCount; i++) {
        if (id != -1) {
            break;
        }

        if (m_items[i].m_isVisible && pt.x >= pos && pt.x <= pos + 42 && pt.y >= 8 && pt.y <= 44) {
            id = i;
        }

        pos += 42;
    }

    if (id >= 0) {
        m_items[id].m_isActive = !m_items[id].m_isActive;

        HBITMAP bitmap = m_items[id].m_inactiveBitmap;

        if (m_items[id].m_isActive) {
            bitmap = m_items[id].m_activeBitmap;
        }

        HDC dc = ::GetDC(m_hWnd);

        if (dc != nullptr) {
            RECT r;
            GetClientRect(&r);
            int cx = ((r.bottom - r.top) >> 1) - 18;
            HDC dc2 = CreateCompatibleDC(dc);

            if (dc2 != nullptr) {
                HGDIOBJ obj = SelectObject(dc2, bitmap);
                BitBlt(dc, 42 * id + 8, cx, 42, 36, dc2, 0, 0, SRCCOPY);
                SelectObject(dc2, obj);
                DeleteDC(dc2);
            }

            ::ReleaseDC(m_hWnd, dc);
        }

        if (!m_items[id].m_activateOnPress) {
            m_activeItem = id;
            ::SetCapture(m_hWnd);
        } else {
            m_activeItem = -1;
            AfxGetMainWnd()->PostMessage(WM_COMMAND, MAKEWPARAM(m_items[id].m_id, 0), (LPARAM)m_hWnd);
        }
    }
}

void CFancyToolbar::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_activeItem >= 0) {
        int id = -1;
        int pos = 8;

        for (int i = 0; i < m_itemCount; i++) {
            if (id != -1) {
                break;
            }

            if (m_items[i].m_isVisible && pt.x >= pos && pt.x <= pos + 42 && pt.y >= 8 && pt.y <= 44) {
                id = i;
            }

            pos += 42;
        }

        if (id == m_activeItem) {
            AfxGetMainWnd()->PostMessage(WM_COMMAND, MAKEWPARAM(m_items[id].m_id, 0), (LPARAM)m_hWnd);
        }

        m_items[m_activeItem].m_isActive = FALSE;

        HDC dc = ::GetDC(m_hWnd);

        if (dc != nullptr) {
            RECT r;
            GetClientRect(&r);
            int cx = ((r.bottom - r.top) >> 1) - 18;
            HDC dc2 = CreateCompatibleDC(dc);

            if (dc2 != nullptr) {
                HGDIOBJ obj = SelectObject(dc2, m_items[m_activeItem].m_inactiveBitmap);
                BitBlt(dc, 42 * m_activeItem + 8, cx, 42, 36, dc2, 0, 0, SRCCOPY);
                SelectObject(dc2, obj);
                DeleteDC(dc2);
            }

            ::ReleaseDC(m_hWnd, dc);
        }

        ReleaseCapture();
    } else {
        CControlBar::OnLButtonUp(nFlags, pt);
    }
}

void CFancyToolbar::DoPaint()
{
    HDC dc = ::GetDC(m_hWnd);

    if (dc != nullptr) {
        RECT r;
        GetClientRect(&r);
        HBRUSH brush = CreateSolidBrush(0xC0C0C0);
        FillRect(dc, &r, brush);
        DeleteObject(brush);
        int pos = 8;

        for (int i = 0; i < m_itemCount; i++) {
            if (m_items[i].m_isVisible) {
                HBITMAP bitmap = m_items[i].m_inactiveBitmap;

                if (m_items[i].m_isActive) {
                    bitmap = m_items[i].m_activeBitmap;
                }

                int cx = ((r.bottom - r.top) >> 1) - 18;
                HDC dc2 = CreateCompatibleDC(dc);

                if (dc2 != nullptr) {
                    HGDIOBJ obj = SelectObject(dc2, bitmap);
                    BitBlt(dc, pos, cx, 42, 36, dc2, 0, 0, SRCCOPY);
                    SelectObject(dc2, obj);
                    DeleteDC(dc2);
                }
            }

            pos += 42;
        }

        ::ReleaseDC(m_hWnd, dc);
    }

    ValidateRect(nullptr);
}
