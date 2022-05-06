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
#pragma once
#include "w3dafx.h"

#define CLPN_COLORCHANGED 0x1

#define CLPS_SUNKENFRAME 0x1
#define CLPS_RAISEDFRAME 0x2

struct ColorPickerNotify
{
    NMHDR base;
    float red;
    float green;
    float blue;
    float xpos;
};

class ColorPickerClass : public CWnd
{
public:
    ColorPickerClass();
    virtual ~ColorPickerClass() override;
    virtual BOOL Create(LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle,
        const RECT &rect,
        CWnd *pParentWnd,
        UINT nID,
        CCreateContext *pContext = NULL) override;

    static void RegisterWndClass(HINSTANCE instance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    void SetColor(BYTE red, BYTE green, BYTE blue);
    void GetColor(int *red, int *green, int *blue);

private:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC *pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    void Resize();
    void Free();
    void DrawHSV(int width, int height, BYTE *bits);
    void DrawCrosshair();
    int ColorFromPoint(int x, int y);
    void Redraw();
    void GetRect(RECT *rect);
    CPoint PointFromColor(int color);
    void HueValueFromColor(int red, int green, int blue, float *hue, float *value);
    void GetSinCos(float color, float angle, float *c, float *s);
    void GetDistanceAngle(float c, float s, float *distance, float *angle);

    HBITMAP m_DIBSection;
    HDC m_deviceContext;
    BYTE *m_DIBSectionBits;
    int m_width;
    int m_height;
    CPoint m_mousePos;
    int m_currentColor;
    bool m_mouseDown;
    float m_xPos;
};
