/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View color bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorbar.h"
#include "colorutils.h"
#include "resource.h"

// clang-format off
BEGIN_MESSAGE_MAP(ColorBarClass, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
// clang-format on

ColorBarClass::ColorBarClass() :
    m_DIBSection(nullptr),
    m_keyDIBSection(nullptr),
    m_deviceContext(nullptr),
    m_DIBSectionBits(nullptr),
    m_keyDIBSectionBits(nullptr),
    m_width(0),
    m_height(0),
    m_keyWidth(0),
    m_keyHeight(0),
    m_stride(0),
    m_sliderMin(0.0f),
    m_sliderMax(1.0f),
    m_currentKey(0),
    m_mouseDown(false),
    m_mouseMoved(false),
    m_redrawEnabled(true),
    m_sliderPos(0.0f)
{
    memset(m_keys, 0, sizeof(m_keys));
    m_keyCount = 1;
    m_keys[0].m_red = 0.0f;
    m_keys[0].m_green = 0.0f;
    m_keys[0].m_blue = 0.0f;
    m_keys[0].m_keyFlags = 1;
    m_keys[0].m_keyTime = 0.0f;
}

ColorBarClass::~ColorBarClass()
{
    if (m_deviceContext != nullptr) {
        DeleteObject(m_deviceContext);
        m_deviceContext = nullptr;
    }

    FreeKeyBitmap();
    FreeDibSection();
}

BOOL ColorBarClass::Create(LPCTSTR lpszClassName,
    LPCTSTR lpszWindowName,
    DWORD dwStyle,
    const RECT &rect,
    CWnd *pParentWnd,
    UINT nID,
    CCreateContext *pContext)
{
    HWND parent;

    if (pParentWnd != nullptr) {
        parent = pParentWnd->m_hWnd;
    } else {
        parent = nullptr;
    }

    return CreateWindowEx(0,
               "WWCOLORBAR",
               lpszWindowName,
               dwStyle,
               rect.left,
               rect.top,
               rect.right - rect.left,
               rect.bottom - rect.top,
               parent,
               reinterpret_cast<HMENU>(nID),
               AfxGetInstanceHandle(),
               static_cast<LPVOID>(this))
        != 0;
}

void ColorBarClass::RegisterWndClass(HINSTANCE instance)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));

    if (!GetClassInfo(instance, "WWCOLORBAR", &wc)) {
        wc.style = CS_GLOBALCLASS | CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = instance;
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNSHADOW);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "WWCOLORBAR";
        RegisterClass(&wc);
    }
}

LRESULT CALLBACK ColorBarClass::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg) {
        case WM_CREATE:
            if (lParam) {
                ColorBarClass *bar = *reinterpret_cast<ColorBarClass **>(lParam);
                bool created = false;

                if (bar == nullptr) {
                    bar = new ColorBarClass();
                    created = true;
                }

                bar->Attach(hWnd);
                bar->OnCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));

                WNDPROC *wp = bar->GetSuperWndProcAddr();

                if (wp != nullptr) {
                    *wp = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)AfxGetAfxWndProc()));
                }

                SetProp(hWnd, "CLASSPOINTER", bar);
                SetProp(hWnd, "CREATED", reinterpret_cast<HANDLE>(created));
            }

            break;
        case WM_DESTROY: {
            ColorBarClass *bar = static_cast<ColorBarClass *>(GetProp(hWnd, "CLASSPOINTER"));
            HANDLE created = GetProp(hWnd, "CREATED");

            if (bar != nullptr) {
                bar->Detach();
                WNDPROC *wp = bar->GetSuperWndProcAddr();

                if (wp != nullptr) {
                    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)*wp);
                    *wp = nullptr;
                }

                if (created != nullptr) {
                    delete bar;
                }
            }

            break;
        }
    }

    return ::DefWindowProc(hWnd, Msg, wParam, lParam);
}

bool ColorBarClass::GetColor(int key, float *keytime, float *red, float *green, float *blue)
{
    if (key >= 0 && key < m_keyCount) {
        if (keytime != nullptr) {
            *keytime = (m_sliderMax - m_sliderMin) * m_keys[key].m_keyTime + m_sliderMin;
        }

        if (red != nullptr) {
            *red = m_keys[key].m_red;
        }

        if (green != nullptr) {
            *green = m_keys[key].m_green;
        }

        if (blue != nullptr) {
            *blue = m_keys[key].m_blue;
        }
    }

    return false;
}

bool ColorBarClass::AddColor(int key, float keytime, float red, float green, float blue, int flags)
{
    if (m_keyCount + 1 < 15 && key >= 0 && key <= m_keyCount) {
        memcpy(&m_keys[key + 1], &m_keys[key], sizeof(ColorBarKey) * m_keyCount - sizeof(ColorBarKey) * key);
        m_keyCount++;
        return SetColor(key, keytime, red, green, blue, flags);
    }

    return false;
}

bool ColorBarClass::SetColor(int key, float keytime, float red, float green, float blue, int flags)
{
    bool b = false;

    if (key < 0 || key >= m_keyCount) {
        int current = m_keyCount;
        if (key >= current) {
            b = false;

            if (current + 1 < 15 && current >= 0) {
                m_keyCount++;
                return SetColor(current, keytime, red, green, blue, 3);
            }
        }
    } else {
        m_keys[key].m_keyTime = (keytime - m_sliderMin) / (m_sliderMax - m_sliderMin);
        m_keys[key].m_red = red;
        m_keys[key].m_green = green;
        m_keys[key].m_blue = blue;
        m_keys[key].m_keyFlags = flags;

        if (key == 0) {
            m_keys[key].m_keyFlags = flags & ~2;
        }

        UpdateDeltas();
        Refresh();
        return true;
    }

    return b;
}

void ColorBarClass::SetMinMax(float min, float max)
{
    m_sliderMin = min;
    m_sliderMax = max;
    m_sliderPos = min;
}

void ColorBarClass::SetSliderPos(float pos)
{
    if (pos <= m_sliderMin) {
        pos = m_sliderMin;
    }

    if (pos < m_sliderMax) {
        UpdateSliderPos(pos, false);
    } else {
        UpdateSliderPos(m_sliderMax, false);
    }
}

void ColorBarClass::Clear()
{
    m_keyCount = 1;
    UpdateDeltas();
    Refresh();
}

void ColorBarClass::SetAlphaVector(int key, AlphaVectorStruct *vector)
{
    if (key >= 0 && key < m_keyCount) {
        m_keys[key].m_alphaVector = vector;
    }
}

AlphaVectorStruct *ColorBarClass::GetAlphaVector(int key)
{
    if (key >= 0 && key < m_keyCount) {
        return m_keys[key].m_alphaVector;
    }

    return nullptr;
}

bool ColorBarClass::SetGradientValue(int key, float value)
{
    if (key >= 0 && key < m_keyCount) {
        m_keys[key].m_gradientValue = value;
        UpdateDeltas();
        Refresh();
        return true;
    }

    return false;
}

float ColorBarClass::GetGradientValue(int key)
{
    if (key >= 0 && key < m_keyCount) {
        return m_keys[key].m_gradientValue;
    }

    return 0.0f;
}

void ColorBarClass::EnableRedraw(bool redraw)
{
    m_redrawEnabled = redraw;

    if (redraw) {
        UpdateWindow();
    }
}

int ColorBarClass::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int result = CWnd::OnCreate(lpCreateStruct);

    if (result != -1) {
        m_deviceContext = CreateCompatibleDC(nullptr);
        Resize();
        return 0;
    }

    return result;
}

void ColorBarClass::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    Resize();
}

void ColorBarClass::OnPaint()
{
    CPaintDC dc(this);
    Paint(dc.GetSafeHdc());
}

void ColorBarClass::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }

    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if ((style & CLBS_HASKEYS) == 0) {
        if ((style & CLBS_HASSLIDER) != 0) {
            SetSliderPosFromPoint(pt.x, pt.y, true);
            m_mouseDown = true;
            m_mouseMoved = false;
            SetCapture();
        }

        CWnd::OnLButtonDown(nFlags, pt);
        return;
    }

    int key = FindKey(pt.x, pt.y);

    if (key != -1) {
        if (m_currentKey != key) {
            m_currentKey = key;
            Refresh();
        }

        if (((m_keys[m_currentKey].m_keyFlags & 2) != 0)) {
            m_mouseDown = true;
            m_mouseMoved = false;
            SetCapture();
            CWnd::OnLButtonDown(nFlags, pt);
            return;
        }

        CWnd::OnLButtonDown(nFlags, pt);
        return;
    }

    if (GetAsyncKeyState(VK_CONTROL) < 0) {
        AddKey(pt.x, pt.y, 3);
        SendNotification(CLBN_ADDKEY, m_currentKey);
    }

    m_mouseDown = false;
    m_mouseMoved = false;
    ReleaseCapture();
    CWnd::OnLButtonDown(nFlags, pt);
}

void ColorBarClass::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_mouseDown) {
        ReleaseCapture();
        m_mouseDown = false;

        if (m_mouseMoved) {
            SendNotification(CLBN_KEYCHANGED, m_currentKey);
        }
    }

    CWnd::OnLButtonUp(nFlags, pt);
}

void ColorBarClass::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_mouseDown) {
        CWnd::OnMouseMove(nFlags, point);
        return;
    }

    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if ((style & CLBS_HASKEYS) == 0) {
        if ((style & CLBS_HASSLIDER) != 0) {
            SetSliderPosFromPoint(point.x, point.y, true);
        }

        CWnd::OnMouseMove(nFlags, point);
        return;
    }

    float min = 0.0f;
    float max = 1.0f;

    if (m_currentKey > 0) {
        min = m_keys[m_currentKey - 1].m_keyTime + 0.01f;
    }

    if (m_currentKey < m_keyCount - 1) {
        max = m_keys[m_currentKey + 1].m_keyTime - 0.01f;
    }

    float f3;
    float f4;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        f3 = (float)(point.x - m_size.left);
        f4 = (float)(m_size.right - m_size.left);
    } else {
        f3 = (float)(point.y - m_size.top);
        f4 = (float)(m_size.bottom - m_size.top);
    }

    float keytime = f3 / f4;

    if (min > keytime) {
        keytime = min;
    }

    if (max < keytime) {
        keytime = max;
    }

    m_keys[m_currentKey].m_keyTime = keytime;
    UpdateDeltas();
    HDC dc = ::GetDC(m_hWnd);
    Paint(dc);
    ::ReleaseDC(m_hWnd, dc);
    SendNotification(CLBN_KEYCHANGING, m_currentKey);
    m_mouseMoved = true;
    CWnd::OnMouseMove(nFlags, point);
}

void ColorBarClass::OnKillFocus(CWnd *pNewWnd)
{
    Refresh();
    CWnd::OnKillFocus(pNewWnd);
}

void ColorBarClass::OnSetFocus(CWnd *pOldWnd)
{
    Refresh();
    CWnd::OnSetFocus(pOldWnd);
}

void ColorBarClass::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_DELETE && ::GetFocus() == m_hWnd && SendNotification(CLBN_CANDELETE, m_currentKey) != 119) {
        RemoveKey(m_currentKey);
        SendNotification(CLBN_DELETE, m_currentKey);
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void ColorBarClass::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    int key = FindKey(point.x, point.y);

    if (key != -1) {
        SendNotification(CLBN_DBLCLK, key);
    }

    CWnd::OnLButtonDblClk(nFlags, point);
}

void ColorBarClass::Resize()
{
    FreeDibSection();

    int i1 = 0;
    m_width = 0;
    m_height = 0;
    RECT r;
    GetClientRect(&r);
    int h = r.bottom - r.top;
    int w = r.right - r.left;
    m_width = w;
    m_height = h;
    m_size.left = 0;
    m_size.top = 0;
    m_size.right = w;
    m_size.bottom = h;

    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if ((style & CLBS_OUTLINEFRAME | (CLBS_RAISEDFRAME | CLBS_SUNKENFRAME)) != 0) {
        InflateRect(&m_size, -1, -1);
    }

    if ((style & CLBS_HASKEYS) != 0) {
        LoadKeyBitmap();

        if ((style & CLBS_HORIZONTAL) == 0) {
            m_size.bottom -= m_keyHeight >> 1;
            m_size.top += m_keyHeight >> 1;
            m_size.right -= m_keyWidth >> 1;
        } else {
            m_size.left += m_keyWidth >> 1;
            m_size.right -= m_keyWidth >> 1;
            m_size.bottom -= m_keyHeight >> 1;
        }
    } else {
        if ((style & CLBS_HASSLIDER) != 0) {
            if ((style & CLBS_HORIZONTAL) != 0) {
                m_size.right -= 2;
                m_size.left += 2;
            } else {
                m_size.bottom -= 2;
                m_size.top += 2;
            }
        }
    }

    BITMAPINFOHEADER bm;
    memset(&bm, 0, sizeof(bm));
    bm.biSize = sizeof(bm);
    bm.biWidth = m_width;
    bm.biHeight = -m_height;
    bm.biPlanes = 1;
    bm.biBitCount = 24;
    bm.biCompression = 0;
    bm.biSizeImage = 3 * m_width * m_height;

    HDC dc = ::GetDC(nullptr);
    m_DIBSection = CreateDIBSection(
        dc, reinterpret_cast<BITMAPINFO *>(&bm), 0, reinterpret_cast<void **>(&m_DIBSectionBits), nullptr, 0);
    ::ReleaseDC(nullptr, dc);

    int i2 = 3 * m_width % 4;
    if (i2) {
        i1 = 4 - i2;
    }

    m_stride = i1 + 3 * m_width;
    UpdateDeltas();
}

void ColorBarClass::DrawVertical(int left, int top, int w, int h, BYTE *bits)
{
    for (int i = 0; i < m_keyCount; i++) {
        float red = m_keys[i].m_red;
        float green = m_keys[i].m_green;
        float blue = m_keys[i].m_blue;
        int pos = 3 * left + top * m_stride;
        int l = left + w;

        for (int j = top; j < m_keys[i].m_keyLength; j++) {
            if (left < l) {
                BYTE *b = &bits[pos];

                for (int k = l - left; k != 0; k--) {
                    b[0] = (BYTE)blue;
                    b[1] = (BYTE)green;
                    b[2] = (BYTE)red;
                    b += 3;
                }
            }

            red += m_keys[i].m_redDelta;
            green += m_keys[i].m_greenDelta;
            blue += m_keys[i].m_blueDelta;
            pos += m_stride;
        }
    }
}

void ColorBarClass::DrawHorizontal(int left, int top, int w, int h, BYTE *bits)
{
    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_GRADIENT) == 0) {
        int j = left;

        for (int i = 0; i < m_keyCount; i++) {
            float red = m_keys[i].m_red;
            float green = m_keys[i].m_green;
            float blue = m_keys[i].m_blue;
            BYTE *b = &bits[3 * j + top * m_stride];

            for (; j < m_keys[i].m_keyLength; j++) {
                int b2 = 0;

                if (top < top + h) {

                    for (int k = h; k != 0; k--) {
                        b[b2 + 0] = (BYTE)blue;
                        b[b2 + 1] = (BYTE)green;
                        b[b2 + 2] = (BYTE)red;
                        b2 += m_stride;
                    }
                }

                b += 3;
                red += m_keys[i].m_redDelta;
                green += m_keys[i].m_greenDelta;
                blue += m_keys[i].m_blueDelta;
            }
        }
    } else {
        int j = left;

        for (int i = 0; i < m_keyCount; i++) {
            BYTE *b = &bits[3 * j + top * m_stride];
            float gradient = m_keys[i].m_gradientValue;

            for (; j < m_keys[i].m_keyLength; j++) {
                int b2 = 0;
                bool color = true;

                for (int k = top; k < top + h; k++) {
                    float val = (float)k / (float)h;

                    if (1.0f - gradient >= val) {
                        b[b2 + 0] = -128;
                        b[b2 + 1] = 128;
                        b[b2 + 2] = 128;
                        color = false;
                    } else {
                        if (color) {
                            float val2 = 1.0f - val;
                            b[b2 + 0] = 255;
                            b[b2 + 1] = 128 - (int)(val2 * -128.0f);
                            b[b2 + 2] = (int)(val2 * 255.0f);
                        } else {
                            b[b2 + 0] = 0;
                            b[b2 + 1] = 0;
                            b[b2 + 2] = 0;
                        }

                        color = true;
                    }

                    b2 += m_stride;
                }

                b += 3;
                gradient += m_keys[i].m_gradientValueDelta;
            }
        }
    }
}

void ColorBarClass::DrawBar()
{
    RECT r = m_size;
    InflateRect(&r, 1, 1);
    RECT r2;
    r2.left = 0;
    r2.top = 0;
    r2.right = m_width;
    r2.bottom = m_height;

    HGDIOBJ obj = SelectObject(m_deviceContext, m_DIBSection);
    FillRect(m_deviceContext, &r2, reinterpret_cast<HBRUSH>(COLOR_BTNSHADOW));
    SelectObject(m_deviceContext, obj);
    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if ((style & CLBS_SUNKENFRAME) != 0) {
        DrawSunkenRect(m_DIBSectionBits, &r, m_stride);
    } else if ((style & CLBS_RAISEDFRAME) != 0) {
        DrawRaisedRect(m_DIBSectionBits, &r, m_stride);
    } else if ((style & CLBS_OUTLINEFRAME) != 0) {
        DrawOutlinedRect(m_DIBSectionBits, &r, 0, m_stride);
    }

    if ((style & CLBS_HORIZONTAL) != 0) {
        DrawHorizontal(m_size.left, m_size.top, m_size.right - m_size.left, m_size.bottom - m_size.top, m_DIBSectionBits);
    } else {
        DrawVertical(m_size.left, m_size.top, m_size.right - m_size.left, m_size.bottom - m_size.top, m_DIBSectionBits);
    }

    if ((style & CLBS_HASKEYS) != 0) {
        int x;
        int y;
        int *ptr;
        int size;

        if ((style & CLBS_HORIZONTAL) != 0) {
            ptr = &x;
            size = m_keyWidth;
            x = m_size.left;
            y = m_size.bottom - (m_keyHeight >> 1) - m_size.top;
        } else {
            ptr = &y;
            size = m_keyHeight;
            x = m_size.right - (m_keyWidth >> 1) - m_size.left;
            y = m_size.top;
        }

        int size2 = -(size >> 1);

        for (int i = 0; i < m_keyCount; i++) {
            *ptr = size2 + m_keys[i].m_keyPosition;
            DrawKey(x, y);
        }
    } else if ((style & CLBS_HASSLIDER) != 0) {
        RECT r3;
        GetSliderRect(&r3);
        DrawOutlinedRect(m_DIBSectionBits, &r3, 0, m_stride);
        InflateRect(&r3, -1, -1);
        DrawOutlinedRect(m_DIBSectionBits, &r3, 0xFFFFFF, m_stride);
    }
}

void ColorBarClass::Paint(HDC dc)
{
    if (m_deviceContext != nullptr) {
        HGDIOBJ obj = SelectObject(m_deviceContext, m_DIBSection);
        BitBlt(dc, 0, 0, m_width, m_height, m_deviceContext, 0, 0, SRCCOPY);
        SelectObject(m_deviceContext, obj);

        if (::GetFocus() == m_hWnd) {
            LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

            if ((style & CLBS_HASKEYS) != 0) {
                int r;
                int t;
                int b;

                if ((style & CLBS_HORIZONTAL) != 0) {
                    r = m_keys[m_currentKey].m_keyPosition - (m_keyWidth >> 1);
                    t = m_size.bottom - (m_keyHeight >> 1);
                    b = m_size.top;
                } else {
                    r = m_size.right - (m_keyWidth >> 1) - m_size.left;
                    b = m_keyHeight >> 1;
                    t = m_keys[m_currentKey].m_keyPosition;
                }

                RECT rc;
                rc.left = r;
                rc.right = r + m_keyWidth;
                rc.top = t - b;
                rc.bottom = t - b + m_keyHeight;
                DrawFocusRect(dc, &rc);
            } else if ((style & CLBS_HASSLIDER) != 0) {
                RECT rc;
                GetSliderRect(&rc);
                DrawFocusRect(dc, &rc);
            }
        }
    }
}

void ColorBarClass::AddKey(int x, int y, int flags)
{
    int key = 0;
    int i1;
    float f1;
    float red;
    float green;
    float blue;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        i1 = x - m_size.left;
        int pos = 3 * x + m_stride * (m_size.top + 1);
        blue = m_DIBSectionBits[pos];
        green = m_DIBSectionBits[pos + 1];
        red = m_DIBSectionBits[pos + 2];
        f1 = (float)i1 / (float)(m_size.right - m_size.left);
    } else {
        i1 = y - m_size.top;
        int pos = 3 * m_size.left + y * m_stride;
        blue = m_DIBSectionBits[pos];
        green = m_DIBSectionBits[pos + 1];
        red = m_DIBSectionBits[pos + 2];
        f1 = (float)i1 / (float)(m_size.bottom - m_size.top);
    }

    bool b1 = false;
    bool b2 = false;
    float value = 0.0f;

    for (int i = 0; i < m_keyCount; i++) {
        if (b1) {
            break;
        }

        if (i1 > m_keys[i].m_keyPosition) {
            if (i1 > m_keys[i].m_keyLength) {
                b1 = b2;
            } else {
                b1 = true;
                key = i + 1;
                b2 = true;
                value = ((float)i1 - (float)m_keys[i].m_keyPosition)
                        / (float)(m_keys[i].m_keyLength - m_keys[i].m_keyPosition)
                        * ((float)(m_keys[i].m_keyLength - m_keys[i].m_keyPosition) * m_keys[i].m_gradientValueDelta)
                    + m_keys[i].m_gradientValue;
            }
        }
    }

    if (b1) {
        EnableRedraw(false);
        float time = (m_sliderMax - m_sliderMin) * f1 + m_sliderMin;

        if (AddColor(key, time, red, green, blue, 3)) {
            SetGradientValue(key, value);
            m_currentKey = key;
        }
    }
}

void ColorBarClass::UpdateDeltas()
{
    int w = m_size.right - m_size.left;
    int h = m_size.bottom - m_size.top;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        for (int i = 0; i < m_keyCount; i++) {
            int pos = m_size.left + ((float)w * m_keys[i].m_keyTime);
            m_keys[i].m_keyPosition = pos;

            if (i > 0) {
                m_keys[i - 1].m_keyLength = pos;
                float len = pos - m_keys[i - 1].m_keyPosition;
                m_keys[i - 1].m_redDelta = (m_keys[i].m_red - m_keys[i - 1].m_red) / len;
                m_keys[i - 1].m_greenDelta = (m_keys[i].m_green - m_keys[i - 1].m_green) / len;
                m_keys[i - 1].m_blueDelta = (m_keys[i].m_blue - m_keys[i - 1].m_blue) / len;
                m_keys[i - 1].m_gradientValueDelta = (m_keys[i].m_gradientValue - m_keys[i - 1].m_gradientValue) / len;
            }
        }

        m_keys[m_keyCount - 1].m_redDelta = 0.0f;
        m_keys[m_keyCount - 1].m_greenDelta = 0.0f;
        m_keys[m_keyCount - 1].m_blueDelta = 0.0f;
        m_keys[m_keyCount - 1].m_gradientValueDelta = 0.0f;
        m_keys[m_keyCount - 1].m_keyLength = m_size.right;
        DrawBar();
    } else {
        for (int i = 0; i < m_keyCount; i++) {
            int pos = m_size.top + ((float)h * m_keys[i].m_keyTime);
            m_keys[i].m_keyPosition = pos;

            if (i > 0) {
                m_keys[i - 1].m_keyLength = pos;
                float len = pos - m_keys[i - 1].m_keyPosition;
                m_keys[i - 1].m_redDelta = (m_keys[i].m_red - m_keys[i - 1].m_red) / len;
                m_keys[i - 1].m_greenDelta = (m_keys[i].m_green - m_keys[i - 1].m_green) / len;
                m_keys[i - 1].m_blueDelta = (m_keys[i].m_blue - m_keys[i - 1].m_blue) / len;
            }
        }

        m_keys[m_keyCount - 1].m_redDelta = 0.0f;
        m_keys[m_keyCount - 1].m_greenDelta = 0.0f;
        m_keys[m_keyCount - 1].m_blueDelta = 0.0f;
        m_keys[m_keyCount - 1].m_keyLength = m_size.top;
        DrawBar();
    }
}

void ColorBarClass::LoadKeyBitmap()
{
    FreeKeyBitmap();
    HBITMAP hbm;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        hbm = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_KEYFRAME));
    } else {
        hbm = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_KEYFRAMEVERT));
    }

    BITMAP bm;
    GetObject(hbm, sizeof(BITMAP), &bm);
    int h = bm.bmHeight;
    int w = bm.bmWidth;
    m_keyHeight = h;
    m_keyWidth = w;

    BITMAPINFOHEADER bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.biSize = sizeof(bmi);
    bmi.biWidth = w;
    bmi.biHeight = -h;
    bmi.biPlanes = 1;
    bmi.biBitCount = 24;
    bmi.biCompression = 0;
    bmi.biSizeImage = 3 * w * h;

    HDC dc = ::GetDC(nullptr);
    m_keyDIBSection = CreateDIBSection(
        dc, reinterpret_cast<BITMAPINFO *>(&bmi), 0, reinterpret_cast<void **>(&m_keyDIBSectionBits), nullptr, 0);
    ::ReleaseDC(nullptr, dc);

    dc = CreateCompatibleDC(nullptr);
    HGDIOBJ obj = SelectObject(m_deviceContext, m_keyDIBSection);
    HGDIOBJ obj2 = SelectObject(dc, hbm);
    BitBlt(m_deviceContext, 0, 0, m_keyWidth, m_keyHeight, dc, 0, 0, SRCCOPY);
    SelectObject(m_deviceContext, obj);
    SelectObject(dc, obj2);
    DeleteDC(dc);
    DeleteObject(hbm);
}

void ColorBarClass::DrawKey(int x, int y)
{
    int i1 = 3 * m_keyWidth;
    int i2 = 3 * m_keyWidth % 4;
    int i3;

    if (i2 != 0) {
        i3 = 4 - i2;
    } else {
        i3 = 0;
    }

    if (m_DIBSectionBits != nullptr) {
        if (m_keyDIBSectionBits != nullptr) {
            int dest = 3 * x + y * m_stride;
            int src = 0;

            for (int i = 0; i < m_keyHeight; i++) {
                for (int j = 0; j < m_keyWidth; j++) {
                    BYTE red = m_keyDIBSectionBits[src];
                    BYTE green = m_keyDIBSectionBits[src + 1];
                    BYTE blue = m_keyDIBSectionBits[src + 2];
                    src += 3;

                    if (red == BYTE_MAX && green == 0 && blue == BYTE_MAX) {
                        dest += 3;
                    } else {
                        m_DIBSectionBits[dest] = red;
                        m_DIBSectionBits[dest + 1] = green;
                        m_DIBSectionBits[dest + 2] = blue;
                        dest += 3;
                    }
                }

                src += i3;
                dest += m_stride - i1;
            }
        }
    }
}

int ColorBarClass::FindKey(int x, int y)
{
    int len;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        len = m_keyWidth;
    } else {
        len = m_keyHeight;
        x = y;
    }

    int i1 = (len + 1) >> 1;
    int key = -1;
    int pos = 1024;

    for (int i = 0; i < m_keyCount; i++) {
        int i2 = abs(m_keys[i].m_keyPosition - x);

        if (i2 < pos) {
            pos = i2;
            key = i;
        }
    }

    if (pos >= i1) {
        return -1;
    } else {
        return key;
    }
}

LRESULT ColorBarClass::SendNotification(int code, int key)
{
    int id = GetWindowLong(m_hWnd, GWL_ID);
    ColorBarNotify cbn;
    memset(&cbn, 0, sizeof(cbn));
    cbn.base.code = code;
    cbn.keyposition = key;
    cbn.base.hwndFrom = m_hWnd;
    cbn.base.idFrom = id;
    cbn.red = m_keys[key].m_red;
    cbn.green = m_keys[key].m_green;
    cbn.blue = m_keys[key].m_blue;
    cbn.keytime = ((m_sliderMax - m_sliderMin) * m_keys[key].m_keyTime) + m_sliderMin;
    return GetParent()->SendMessage(WM_NOTIFY, id, reinterpret_cast<LPARAM>(&cbn));
}

void ColorBarClass::GetSliderRect(RECT *r)
{
    *r = m_size;
    float f1 = (m_sliderPos - m_sliderMin) / (m_sliderMax - m_sliderMin);
    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if ((style & CLBS_HORIZONTAL) != 0) {
        int i1 = ((float)(m_size.right - m_size.left) * f1) + m_size.left - 3;
        r->left = i1;
        r->right = i1 + 6;

        if ((style & CLBS_OUTLINEFRAME | (CLBS_RAISEDFRAME | CLBS_SUNKENFRAME)) != 0) {
            r->top--;
            r->bottom++;
        }
    } else {
        int i1 = ((float)(m_size.bottom - m_size.top) * f1) + m_size.top - 3;
        r->top = i1;
        r->bottom = i1 + 6;

        if ((style & CLBS_OUTLINEFRAME | (CLBS_RAISEDFRAME | CLBS_SUNKENFRAME)) != 0) {
            r->left--;
            r->right++;
        }
    }
}

void ColorBarClass::SetSliderPosFromPoint(int x, int y, bool notify)
{
    float f1;
    float f2;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        f1 = x - m_size.left;
        f2 = m_size.right - m_size.left;
    } else {
        f1 = y - m_size.top;
        f2 = m_size.bottom - m_size.top;
    }

    float newpos = (m_sliderMax - m_sliderMin) * (f1 / f2) + m_sliderMin;

    if (newpos <= m_sliderMin) {
        newpos = m_sliderMin;
    }

    if (newpos >= m_sliderMax) {
        newpos = m_sliderMax;
    }

    UpdateSliderPos(newpos, notify);
}

void ColorBarClass::UpdateSliderPos(float pos, bool notify)
{
    if (pos != m_sliderPos) {
        m_sliderPos = pos;

        if (notify) {
            int id = GetWindowLong(m_hWnd, GWL_ID);
            ColorBarNotify cbn;
            memset(&cbn, 0, sizeof(cbn));
            cbn.keytime = m_sliderPos;
            cbn.base.hwndFrom = m_hWnd;
            cbn.base.idFrom = id;
            cbn.base.code = CLBN_SLIDERCHANGE;
            GetColorByPos(m_sliderPos, &cbn.red, &cbn.green, &cbn.blue);
            GetParent()->SendMessage(WM_NOTIFY, id, reinterpret_cast<LPARAM>(&cbn));
        }

        DrawBar();
        HDC dc = ::GetDC(m_hWnd);
        Paint(dc);
        ::ReleaseDC(m_hWnd, dc);
    }
}

void ColorBarClass::GetColorByPos(float pos, float *red, float *green, float *blue)
{
    float f1 = (pos - m_sliderMin) / (m_sliderMax - m_sliderMin) + m_sliderMin;
    float f2;

    if ((GetWindowLong(m_hWnd, GWL_STYLE) & CLBS_HORIZONTAL) != 0) {
        f2 = f1 * (float)(m_size.right - m_size.left) + (float)m_size.left;
    } else {
        f2 = f1 * (float)(m_size.bottom - m_size.top) + (float)m_size.top;
    }

    int i1 = f2;
    bool b = false;
    int i2 = 0;
    int i = 0;

    if (m_keyCount > 0) {
        while (!b) {
            if (i1 >= m_keys[i].m_keyPosition && i1 <= m_keys[i].m_keyLength) {
                i2 = i;
                b = true;
            }

            i++;

            if (i >= m_keyCount) {
                if (!b) {
                    return;
                }

                break;
            }
        }

        float f3 = (float)(int)f2 - (float)m_keys[i2].m_keyPosition;
        *red = m_keys[i2].m_red;
        *blue = m_keys[i2].m_blue;
        *green = m_keys[i2].m_green;
        *red += f3 * m_keys[i2].m_redDelta;
        *green += f3 * m_keys[i2].m_greenDelta;
        *blue += f3 * m_keys[i2].m_blueDelta;
    }
}

void ColorBarClass::FreeKeyBitmap()
{
    if (m_keyDIBSection != nullptr) {
        DeleteObject(m_keyDIBSection);
        m_keyDIBSection = nullptr;
        m_keyDIBSectionBits = nullptr;
    }
}

void ColorBarClass::FreeDibSection()
{
    if (m_DIBSection != nullptr) {
        DeleteObject(m_DIBSection);
        m_DIBSection = nullptr;
        m_DIBSectionBits = nullptr;
    }
}

bool ColorBarClass::RemoveKey(int key)
{
    if (key > 0 && key < m_keyCount) {
        memcpy(&m_keys[key], &m_keys[key + 1], sizeof(ColorBarKey) * m_keyCount - 4 * (key + 4 * (3 * key + 3) + 1));
        m_keyCount--;

        if (m_currentKey > 0) {
            m_currentKey -= 1;
        }

        UpdateDeltas();
        Refresh();
        return true;
    }

    return false;
}

void ColorBarClass::Refresh()
{
    InvalidateRect(nullptr, FALSE);

    if (m_redrawEnabled) {
        UpdateWindow();
    }
}
