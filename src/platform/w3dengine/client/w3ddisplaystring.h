/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Display String
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "displaystring.h"
#include "render2dsentence.h"

class W3DDisplayString : public DisplayString
{
    IMPLEMENT_POOL(W3DDisplayString);
    friend class W3DDisplayStringManager;

protected:
    virtual ~W3DDisplayString() override {}

public:
    W3DDisplayString();
    virtual void Notify_Text_Changed() override;
    virtual void Set_Font(GameFont *font) override;
    virtual void Set_Word_Wrap(int wrap_width) override;
    virtual void Set_Word_Wrap_Centered(bool on) override;
    virtual void Draw(int x, int y, int color, int border_color) override;
    virtual void Draw(int x, int y, int color, int border_color, int border_x_offset, int border_y_offset) override;
    virtual void Get_Size(int *x, int *y) override;
    virtual int Get_Width(int char_count) override;
    virtual void Set_Use_Hotkey(bool state, int color) override;
    virtual void Set_Clip_Region(IRegion2D *region) override;

    void Compute_Extents();

private:
    Render2DSentenceClass m_sentence;
    Render2DSentenceClass m_hotKeySentence;
    bool m_textChanged;
    bool m_fontChanged;
    Utf16String m_hotKeyString;
    bool m_hotKeyState;
    int m_xPos;
    int m_yPos;
    int m_hotKeyColor;
    int m_x;
    int m_y;
    int m_textColor;
    int m_borderColor;
    int m_xExtent;
    int m_yExtent;
    IRegion2D m_clipRegion;
    unsigned int m_frame;
};
