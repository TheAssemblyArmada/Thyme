/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Game Window
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
#include "gamewindow.h"
#include "render2dsentence.h"

class W3DGameWindow : public GameWindow
{
    IMPLEMENT_POOL(W3DGameWindow)

public:
    W3DGameWindow();
    virtual ~W3DGameWindow() override;
    virtual void Win_Draw_Border() override;
    virtual int Win_Set_Text(Utf16String new_text) override;
    virtual void Win_Set_Font(GameFont *font) override;

    void Blit_Border_Rect(int left, int top, int width, int height);

private:
    Render2DSentenceClass m_text;
    int m_xTextLoc;
    int m_yTextLoc;
    unsigned int m_color;
    bool m_textChanged;
    bool m_textLocChanged;
};

void W3D_Game_Win_Default_Draw(GameWindow *window, WinInstanceData *instance);
