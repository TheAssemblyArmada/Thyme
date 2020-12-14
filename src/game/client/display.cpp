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
#include "displaystringmanager.h"
#include "gamefont.h"
#include "gametext.h"
#include "mouse.h"
#include "rtsutils.h"
#include "videobuffer.h"
#include "videoplayer.h"
#include "videostream.h"
#include "view.h"

#ifndef GAME_DLL
Display *g_theDisplay;
#endif

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
    m_letterBoxFadeStartTime(0),
    m_someLogoMovieInt1(-1),
    m_someLogoMovieInt2(-1),
    m_someLogoMovieTime(0),
    m_unkInt(0),
    m_unkDisplayString(nullptr)
{
    m_cinematicText = Utf8String::s_emptyString;
    m_currentlyPlayingMovie.Clear();
}

/**
 * 0x004211A0
 */
Display::~Display()
{
    Stop_Movie();
    Delete_Views();
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
    if (Is_Movie_Playing() && m_videoStream->Is_Frame_Ready()) {
        m_videoStream->Decompress_Frame();
        m_videoStream->Render_Frame(m_videoBuffer);

        if (m_videoStream->Frame_Index() != m_videoStream->Frame_Count() - 1) {
            m_videoStream->Next_Frame();
        } else if (m_someLogoMovieInt2 < 0 && m_someLogoMovieInt1 < 0) {
            Stop_Movie();
        } else {
            // some sort of text timing, we never see the text cause its color is black
            if (!m_unkInt) {
                if (m_unkDisplayString != nullptr) {
                    m_unkDisplayString->Delete_Instance();
                }

                m_unkDisplayString = g_theDisplayStringManager->New_Display_String();
                m_unkDisplayString->Set_Text(g_theGameText->Fetch("GUI:EACopyright"));

                Utf8String font_name;
                int point_size = 0;
                bool bold = false;

                if (g_theGlobalLanguage && g_theGlobalLanguage->Copyright_Font().Name().Is_Not_Empty()) {
                    FontDesc cpy_fnt = g_theGlobalLanguage->Copyright_Font();
                    font_name = cpy_fnt.Name();
                    point_size = cpy_fnt.Point_Size();
                    bold = cpy_fnt.Bold();
                } else {
                    font_name = "Courier";
                    point_size = 12;
                    bold = true;
                }

                int size = g_theGlobalLanguage->Adjust_Font_Size(point_size);
                GameFont *font = g_theFontLibrary->Get_Font(font_name, size, bold);
                m_unkDisplayString->Set_Font(font);

                m_unkInt = rts::Get_Time();
            }

            if (m_someLogoMovieTime + m_someLogoMovieInt1 < rts::Get_Time()) {
                if (m_unkInt + m_someLogoMovieInt2 < rts::Get_Time()) {
                    m_someLogoMovieInt1 = -1;
                    m_someLogoMovieTime = 0;
                    m_unkInt = 0;
                    m_someLogoMovieInt2 = -1;
                }
            }
        }
    }
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
bool Display::Set_Display_Mode(unsigned width, unsigned height, unsigned bits, bool windowed)
{
    int old_width = Get_Width();
    int old_height = Get_Height();

    int tactical_width = g_theTacticalView->Get_Width();
    int tactical_height = g_theTacticalView->Get_Height();

    int origin_x;
    int origin_y;
    g_theTacticalView->Get_Origin(&origin_x, &origin_y);

    Set_Width(width);
    Set_Height(height);

    g_theTacticalView->Set_Width(tactical_width / old_width * width);
    g_theTacticalView->Set_Height(tactical_height / old_height * height);

    g_theTacticalView->Set_Origin(origin_x / old_width * width, origin_y / old_height * height);

    return true;
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
    Stop_Movie();
    m_videoStream = g_theVideoPlayer->Open(name);
    if (m_videoStream != nullptr) {
        m_currentlyPlayingMovie = name;
        m_someLogoMovieInt1 = a3;
        m_someLogoMovieInt2 = a4;
        m_someLogoMovieTime = rts::Get_Time();

        m_videoBuffer = Create_VideoBuffer();

        if (m_videoBuffer == nullptr || !m_videoBuffer->Allocate(m_videoStream->Width(), m_videoStream->Height())) {
            Stop_Movie();
        }
    }
}

/**
 * 0x00421670
 */
void Display::Play_Movie(Utf8String name)
{
    Stop_Movie();
    m_videoStream = g_theVideoPlayer->Open(name);
    if (m_videoStream != nullptr) {
        m_currentlyPlayingMovie = name;
        m_videoBuffer = Create_VideoBuffer();

        if (m_videoBuffer == nullptr || !m_videoBuffer->Allocate(m_videoStream->Width(), m_videoStream->Height())) {
            Stop_Movie();
        }
    }
}

/**
 * 0x004217D0
 */
void Display::Stop_Movie()
{
    delete m_videoBuffer;
    m_videoBuffer = nullptr;

    if (m_videoStream != nullptr) {
        m_videoStream->Close();
    }
    m_videoStream = nullptr;

    if (m_currentlyPlayingMovie.Is_Not_Empty()) {
        m_currentlyPlayingMovie = Utf8String::s_emptyString;
    }

    if (m_unkDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_unkDisplayString);
    }
    m_unkDisplayString = nullptr;

    m_someLogoMovieInt1 = -1;
    m_someLogoMovieInt2 = -1;
}

/**
 * 0x004212C0
 */
void Display::Delete_Views()
{
    auto *view = m_viewList;
    while (view != nullptr) {
        // Save the next view before destroying the current view
        auto *next_view = view->Get_Next_View();
        delete view;
        view = next_view;
    }
    m_viewList = nullptr;
}
