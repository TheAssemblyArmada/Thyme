/**
 * @file
 *
 * @author tomsons26
 * @author OmniBlade
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
#include "view.h"

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
    m_letterBoxFadeLevel(0.0f),
    m_letterBoxEnabled(false),
    m_letterBoxFadeStart(0),
    m_someLogoMovieInt1(-1),
    m_someLogoMovieInt2(-1),
    m_someLogoMovieTime(0),
    m_unkInt(0),
    m_unkDisplayString(nullptr)
{
}

/**
 * 0x004211A0
 */
Display::~Display()
{
#ifdef GAME_DLL
    Call_Method<void, Display>(PICK_ADDRESS(0x004211A0, 0x007BD4D7), this);
#endif
}

/**
 * 0x00421A00
 */
void Display::Reset()
{
    m_letterBoxFadeLevel = 0;
    m_letterBoxEnabled = false;
    Stop_Movie();

    for (View *i = m_viewList; i; i = i->Get_Next_View()) {
        i->Reset();
    }
}

/**
 * 0x00421870
 */
void Display::Update()
{
#ifdef GAME_DLL
    Call_Method<void, Display>(PICK_ADDRESS(0x00421870, 0x007BDB67), this);
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

/**
 * 0x00421390
 */
bool Display::Set_Display_Mode(unsigned a2, unsigned a3, unsigned a4, bool a5)
{
// TODO Requires TacticalView
#ifdef GAME_DLL
    return Call_Method<bool, Display, unsigned, unsigned, unsigned, bool>(
        PICK_ADDRESS(0x00421390, 0x007BD676), this, a2, a3, a4, a5);
#else
    return 0;
#endif
}

/**
 * 0x0073C5D0
 */
int Display::Get_Display_Mode_Count()
{
// TODO Requires WW3D
#ifdef GAME_DLL
    return Call_Method<int, Display>(PICK_ADDRESS(0x0073C5D0, 0x007BDEA0), this);
#else
    return 0;
#endif
}

/**
 * 0x0073C650
 */
void Display::Get_Display_Mode_Description(int a1, int *a2, int *a3, int *a4)
{
// TODO Requires WW3D
#ifdef GAME_DLL
    Call_Method<void, Display, int, int *, int *, int *>(PICK_ADDRESS(0x0073C650, 0x007BDEB0), this, a1, a2, a3, a4);
#endif
}

/**
 * 0x0073C650
 */
void Display::Set_Gamma(float a1, float a2, float a3, bool a4)
{
// TODO Requires DX8Wrapper
#ifdef GAME_DLL
    Call_Method<void, Display, float, float, float, bool>(PICK_ADDRESS(0x0073C650, 0x007BDEC0), this, a1, a2, a3, a4);
#endif
}

/**
 * 0x00421300
 */
void Display::Attach_View(View *view)
{
    m_viewList = view->Prepend_View_To_List(m_viewList);
}

View *Display::Get_First_View()
{
    return m_viewList;
}

/**
 * 0x004210A0
 */
View *Display::Get_Next_View(View *view)
{
    if (view) {
        return view->Get_Next_View();
    }

    return nullptr;
}

/**
 * 0x00421320
 */
void Display::Draw_Views()
{
    for (View *i = m_viewList; i; i = i->Get_Next_View()) {
        i->Draw_View();
    }
}

/**
 * 0x00421350
 */
void Display::Update_Views()
{
    for (View *i = m_viewList; i; i = i->Get_Next_View()) {
        i->Update_View();
    }
}

/**
 * 0x00421500
 */
void Display::Play_Logo_Movie(Utf8String name, int a3, int a4)
{
#ifdef GAME_DLL
    Call_Method<void, Display, Utf8String, int, int>(PICK_ADDRESS(0x00421500, 0x007BD836), this, name, a3, a4);
#endif
}

/**
 * 0x00421670
 */
void Display::Play_Movie(Utf8String name)
{
#ifdef GAME_DLL
    Call_Method<void, Display, Utf8String>(PICK_ADDRESS(0x00421670, 0x007BD978), this, name);
#endif
}

/**
 * 0x004217D0
 */
void Display::Stop_Movie()
{
#ifdef GAME_DLL
    Call_Method<void, Display>(PICK_ADDRESS(0x004217D0, 0x007BDA9C), this);
#endif
}

/**
 * 0x004212C0
 */
void Display::Delete_Views()
{
#ifdef GAME_DLL
    Call_Method<void, Display>(PICK_ADDRESS(0x004212C0, 0x007BD54E), this);
#endif
}
