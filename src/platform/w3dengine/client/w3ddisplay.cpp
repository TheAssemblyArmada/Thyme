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

#include "w3ddisplay.h"
#include "baseheightmap.h"
#include "colorspace.h"
#include "displaystring.h"
#include "displaystringmanager.h"
#include "drawable.h"
#include "drawmodule.h"
#include "dx8wrapper.h"
#include "filesystem.h"
#include "gameclient.h"
#include "gameengine.h"
#include "gamelogic.h"
#include "gametext.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "hlod.h"
#include "image.h"
#include "ingameui.h"
#include "keyboard.h"
#include "line2d.h"
#include "main.h"
#include "mesh.h"
#include "meshmdl.h"
#include "mouse.h"
#include "network.h"
#include "particlesysmanager.h"
#include "physicsupdate.h"
#include "rddesc.h"
#include "rtsutils.h"
#include "scriptengine.h"
#include "shadermanager.h"
#include "sortingrenderer.h"
#include "videobuffer.h"
#include "w3d.h"
#include "w3dassetmanager.h"
#include "w3ddebugdisplay.h"
#include "w3ddynamiclight.h"
#include "w3dfilesystem.h"
#include "w3dprojectedshadow.h"
#include "w3dscene.h"
#include "w3dshroud.h"
#include "w3dterraintracks.h"
#include "w3dview.h"
#include "w3dwater.h"
#include "water.h"
#include "worldheightmap.h"
#ifdef BUILD_WITH_D3D8
#include <io.h>
#endif
#include <stdio.h>

#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifdef GAME_DEBUG_STRUCTS
#ifdef GAME_DLL
ICoord2D &s_leftClickReleased = Make_Global<ICoord2D>(0x00E1B118);
#else
ICoord2D s_leftClickReleased;
#endif
#endif

#ifndef GAME_DLL
GameAssetManager *W3DDisplay::s_assetManager;
RTS3DScene *W3DDisplay::s_3DScene;
RTS2DScene *W3DDisplay::s_2DScene;
RTS3DInterfaceScene *W3DDisplay::s_3DInterfaceScene;
#endif

int g_theW3DFrameLengthInMsec = 33;

// PerfStatsClass PerfStats("StatisticsDump.txt");

// 0x0073C3C0
W3DDisplay::W3DDisplay() :
    m_initialized(false), m_2DRender(0), m_clipRegion({ { 0, 0 }, { 0, 0 } }), m_isClippedEnabled(false)
{
    s_assetManager = nullptr;
    s_3DScene = nullptr;
    s_2DScene = nullptr;
    s_3DInterfaceScene = nullptr;
    m_averageFps = g_theWriteableGlobalData->m_framesPerSecondLimit;
#ifdef GAME_DEBUG_STRUCTS
    m_performanceCounter = 0;
#endif
    for (auto &str : m_displayStrings) {
        str = nullptr;
    }
    for (auto &light : m_myLight) {
        light = nullptr;
    }

    // #BUGFIX Initialize all members
    for (auto &str : m_benchmarkDisplayString) {
        str = nullptr;
    }
    m_nativeDebugDisplay = nullptr;
}

// 0x0073C453
W3DDisplay::~W3DDisplay()
{
    delete m_debugDisplay;

    for (int i = 0; i < ARRAY_SIZE(m_displayStrings); i++) {
        g_theDisplayStringManager->Free_Display_String(m_displayStrings[i]);
    }

    delete m_2DRender;
    m_2DRender = nullptr;

    Delete_Views();

    Ref_Ptr_Release(s_3DScene);
    Ref_Ptr_Release(s_2DScene);
    Ref_Ptr_Release(s_3DInterfaceScene);

    for (int i = 0; i < ARRAY_SIZE(m_myLight); i++) {
        Ref_Ptr_Release(m_myLight[i]);
    }

    // Debug_Statistics::Shutdown_Statistics
    W3DShaderManager::Shutdown();
    s_assetManager->Free_Assets();
    delete s_assetManager;
    W3D::Shutdown();
    GameMath::Shutdown();
    // DX8WebBrowser::Shutdown
    delete g_theW3DFileSystem;
}

void Stat_Debug_Display(DebugDisplayInterface *dd, void *user_data, FILE *handle)
{
    captainslog_dbgassert(false, "This should never be called directly, but is just a placeholder for drawDebugStats()");
}

// 0x0073CA80
void W3DDisplay::Init()
{
    Display::Init();

    if (!m_initialized) {
        g_theW3DFileSystem = new W3DFileSystem();
        GameMath::Init();

        s_3DInterfaceScene = new RTS3DInterfaceScene();
        s_3DInterfaceScene->Set_Ambient_Light(Vector3(1.0f, 1.0f, 1.0f));

        s_2DScene = new RTS2DScene();
        s_2DScene->Set_Ambient_Light(Vector3(1.0f, 1.0f, 1.0f));

        s_3DScene = new RTS3DScene();

#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData->m_wireframe) {
            s_3DScene->Set_Polygon_Mode(SceneClass::LINE);
        }
#endif
        static_assert(LIGHT_COUNT <= ARRAY_SIZE(m_myLight), "Error");
        captainslog_assert(g_theWriteableGlobalData->m_numberGlobalLights <= ARRAY_SIZE(m_myLight));

        for (int i = 0; i < g_theWriteableGlobalData->m_numberGlobalLights; i++) {
            m_myLight[i] = new LightClass(LightClass::DIRECTIONAL);
        }

        Set_Time_Of_Day(g_theWriteableGlobalData->m_timeOfDay);

        for (int i = 0; i < g_theWriteableGlobalData->m_numberGlobalLights; i++) {
            s_3DScene->Set_Global_Light(m_myLight[i], i);
        }

        s_assetManager = new GameAssetManager();
        s_assetManager->Set_W3D_Load_On_Demand(true);

        if (g_theWriteableGlobalData->m_setMinVertexBufferSize) {
            SortingRendererClass::Set_Min_Vertex_Buffer_Size(1);
        }

#ifdef BUILD_WITH_D3D8
        if (W3D::Init(g_applicationHWnd)) {
            throw CODE_07;
        }
#endif

        W3D::Set_Prelit_Mode(W3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
        W3D::Set_Collision_Box_Display_Mask(0);
        W3D::Enable_Static_Sort_Lists(true);
        W3D::Set_Thumbnail_Enabled(false);
        W3D::Set_Screen_UV_Bias(true);
        W3D::Set_Texture_Bit_Depth(32);

        Set_Windowed(g_theWriteableGlobalData->m_windowed);

        m_2DRender = new Render2DClass();
        captainslog_dbgassert(m_2DRender, "Cannot create Render2DClass");

        Set_Width(g_theWriteableGlobalData->m_xResolution);
        Set_Height(g_theWriteableGlobalData->m_yResolution);
        Set_Bit_Depth(32);

        if (W3D::Set_Render_Device(0, Get_Width(), Get_Height(), Get_Bit_Depth(), Get_Windowed(), true, false, true)) {
            Set_Bit_Depth(16);

            if (W3D::Set_Render_Device(0, Get_Width(), Get_Height(), Get_Bit_Depth(), Get_Windowed(), true, false, true)) {
                W3D::Shutdown();
                GameMath::Shutdown();
                throw CODE_07;
                captainslog_debug("Unable to set render device");
            }
        }

        if (g_theGameLODManager->Get_Static_LOD_Level() == STATLOD_INVALID) {
            g_theGameLODManager->Set_Static_LOD_Level(g_theGameLODManager->Find_Static_LOD_Level());
        } else {
            if (g_theWriteableGlobalData->m_textureReductionFactor > 0) {
                W3D::Set_Texture_Reduction(g_theWriteableGlobalData->m_textureReductionFactor, 32);
                g_theGameLODManager->Set_Texture_Reduction_Factor(g_theWriteableGlobalData->m_textureReductionFactor);
            }
        }

        if (g_theWriteableGlobalData->m_gammaValue != 1.0f) {
            Set_Gamma(g_theWriteableGlobalData->m_gammaValue, 0.0f, 1.0f, false);
        }

        Init_Assets();
        Init_2D_Scene();
        Init_3D_Scene();
        W3DShaderManager::Init();

        m_nativeDebugDisplay = new W3DDebugDisplay();
        m_debugDisplay = m_nativeDebugDisplay;

        if (m_nativeDebugDisplay != nullptr) {
            m_nativeDebugDisplay->Init();
            GameFont *font;

            if (g_theGlobalLanguage && g_theGlobalLanguage->Debug_Display_Font().Name().Is_Not_Empty()) {
                font = g_theFontLibrary->Get_Font(g_theGlobalLanguage->Debug_Display_Font().Name(),
                    g_theGlobalLanguage->Debug_Display_Font().Point_Size(),
                    g_theGlobalLanguage->Debug_Display_Font().Bold());
            } else {
                font = g_theFontLibrary->Get_Font("FixedSys", 8, false);
            }

            m_nativeDebugDisplay->Set_Font(font);
            m_nativeDebugDisplay->Set_Font_Width(13);
            m_nativeDebugDisplay->Set_Font_Height(9);
        }

        // DX8WebBrowser::Initialize
        m_initialized = true;

        if (g_theWriteableGlobalData->m_displayDebug) {
            m_debugDisplayCallback = Stat_Debug_Display;
        }
    }
}

// 0x0073D030
void W3DDisplay::Reset()
{
    Display::Reset();
    auto *iter = s_3DScene->Create_Iterator(false);

    for (iter->First(); !iter->Is_Done(); iter->Next()) {
        auto *renderObj = iter->Current_Item();
        renderObj->Add_Ref();
        s_3DScene->Remove_Render_Object(renderObj);
        renderObj->Release_Ref();
    }

    s_3DScene->Destroy_Iterator(iter);
    m_isClippedEnabled = false;
    s_assetManager->Release_Unused_Assets();

    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_drawSkyBox = false;
    }
}

void Draw_Graphical_Framerate_Bar()
{
    static int lastTime = rts::Get_Time();
    int time = rts::Get_Time();
    float f1 = 1000.0f / (float)(time - lastTime) / (1000.0f / (float)g_theWriteableGlobalData->m_framesPerSecondLimit);
    f1 = std::clamp(f1, 0.0f, 1.0f);

    unsigned int red = GameMath::Fast_To_Int_Truncate((1.0f - f1) * 255.0f);
    unsigned int green = GameMath::Fast_To_Int_Truncate(f1 * 255.0f);
    unsigned int color = Make_Color(red, green, 0, 127);
    g_theDisplay->Draw_Fill_Rect(1, 1, GameMath::Fast_To_Int_Truncate((float)g_theDisplay->Get_Width() * f1), 15, color);
    lastTime = time;
}

// 0x0073E360
void W3DDisplay::Draw()
{
    // TODO figure out gotos

#ifdef BUILD_WITH_D3D8
    if (g_applicationHWnd && IsIconic(g_applicationHWnd)) {
        return;
    }

    static int lastFrame = -1;
    static int syncTime = 0;

    Update_Average_FPS();

    if (g_theWriteableGlobalData->m_dynamicLOD && g_theGameLogic->Get_Dynamic_LOD()) {
        g_theGameLODManager->Set_Dynamic_LOD_Level(g_theGameLODManager->Find_Dynamic_LOD_Level(m_averageFps));
    } else {
        g_theGameLODManager->Set_Dynamic_LOD_Level(DYNLOD_VERYHIGH);
    }

    if (g_theWriteableGlobalData->m_terrainLOD == TERRAIN_LOD_AUTOMATIC) {
        if (g_theTerrainRenderObject) {
            Calculate_Terrain_LOD();
        }
    }

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_doStatDump) {
        // PerfStats.Log(false, true);
        g_theWriteableGlobalData->m_doStatDump = false;
    } else if (g_theWriteableGlobalData->m_doStats && g_theGameLogic->Get_Game_Mode() <= GAME_REPLAY) {
        if (g_theGameLogic->Get_Frame()) {
            if (!(g_theGameLogic->Get_Frame() % g_theWriteableGlobalData->m_statsInterval)) {
                // PerfStats.Log(true, true);
                g_theInGameUI->Message(L"-stats is running, at interval: %d.", g_theWriteableGlobalData->m_statsInterval);
            }
        }
    }

    if (m_debugDisplayCallback == Stat_Debug_Display || g_theWriteableGlobalData->m_benchmarkTimer > 0) {
        Gather_Debug_Stats();
    }
#endif

    bool time_frozen = g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished();
    time_frozen |= g_theScriptEngine->Is_Time_Frozen_Debug() || g_theScriptEngine->Is_Time_Frozen_Script();
    time_frozen |= g_theGameLogic->Is_Game_Paused();
    time_frozen |= lastFrame == g_theGameClient->Get_Frame();
    lastFrame = g_theGameClient->Get_Frame();

    W3DView *primary_view = static_cast<W3DView *>(Get_First_View());

    if (time_frozen && g_theScriptEngine->Is_Time_Fast()) {
        primary_view->Update_Camera_Movements();
        syncTime += g_theW3DFrameLengthInMsec;
        return;
    }

    // Debug_Statistics::Begin_Statistics();

    if (!g_theWriteableGlobalData->m_unkBool17) {
        if (g_theTerrainTracksRenderObjClassSystem) {
            g_theTerrainTracksRenderObjClassSystem->Update();
        }

        if (g_theTerrainRenderObject) {
            if (g_theTerrainRenderObject->Get_Map()) {
                if (g_theTerrainRenderObject->Get_Shroud()) {
                    g_theTerrainRenderObject->Get_Shroud()->Render(primary_view->Get_3D_Camera());
                }
            }
        }
    }

    if (!time_frozen) {
        syncTime += g_theW3DFrameLengthInMsec;
    }

    W3D::Sync(syncTime);
    int time = 30;
    static unsigned int prevTime = rts::Get_Time();
    unsigned int now = rts::Get_Time();

    if (g_theTacticalView->Get_Time_Multiplier() <= 1) {
        prevTime = now - time;

        static bool couldRender = false;

        for (;;) {
        l1:
            if (!g_theWriteableGlobalData->m_unkBool17) {
                while (g_theWriteableGlobalData->m_useFPSLimit && (int)(now - prevTime) < time - 1) {
                    now = rts::Get_Time();
                }

                prevTime = now;
            }

            if (DX8Wrapper::Get_D3D_Device8() != nullptr) {
                if (SUCCEEDED(DX8Wrapper::Get_D3D_Device8()->TestCooperativeLevel())) {
                    Update_Views();
                    g_theParticleSystemManager->Update();

                    if (g_theWaterRenderObj) {
                        if (g_theWriteableGlobalData->m_waterType == WaterRenderObjClass::WATER_TYPE_2_PVSHADER) {
                            g_theWaterRenderObj->Update_Render_Target_Textures(primary_view->Get_3D_Camera());
                        }
                    }

                    if (g_theW3DProjectedShadowManager) {
                        g_theW3DProjectedShadowManager->Update_Render_Target_Textures();
                    }
                }
            }

            // Debug_Statistics::End_Statistics();
            int polys = 0;
            // polys = Debug_Statistics::Get_DX8_Polygons();
            int verts = 0;
            // verts = Debug_Statistics::Get_DX8_Vertices();
#ifdef GAME_DEBUG_STRUCTS
            if (g_theGameLogic->Get_Frame() % 30 != 1 && !g_theGameLogic->Is_Game_Paused()
                && g_theWriteableGlobalData->m_TiVOFastMode && g_theGameLogic->Is_In_Replay_Game()) {
                goto l2;
            }
#else
            if (g_theGameLogic->Get_Frame() % 30 != 1 && !g_theGameLogic->Is_Game_Paused()
                && g_theWriteableGlobalData->m_demoToggleSpecialPowerDelays && g_theGameLogic->Is_In_Replay_Game()) {
                goto l2;
            }
#endif

            if (g_theWriteableGlobalData->m_unkBool26) {
                break;
            }

            if (g_theWriteableGlobalData->m_demoToggleRender) {
                break;
            }

            if (W3D::Begin_Render(
                    true, true, Vector3(0.0f, 0.0f, 0.0f), g_theWaterTransparency->m_transparentWaterMinOpacity, nullptr)) {
                break;
            }

            if (!g_theWriteableGlobalData->m_unkBool17) {
                couldRender = true;

                if (polys || verts) {
                    // Debug_Statistics::Record_DX8_Polys_And_Vertices(polys, verts, &ShaderClass::s_presetOpaqueShader);
                }

                Draw_Views();
                // Calls some debug stuff in WB
                g_theInGameUI->Draw();

                if (g_theMouse) {
                    // Calls some debug stuff in WB
                    g_theMouse->Draw();
                }

                if (m_videoStream) {
                    if (m_videoBuffer) {
                        Draw_VideoBuffer(m_videoBuffer, 0, 0, Get_Width(), Get_Height());
                    }
                }

                if (m_unkDisplayString) {
                    int x;
                    int y;
                    m_unkDisplayString->Get_Size(&x, &y);
                    m_unkDisplayString->Draw(
                        (Get_Width() / 2) - x / 2, Get_Height() - y - 20, Make_Color(0, 0, 0, 0), Make_Color(0, 0, 0, 0xFF));
                }

                Render_LetterBox(now);

                if (m_cinematicText != Utf8String::s_emptyString) {
                    if (m_cinematicTextFrames) {
                        DisplayString *display_str = g_theDisplayStringManager->New_Display_String();
                        display_str->Set_Word_Wrap(g_theDisplay->Get_Width() - 20);
                        display_str->Set_Word_Wrap_Centered(true);
                        Utf16String str;
                        str.Translate(m_cinematicText);
                        display_str->Set_Text(str);
                        display_str->Set_Font(m_cinematicFont);
                        int h = (int)((float)g_theDisplay->Get_Height() * 0.9f);
                        int w;

                        const int str_width = display_str->Get_Width(-1);
                        const int display_width = g_theDisplay->Get_Width();

                        if (str_width <= display_width) {
                            w = (display_width - str_width) / 2;
                        } else {
                            w = 20;
                        }

                        display_str->Draw(w, h, Make_Color(0xFF, 0xFF, 0xFF, 0xFF), Make_Color(0, 0, 0, 0));
                        m_cinematicTextFrames--;
                    }
                }

                if (m_debugDisplayCallback) {
                    Draw_Current_Debug_Display();
                }

#ifdef GAME_DEBUG_STRUCTS
                if (g_theWriteableGlobalData->m_benchmarkTimer) {
                    Draw_Benchmark();
                }
#endif
                if (g_theWriteableGlobalData->m_showFrameRateBar) {
                    Draw_Graphical_Framerate_Bar();
                }

                W3D::End_Render(true);
            l2:
                if (g_theScriptEngine->Is_Time_Frozen_Debug() || g_theScriptEngine->Is_Time_Frozen_Script()
                    || g_theGameLogic->Is_Game_Paused()) {
                    time_frozen = false;
                }
                goto l3;
            }

            g_theInGameUI->Draw();

            if (g_theMouse) {
                g_theMouse->Draw();
            }

            W3D::End_Render(true);
        l3:
            if (!time_frozen || g_theTacticalView->Is_Camera_Movement_Finished()) {
                return;
            }
        }

        if (couldRender) {
            couldRender = false;
            captainslog_debug("Could not do WW3D::Begin_Render()!  Are we ALT-Tabbed out?\n");
        }

        goto l2;
    }

    static int timeMultiplierCounter = 1;
    timeMultiplierCounter--;

    if (timeMultiplierCounter <= 1) {
        timeMultiplierCounter = g_theTacticalView->Get_Time_Multiplier();
        goto l1;
    }
#endif
}

// 0x0073C980
void W3DDisplay::Set_Width(uint32_t width)
{
    Display::Set_Width(width);
    RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(Get_Width()), static_cast<float>(Get_Height()) } };
    m_2DRender->Set_Coordinate_Range(rect);
}

// 0x0073CA00
void W3DDisplay::Set_Height(uint32_t height)
{
    Display::Set_Height(height);
    RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(Get_Width()), static_cast<float>(Get_Height()) } };
    m_2DRender->Set_Coordinate_Range(rect);
}

// 0x0073C840
bool W3DDisplay::Set_Display_Mode(uint32_t width, uint32_t height, uint32_t bits, bool windowed)
{
    auto success = W3D::Set_Device_Resolution(width, height, bits, windowed, true) == W3D_ERROR_OK;

    if (success) {
        RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(width), static_cast<float>(height) } };
        m_2DRender->Set_Screen_Resolution(rect);

        Display::Set_Display_Mode(width, height, bits, windowed);

        return true;
    } else {
        // Set resolution failed revert to previous resolution
        W3D::Set_Device_Resolution(Get_Width(), Get_Height(), Get_Bit_Depth(), Get_Windowed(), true);

        RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(Get_Width()), static_cast<float>(Get_Height()) } };
        m_2DRender->Set_Screen_Resolution(rect);

        Display::Set_Display_Mode(Get_Width(), Get_Height(), Get_Bit_Depth(), Get_Windowed() != 0);

        return false;
    }
}

static bool Is_Resolution_Valid(const ResolutionDescClass &res)
{
    if (res.Get_Depth() < 24) {
        return false;
    }

    if (res.Get_Width() < 800) {
        return false;
    }

    if (res.Get_Height() == 0) {
        return false;
    }

    // #FEATURE Aspect ratio check removed as users do not mind some aspect issues
#if 0
     float aspect_ratio = GameMath::Fabs(static_cast<float>(res.Get_Width()) / res.Get_Height());
     if (aspect_ratio < 1.332f || aspect_ratio > 1.334f) {
        return false;
    }
#endif
    return true;
}

// 0x0073C5D0
int W3DDisplay::Get_Display_Mode_Count()
{
    auto &desc = W3D::Get_Render_Device_Desc(0);
    auto &resolutions = desc.Get_Resolution_Array();

    int valid_resolutions = 0;
    for (auto i = 0; i < resolutions.Count(); ++i) {
        auto &resolution = resolutions[i];

        if (Is_Resolution_Valid(resolution) == false) {
            continue;
        }

        valid_resolutions++;
    }
    return valid_resolutions;
}

// 0x0073C650
void W3DDisplay::Get_Display_Mode_Description(int id, int *width, int *height, int *bit_depth)
{
    auto &desc = W3D::Get_Render_Device_Desc(0);
    auto &resolutions = desc.Get_Resolution_Array();

    // Id passed into function is in reference to valid resolutions
    int valid_resolutions = 0;
    for (auto i = 0; i < resolutions.Count(); ++i) {
        auto &resolution = resolutions[i];

        if (Is_Resolution_Valid(resolution) == false) {
            continue;
        }

        if (valid_resolutions != id) {
            valid_resolutions++;
            continue;
        }
        *width = resolution.Get_Width();
        *height = resolution.Get_Height();
        *bit_depth = resolution.Get_Depth();
        return;
    }
}

// 0x0073C710
void W3DDisplay::Set_Gamma(float gamma, float bright, float contrast, bool calibrate)
{
    if (m_windowed == true) {
        return;
    }

    DX8Wrapper::Set_Gamma(gamma, bright, contrast, calibrate, false);
}

// 0x00741360 Unsure if used
void W3DDisplay::Do_Smart_Asset_Purge_And_Preload(const char *asset)
{
    if (s_assetManager == nullptr) {
        return;
    }
    if (asset == nullptr) {
        return;
    }
    if (*asset == '\0') {
        return;
    }

    DynamicVectorClass<StringClass> assets_list(8000);
    auto *file = g_theFileSystem->Open_File(asset, File::TEXT | File::READ);
    if (file != nullptr) {
        Utf8String file_content;
        while (file->Scan_String(file_content)) {
            if (file_content.Starts_With(";") == false) {
                StringClass line{ file_content.Str() };
                assets_list.Add(line);
            }
        }
        file->Close();
    }
    s_assetManager->Free_Assets_With_Exclusion_List(assets_list);
}

// 0x00740B10
VideoBuffer *W3DDisplay::Create_VideoBuffer()
{
    W3DVideoBuffer::Type type = W3DVideoBuffer::TYPE_UNKNOWN;
    WW3DFormat format = DX8Wrapper::Get_Back_Buffer_Format();
    if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(format)) {
        type = W3DVideoBuffer::W3D_Format_To_Type(format);
    }

    if (type == W3DVideoBuffer::TYPE_UNKNOWN) {
        if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_X8R8G8B8)) {
            type = W3DVideoBuffer::TYPE_X8R8G8B8;
        } else if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_R8G8B8)) {
            type = W3DVideoBuffer::TYPE_R8G8B8;
        } else if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_R5G6B5)) {
            type = W3DVideoBuffer::TYPE_R5G6B5;
        } else if (DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_X1R5G5B5)) {
            type = W3DVideoBuffer::TYPE_X1R5G5B5;
        } else {
            return nullptr;
        }
    }

    if (!g_theWriteableGlobalData->m_playIntro) {
        type = W3DVideoBuffer::TYPE_R5G6B5;
    }

    return new W3DVideoBuffer(type);
}

// 0x00740C80
void W3DDisplay::Set_Clip_Region(IRegion2D *region)
{
    m_clipRegion = *region;
    m_isClippedEnabled = true;
}

// 0x0073EE10
void W3DDisplay::Set_Time_Of_Day(TimeOfDayType time)
{
    if (s_3DScene != nullptr) {
        const auto &ambient = g_theWriteableGlobalData->m_terrainObjectLighting[time][0].ambient;
        s_3DScene->Set_Ambient_Light({ ambient.red, ambient.green, ambient.blue });
    }

    for (auto i = 0; i < 4; ++i) {
        if (m_myLight[i] == nullptr) {
            continue;
        }

        auto &light = *m_myLight[i];
        light.Set_Ambient({ 0.0f, 0.0f, 0.0f });
        const auto &lighting = g_theWriteableGlobalData->m_terrainObjectLighting[time][i];
        light.Set_Diffuse(Vector3(lighting.diffuse.red, lighting.diffuse.green, lighting.diffuse.blue));
        light.Set_Specular(Vector3(0.0f, 0.0f, 0.0f));

        Matrix3D mat;
        mat.Set(Vector3(1.0f, 0.0f, 0.0f),
            Vector3(0.0f, 1.0f, 0.0f),
            Vector3(lighting.lightPos.x, lighting.lightPos.y, lighting.lightPos.z),
            Vector3(0.0f, 0.0f, 0.0f));
        light.Set_Transform(mat);
    }

    if (g_theTerrainRenderObject != nullptr) {
        g_theTerrainRenderObject->Set_Time_Of_Day(time);
        g_theTacticalView->Force_Redraw();
    }
}

// 0x0073EC50
void W3DDisplay::Create_Light_Pulse(const Coord3D *pos,
    const RGBColor *color,
    float far_start,
    float far_dist,
    unsigned frame_increase_time,
    unsigned decay_frame_time)
{
    if (far_start + far_dist >= 21.0f) {
        W3DDynamicLight *light = s_3DScene->Get_A_Dynamic_Light();
        light->Set_Enabled(true);
        light->Set_Ambient(Vector3(color->red, color->green, color->blue));
        light->Set_Diffuse(Vector3(color->red, color->green, color->blue));
        light->Set_Position(Vector3(pos->x, pos->y, pos->z));
        light->Set_Far_Attenuation_Range(far_start, far_start + far_dist);
        light->Set_Frame_Fade(frame_increase_time, decay_frame_time);
        light->Set_Decay_Range();
        light->Set_Decay_Color();
        light->Set_Flag(LightClass::FAR_ATTENUATION, true);
    }
}

// 0x0073EF90
void W3DDisplay::Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color)
{
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);
    Vector2 pos1 = { static_cast<float>(x1), static_cast<float>(y1) };
    Vector2 pos2 = { static_cast<float>(x2), static_cast<float>(y2) };
    m_2DRender->Add_Line(pos1, pos2, width, color);
    m_2DRender->Render();
}

// 0x0073F000
void W3DDisplay::Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color1, uint32_t color2)
{
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);
    Vector2 pos1 = { static_cast<float>(x1), static_cast<float>(y1) };
    Vector2 pos2 = { static_cast<float>(x2), static_cast<float>(y2) };
    m_2DRender->Add_Line(pos1, pos2, width, color1, color2);
    m_2DRender->Render();
}

// 0x0073F070
void W3DDisplay::Draw_Open_Rect(int32_t x, int32_t y, int32_t width, int32_t height, float border_width, uint32_t color)
{
    if (m_isClippedEnabled) {
        ICoord2D p1;
        ICoord2D p2;
        ICoord2D c1;
        ICoord2D c2;
        p1.x = x;
        p1.y = y;
        p2.x = x;
        p2.y = height + y;

        if (Clip_Line_2D(&p1, &p2, &c1, &c2, &m_clipRegion)) {
            Draw_Line(c1.x, c1.y, c2.x, c2.y, border_width, color);
        }

        p2.x = width + p1.x;
        p2.y = p1.y;

        if (Clip_Line_2D(&p1, &p2, &c1, &c2, &m_clipRegion)) {
            Draw_Line(c1.x, c1.y, c2.x, c2.y, border_width, color);
        }

        p1.x = width + x;
        p1.y = y;
        p2.x = width + x;
        p2.y = height + y;

        if (Clip_Line_2D(&p1, &p2, &c1, &c2, &m_clipRegion)) {
            Draw_Line(c1.x, c1.y, c2.x, c2.y, border_width, color);
        }

        p1.x = x;
        p1.y = height + y;
        p2.x = width + x;
        p2.y = height + y;

        if (Clip_Line_2D(&p1, &p2, &c1, &c2, &m_clipRegion)) {
            Draw_Line(c1.x, c1.y, c2.x, c2.y, border_width, color);
        }
    } else {
        m_2DRender->Reset();
        m_2DRender->Enable_Texturing(false);
        RectClass rect = { { static_cast<float>(x), static_cast<float>(y) },
            { static_cast<float>(x + width), static_cast<float>(y + height) } };
        m_2DRender->Add_Outline(rect, border_width, color);
        m_2DRender->Render();
    }
}

// 0x0073F2A0
void W3DDisplay::Draw_Fill_Rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);
    RectClass rect = { { static_cast<float>(x), static_cast<float>(y) },
        { static_cast<float>(x + width), static_cast<float>(y + height) } };
    m_2DRender->Add_Rect(rect, 0.0f, 0, color);
    m_2DRender->Render();
}

// 0x0073F320 Draws the percentage ontop of a rectangle
void W3DDisplay::Draw_Rect_Clock(int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color)
{
    // TODO: Function maybe not used at all
    if (percentage < 1 || percentage > 100) {
        return;
    }

    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);

    float left = x;
    float top = y;
    float right = x + width;
    float bottom = y + height;
    float mid_x = width / 2;
    float mid_left = x + width / 2;
    float mid_y = height / 2;
    float mid_top = y + height / 2;
    if (percentage == 100) {
        // Draw a full rect for the whole area
        RectClass rect = { { left, top }, { right, bottom } };
        m_2DRender->Add_Rect(rect, 0.0f, 0, color);
        m_2DRender->Render();
        return;
    }

    Vector2 uv0{ 0.0f, 0.0f };
    Vector2 uv1{ 0.0f, 0.0f };
    Vector2 uv2{ 0.0f, 0.0f };

    if (percentage <= 25) {
        // At 12, clock points at corner of Right Top of box
        if (percentage <= 12) {
            // Right Top Moving Wedge

            float pt = mid_left + static_cast<float>(percentage) * (1.0f / 12.0f) * mid_x;
            Vector2 pt0{ mid_left, top };
            Vector2 pt1{ mid_left, mid_top };
            Vector2 pt2{ pt, top };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Right Top Static Wedge
            {
                Vector2 pt0{ mid_left, mid_top };
                Vector2 pt1{ right, top };
                Vector2 pt2{ mid_left, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Top Moving Wedge
            {
                float pt = static_cast<float>(percentage - 12) * (1.0f / 13.0f) * mid_y + top;
                Vector2 pt0{ right, top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ right, pt };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    } else if (percentage <= 50) {
        // Right Top Box
        RectClass right_top_box = { { mid_left, top }, { right, mid_top } };
        m_2DRender->Add_Rect(right_top_box, 0.0f, 0, color);

        // At 37, clock points at corner of Right Bottom of box
        if (percentage <= 37) {

            // Right Bottom Moving Wedge
            float pt = static_cast<float>(percentage - 25) * (1.0f / 12.0f) * mid_y + mid_top;
            Vector2 pt0{ right, mid_top };
            Vector2 pt1{ mid_left, mid_top };
            Vector2 pt2{ right, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Right Bottom Static Wedge
            {
                Vector2 pt0{ right, mid_top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ right, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Bottom Moving Wedge
            {
                float pt = right - static_cast<float>(percentage - 37) * (1.0f / 13.0f) * mid_x;
                Vector2 pt0{ right, bottom };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ pt, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    } else if (percentage <= 75) {
        // Right Two Boxes
        RectClass right_rect = { { mid_left, top }, { right, bottom } };
        m_2DRender->Add_Rect(right_rect, 0.0f, 0, color);

        // At 62, clock points at corner of Left Bottom of box
        Vector2 pt0{ mid_left, mid_top };
        if (percentage <= 62) {

            // Left Bottom Moving Wedge
            float pt = mid_left - static_cast<float>(percentage - 50) * (1.0f / 12.0f) * mid_x;
            Vector2 pt1{ pt, bottom };
            Vector2 pt2{ mid_left, bottom };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Left Bottom Static Wedge
            {
                Vector2 pt1{ left, bottom };
                Vector2 pt2{ mid_left, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Bottom Moving Wedge
            {
                float pt = bottom - static_cast<float>(percentage - 62) * (1.0f / 13.0f) * mid_y;
                Vector2 pt1{ left, pt };
                Vector2 pt2{ left, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    } else {
        // Right Two Boxes
        RectClass right_rect = { { mid_left, top }, { right, bottom } };
        m_2DRender->Add_Rect(right_rect, 0.0f, 0, color);

        // Left Bottom Box
        RectClass left_bottom_rect = { { left, mid_top }, { mid_left, bottom } };
        m_2DRender->Add_Rect(left_bottom_rect, 0.0f, 0, color);

        // At 87, clock points at corner of Left Top of box
        if (percentage <= 87) {

            // Left Top Moving Wedge
            float pt = mid_top - static_cast<float>(percentage - 75) * (1.0f / 12.0f) * mid_y;
            Vector2 pt0{ left, mid_top };
            Vector2 pt1{ mid_left, mid_top };
            Vector2 pt2{ left, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Left Top Static Wedge
            {
                Vector2 pt0{ left, mid_top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ left, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Top Moving Wedge
            {
                float pt = left + static_cast<float>(percentage - 87) * (1.0f / 13.0f) * mid_x;
                Vector2 pt0{ left, top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ pt, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    }
    m_2DRender->Render();
}

// 0x0073FD40 Draws the percentage remaining ontop of a rectangle
void W3DDisplay::Draw_Remaining_Rect_Clock(
    int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color)
{
    if (percentage < 0 || percentage > 99) {
        return;
    }

    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);

    float left = x;
    float top = y;
    float right = x + width;
    float bottom = y + height;
    float mid_x = width / 2;
    float mid_left = x + width / 2;
    float mid_y = height / 2;
    float mid_top = y + height / 2;
    if (percentage == 0) {
        // Draw a full rect for the whole area
        RectClass rect = { { left, top }, { right, bottom } };
        m_2DRender->Add_Rect(rect, 0.0f, 0, color);
        m_2DRender->Render();
        return;
    }

    Vector2 uv0{ 0.0f, 0.0f };
    Vector2 uv1{ 0.0f, 0.0f };
    Vector2 uv2{ 0.0f, 0.0f };

    Vector2 pt0{ mid_left, mid_top };

    if (percentage < 25) {
        // Left 2 Boxes
        RectClass left_rect = { { left, top }, { mid_left, bottom } };
        m_2DRender->Add_Rect(left_rect, 0.0f, 0, color);

        // Right Bottom Box
        RectClass right_bottom_rect = { { mid_left, mid_top }, { right, bottom } };
        m_2DRender->Add_Rect(right_bottom_rect, 0.0f, 0, color);

        // At 13, clock points at corner of Right Top of box
        if (percentage < 13) {
            // Right Top Static Wedge
            {
                Vector2 pt1{ right, mid_top };
                Vector2 pt2{ right, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Top Moving Wedge
            {
                float pt = right - static_cast<float>(13 - percentage) * (1.0f / 13.0f) * mid_x;
                Vector2 pt1{ right, top };
                Vector2 pt2{ pt, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Right Top Moving Wedge
            float pt = static_cast<float>(percentage - 13) * (1.0f / 12.0f) * mid_y + top;
            Vector2 pt1{ right, mid_top };
            Vector2 pt2{ right, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    } else if (percentage < 50) {
        // Left 2 Boxes
        RectClass left_rect = { { left, top }, { mid_left, bottom } };
        m_2DRender->Add_Rect(left_rect, 0.0f, 0, color);

        // At 38, clock points at corner of Right Bottom of box
        if (percentage < 38) {
            // Right Bottom Static Wedge
            {
                Vector2 pt1{ mid_left, bottom };
                Vector2 pt2{ right, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Bottom Moving Wedge
            {
                float pt = static_cast<float>(percentage - 25) * (1.0f / 13.0f) * mid_y + mid_top;
                Vector2 pt1{ right, bottom };
                Vector2 pt2{ right, pt };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Right Bottom Moving Wedge
            float pt = right - static_cast<float>(percentage - 38) * (1.0f / 12.0f) * mid_x;
            Vector2 pt1{ mid_left, bottom };
            Vector2 pt2{ pt, bottom };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    } else if (percentage < 75) {
        // Left Top Box
        RectClass left_top_rect = { { left, top }, { mid_left, mid_top } };
        m_2DRender->Add_Rect(left_top_rect, 0.0f, 0, color);

        // At 63, clock points at corner of Left Bottom of box
        if (percentage < 63) {
            // Left Bottom Static Wedge
            {
                Vector2 pt1{ left, mid_top };
                Vector2 pt2{ left, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Bottom Moving Wedge
            {
                float pt = mid_left - static_cast<float>(percentage - 50) * (1.0f / 13.0f) * mid_x;
                Vector2 pt1{ left, bottom };
                Vector2 pt2{ pt, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Left Bottom Moving Wedge
            float pt = bottom - static_cast<float>(percentage - 62) * (1.0f / 12.0f) * mid_y;
            Vector2 pt1{ left, mid_top };
            Vector2 pt2{ left, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    } else {
        // At 87, clock points at corner of Left Top of box
        if (percentage < 87) {
            // Left Top Static Wedge
            {
                Vector2 pt1{ mid_left, top };
                Vector2 pt2{ left, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Top Moving Wedge
            {
                float pt = mid_top - static_cast<float>(percentage - 75) * (1.0f / 13.0f) * mid_y;
                Vector2 pt1{ left, top };
                Vector2 pt2{ left, pt };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Left Top Moving Wedge
            float pt = left + static_cast<float>(percentage - 88) * (1.0f / 12.0f) * mid_x;
            Vector2 pt1{ mid_left, top };
            Vector2 pt2{ pt, top };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    }
    m_2DRender->Render();
}

// 0x00740640
void W3DDisplay::Draw_Image(
    const Image *image, int32_t left, int32_t top, int32_t right, int32_t bottom, uint32_t color, DrawImageMode mode)
{
    if (image == nullptr) {
        return;
    }

    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(true);

    bool renable_alpha = false;

    switch (mode) {
        case Display::DRAWIMAGE_ADDITIVE_ALPHA:
            m_2DRender->Enable_Additive(false);
            m_2DRender->Enable_Alpha(false);
            renable_alpha = true;
            break;
        case Display::DRAWIMAGE_GREYSCALE:
            m_2DRender->Enable_GreyScale(true);
            break;
        case Display::DRAWIMAGE_ADDITIVE:
            break;
        default:
            m_2DRender->Enable_Additive(true);
            renable_alpha = true;
            break;
    }

    if (image->Is_Set_Status(Image::IMAGE_STATUS_RAW_TEXTURE)) {
        m_2DRender->Set_Texture(image->Get_Raw_Texture_Data());
    } else {
        auto file_name = image->Get_File_Name();
        if (file_name.Is_Empty()) {
            file_name = Utf8String::s_emptyString;
        }
        m_2DRender->Set_Texture(file_name.Str());
    }

    float f_left = static_cast<float>(left);
    float f_top = static_cast<float>(top);
    float f_right = static_cast<float>(right);
    float f_bottom = static_cast<float>(bottom);
    RectClass rect{ f_left, f_top, f_right, f_bottom };
    auto uv = image->Get_UV_Region();
    RectClass uv_rect{ uv.lo.x, uv.lo.y, uv.hi.x, uv.hi.y };
    if (m_isClippedEnabled == true) {
        if (right <= m_clipRegion.lo.x) {
            return;
        }
        if (bottom <= m_clipRegion.lo.y) {
            return;
        }

        f_left = static_cast<float>(std::max(left, m_clipRegion.lo.x));
        f_right = static_cast<float>(std::min(right, m_clipRegion.hi.x));
        f_top = static_cast<float>(std::max(top, m_clipRegion.lo.y));
        f_bottom = static_cast<float>(std::min(bottom, m_clipRegion.hi.y));

        float width = rect.Width();
        float height = rect.Height();

        float uv_width = uv_rect.Width();
        float uv_height = uv_rect.Height();

        float uv_left = uv_rect.left;
        float uv_right = uv_rect.right;
        float uv_top = uv_rect.top;

        if (image->Is_Set_Status(Image::IMAGE_STATUS_ROTATED_90_CLOCKWISE)) {
            uv_rect.top = ((f_left - rect.left) / width) * uv_height + uv_top;
            uv_rect.bottom = ((f_right - rect.left) / width) * uv_height + uv_top;
            uv_rect.right = uv_right - ((f_top - rect.top) / height) * uv_width;
            uv_rect.left = uv_right - ((f_bottom - rect.top) / height) * uv_width;
            rect.Set(f_left, f_top, f_right, f_bottom);
        } else {
            uv_rect.left = ((f_left - rect.left) / width) * uv_width + uv_left;
            uv_rect.right = ((f_right - rect.left) / width) * uv_width + uv_left;
            uv_rect.top = ((f_top - rect.top) / height) * uv_height + uv_top;
            uv_rect.bottom = ((f_bottom - rect.top) / height) * uv_height + uv_top;
            rect.Set(f_left, f_top, f_right, f_bottom);
        }
    }

    if (image->Is_Set_Status(Image::IMAGE_STATUS_ROTATED_90_CLOCKWISE)) {
        m_2DRender->Add_Tri(rect.Upper_Left(),
            rect.Lower_Left(),
            rect.Upper_Right(),
            uv_rect.Upper_Right(),
            uv_rect.Upper_Left(),
            uv_rect.Lower_Right(),
            color);
        m_2DRender->Add_Tri(rect.Lower_Right(),
            rect.Upper_Right(),
            rect.Lower_Left(),
            uv_rect.Lower_Left(),
            uv_rect.Lower_Right(),
            uv_rect.Upper_Left(),
            color);
    } else {
        m_2DRender->Add_Quad(rect, uv_rect, color);
    }

    m_2DRender->Render();
    m_2DRender->Enable_GreyScale(false);
    if (renable_alpha == true) {
        m_2DRender->Enable_Alpha(true);
    }
}

// 0x00740BF0
void W3DDisplay::Draw_VideoBuffer(VideoBuffer *vb, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    W3DVideoBuffer *buffer = static_cast<W3DVideoBuffer *>(vb);
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(true);
    m_2DRender->Set_Texture(buffer->Get_Texture());
    RectClass uv = buffer->Rect(0.0f, 0.0f, 1.0f, 1.0f);
    RectClass screen((float)x1, (float)y1, (float)x2, (float)y2);
    m_2DRender->Add_Quad(screen, uv, 0xFFFFFFFF);
    m_2DRender->Render();
}

// 0x00740CD0
void W3DDisplay::Set_Shroud_Level(int x, int y, CellShroudStatus status)
{
    if (g_theTerrainRenderObject) {
        if (g_theTerrainRenderObject->Get_Shroud()) {
            switch (status) {
                case SHROUD_STATUS_SHROUD:
                    g_theTerrainRenderObject->Get_Shroud()->Set_Shroud_Level(
                        x, y, g_theWriteableGlobalData->m_shroudAlpha, false);
                    break;
                case SHROUD_STATUS_FOG:
                    g_theTerrainRenderObject->Get_Shroud()->Set_Shroud_Level(
                        x, y, g_theWriteableGlobalData->m_fogAlpha, false);
                    break;
                default:
                    g_theTerrainRenderObject->Get_Shroud()->Set_Shroud_Level(
                        x, y, g_theWriteableGlobalData->m_clearAlpha, false);
                    break;
            }

            g_theTerrainRenderObject->Notify_Shroud_Changed();
        }
    }
}

// 0x00740CB0
void W3DDisplay::Set_Border_Shroud_Level(uint8_t level)
{
    if (g_theTerrainRenderObject) {
        if (g_theTerrainRenderObject->Get_Shroud()) {
            g_theTerrainRenderObject->Get_Shroud()->Set_Border_Shroud_Level(level);
        }
    }
}

// 0x007411F0
void W3DDisplay::Preload_Model_Assets(Utf8String model)
{
    if (s_assetManager != nullptr) {
        Utf8String filename;
        filename.Format("%s.w3d", model.Str());
        s_assetManager->Load_3D_Assets(filename.Str());
    }
}

// 0x007412D0
void W3DDisplay::Preload_Texture_Assets(Utf8String texture)
{
    if (s_assetManager != nullptr) {
        auto *tex = s_assetManager->Get_Texture(texture.Str());
        tex->Release_Ref();
    }
}

void Create_Bmp_File(const char *filename, const char *data, int width, int height)
{
#ifdef BUILD_WITH_D3D8

#define BFT_BITMAP 0x4d42 // 'BM'
#define DibNumColors(lpbi) \
    ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 ? (int)(1 << (int)(lpbi)->biBitCount) : (int)(lpbi)->biClrUsed)
#define DibSize(lpbi) ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))
#define DibPaletteSize(lpbi) (DibNumColors(lpbi) * sizeof(RGBQUAD))

    // #BUGFIX Simplify code and avoid LocalAlloc. Use the stack.
    BITMAPINFOHEADER iheader = { 0 };
    iheader.biSize = sizeof(BITMAPINFOHEADER);
    iheader.biWidth = width;
    iheader.biHeight = height;
    iheader.biPlanes = 1;
    iheader.biBitCount = 24;
    iheader.biCompression = BI_RGB;
    iheader.biSizeImage = 3 * iheader.biHeight * iheader.biWidth;
    iheader.biClrImportant = 0;
    HANDLE h =
        CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (h != INVALID_HANDLE_VALUE) {
        BITMAPFILEHEADER fheader;
        fheader.bfType = BFT_BITMAP;
        fheader.bfSize = DibSize(&iheader) + sizeof(BITMAPFILEHEADER);
        fheader.bfReserved1 = 0;
        fheader.bfReserved2 = 0;
        fheader.bfOffBits = iheader.biSize + DibPaletteSize(&iheader) + sizeof(BITMAPFILEHEADER);
        DWORD NumberOfBytesWritten;

        if (WriteFile(h, &fheader, sizeof(BITMAPFILEHEADER), &NumberOfBytesWritten, 0)) {
            if (WriteFile(h, &iheader, 4 * iheader.biClrUsed + sizeof(BITMAPINFOHEADER), &NumberOfBytesWritten, 0)) {
                WriteFile(h, data, iheader.biSizeImage, &NumberOfBytesWritten, 0);
            }
        }
        CloseHandle(h);
    }

#undef BFT_BITMAP
#undef DibNumColors
#undef DibSize
#undef DibPaletteSize

#endif
}

// 0x00740D40
void W3DDisplay::Take_ScreenShot()
{
#ifdef BUILD_WITH_D3D8
    static int currentShot = 1;
    char fname[1024];
    char buf[256];

    while (true) {
        sprintf(buf, "%s%.3d.bmp", "sshot", currentShot);
        currentShot++;
        strcpy(fname, g_theWriteableGlobalData->Get_Path_User_Data().Str());
        strcat(fname, buf);

        if (_access(fname, 0) == -1) {
            break;
        }
    }

    IDirect3DSurface8 *surface = DX8Wrapper::Get_DX8_Front_Buffer();
    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);
    RECT rect;
    POINT point;
    GetClientRect(g_applicationHWnd, &rect);

    point.x = rect.left;
    point.y = rect.top;
    ClientToScreen(g_applicationHWnd, &point);
    rect.left = point.x;
    rect.top = point.y;

    point.x = rect.right;
    point.y = rect.bottom;
    ClientToScreen(g_applicationHWnd, &point);
    rect.right = point.x;
    rect.bottom = point.y;

    D3DLOCKED_RECT locked_rect;
    DX8Wrapper::Handle_DX8_ErrorCode(surface->LockRect(&locked_rect, &rect, D3DLOCK_READONLY));

    unsigned int x = rect.right - rect.left;
    unsigned int y = rect.bottom - rect.top;
    char *data = new char[(rect.bottom - rect.top) * 3 * (rect.right - rect.left)];

    for (unsigned int i = 0; i < y; i++) {
        for (unsigned int j = 0; j < x; j++) {
            int i1 = 3 * (x * i + j);
            int i2 = locked_rect.Pitch * i + 4 * j;
            data[i1 + 0] = *(static_cast<char *>(locked_rect.pBits) + i2 + 0);
            data[i1 + 1] = *(static_cast<char *>(locked_rect.pBits) + i2 + 1);
            data[i1 + 2] = *(static_cast<char *>(locked_rect.pBits) + i2 + 2);
        }
    }

    surface->Release();

    for (unsigned int i = 0; i < y / 2; i++) {
        char *c1 = &data[3 * i * x];
        char *c2 = &data[3 * (y - 1) * x + -3 * i * x];

        for (unsigned int j = 0; j < 3 * x; j++) {
            char c3 = *c1;
            char c4 = *c2;
            *c1 = c4;
            *c2 = c3;
            ++c1;
            ++c2;
        }
    }

    Create_Bmp_File(fname, data, x, y);
    delete[] data;
    Utf8String str(buf);
    Utf16String str2;
    str2.Translate(str);
    g_theInGameUI->Message(g_theGameText->Fetch("GUI:ScreenCapture"), str2.Str());
#endif
}

// 0x007411DA
void W3DDisplay::Toggle_Movie_Capture()
{
    captainslog_dbgassert(false, "Movie capture not supported");
}

// 0x0073ED70
void W3DDisplay::Toggle_LetterBox()
{
    m_letterBoxEnabled = !m_letterBoxEnabled;
    m_letterBoxFadeStartTime = rts::Get_Time();
    auto *view = g_theTacticalView;
    if (view != nullptr) {
        view->Set_Zoom_Limited(!m_letterBoxEnabled);
    }
}

// 0x0073EDB0
void W3DDisplay::Enable_LetterBox(bool enable)
{
    m_letterBoxEnabled = enable;
    m_letterBoxFadeStartTime = rts::Get_Time();
    auto *view = g_theTacticalView;
    if (view != nullptr) {
        view->Set_Zoom_Limited(!enable);
    }
}

// 0x0073EC10
bool W3DDisplay::Is_LetterBox_Fading()
{
    if (m_letterBoxEnabled == true) {
        return m_letterBoxFadeLevel == 1.0f;
    }

    return m_letterBoxFadeLevel > 0.0f;
}

// 0x0073E350
int W3DDisplay::Get_Last_Frame_Draw_Calls()
{
    // return Debug_Statistics::Get_Draw_Calls();
    return 0;
}

void W3DDisplay::Update_Average_FPS()
{
#ifdef PLATFORM_WINDOWS
    static uint64_t lastUpdateTime64 = 0;
    static int historyOffset = 0;
    static int numSamples = 0;
    static double fpsHistory[30] = {};

    LARGE_INTEGER PerformanceFrequency;
    LARGE_INTEGER PerformanceCounter;
    QueryPerformanceFrequency(&PerformanceFrequency);
    QueryPerformanceCounter(&PerformanceCounter);

#ifdef GAME_DEBUG_STRUCTS
    if (g_theGameLogic->Get_Frame() == 15) {
        m_performanceCounter = PerformanceCounter.QuadPart;
    }
#endif

    double frameTime = (double)(PerformanceCounter.QuadPart - lastUpdateTime64) / (double)PerformanceFrequency.QuadPart;

    if (frameTime <= 0.5) {
        if (historyOffset >= 30) {
            historyOffset = 0;
        }

        fpsHistory[historyOffset] = 1.0 / frameTime;
        historyOffset++;
        numSamples++;

        if (numSamples > 30) {
            numSamples = 30;
        }
    }

    if (numSamples) {
        double average = 0.0;

        for (int i1 = 0, i2 = historyOffset - 1; i1 < numSamples; i1++, i2--) {
            if (i2 < 0) {
                i2 = 29;
            }

            average += fpsHistory[i2];
        }

        m_averageFps = average / (double)numSamples;
    }

    lastUpdateTime64 = PerformanceCounter.QuadPart;
#endif
}

void W3DDisplay::Gather_Debug_Stats()
{
#ifdef PLATFORM_WINDOWS
    static uint64_t lastUpdateTime64 = 0;
    static double frameTime = 0.0;
    static unsigned int frameCount = 0;
    static int drawCalls = 0;
    static int sortingPolygons = 0;

    if (!m_displayStrings[0]) {
        GameFont *font;

        if (g_theGlobalLanguage && g_theGlobalLanguage->Debug_Display_Font().Name().Is_Not_Empty()) {
            font = g_theFontLibrary->Get_Font(g_theGlobalLanguage->Debug_Display_Font().Name(),
                g_theGlobalLanguage->Debug_Display_Font().Point_Size(),
                g_theGlobalLanguage->Debug_Display_Font().Bold());
        } else {
            font = g_theFontLibrary->Get_Font("FixedSys", 8, false);
        }

        for (int i = 0; i < ARRAY_SIZE(m_displayStrings); i++) {
            if (!m_displayStrings[i]) {
                m_displayStrings[i] = g_theDisplayStringManager->New_Display_String();
                captainslog_dbgassert(m_displayStrings[i], "Failed to create DisplayString");
                m_displayStrings[i]->Set_Font(font);
            }
        }
    }

    if (!m_benchmarkDisplayString[0]) {
        GameFont *font = g_theFontLibrary->Get_Font("FixedSys", 8, false);
        m_benchmarkDisplayString[0] = g_theDisplayStringManager->New_Display_String();
        captainslog_dbgassert(m_benchmarkDisplayString, "Failed to create DisplayString");
        m_benchmarkDisplayString[0]->Set_Font(font);
    }

    frameCount++;
    // drawCalls += Debug_Statistics::Get_Draw_Calls()
    // sortingPolygons += Debug_Statistics::Get_Sorting_Polygons()

    LARGE_INTEGER PerformanceFrequency;
    LARGE_INTEGER PerformanceCounter;
    QueryPerformanceFrequency(&PerformanceFrequency);
    QueryPerformanceCounter(&PerformanceCounter);
    frameTime = (double)(PerformanceCounter.QuadPart - lastUpdateTime64) / (double)PerformanceFrequency.QuadPart;

    if (frameTime >= 2.0 || g_theWriteableGlobalData->m_constantDebugUpdate) {
        Utf16String str1;
        Utf16String str2;
        Utf16String str3;
        // Debug_Statistics::Record_Texture_Mode(RECORD_TEXTURE_SIMPLE);

        double fps = frameCount / frameTime;
        double draw_calls = 0;
        // draw_calls = (double)Debug_Statistics::Get_Draw_Calls();
        double sorting_polygons = 0;
        // sorting_polygons = (double)Debug_Statistics::Get_Sorting_Polygons();
        double dx8_skin_renders = 0;
        // dx8_skin_renders = (double)Debug_Statistics::Get_DX8_Skin_Renders();
        fps = std::max(0.1, fps);
        double fps2 = 1000.0 / fps;
#ifdef GAME_DEBUG_STRUCTS
        double fps3 = (double)(PerformanceCounter.QuadPart - m_performanceCounter);
        fps = std::max(0.1, fps);
        int frame = g_theGameLogic->Get_Frame() - 15;
        double fps4;

        if (frame <= 0 || fps3 <= 0.0) {
            fps4 = 0.0;
        } else {
            fps4 = (double)frame / fps3;
        }

        double dx8_skin_polygons = 0;
        // dx8_skin_polygons = (double)Debug_Statistics::Get_DX8_Skin_Polygons();

        if (g_theWriteableGlobalData->m_useFPSLimit) {
            str1.Format(L"%.2f/%d FPS, ", fps, g_theGameEngine->Get_FPS_Limit());
        } else {
            str1.Format(L"%.2f FPS, ", fps);
        }

        str2.Format(L"%.2fms [cumuFPS=%.2f] draws: %d skins: %d sortP: %d skinP: %d LOD %d",
            fps2,
            fps4,
            (unsigned int)draw_calls,
            (unsigned int)dx8_skin_renders,
            (unsigned int)sorting_polygons,
            (unsigned int)dx8_skin_polygons,
            g_theWriteableGlobalData->m_terrainLOD);
        str1.Concat(str3);
#else
        str1.Format(L"FPS: %.2f, %.2fms draws: %.2f skins: %.2f sort %.2f",
            fps,
            fps2,
            draw_calls,
            dx8_skin_renders,
            sorting_polygons);

        if (g_theWriteableGlobalData->m_useFPSLimit) {
            str2.Format(L", FPSLock %d", g_theWriteableGlobalData->m_framesPerSecondLimit);
        }

        str1.Concat(str2);
#endif

        str3.Format(L"FPS: %.2f", fps);
        m_benchmarkDisplayString[0]->Set_Text(str3);

        int dx8_polygons = 0;
        // dx8_polygons = Debug_Statistics::Get_DX8_Polygons();
        // obsolete stuff checking debug runtime registry key here
        m_displayStrings[0]->Set_Text(str1);

        str1.Format(L"Frame: %d", g_theGameLogic->Get_Frame());
        m_displayStrings[1]->Set_Text(str1);

        str1.Format(L"Polygons: per frame %d, per second %d", dx8_polygons, (unsigned int)((double)dx8_polygons * fps));
        m_displayStrings[2]->Set_Text(str1);

        int dx8_vertices = 0;
        // dx8_vertices = Debug_Statistics::Get_DX8_Vertices();
        str1.Format(L"Vertices: %d", dx8_vertices);
        m_displayStrings[3]->Set_Text(str1);

        int record_texture_size = 0;
        // record_texture_size = Debug_Statistics::Get_Record_Texture_Size();
        str1.Format(L"Video RAM: %d", record_texture_size - 0x150000);
        m_displayStrings[4]->Set_Text(str1);

        lastUpdateTime64 = PerformanceCounter.QuadPart;
        frameTime = 0.0;
        frameCount = 0;
        drawCalls = 0;
        sortingPolygons = 0;

        str1.Format(L"3-Way Blends: %d/%d, Shoreline Blends: %d/%d",
            g_theTerrainRenderObject->Get_Num_Extra_Blend_Tiles(true),
            g_theTerrainRenderObject->Get_Num_Extra_Blend_Tiles(false),
            g_theTerrainRenderObject->Get_Num_Water_Blend_Tiles(true),
            g_theTerrainRenderObject->Get_Num_Water_Blend_Tiles(false));
        m_displayStrings[15]->Set_Text(str1);

        Coord3D pos;
        g_theTacticalView->Get_Position(&pos);
        str1.Format(L"Camera zoom: %g, pitch: %g/%g, yaw: %g, pos: %g, %g, %g, FOV: %g\n"
                    L"       Height above ground: %g Terrain height: %g",
            g_theTacticalView->Get_Zoom(),
            g_theTacticalView->Get_Pitch(),
            g_theTacticalView->Get_FX_Pitch(),
            g_theTacticalView->Get_Angle(),
            pos.x,
            pos.y,
            pos.z,
            g_theTacticalView->Get_Field_Of_View(),
            g_theTacticalView->Get_Current_Height_Above_Ground(),
            g_theTacticalView->Get_Terrain_Height_Under_Camera());
        m_displayStrings[5]->Set_Text(str1);

        str1.Format(L"States: ");

        if (g_theKeyboard->Is_Shift()) {
            str1.Concat(L"Shift(");

            if ((g_theKeyboard->Get_Modifiers() & Keyboard::MODIFIER_LSHIFT) != 0) {
                str1.Concat(L"L");
            }

            if ((g_theKeyboard->Get_Modifiers() & Keyboard::MODIFIER_RSHIFT) != 0) {
                str1.Concat(L"R");
            }

            str1.Concat(L") ");
        }

        if (g_theKeyboard->Is_Ctrl()) {
            str1.Concat(L"Ctrl(");
            if ((g_theKeyboard->Get_Modifiers() & Keyboard::MODIFIER_LCTRL) != 0) {
                str1.Concat(L"L");
            }
            if ((g_theKeyboard->Get_Modifiers() & Keyboard::MODIFIER_RCTRL) != 0) {
                str1.Concat(L"R");
            }
            str1.Concat(L") ");
        }

        if (g_theKeyboard->Is_Alt()) {
            str1.Concat(L"Alt(");

            if ((g_theKeyboard->Get_Modifiers() & Keyboard::MODIFIER_LALT) != 0) {
                str1.Concat(L"L");
            }

            if ((g_theKeyboard->Get_Modifiers() & Keyboard::MODIFIER_RALT) != 0) {
                str1.Concat(L"R");
            }
            str1.Concat(L") ");
        }

        MouseIO *status = g_theMouse->Get_Mouse_Status();

        if (status->left_state) {
            str1.Concat(L"LMB ");
        }

        if (status->middle_state) {
            str1.Concat(L"MMB ");
        }

        if (status->right_state) {
            str1.Concat(L"RMB ");
        }

        const Object *object = nullptr;
#ifdef GAME_DEBUG_STRUCTS
        Drawable *drawable = g_theTacticalView->Pick_Drawable(&s_leftClickReleased, false, PICK_TYPE_UNK);
#else
        DrawableID id = g_theInGameUI->Get_Moused_Over_Drawable_ID();
        Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(id);

        if (id) {
            object = drawable->Get_Object();
        }
#endif

        if (object) {
            str2.Format(L"Moused over object: %S (%d) ", object->Get_Template()->Get_Name().Str(), object->Get_ID());
            str1.Concat(str2);
        } else {
            str1.Concat(L"Moused over object: TERRAIN ");
        }

        m_displayStrings[6]->Set_Text(str1);

        MouseIO *status2 = g_theMouse->Get_Mouse_Status();
        Coord3D pos2;
        g_theTacticalView->Screen_To_Terrain(&status2->pos, &pos2);
        str1.Format(L"Mouse position: screen: (%d, %d), world: (%g, %g, %g)",
            status2->pos.x,
            status2->pos.y,
            pos2.x,
            pos2.y,
            pos2.z);
        m_displayStrings[7]->Set_Text(str1);

        str1.Format(L"Particles: %d in world, %d being displayed",
            g_theParticleSystemManager->Get_Particle_Count(),
            g_theParticleSystemManager->Get_On_Screen_Particle_Count());
        m_displayStrings[8]->Set_Text(str1);

        str1.Format(L"Objects: %d in world, %d being displayed",
            g_theGameLogic->Get_Object_Count(),
            g_theGameClient->Get_On_Screen_Object_Count());
        m_displayStrings[9]->Set_Text(str1);

        if (g_theNetwork) {
            str1.Format(L"IN: %.2f bytes/sec, %.2f packets/sec",
                g_theNetwork->Get_Incoming_Bytes_Per_Second(),
                g_theNetwork->Get_Incoming_Packets_Per_Second());
            m_displayStrings[10]->Set_Text(str1);

            str1.Format(L"OUT: %.2f bytes/sec, %.2f packets/sec",
                g_theNetwork->Get_Outgoing_Bytes_Per_Second(),
                g_theNetwork->Get_Outgoing_Packets_Per_Second());
            m_displayStrings[11]->Set_Text(str1);

            str1.Format(L"Run Ahead: %d, Net FPS: %d, Packet arrival cushion: %d",
                g_theNetwork->Get_Run_Ahead(),
                g_theNetwork->Get_Frame_Rate(),
                g_theNetwork->Get_Packet_Arrival_Cushion());
            m_displayStrings[12]->Set_Text(str1);

            str1.Clear();
            int count = g_theNetwork->Get_Num_Players();

            for (int i = 0; i < count; i++) {
                Utf16String str4;
                str4.Format(L"%s: %d ", g_theNetwork->Get_Player_Name(i).Str(), g_theNetwork->Get_Slot_Average_FPS(i));
                str1.Concat(str4);
            }

            m_displayStrings[13]->Set_Text(str1);
        } else {
            str1.Format(L"");
            m_displayStrings[11]->Set_Text(str1);
            m_displayStrings[10]->Set_Text(str1);
            m_displayStrings[12]->Set_Text(str1);
            m_displayStrings[13]->Set_Text(str1);
        }

        int count = g_theInGameUI->Get_Select_Count();
        str1.Format(L"Select Info: '%d' drawables selected", count);

        if (g_theInGameUI->Get_Select_Count() == 1) {
            drawable = g_theInGameUI->Get_First_Selected_Drawable();
        }

        if (drawable) {
            const Object *drawable_object = drawable->Get_Object();
            Utf8String str4;
            str4.Set("No-Name");

            if (drawable_object) {
                Utf8String name = drawable_object->Get_Name();

                if (!name.Is_Empty()) {
                    str4 = drawable_object->Get_Name();
                }
            }

            str1.Format(L"Select Info: '%S'(%S) at (%.3f,%.3f,%.3f)",
                drawable->Get_Template()->Get_Name().Str(),
                str4.Str(),
                drawable->Get_Position()->x,
                drawable->Get_Position()->y,
                drawable->Get_Position()->z);

            // #BUGFIX Test object before getting physics.
            const PhysicsBehavior *phys = drawable_object ? drawable_object->Get_Physics() : nullptr;

            PhysicsTurningType turn;
            if (phys) {
                turn = phys->Get_Turning();
            } else {
                turn = TURN_NONE;
            }

            DrawableLocoInfo *info = drawable->Get_Loco_Info();
            if (info) {
                str2.Format(L"\nPhysics Info -- Turn: %d, Pitch(accel): %.3f(%.3f), Roll(accel): %.3f(%.3f)",
                    turn,
                    info->m_accelerationPitch,
                    info->m_accelerationPitchRate,
                    info->m_accelerationRoll,
                    info->m_accelerationRollRate);
                str1.Concat(str2);
            }

#ifdef GAME_DEBUG_STRUCTS
            DebugDrawStats stats;

            for (DrawModule **m = drawable->Get_Draw_Modules(); *m; m++) {
                (*m)->Gather_Draw_Stats(&stats);
            }

            if (stats.Get_Draw_Calls() > 0) {
                str2.Format(L"\ndraw calls: %d(+%d) sort meshes: %d skins: %d  bones: %d",
                    stats.Get_Draw_Calls(),
                    stats.Get_Extra_Draw_Calls(),
                    stats.Get_Sort_Meshes(),
                    stats.Get_Skins(),
                    stats.Get_Bones());
                str1.Concat(str2);
            }
#endif

            const BitFlags<MODELCONDITION_COUNT> &flags = drawable->Get_Condition_State();
            int newline = 0;

            for (int i = 0; i < MODELCONDITION_COUNT; i++) {
                if (flags.Test(i)) {
                    str2.Format(L"%S ", BitFlags<MODELCONDITION_COUNT>::Get_Bit_Names_List()[i]);
                    str1.Concat(str2);
                    newline++;

                    if (newline == 4) {
                        newline = 0;
                        str1.Concat(L"\n");
                    }
                }
            }
        }

        m_displayStrings[14]->Set_Text(str1);
    }
#endif
}

void W3DDisplay::Draw_Debug_Stats()
{
    int y_pos = 3;

    for (int i = 0; i < ARRAY_SIZE(m_displayStrings); i++) {
        m_displayStrings[i]->Draw(3, y_pos, Make_Color(0xFF, 0xFF, 0xFF, 0xFF), Make_Color(0, 0, 0, 0xFF));
        int x;
        int y;
        m_displayStrings[i]->Get_Size(&x, &y);
        y_pos += y;
    }
}

void W3DDisplay::Draw_Benchmark()
{
    for (int i = 0; i < ARRAY_SIZE(m_benchmarkDisplayString); i++) {
        m_benchmarkDisplayString[i]->Draw(3, 20, Make_Color(0xFF, 0xFF, 0xFF, 0xFF), Make_Color(0, 0, 0, 0xFF));
    }
}

void W3DDisplay::Draw_Current_Debug_Display()
{
    if (m_debugDisplayCallback == Stat_Debug_Display) {
        Draw_Debug_Stats();
    } else if (m_debugDisplay != nullptr) {
        if (m_debugDisplayCallback) {
            m_debugDisplay->Reset();
            m_debugDisplayCallback(m_debugDisplay, m_debugDisplayUserData, nullptr);
        }
    }
}

void W3DDisplay::Calculate_Terrain_LOD()
{
#ifdef PLATFORM_WINDOWS
    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);
    float frame_time = 0.0f;
    float max_time_limit = (float)g_theWriteableGlobalData->m_terrainLODTargetTimeMS / 1000.0f;
    TerrainLOD good_lod = TERRAIN_LOD_MIN;
    TerrainLOD lod = TERRAIN_LOD_AUTOMATIC;
    int count = 0;

    do {
        float time_for_frame = 0.0f;
        frame_time = 0.0f;

        switch (lod) {
            case TERRAIN_LOD_HALF_CLOUDS:
                lod = TERRAIN_LOD_DISABLE;
                break;
            case TERRAIN_LOD_NO_WATER:
                lod = TERRAIN_LOD_HALF_CLOUDS;
                break;
            case TERRAIN_LOD_MAX:
                lod = TERRAIN_LOD_NO_WATER;
                break;
            case TERRAIN_LOD_AUTOMATIC:
                lod = TERRAIN_LOD_MAX;
                break;
            default:
                lod = TERRAIN_LOD_DISABLE;
                break;
        }

        if (lod == TERRAIN_LOD_DISABLE) {
            break;
        }

        g_theWriteableGlobalData->m_terrainLOD = lod;
        s_3DScene->Draw_Terrain_Only(true);
        g_theTerrainRenderObject->Adjust_Terrain_LOD(0);
        int i;
        char buf[260];

        for (i = 0; i < 20; i++) {
            LARGE_INTEGER PerformanceCounter;
            QueryPerformanceCounter(&PerformanceCounter);
            Update_Views();

            if (!W3D::Begin_Render(true)) {
                Draw_Views();
                W3D::End_Render();
            }

            LARGE_INTEGER PerformanceCounter2;
            QueryPerformanceCounter(&PerformanceCounter2);
            time_for_frame =
                (double)(PerformanceCounter2.QuadPart - PerformanceCounter.QuadPart) / (double)PerformanceFrequency.QuadPart;

            sprintf(buf, "%.2fms ", time_for_frame * 1000.0f);
            OutputDebugString(buf);

            if (i > 4) {
                frame_time += time_for_frame;
                if (i > 6 && (max_time_limit + max_time_limit) < time_for_frame / (float)(i - 4)) {
                    i++;
                    break;
                }
            }
        }

        frame_time = frame_time / (float)(i - 5);
        count++;
        sprintf(buf, "\n LOD %d, time %.2fms\n", lod, frame_time * 1000.0f);
        OutputDebugString(buf);

        if (frame_time < (float)max_time_limit && good_lod < lod) {
            good_lod = lod;
        }

    } while (frame_time >= (float)max_time_limit && count < 10);

    g_theWriteableGlobalData->m_terrainLOD = good_lod;
    s_3DScene->Draw_Terrain_Only(false);
    g_theTerrainRenderObject->Adjust_Terrain_LOD(0);
#endif
}

void W3DDisplay::Render_LetterBox(unsigned int current_time)
{
    if (m_letterBoxEnabled) {
        if (m_letterBoxFadeLevel != 1.0f) {
            m_letterBoxFadeLevel = (float)(current_time - m_letterBoxFadeStartTime) / 1000.0f;

            if (m_letterBoxFadeLevel > 1.0f) {
                m_letterBoxFadeLevel = 1.0f;
            }
        }

        unsigned int color = (unsigned int)(m_letterBoxFadeLevel * 255.0f) << 24;
        Draw_Fill_Rect(0, 0, m_width, (int)(((float)m_height - (float)m_width * 0.5625f) * 0.5f), color);
        Draw_Fill_Rect(
            0, (int)((float)m_height - (((float)m_height - (float)m_width * 0.5625f) * 0.5f)), m_width, m_height, color);
    } else if (m_letterBoxFadeLevel == 0.0f) {
        m_letterBoxEnabled = false;
    } else {
        m_letterBoxFadeLevel = 1.0f - (float)(current_time - m_letterBoxFadeStartTime) / 1000.0f;

        if (m_letterBoxFadeLevel < 0.0f) {
            m_letterBoxFadeLevel = 0.0f;
        }

        Draw_Fill_Rect(0,
            0,
            m_width,
            (unsigned int)(((float)m_height - (float)m_width * 0.5625f) * 0.5f),
            (unsigned int)(m_letterBoxFadeLevel * 255.0f) << 24);
    }
}

void Reset_D3D_Device(bool restore_assets)
{
#ifdef BUILD_WITH_D3D8
    if (g_theDisplay) {
        if (W3D::Is_Initted()) {
            if (!g_theDisplay->Get_Windowed()) {
                if (restore_assets) {
                    W3D::Set_Render_Device(W3D::Get_Render_Device(),
                        g_theDisplay->Get_Width(),
                        g_theDisplay->Get_Height(),
                        g_theDisplay->Get_Bit_Depth(),
                        g_theDisplay->Get_Windowed(),
                        true,
                        true,
                        true);

#if 0 // disable some code that is impossible to be triggered for Thyme since we aren't compatible with Windows 9x
                    OSVERSIONINFOA VersionInformation;

                    if (GetVersionEx(&VersionInformation)) {
                        if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
                            W3D::Invalidate_Textures();
                        }
                    }
#endif
                } else {
                    W3D::Set_Render_Device(W3D::Get_Render_Device(),
                        g_theDisplay->Get_Width(),
                        g_theDisplay->Get_Height(),
                        g_theDisplay->Get_Bit_Depth(),
                        g_theDisplay->Get_Windowed(),
                        true,
                        true,
                        false);
                }
            }
        }
    }
#endif
}

#ifdef GAME_DEBUG_STRUCTS
void W3DDisplay::Write_Asset_Usage(const char *str)
{
#ifdef BUILD_WITH_D3D8
    if (s_assetManager && str && *str) {
        DynamicVectorClass<StringClass> list(8000, nullptr);
        s_assetManager->Create_Asset_List(list);

        const char *c = strrchr(str, '\\');

        if (c) {
            c = c + 1;
        } else {
            c = str;
        }

        char buf[256];
        for (int i = 1;; i++) {
            sprintf(buf, "AssetUsage_%s_%04d.txt", c, i);
            if (_access(buf, 0) != 0) {
                break;
            }
        }

        FILE *f = fopen(buf, "w");

        if (f) {
            for (int i = 0; i < list.Count(); i++) {
                fprintf(f, "%s\n", list[i].Peek_Buffer());
            }

            fclose(f);
        }
    }
#endif
}

static FILE *s_modelUsageFile;

void Write_Mesh_Model_And_Texture_Usage(MeshClass *mesh)
{
    if (mesh) {
        MeshModelClass *model = mesh->Get_Model();

        for (int stage = 0; stage < 2; stage++) {
            for (int pass = 0; pass < model->Material_Pass_Count(); pass++) {
                if (model->Has_Texture_Array(pass, stage)) {
                    for (int poly = 0; poly < model->Get_Polygon_Count(); poly++) {
                        TextureClass *t = model->Peek_Texture(poly, pass, stage);

                        if (t) {
                            fprintf(s_modelUsageFile, "\t%s\n", t->Get_Name().Peek_Buffer());
                        }
                    }
                } else {
                    TextureClass *t = model->Peek_Single_Texture(pass, stage);

                    if (t) {
                        fprintf(s_modelUsageFile, "\t%s\n", t->Get_Name().Peek_Buffer());
                    }
                }
            }
        }
    }
}

void Write_Subobject_Model_And_Texture_Usage(HLodClass *hlod)
{
    if (hlod) {
        for (int i = 0; i < hlod->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *robj = hlod->Get_Sub_Object(i);

            if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                Write_Subobject_Model_And_Texture_Usage(static_cast<HLodClass *>(robj));
            } else if (robj->Class_ID() == RenderObjClass::CLASSID_MESH) {
                Write_Mesh_Model_And_Texture_Usage(static_cast<MeshClass *>(robj));
            }
        }
    }
}

void W3DDisplay::Write_Model_And_Texture_Usage(const char *filename)
{
    if (s_3DScene) {
        s_modelUsageFile = fopen(filename, "w");

        if (s_modelUsageFile) {
            fprintf(s_modelUsageFile, "Models and Textures used on %s:\n\n", g_theWriteableGlobalData->m_mapName.Str());
            SceneIterator *iter = s_3DScene->Create_Iterator(false);
            iter->First();

            while (!iter->Is_Done()) {
                RenderObjClass *robj = iter->Current_Item();

                if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                    Write_Subobject_Model_And_Texture_Usage(static_cast<HLodClass *>(robj));
                } else if (robj->Class_ID() == RenderObjClass::CLASSID_MESH) {
                    fprintf(s_modelUsageFile, "%s.W3D:\n", robj->Get_Name());
                    Write_Mesh_Model_And_Texture_Usage(static_cast<MeshClass *>(robj));
                }

                iter->Next();
            }

            s_3DScene->Destroy_Iterator(iter);
            fclose(s_modelUsageFile);
        }
    }
}
#endif
