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
#pragma once
#include "w3dafx.h"
#include "quat.h"

struct AlphaVectorStruct
{
    Quaternion Quat;
    float Magnitude;
    AlphaVectorStruct() : Quat(0.0f, 0.0f, 0.0f, 1.0f), Magnitude(1.0f) {}
    AlphaVectorStruct(Quaternion &quat, float magnitude) : Quat(quat), Magnitude(magnitude) {}
};

#define CLBN_KEYCHANGED 0x1
#define CLBN_KEYCHANGING 0x2
#define CLBN_DBLCLK 0x3
#define CLBN_SLIDERCHANGE 0x4
#define CLBN_CANDELETE 0x5
#define CLBN_DELETE 0x6
#define CLBN_ADDKEY 0x7

#define CLBS_SUNKENFRAME 0x1
#define CLBS_RAISEDFRAME 0x2
#define CLBS_OUTLINEFRAME 0x4
#define CLBS_HORIZONTAL 0x8
#define CLBS_UNUSED 0x10
#define CLBS_HASSLIDER 0x20
#define CLBS_HASKEYS 0x40
#define CLBS_GRADIENT 0x80

struct ColorBarNotify
{
    NMHDR base;
    int keyposition;
    float red;
    float green;
    float blue;
    float keytime;
};

class ColorBarClass : public CWnd
{
public:
    ColorBarClass();
    virtual ~ColorBarClass() override;
    virtual BOOL Create(LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle,
        const RECT &rect,
        CWnd *pParentWnd,
        UINT nID,
        CCreateContext *pContext = NULL) override;

    static void RegisterWndClass(HINSTANCE instance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    bool GetColor(int key, float *keytime, float *red, float *green, float *blue);
    bool AddColor(int key, float keytime, float red, float green, float blue, int flags);
    bool SetColor(int key, float keytime, float red, float green, float blue, int flags);
    void SetMinMax(float min, float max);
    void SetSliderPos(float pos);
    void Clear();
    void SetAlphaVector(int key, AlphaVectorStruct *vector);
    AlphaVectorStruct *GetAlphaVector(int key);
    bool SetGradientValue(int key, float value);
    float GetGradientValue(int key);
    void EnableRedraw(bool redraw);

    int GetSliderPos() const { return m_sliderPos; }
    int GetKeyCount() const { return m_keyCount; }

private:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnKillFocus(CWnd *pNewWnd);
    afx_msg void OnSetFocus(CWnd *pOldWnd);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

    void Resize();
    void DrawVertical(int left, int top, int w, int h, BYTE *bits);
    void DrawHorizontal(int left, int top, int w, int h, BYTE *bits);
    void DrawBar();
    void Paint(HDC dc);
    void AddKey(int x, int y, int flags);
    void UpdateDeltas();
    void LoadKeyBitmap();
    void DrawKey(int x, int y);
    int FindKey(int x, int y);
    LRESULT SendNotification(int code, int key);
    void GetSliderRect(RECT *r);
    void SetSliderPosFromPoint(int x, int y, bool notify);
    void UpdateSliderPos(float pos, bool notify);
    void GetColorByPos(float pos, float *red, float *green, float *blue);
    void FreeKeyBitmap();
    void FreeDibSection();
    bool RemoveKey(int key);
    void Refresh();

    struct ColorBarKey
    {
        float m_keyTime;
        int m_keyPosition;
        int m_keyLength;
        float m_gradientValue;
        float m_gradientValueDelta;
        float m_red;
        float m_green;
        float m_blue;
        float m_redDelta;
        float m_greenDelta;
        float m_blueDelta;
        AlphaVectorStruct *m_alphaVector;
        int m_keyFlags;
    };

    HGDIOBJ m_DIBSection;
    HGDIOBJ m_keyDIBSection;
    HDC m_deviceContext;
    BYTE *m_DIBSectionBits;
    BYTE *m_keyDIBSectionBits;
    int m_unk1;
    int m_unk2;
    int m_width;
    int m_height;
    int m_keyWidth;
    int m_keyHeight;
    int m_stride;
    int m_keyCount;
    float m_sliderMin;
    float m_sliderMax;
    ColorBarKey m_keys[15];
    RECT m_size;
    int m_currentKey;
    bool m_mouseDown;
    bool m_mouseMoved;
    bool m_redrawEnabled;
    float m_sliderPos;
};
