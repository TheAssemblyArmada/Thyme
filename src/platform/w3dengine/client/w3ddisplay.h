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

class W3DDisplay final : public Display
{
public:
    W3DDisplay();
    ~W3DDisplay();
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Draw() override;
    virtual void Set_Width(uint32_t width) override;
    virtual void Set_Height(uint32_t height) override;
    virtual bool Set_Display_Mode(uint32_t width, uint32_t height, uint32_t bits, bool windowed) override;
    virtual int Get_Display_Mode_Count() override;
    virtual void Get_Display_Mode_Description(int id, int *width, int *height, int *bit_depth) override;
    virtual void Set_Gamma(float gamma, float bright, float contrast, bool calibrate) override;
    virtual void Do_Smart_Asset_Purge_And_Preload(const char *) override;
    // WB additional virtual here
    virtual VideoBuffer *Create_VideoBuffer() override;
    virtual void Set_Clip_Region(IRegion2D *region) override;
    virtual bool Is_Clipping_Enabled() override { return m_isClippedEnabled; }
    virtual void Enable_Clipping(bool isEnabled) override { m_isClippedEnabled = isEnabled; }
    virtual void Set_Time_Of_Day(TimeOfDayType time) override;
    virtual void Create_Light_Pulse(const Coord3D *, const RGBColor *, float, float, unsigned, unsigned) override;
    virtual void Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color) override;
    virtual void Draw_Line(
        int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color1, uint32_t color2) override;
    virtual void Draw_Open_Rect(
        int32_t x, int32_t y, int32_t width, int32_t height, float border_width, uint32_t color) override;
    virtual void Draw_Fill_Rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) override;
    virtual void Draw_Rect_Clock(
        int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color) override;
    virtual void Draw_Remaining_Rect_Clock(
        int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color) override;
    virtual void Draw_Image(
        Image *image, int32_t left, int32_t top, int32_t right, int32_t bottom, uint32_t color, DrawImageMode mode) override;
    virtual void Draw_VideoBuffer(VideoBuffer *vb, int32_t x1, int32_t y1, int32_t x2, int32_t y2) override;
    virtual void Set_Shroud_Level(int, int, CellShroudStatus) override;
    virtual void Clear_Shroud() override{};
    virtual void Set_Border_Shroud_Level(uint8_t level) override;
    // WB additional virtual here
    virtual void Preload_Model_Assets(Utf8String model) override;
    virtual void Preload_Texture_Assets(Utf8String texture) override;
    virtual void Take_ScreenShot() override;
    virtual void Toggle_Movie_Capture() override;
    virtual void Toggle_LetterBox() override;
    virtual void Enable_LetterBox(bool enable) override;
    virtual bool Is_LetterBox_Fading() override;
    virtual bool Is_LetterBoxed() override { return m_letterBoxEnabled; }
    virtual float Get_Average_FPS() override { return m_averageFps; }
    virtual int Get_Last_Frame_Draw_Calls() override;

    static GameAssetManager *Get_AssetManager() { return s_assetManager; }
    static SceneClass *Get_3DInterfaceScene() { return s_3DInterfaceScene; }

private:
    bool m_initialized;
    LightClass *m_myLight[4];
    Render2DClass *m_2DRender;
    IRegion2D m_clipRegion;
    bool m_isClippedEnabled;
    float m_averageFps;
#ifdef GAME_DEBUG_STRUCTS
    uint64_t m_unknqword;
#endif
    DisplayString *m_displayStrings[16];
    DisplayString *m_benchmarkDisplayString;
    uint32_t m_nativeDebugDisplay;

#ifdef GAME_DLL
    static GameAssetManager *&s_assetManager;
    static SceneClass *&s_3DScene; // TODO: Actual type is RTS2DScene
    static SceneClass *&s_2DScene; // TODO: Actual type is RTS2DScene
    static SceneClass *&s_3DInterfaceScene; // TODO: Actual type is RTS3DInterfaceScene
#else
    static GameAssetManager *s_assetManager;
    static SceneClass *s_3DScene; // TODO: Actual type is RTS2DScene
    static SceneClass *s_2DScene; // TODO: Actual type is RTS2DScene
    static SceneClass *s_3DInterfaceScene; // TODO: Actual type is RTS3DInterfaceScene
#endif
};

// TODO: This is a member function of W3DGameClient
W3DDisplay *Create_Game_Display();
