/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief W3D class for the display handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "coord.h"
#include "display.h"
#include "light.h"
#include "render2d.h"
#include "scene.h"

class GameAssetManager;
class RTS3DScene;
class RTS2DScene;
class RTS3DInterfaceScene;
class W3DDebugDisplay;
class W3DVideoBuffer;

class W3DDisplay final : public Display
{
public:
    W3DDisplay();
    ~W3DDisplay() override;
    void Init() override;
    void Reset() override;
    void Draw() override;
    void Set_Width(uint32_t width) override;
    void Set_Height(uint32_t height) override;
    bool Set_Display_Mode(uint32_t width, uint32_t height, uint32_t bits, bool windowed) override;
    int Get_Display_Mode_Count() override;
    void Get_Display_Mode_Description(int id, int *width, int *height, int *bit_depth) override;
    void Set_Gamma(float gamma, float bright, float contrast, bool calibrate) override;
    void Do_Smart_Asset_Purge_And_Preload(const char *) override;
#ifdef GAME_DEBUG_STRUCTS
    void Write_Asset_Usage(const char *str) override;
#endif
    VideoBuffer *Create_VideoBuffer() override;
    void Set_Clip_Region(IRegion2D *region) override;
    bool Is_Clipping_Enabled() override { return m_isClippedEnabled; }
    void Enable_Clipping(bool isEnabled) override { m_isClippedEnabled = isEnabled; }
    void Set_Time_Of_Day(TimeOfDayType time) override;
    void Create_Light_Pulse(const Coord3D *pos,
        const RGBColor *color,
        float far_start,
        float far_dist,
        unsigned frame_increase_time,
        unsigned decay_frame_time) override;
    void Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color) override;
    void Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color1, uint32_t color2) override;
    void Draw_Open_Rect(int32_t x, int32_t y, int32_t width, int32_t height, float border_width, uint32_t color) override;
    void Draw_Fill_Rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) override;
    void Draw_Rect_Clock(int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color) override;
    void Draw_Remaining_Rect_Clock(
        int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color) override;
    void Draw_Image(
        Image *image, int32_t left, int32_t top, int32_t right, int32_t bottom, uint32_t color, DrawImageMode mode) override;
    void Draw_VideoBuffer(VideoBuffer *vb, int32_t x1, int32_t y1, int32_t x2, int32_t y2) override;
    void Set_Shroud_Level(int, int, CellShroudStatus) override;
    void Clear_Shroud() override{};
    void Set_Border_Shroud_Level(uint8_t level) override;
#ifdef GAME_DEBUG_STRUCTS
    void Write_Model_And_Texture_Usage(const char *filename) override;
#endif
    void Preload_Model_Assets(Utf8String model) override;
    void Preload_Texture_Assets(Utf8String texture) override;
    void Take_ScreenShot() override;
    void Toggle_Movie_Capture() override;
    void Toggle_LetterBox() override;
    void Enable_LetterBox(bool enable) override;
    bool Is_LetterBox_Fading() override;
    bool Is_LetterBoxed() override { return m_letterBoxEnabled; }
    float Get_Average_FPS() override { return m_averageFps; }
    int Get_Last_Frame_Draw_Calls() override;
    void Init_Assets() {}
    void Init_2D_Scene() {}
    void Init_3D_Scene() {}
    void Update_Average_FPS();
    void Gather_Debug_Stats();
    void Draw_Debug_Stats();
    void Draw_Benchmark();
    void Draw_Current_Debug_Display();
    void Calculate_Terrain_LOD();
    void Render_LetterBox(unsigned int current_time);

#ifdef GAME_DLL
    static GameAssetManager *&s_assetManager;
    static RTS3DScene *&s_3DScene;
    static RTS2DScene *&s_2DScene;
    static RTS3DInterfaceScene *&s_3DInterfaceScene;
#else
    static GameAssetManager *s_assetManager;
    static RTS3DScene *s_3DScene;
    static RTS2DScene *s_2DScene;
    static RTS3DInterfaceScene *s_3DInterfaceScene;
#endif

private:
    bool m_initialized;
    LightClass *m_myLight[4];
    Render2DClass *m_2DRender;
    IRegion2D m_clipRegion;
    bool m_isClippedEnabled;
    float m_averageFps;
#ifdef GAME_DEBUG_STRUCTS
    uint64_t m_performanceCounter;
#endif
    DisplayString *m_displayStrings[16];
    DisplayString *m_benchmarkDisplayString[1];
    W3DDebugDisplay *m_nativeDebugDisplay;
};

void Reset_D3D_Device(bool restore_assets);
