/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View color picker
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorpicker.h"
#include "colorutils.h"
#include "gamemath.h"
#include "resource.h"

// clang-format off
BEGIN_MESSAGE_MAP(ColorPickerClass, CWnd)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()
// clang-format on

ColorPickerClass::ColorPickerClass() :
    m_DIBSection(nullptr),
    m_deviceContext(nullptr),
    m_DIBSectionBits(nullptr),
    m_width(0),
    m_height(0),
    m_mousePos(0, 0),
    m_currentColor(0),
    m_mouseDown(false),
    m_xPos(0.0f)
{
}

ColorPickerClass::~ColorPickerClass()
{
    if (m_deviceContext != nullptr) {
        DeleteObject(m_deviceContext);
        m_deviceContext = nullptr;
    }

    Free();
}

BOOL ColorPickerClass::Create(LPCTSTR lpszClassName,
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
               "WWCOLORPICKER",
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

void ColorPickerClass::RegisterWndClass(HINSTANCE instance)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));

    if (!GetClassInfo(instance, "WWCOLORPICKER", &wc)) {
        wc.style = CS_GLOBALCLASS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = instance;
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_INACTIVECAPTIONTEXT);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "WWCOLORPICKER";
        RegisterClass(&wc);
    }
}

LRESULT CALLBACK ColorPickerClass::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg) {
        case WM_CREATE:
            if (lParam) {
                ColorPickerClass *picker = *reinterpret_cast<ColorPickerClass **>(lParam);
                bool created = false;

                if (picker == nullptr) {
                    picker = new ColorPickerClass();
                    created = true;
                }

                picker->Attach(hWnd);
                picker->OnCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));

                WNDPROC *wp = picker->GetSuperWndProcAddr();

                if (wp != nullptr) {
                    *wp = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)AfxGetAfxWndProc()));
                }

                SetProp(hWnd, "CLASSPOINTER", picker);
                SetProp(hWnd, "CREATED", reinterpret_cast<HANDLE>(created));
            }

            break;
        case WM_DESTROY: {
            ColorPickerClass *picker = static_cast<ColorPickerClass *>(GetProp(hWnd, "CLASSPOINTER"));
            HANDLE created = GetProp(hWnd, "CREATED");

            if (picker != nullptr) {
                picker->Detach();
                WNDPROC *wp = picker->GetSuperWndProcAddr();

                if (wp != nullptr) {
                    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)*wp);
                    *wp = nullptr;
                }

                if (created != nullptr) {
                    delete picker;
                }
            }

            break;
        }
    }

    return ::DefWindowProc(hWnd, Msg, wParam, lParam);
}

void ColorPickerClass::SetColor(BYTE red, BYTE green, BYTE blue)
{
    m_currentColor = red | (green << 8) | (blue << 16);
    m_mousePos = PointFromColor(m_currentColor);
    m_currentColor = ColorFromPoint(m_mousePos.x, m_mousePos.y);
    InvalidateRect(nullptr, FALSE);
    UpdateWindow();
}

void ColorPickerClass::GetColor(int *red, int *green, int *blue)
{
    *red = m_currentColor & 0xFF;
    *green = (m_currentColor >> 8) & 0xFF;
    *blue = (m_currentColor >> 16) & 0xFF;
}

void ColorPickerClass::OnPaint()
{
    CPaintDC dc(this);

    if (m_deviceContext != nullptr) {
        HGDIOBJ obj = SelectObject(m_deviceContext, m_DIBSection);
        RECT r;
        GetClientRect(&r);
        BitBlt(dc.GetSafeHdc(), 0, 0, r.right - r.left, r.bottom - r.top, m_deviceContext, 0, 0, SRCCOPY);
        SelectObject(m_deviceContext, obj);
        DrawCrosshair();
    }
}

int ColorPickerClass::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int result = CWnd::OnCreate(lpCreateStruct);

    if (result != -1) {
        m_deviceContext = CreateCompatibleDC(nullptr);
        Resize();
        return 0;
    }

    return result;
}

void ColorPickerClass::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    Resize();
}

BOOL ColorPickerClass::OnEraseBkgnd(CDC *pDC)
{
    return TRUE;
}

void ColorPickerClass::OnLButtonDown(UINT nFlags, CPoint pt)
{
    SetCapture();
    m_mouseDown = true;
    RECT r;
    GetRect(&r);
    ClientToScreen(&r);
    ClipCursor(&r);
    Redraw();
    m_mousePos = pt;
    m_currentColor = ColorFromPoint(pt.x, pt.y);

    ColorPickerNotify cpn;
    memset(&cpn, 0, sizeof(cpn));
    cpn.base.hwndFrom = m_hWnd;
    int id = GetWindowLong(m_hWnd, GWL_ID);
    cpn.base.idFrom = id;
    cpn.base.code = CLPN_COLORCHANGED;
    cpn.red = m_currentColor & 0xFF;
    cpn.green = (m_currentColor >> 8) & 0xFF;
    cpn.blue = (m_currentColor >> 16) & 0xFF;
    cpn.xpos = m_xPos;
    GetParent()->SendMessage(WM_NOTIFY, id, (LPARAM)&cpn);
    DrawCrosshair();
    CWnd::OnLButtonDown(nFlags, pt);
}

void ColorPickerClass::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_mouseDown) {
        ClipCursor(nullptr);
        ReleaseCapture();
        m_mouseDown = false;
    }

    CWnd::OnLButtonUp(nFlags, pt);
}

void ColorPickerClass::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_mouseDown) {
        Redraw();
        m_mousePos = point;
        m_currentColor = ColorFromPoint(point.x, point.y);

        ColorPickerNotify cpn;
        memset(&cpn, 0, sizeof(cpn));
        cpn.base.hwndFrom = m_hWnd;
        int id = GetWindowLong(m_hWnd, GWL_ID);
        cpn.base.idFrom = id;
        cpn.base.code = CLPN_COLORCHANGED;
        cpn.red = m_currentColor & 0xFF;
        cpn.green = (m_currentColor >> 8) & 0xFF;
        cpn.blue = (m_currentColor >> 16) & 0xFF;
        cpn.xpos = m_xPos;
        GetParent()->SendMessage(WM_NOTIFY, id, (LPARAM)&cpn);
        DrawCrosshair();
    }

    CWnd::OnMouseMove(nFlags, point);
}

void ColorPickerClass::Resize()
{
    Free();
    RECT r;
    GetClientRect(&r);
    int h = r.bottom - r.top;
    int w = r.right - r.left;
    m_width = w;
    m_height = h;

    BITMAPINFOHEADER bm;
    memset(&bm, 0, sizeof(bm));
    bm.biSize = sizeof(bm);
    bm.biWidth = w;
    bm.biHeight = -h;
    bm.biPlanes = 1;
    bm.biBitCount = 24;
    bm.biCompression = 0;
    bm.biSizeImage = 3 * w * h;

    HDC dc = ::GetDC(nullptr);
    m_DIBSection = CreateDIBSection(
        dc, reinterpret_cast<BITMAPINFO *>(&bm), 0, reinterpret_cast<void **>(&m_DIBSectionBits), nullptr, 0);
    ::ReleaseDC(nullptr, dc);

    DrawHSV(m_width, m_height, m_DIBSectionBits);
}

void ColorPickerClass::Free()
{
    if (m_DIBSection != nullptr) {
        DeleteObject(m_DIBSection);
        m_DIBSection = nullptr;
        m_DIBSectionBits = nullptr;
    }

    m_width = 0;
    m_height = 0;
}

void ColorPickerClass::DrawHSV(int width, int height, BYTE *bits)
{
    RECT bounds;
    bounds.top = 0;
    bounds.left = 0;
    bounds.right = width;
    bounds.bottom = height;

    int stride;

    if (3 * width % 4) {
        stride = 4 - (3 * width) % 4;
    } else {
        stride = 0;
    }

    stride += 3 * width;

    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if (style & CLPS_SUNKENFRAME) {
        DrawSunkenRect(bits, &bounds, stride);
        InflateRect(&bounds, -1, -1);
    }

    if (style & CLPS_RAISEDFRAME) {
        DrawRaisedRect(bits, &bounds, stride);
        InflateRect(&bounds, -1, -1);
    }

    if (style & WS_BORDER) {
        DrawOutlinedRect(bits, &bounds, 0xFF, stride);
        InflateRect(&bounds, -1, -1);
    }

    int w = bounds.right - bounds.left;
    int h = bounds.bottom - bounds.top;

    int offset[6];
    int remain = w % 6;
    int hstep = w / 6;

    for (int i = 0; i < 6; i++) {
        offset[i] = hstep;

        if (remain > 0) {
            offset[i] = hstep + 1;
            --remain;
        }

        if (i > 0) {
            offset[i] += offset[i - 1];
        }
    }

    int bufoffset = bounds.left * 3;
    float red = 255.0f;
    float green = 0.0f;
    float blue = 0.0f;
    int hueoffset = 0;
    float *ptr = &green;
    float step = 255.0f / (float)offset[0];

    for (int x = bounds.left; x < bounds.right; x++) {
        BYTE *b = &bits[bufoffset + stride * bounds.left];
        float rstep = -(red / (h - 1));
        float gstep = -(green / (h - 1));
        float bstep = -(blue / (h - 1));
        float red2 = red;
        float green2 = green;
        float blue2 = blue;

        for (int y = bounds.top; y < bounds.bottom; y++) {
            b[0] = blue2;
            b[1] = green2;
            b[2] = red2;
            red2 += rstep;
            green2 += gstep;
            blue2 += bstep;
            b += stride;
        }

        if (x - bounds.left == offset[hueoffset]) {
            step = 255.0f / (offset[hueoffset + 1] - offset[hueoffset]);
            hueoffset++;

            switch (hueoffset) {
                case 1: {
                    ptr = &red;
                    step = -step;
                    break;
                }
                case 2: {
                    ptr = &blue;
                    break;
                }
                case 3: {
                    ptr = &green;
                    step = -step;
                    break;
                }
                case 4: {
                    ptr = &red;
                    break;
                }
                case 5: {
                    ptr = &blue;
                    step = -step;
                    break;
                }
            }
        }

        *ptr += step;
        bufoffset += 3;
    }
}

void ColorPickerClass::DrawCrosshair()
{
    HDC dc = ::GetDC(m_hWnd);

    if (m_deviceContext != nullptr) {
        HBITMAP hbm = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_MARKER));
        HGDIOBJ obj = SelectObject(m_deviceContext, hbm);
        RECT r;
        GetClientRect(&r);
        BitBlt(dc, m_mousePos.x - 5, m_mousePos.y - 5, 11, 11, m_deviceContext, 0, 0, SRCINVERT);
        SelectObject(m_deviceContext, obj);
        DeleteObject(hbm);
    }

    ::ReleaseDC(m_hWnd, dc);
}

int ColorPickerClass::ColorFromPoint(int x, int y)
{
    int color = 0;

    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        int i1 = 3 * m_width;
        int i2;

        if (i1 % 4) {
            i2 = 4 - i1 % 4;
        } else {
            i2 = 0;
        }

        int i3 = i1 + i2;
        int i4 = 3 * x + y * i3;
        color = m_DIBSectionBits[i4 + 2] | (m_DIBSectionBits[i4 + 1] << 8) | (m_DIBSectionBits[i4 + 0] << 16);
        RECT r;
        GetRect(&r);
        m_xPos = (float)x / (float)(r.right - r.left);
    }

    return color;
}

void ColorPickerClass::Redraw()
{
    HDC dc = ::GetDC(m_hWnd);

    if (m_deviceContext != nullptr) {
        HGDIOBJ obj = SelectObject(m_deviceContext, m_DIBSection);
        RECT r;
        GetClientRect(&r);
        BitBlt(dc, m_mousePos.x - 5, m_mousePos.y - 5, 11, 11, m_deviceContext, m_mousePos.x - 5, m_mousePos.y - 5, SRCCOPY);
        SelectObject(m_deviceContext, obj);
    }

    ::ReleaseDC(m_hWnd, dc);
}

void ColorPickerClass::GetRect(RECT *rect)
{
    GetClientRect(rect);
    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

    if ((style & SS_ICON) != 0 || (style & WS_BORDER) != 0) {
        rect->left += 1;
        rect->right -= 1;
        rect->top += 1;
        rect->bottom -= 1;
    }
}

CPoint ColorPickerClass::PointFromColor(int color)
{
    int red = color & 0xFF;
    int green = (color >> 8) & 0xFF;
    int blue = (color >> 16) & 0xFF;
    float hue = 0.0f;
    float value = 0.0f;
    HueValueFromColor(red, green, blue, &hue, &value);

    RECT r;
    GetRect(&r);
    int h = r.bottom - r.top;
    int w = r.right - r.left;

    int c = red;

    if (red >= green) {
        c = green;
    }

    if (red >= blue) {
        c = blue;
    }

    float f1 = c;
    float f2 = f1 / 255.0f;

    float f3 = 0.0f;

    if (f2 != 1.0f) {
        float f4 = 1.0f - f2;
        float red2 = (red - f1) / f4;
        float green2 = (green - f1) / f4;
        float blue2 = (blue - f1) / f4;

        if (red2 <= green2) {
            f3 = green2;
        } else {
            f3 = red2;
        }

        if (f3 <= blue2) {
            f3 = blue2;
        }
    }

    return CPoint((float)w * hue, h * (255 - (int)f3) / 255);
}

void ColorPickerClass::HueValueFromColor(int red, int green, int blue, float *hue, float *value)
{
    float cos1 = 0.0f;
    float sin1 = 0.0f;
    float cos2 = 0.0f;
    float sin2 = 0.0f;
    float cos3 = 0.0f;
    float sin3 = 0.0f;

    GetSinCos(red, DEG_TO_RADF(0.0f), &cos1, &sin1);
    GetSinCos(green, DEG_TO_RADF(120.0f), &cos2, &sin2);
    GetSinCos(blue, DEG_TO_RADF(240.0f), &cos3, &sin3);

    float angle = 0.0f;
    float distance = 0.0f;
    float cos = cos3 + cos2 + cos1;
    float sin = sin3 + sin2 + sin1;
    GetDistanceAngle(cos, sin, &distance, &angle);

    *hue = GameMath::Wrap(angle * DEG_TO_RADF(360.0f), 0.0f, 1.0f);
    *value = fabs(distance / 255.0f);
}

void ColorPickerClass::GetSinCos(float color, float angle, float *c, float *s)
{
    *c = cos(angle) * color;
    *s = sin(angle) * color;
}

void ColorPickerClass::GetDistanceAngle(float c, float s, float *distance, float *angle)
{
    if (c == 0.0f && s == 0.0f) {
        *distance = 0.0f;
        *angle = 0.0f;
    } else {
        *distance = sqrt(c * c + s * s);
        *angle = atan2(s, c);
    }
}
