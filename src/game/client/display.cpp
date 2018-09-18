/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Base class for the display handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "display.h"
#include "gamefont.h"
#include "mouse.h"

Display::Display() :
    m_width(0),
    m_height(0),
    m_bitDepth(0),
    m_windowed(false),
    m_viewList(nullptr),
    m_cinematicFont(nullptr),
    m_cinematicTextFrames(0),
    m_videoBuffer(nullptr),
    m_videoStream(nullptr),
    m_debugDisplay(nullptr),
    m_debugDisplayCallback(nullptr),
    m_debugDisplayUserData(nullptr),
    m_letterBoxFadeLevel(0.0),
    m_letterBoxEnabled(false),
    m_letterBoxFadeStart(0),
    m_someLogoMovieInt1(-1),
    m_someLogoMovieInt2(-1),
    m_someLogoMovieTime(0),
    m_unkInt(0),
    m_unkDisplayString(nullptr)
{
}

Display::~Display()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x004211A0, this);
#endif
}

void Display::Reset()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x00421A00, this);
#else
    /*m_letterBoxFadeLevel = 0;
    m_letterBoxEnabled = 0;
    Stop_Movie();
    for (View *i = m_viewList; i; i = i->getNextView()) {
        i->Reset();
    }*/
#endif
}

void Display::Update()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x00421870, this);
#endif
}

/**
 * 0x00421380
 */
void Display::Draw()
{
    Draw_Views();
}

void Display::Set_Width(unsigned width)
{
    m_width = width;
    if (g_theMouse) {
        g_theMouse->Set_Mouse_Limits();
    }
}

void Display::Set_Height(unsigned height)
{
    m_height = height;
    if (g_theMouse) {
        g_theMouse->Set_Mouse_Limits();
    }
}

bool Display::Set_Display_Mode(unsigned a2, unsigned a3, unsigned a4, bool a5)
{
// TODO Requires TacticalView
#ifndef THYME_STANDALONE
    return Call_Method<bool, Display, unsigned, unsigned, unsigned, BOOL>(0x00421390, this, a2, a3, a4, a5);
#else
    return 0;
#endif
}

int Display::Get_Display_Mode_Count()
{
// TODO Requires WW3D
#ifndef THYME_STANDALONE
    return Call_Method<int, Display>(0x0073C5D0, this);
#else
    return 0;
#endif
}

void Display::Get_Display_Mode_Description(int a1, int * a2, int * a3, int * a4) 
{
// TODO Requires WW3D
#ifndef THYME_STANDALONE
    Call_Method<void, Display, int, int *, int *, int *>(0x0073C650, this, a1, a2, a3, a4);
#endif
}

void Display::Set_Gamma(float a1, float a2, float a3, BOOL a4)
{
// TODO Requires DX8Wrapper
#ifndef THYME_STANDALONE
    Call_Method<void, Display, float, float, float, BOOL>(0x0073C650, this, a1, a2, a3, a4);
#endif
}

void Display::Attach_View(View *view)
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display, View *>(0x00421300, this, view);
#else
    //m_viewList = view->prependViewToList(m_viewList);
#endif
}

View *Display::Get_First_View()
{
    return m_viewList;
}

View *Display::Get_Next_View(View *view)
{
#ifndef THYME_STANDALONE
    return Call_Method<View *, Display, View *>(0x00421320, this, view);
#else
    /*if (view) {
        return view->getNextView();
    }*/
    return nullptr;
#endif
}

void Display::Draw_Views()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x00421320, this);
#else
    /*for (View *i = m_viewList; i; i = i->getNextView()) {
        i->drawView();
    }*/
#endif
}

void Display::Update_Views()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x00421350, this);
#else
    /*for (View *i = m_viewList; i; i = i->getNextView()) {
        i->updateView();
    }*/
#endif
}

void Display::Play_Logo_Movie(Utf8String name, int a3, int a4)
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display, Utf8String, int, int>(0x00421500, this, name, a3, a4);
#endif
}

void Display::Play_Movie(Utf8String name)
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display, Utf8String>(0x00421670, this, name);
#endif
}
void Display::Stop_Movie()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x004217D0, this);
#endif
}

void Display::Delete_Views()
{
#ifndef THYME_STANDALONE
    Call_Method<void, Display>(0x004212C0, this);
#endif
}
