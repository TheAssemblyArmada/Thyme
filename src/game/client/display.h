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
#pragma once

#include "always.h"
#include "color.h"
#include "coord.h"
#include "gamelod.h"
#include "gametype.h"
#include "subsysteminterface.h"
#include <cstdio>

class DebugDisplayInterface;
class DisplayString;
class GameFont;
class Image;
class VideoBuffer;
class VideoStream;
class View;

typedef void (*debugdisplaycallback_t)(DebugDisplayInterface *, void *, FILE *);

class Display : public SubsystemInterface
{
    // names based on related code
public:
    enum DrawImageMode
    {
        DRAWIMAGE_ADDITIVE_ALPHA = 0,
        DRAWIMAGE_GREYSCALE,
        DRAWIMAGE_ADDITIVE,
    };

    Display();
    virtual ~Display();
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override;

    virtual void Set_Width(unsigned width);
    virtual void Set_Height(unsigned height);
    virtual unsigned Get_Width() { return m_width; }
    virtual unsigned Get_Height() { return m_height; }
    virtual void Set_Bit_Depth(unsigned bitDepth) { m_bitDepth = bitDepth; }
    virtual unsigned Get_Bit_Depth() { return m_bitDepth; };
    virtual void Set_Windowed(int windowed) { m_windowed = windowed != 0; }
    virtual int Get_Windowed() { return m_windowed; }
    virtual bool Set_Display_Mode(unsigned width, unsigned height, unsigned bits, bool windowed);
    virtual int Get_Display_Mode_Count() { return 0; }
    virtual void Get_Display_Mode_Description(int, int *, int *, int *) {}
    virtual void Set_Gamma(float, float, float, bool) {}
    virtual bool Test_Min_Spec_Requirements(bool *a2, bool *a3, bool *a4, StaticGameLODLevel *lod_level, float *a6)
    {
        *a4 = 1;
        *a3 = 1;
        *a2 = 1;
        return true;
    };
    virtual void Do_Smart_Asset_Purge_And_Preload(const char *) = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Write_Asset_Usage(const char *str) = 0;
#endif
    virtual void Attach_View(View *view);
    virtual View *Get_First_View();
    virtual View *Get_Next_View(View *view);
    virtual void Draw_Views();
    virtual void Update_Views();
    virtual VideoBuffer *Create_VideoBuffer() = 0;
    virtual void Set_Clip_Region(IRegion2D *) = 0;
    virtual bool Is_Clipping_Enabled() = 0;
    virtual void Enable_Clipping(bool) = 0;
    virtual void Set_Time_Of_Day(TimeOfDayType) = 0;
    virtual void Create_Light_Pulse(const Coord3D *, const RGBColor *, float, float, unsigned, unsigned) = 0;
    virtual void Draw_Line(int, int, int, int, float, unsigned) = 0;
    virtual void Draw_Line(int, int, int, int, float, unsigned, unsigned) = 0;
    virtual void Draw_Open_Rect(int, int, int, int, float, unsigned) = 0;
    virtual void Draw_Fill_Rect(int, int, int, int, unsigned) = 0;
    virtual void Draw_Rect_Clock(int, int, int, int, int, unsigned) = 0;
    virtual void Draw_Remaining_Rect_Clock(int, int, int, int, int, unsigned) = 0;
    virtual void Draw_Image(const Image *image,
        int32_t left,
        int32_t top,
        int32_t right,
        int32_t bottom,
        uint32_t color,
        DrawImageMode mode) = 0;
    virtual void Draw_VideoBuffer(VideoBuffer *, int, int, int, int) = 0;
    virtual void Play_Logo_Movie(Utf8String name, int a3, int a4);
    virtual void Play_Movie(Utf8String name);
    virtual void Stop_Movie();
    virtual bool Is_Movie_Playing() { return m_videoStream != nullptr && m_videoBuffer != nullptr; }
    virtual void Set_Debug_Display_Callback(debugdisplaycallback_t func, void *data)
    {
        m_debugDisplayCallback = func;
        m_debugDisplayUserData = data;
    }
    virtual debugdisplaycallback_t Get_Debug_Display_Callback() { return m_debugDisplayCallback; }
    virtual void Set_Shroud_Level(int, int, CellShroudStatus) = 0;
    virtual void Clear_Shroud() = 0;
    virtual void Set_Border_Shroud_Level(uint8_t level) = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Write_Model_And_Texture_Usage(const char *filename) = 0;
#endif
    virtual void Preload_Model_Assets(Utf8String model) = 0;
    virtual void Preload_Texture_Assets(Utf8String texture) = 0;
    virtual void Take_ScreenShot() = 0;
    virtual void Toggle_Movie_Capture() = 0;
    virtual void Toggle_LetterBox() = 0;
    virtual void Enable_LetterBox(bool enable) = 0;
    virtual bool Is_LetterBox_Fading() { return 0; }
    virtual bool Is_LetterBoxed() { return 0; }
    virtual void Set_Cinematic_Text(Utf8String text) { m_cinematicText = text; }
    virtual void Set_Cinematic_Font(GameFont *font) { m_cinematicFont = font; }
    virtual void Set_Cinematic_Text_Frames(int frames) { m_cinematicTextFrames = frames; }
    virtual float Get_Average_FPS() = 0;
    virtual int Get_Last_Frame_Draw_Calls() = 0;
    virtual void Delete_Views();

protected:
    unsigned m_width;
    unsigned m_height;
    unsigned m_bitDepth;
    bool m_windowed;
    View *m_viewList;
    Utf8String m_cinematicText;
    GameFont *m_cinematicFont;
    int m_cinematicTextFrames;
    VideoBuffer *m_videoBuffer;
    VideoStream *m_videoStream;
    Utf8String m_currentlyPlayingMovie;
    DebugDisplayInterface *m_debugDisplay;
    debugdisplaycallback_t m_debugDisplayCallback;
    void *m_debugDisplayUserData;
    float m_letterBoxFadeLevel;
    bool m_letterBoxEnabled;
    int m_letterBoxFadeStartTime;
    int m_someLogoMovieInt1;
    int m_someLogoMovieInt2;
    unsigned m_someLogoMovieTime;
    unsigned m_unkInt;
    DisplayString *m_unkDisplayString;
};

#ifdef GAME_DLL
#include "hooker.h"
extern Display *&g_theDisplay;
#else
extern Display *g_theDisplay;
#endif
