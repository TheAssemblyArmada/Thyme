/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dcontrolbar.h"
#include "controlbar.h"
#include "controlbarscheme.h"
#include "display.h"
#include "gamewindowmanager.h"
#include "globaldata.h"
#include "ingameui.h"
#include "maputil.h"
#include "player.h"
#include "playerlist.h"
#include "radar.h"
#include "skirmishgameoptionsmenu.h"
#include "w3dgamewindow.h"

void W3D_Cameo_Movie_Draw(GameWindow *window, WinInstanceData *instance)
{
    VideoBuffer *buffer = g_theInGameUI->Cameo_Video_Buffer();

    if (buffer != nullptr) {
        int screen_x;
        int screen_y;
        int width;
        int height;
        window->Win_Get_Screen_Position(&screen_x, &screen_y);
        window->Win_Get_Size(&width, &height);
        g_theDisplay->Draw_VideoBuffer(buffer, screen_x, screen_y, screen_x + width, screen_y + height);
    }
}

void W3D_Left_HUD_Draw(GameWindow *window, WinInstanceData *instance)
{
    Player *player = g_thePlayerList->Get_Local_Player();
    VideoBuffer *buffer = g_theInGameUI->Video_Buffer();

    if (buffer != nullptr) {
        int screen_x;
        int screen_y;
        int width;
        int height;
        window->Win_Get_Screen_Position(&screen_x, &screen_y);
        window->Win_Get_Size(&width, &height);
        g_theDisplay->Draw_VideoBuffer(buffer, screen_x, screen_y, screen_x + width, screen_y + height);
    } else if (g_theRadar->Is_Radar_Forced() || (!g_theRadar->Is_Radar_Hidden() && player->Has_Radar())) {
        int screen_x;
        int screen_y;
        int width;
        int height;
        window->Win_Get_Screen_Position(&screen_x, &screen_y);
        window->Win_Get_Size(&width, &height);
        g_theRadar->Draw(screen_x + 1, screen_y + 1, width - 2, height - 2);
    }
}

void W3D_Right_HUD_Draw(GameWindow *window, WinInstanceData *instance)
{
    if ((window->Win_Get_Status() & WIN_STATUS_IMAGE) != 0) {
        W3D_Game_Win_Default_Draw(window, instance);
    }
}

float Log_N(float f1, float f2)
{
    return GameMath::Log10(f1) / GameMath::Log10(f2);
}

void W3D_Power_Draw(GameWindow *window, WinInstanceData *instance)
{
    static const Image *centerBarYellow = g_theMappedImageCollection->Find_Image_By_Name("PowerPointY");
    static const Image *centerBarRed = g_theMappedImageCollection->Find_Image_By_Name("PowerPointR");
    static const Image *centerBarGreen = g_theMappedImageCollection->Find_Image_By_Name("PowerPointG");
    static const Image *slider = g_theMappedImageCollection->Find_Image_By_Name("PowerBarSlider");
    Player *player;

    if (g_theControlBar->Is_Observer()) {
        player = g_theControlBar->Get_Observer_Player();
    } else {
        player = g_thePlayerList->Get_Local_Player();
    }

    if (player != nullptr) {
        if (g_theWriteableGlobalData != nullptr) {
            Energy *energy = player->Get_Energy();

            if (energy != nullptr) {
                int consumption = energy->Get_Consumption();
                int production = energy->Get_Production();
                int screen_x;
                int screen_y;
                int width;
                int height;
                window->Win_Get_Screen_Position(&screen_x, &screen_y);
                window->Win_Get_Size(&width, &height);
                static float pixelsPerInterval = width / g_theWriteableGlobalData->m_powerBarIntervals;
                const Image *bar;

                if (consumption <= production - g_theWriteableGlobalData->m_powerBarYellowRange) {
                    bar = centerBarGreen;
                } else if (consumption > production) {
                    bar = centerBarRed;
                } else {
                    bar = centerBarYellow;
                }

                if (slider != nullptr && bar != nullptr) {
                    int production_width = Log_N(production, g_theWriteableGlobalData->m_powerBarBase)
                        * (width / g_theWriteableGlobalData->m_powerBarIntervals);

                    if (production_width >= width) {
                        production_width = width;
                    }

                    if (production_width > 0) {
                        int image_width = bar->Get_Image_Width();
                        int start_x = screen_x;
                        int end_y = screen_y + height;
                        int count = production_width / image_width;

                        if (count > 0) {
                            for (int i = 0; i != count; i++) {
                                g_theWindowManager->Win_Draw_Image(
                                    bar, start_x, screen_y, image_width + start_x, end_y, 0xFFFFFFFF);
                                image_width = bar->Get_Image_Width();
                                start_x += image_width;
                            }
                        }

                        IRegion2D clip_region;
                        clip_region.hi.x = width + screen_x;
                        clip_region.hi.y = screen_y + height;
                        clip_region.lo.x = start_x;
                        clip_region.lo.y = screen_y;
                        production_width = width + screen_x - start_x;

                        if (production_width > 0) {
                            g_theDisplay->Set_Clip_Region(&clip_region);
                            g_theWindowManager->Win_Draw_Image(
                                bar, start_x, screen_y, start_x + bar->Get_Image_Width(), end_y, 0xFFFFFFFF);
                            g_theDisplay->Enable_Clipping(false);
                        }
                    }

                    float f;

                    if (consumption == 1) {
                        f = 1.5f;
                    } else {
                        f = consumption;
                    }

                    int consumption_width = Log_N(f, g_theWriteableGlobalData->m_powerBarBase)
                        * (width / g_theWriteableGlobalData->m_powerBarIntervals);

                    if (production_width > 0 || consumption_width > 0) {
                        int image_width = slider->Get_Image_Width();
                        int start_x;
                        int end_x;

                        if (consumption_width < width) {
                            start_x = consumption_width + screen_x - image_width / 2;
                            end_x = consumption_width + screen_x + image_width / 2;
                        } else {
                            start_x = width + screen_x - image_width;
                            end_x = screen_x + width;
                        }

                        if (start_x <= screen_x) {
                            start_x = screen_x;
                            end_x = image_width + screen_x;
                        }

                        g_theWindowManager->Win_Draw_Image(slider,
                            start_x,
                            screen_y + height - slider->Get_Image_Height(),
                            end_x,
                            height + screen_y,
                            0xFFFFFFFF);
                    }
                }
            }
        }
    }
}

void W3D_Command_Bar_Grid_Draw(GameWindow *window, WinInstanceData *instance)
{
    if ((window->Win_Get_Status() & WIN_STATUS_IMAGE) == 0) {
        int screen_x;
        int screen_y;
        int width;
        int height;
        window->Win_Get_Screen_Position(&screen_x, &screen_y);
        window->Win_Get_Size(&width, &height);
        int color = g_theControlBar->Get_Command_Bar_Border_Color();
        window->Win_Set_Enabled_Border_Color(0, color);
        W3D_Game_Win_Default_Draw(window, instance);
        int i = height * 0.33f + screen_y;
        g_theDisplay->Draw_Line(screen_x, i, screen_x + width, i, 1.0f, color);
        i = height * 0.66f + screen_y;
        g_theDisplay->Draw_Line(screen_x, i, screen_x + width, i, 1.0f, color);
        i = width * 0.33f + screen_x;
        g_theDisplay->Draw_Line(i, screen_y, i, screen_y + height, 1.0f, color);
        i = width * 0.66f + screen_x;
        g_theDisplay->Draw_Line(i, screen_y, i, screen_y + height, 1.0f, color);
    } else {
        W3D_Game_Win_Default_Draw(window, instance);
    }
}

void W3D_Command_Bar_Gen_Exp_Draw(GameWindow *window, WinInstanceData *instance)
{
    Player *player = g_thePlayerList->Get_Local_Player();

    if (player->Is_Player_Active()) {
        static const Image *endBar = g_theMappedImageCollection->Find_Image_By_Name("GenExpBarTop1");
        static const Image *beginBar = g_theMappedImageCollection->Find_Image_By_Name("GenExpBarBottom1");
        static const Image *centerBar = g_theMappedImageCollection->Find_Image_By_Name("GenExpBar1");
        int progress = 100 * (player->Get_Current_Skill_Points() - player->Get_Rank_Progress())
            / (player->Get_Skill_Points_Needed_For_Next_Rank() - player->Get_Rank_Progress());
        int progress_clamped = progress;

        if (progress > 0) {
            if (progress > 100) {
                progress_clamped = 100;
            }

            int screen_x;
            int screen_y;
            int width;
            int height;
            window->Win_Get_Screen_Position(&screen_x, &screen_y);
            window->Win_Get_Size(&width, &height);

            if (endBar != nullptr && beginBar != nullptr && centerBar != nullptr) {
                int begin_bar_height = beginBar->Get_Image_Height();
                int i3 = progress_clamped * height;
                int end_bar_height = endBar->Get_Image_Height();
                int progess_bar_right = screen_x + width;
                int progress_bar_lower = screen_y + height - begin_bar_height;
                int progress_bar_h = i3 / 100;
                int progess_bar_upper = screen_y + height - end_bar_height - progress_bar_h;

                if (progress_bar_lower - progess_bar_upper > 0) {
                    int center_bar_height = centerBar->Get_Image_Height();

                    if ((progress_bar_lower - progess_bar_upper) / center_bar_height > 0) {
                        int count = (progress_bar_lower - progess_bar_upper) / center_bar_height;

                        do {
                            g_theWindowManager->Win_Draw_Image(centerBar,
                                screen_x,
                                progess_bar_upper,
                                progess_bar_right,
                                progess_bar_upper + center_bar_height,
                                0xFFFFFFFF);
                            center_bar_height = centerBar->Get_Image_Height();
                            progess_bar_upper += center_bar_height;
                            count--;
                        } while (count != 0);
                    }

                    IRegion2D clip_region;
                    clip_region.lo.x = screen_x;
                    clip_region.lo.y = progess_bar_upper;
                    clip_region.hi.x = progess_bar_right;
                    clip_region.hi.y = progress_bar_lower;

                    if (progress_bar_lower - progess_bar_upper > 0) {
                        g_theDisplay->Set_Clip_Region(&clip_region);
                        g_theWindowManager->Win_Draw_Image(centerBar,
                            screen_x,
                            progess_bar_upper,
                            progess_bar_right,
                            progess_bar_upper + centerBar->Get_Image_Height(),
                            0xFFFFFFFF);
                        g_theDisplay->Enable_Clipping(false);
                    }

                    g_theWindowManager->Win_Draw_Image(
                        beginBar, screen_x, progress_bar_lower, screen_x + width, screen_y + height, 0xFFFFFFFF);
                    g_theWindowManager->Win_Draw_Image(endBar,
                        screen_x,
                        screen_y + height - progress_bar_h,
                        screen_x + width,
                        screen_y + height - end_bar_height - progress_bar_h,
                        0xFFFFFFFF);
                } else {
                    g_theWindowManager->Win_Draw_Image(
                        beginBar, screen_x, progress_bar_lower, progess_bar_right, screen_y + height, 0xFFFFFFFF);
                    g_theWindowManager->Win_Draw_Image(endBar,
                        screen_x,
                        screen_y + height - end_bar_height - begin_bar_height,
                        width + screen_x,
                        screen_y + height - begin_bar_height,
                        0xFFFFFFFF);
                }
            }
        }
    }
}

void W3D_Command_Bar_Top_Draw(GameWindow *window, WinInstanceData *instance)
{
    GameWindow *win = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonGeneral"));

    if (win != nullptr) {
        if (!win->Win_Is_Hidden()) {
            g_thePlayerList->Get_Local_Player()->Is_Player_Active();
        }
    }
}

void W3D_Command_Bar_Background_Draw(GameWindow *window, WinInstanceData *instance)
{
    ControlBarSchemeManager *manager = g_theControlBar->Get_Control_Bar_Scheme_Manager();

    if (manager != nullptr) {
        static const NameKeyType winNamekey = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:BackgroundMarker");
        GameWindow *win = g_theWindowManager->Win_Get_Window_From_Id(nullptr, winNamekey);

        if (win != nullptr) {
            static ICoord2D basePos;
            g_theControlBar->Get_Background_Marker_Pos(&basePos.x, &basePos.y);
            int screen_x;
            int screen_y;
            win->Win_Get_Screen_Position(&screen_x, &screen_y);
            ICoord2D offset;
            offset.y = screen_y - basePos.y;
            offset.x = screen_x - basePos.x;
            manager->Draw_Background(offset);
        }
    }
}

void W3D_Command_Bar_Foreground_Draw(GameWindow *window, WinInstanceData *instance)
{
    ControlBarSchemeManager *manager = g_theControlBar->Get_Control_Bar_Scheme_Manager();

    if (manager != nullptr) {
        static const NameKeyType winNamekey = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:BackgroundMarker");
        GameWindow *win = g_theWindowManager->Win_Get_Window_From_Id(nullptr, winNamekey);

        if (win != nullptr) {
            static ICoord2D basePos;
            g_theControlBar->Get_Foreground_Marker_Pos(&basePos.x, &basePos.y);
            int screen_x;
            int screen_y;
            win->Win_Get_Screen_Position(&screen_x, &screen_y);
            ICoord2D offset;
            offset.y = screen_y - basePos.y;
            offset.x = screen_x - basePos.x;
            manager->Draw_Foreground(offset);
        }
    }
}

void Draw_Skinny_Border(int x, int y, int width, int height)
{
    int i1 = x + width;
    int i2 = y + height;
    int i3 = y + height - 5;
    int i4 = x + width - 10;
    Image *frame_t = g_theMappedImageCollection->Find_Image_By_Name("FrameT");
    Image *frame_b = g_theMappedImageCollection->Find_Image_By_Name("FrameB");
    int i;

    for (i = x + 3; i <= i4; i += 5) {
        g_theDisplay->Draw_Image(frame_t, i, y - 2, i + 5, y + 3, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(frame_b, i, i3, i + 5, i3 + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    }

    int i5 = i1 - 5;

    if (i1 - 5 - i >= 2) {
        g_theDisplay->Draw_Image(frame_t, i, y - 2, i + 2, y + 3, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(frame_b, i, i3, i + 2, i3 + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        i += 2;
    }

    if (i < i5) {
        int i6 = i - (2 - ((i5 - i + 1) & 0xFFFFFFFE));
        g_theDisplay->Draw_Image(frame_t, i6, y - 2, i6 + 2, y + 3, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(frame_b, i6, i3, i6 + 2, i3 + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    }

    Image *frame_l = g_theMappedImageCollection->Find_Image_By_Name("FrameL");
    Image *frame_r = g_theMappedImageCollection->Find_Image_By_Name("FrameR");
    int i7 = x - 2;
    int j;

    for (j = y + 3; j <= i2 - 10; j += 5) {
        g_theDisplay->Draw_Image(frame_l, x - 2, j, i7 + 5, j + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(frame_r, i1 - 5, j, i1, j + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    }

    int i8 = i2 - 5;

    if (i2 - 5 - j >= 2) {
        g_theDisplay->Draw_Image(frame_l, x - 2, j, i7 + 5, j + 2, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(frame_r, i1 - 5, j, i1, j + 2, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        j += 2;
    }

    if (j < i8) {
        int i9 = j - (2 - ((i8 - j + 1) & 0xFFFFFFFE));
        g_theDisplay->Draw_Image(frame_l, x - 2, i9, i7 + 5, i9 + 2, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(frame_r, i1 - 5, i9, i1, i9 + 2, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    }

    Image *frame_corner_ul = g_theMappedImageCollection->Find_Image_By_Name("FrameCornerUL");
    g_theDisplay->Draw_Image(frame_corner_ul, x - 2, y - 2, x - 2 + 5, y - 2 + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    Image *frame_corner_ur = g_theMappedImageCollection->Find_Image_By_Name("FrameCornerUR");
    g_theDisplay->Draw_Image(frame_corner_ur, i1 - 5, y - 2, i1, y - 2 + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    Image *frame_corner_ll = g_theMappedImageCollection->Find_Image_By_Name("FrameCornerLL");
    g_theDisplay->Draw_Image(frame_corner_ll, x - 2, i2 - 5, x - 2 + 5, i2, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    Image *frame_corner_lr = g_theMappedImageCollection->Find_Image_By_Name("FrameCornerLR");
    g_theDisplay->Draw_Image(frame_corner_lr, i1 - 5, i2 - 5, i1, i2, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
}

void W3D_Draw_Map_Preview(GameWindow *window, WinInstanceData *instance)
{
    MapMetaData *data = static_cast<MapMetaData *>(window->Win_Get_User_Data());
    int screen_x;
    int screen_y;
    int width;
    int height;
    window->Win_Get_Screen_Position(&screen_x, &screen_y);
    window->Win_Get_Size(&width, &height);

    if (data != nullptr) {
        ICoord2D ul;
        ICoord2D lr;
        Find_Draw_Positions(screen_x, screen_y, width, height, data->m_extent, &ul, &lr);

        if ((data->m_extent.hi.y - data->m_extent.lo.y) / height > (data->m_extent.hi.x - data->m_extent.lo.x) / width) {
            g_theDisplay->Draw_Fill_Rect(screen_x, screen_y, ul.x - screen_x - 1, height, 0xFF000000);
            g_theDisplay->Draw_Fill_Rect(lr.x + 1, screen_y, width - lr.x + screen_x - 1, height, 0xFF000000);
            g_theDisplay->Draw_Line(ul.x, screen_y, ul.x, screen_y + height, 1.0f, 0xFF323232);
            g_theDisplay->Draw_Line(lr.x + 1, screen_y, lr.x + 1, screen_y + height, 1.0f, 0xFF323232);
        } else {
            g_theDisplay->Draw_Fill_Rect(screen_x, screen_y, width, ul.y - screen_y - 1, 0xFF000000);
            g_theDisplay->Draw_Fill_Rect(screen_x, lr.y + 1, width, screen_y + height - lr.y - 1, 0xFF000000);
            g_theDisplay->Draw_Line(screen_x, ul.y, screen_x + width, ul.y, 1.0f, 0xFF323232);
            g_theDisplay->Draw_Line(screen_x, lr.y + 1, screen_x + width, lr.y + 1, 1.0f, 0xFF323232);
        }

        if ((window->Win_Get_Status() & WIN_STATUS_IMAGE) != 0
            && window->Win_Get_Instance_Data()->m_enabledDrawData[0].image != nullptr) {
            g_theDisplay->Draw_Image(window->Win_Get_Instance_Data()->m_enabledDrawData[0].image,
                ul.x,
                ul.y,
                lr.x,
                lr.y,
                0xFFFFFFFF,
                Display::DRAWIMAGE_ADDITIVE);
        } else {
            g_theDisplay->Draw_Fill_Rect(ul.x, ul.y, lr.x - ul.x, lr.y - ul.y, 0xFF323232);
        }

        Image *tech_building = g_theMappedImageCollection->Find_Image_By_Name("TecBuilding");

        for (auto it = g_theSupplyAndTechImageLocations.m_techPositions.begin();
             it != g_theSupplyAndTechImageLocations.m_techPositions.end();
             it++) {
            g_theDisplay->Draw_Image(tech_building,
                screen_x + it->x,
                screen_y + it->y,
                screen_x + it->x + 15,
                screen_y + it->y + 15,
                0xFFFFFFFF,
                Display::DRAWIMAGE_ADDITIVE);
        }

        Image *cash = g_theMappedImageCollection->Find_Image_By_Name("Cash");

        for (auto it = g_theSupplyAndTechImageLocations.m_supplyPositions.begin();
             it != g_theSupplyAndTechImageLocations.m_supplyPositions.end();
             it++) {
            g_theDisplay->Draw_Image(cash,
                screen_x + it->x,
                screen_y + it->y,
                screen_x + it->x + 15,
                screen_y + it->y + 15,
                0xFFFFFFFF,
                Display::DRAWIMAGE_ADDITIVE);
        }

        Draw_Skinny_Border(screen_x - 1, screen_y - 1, width + 2, height + 2);
    } else {
        W3D_Game_Win_Default_Draw(window, instance);
        Draw_Skinny_Border(screen_x - 1, screen_y - 1, width + 2, height + 2);
    }
}

void W3D_Command_Bar_Help_Popup_Draw(GameWindow *window, WinInstanceData *instance)
{
    static const Image *endBar = g_theMappedImageCollection->Find_Image_By_Name("Helpbox-top");
    static const Image *beginBar = g_theMappedImageCollection->Find_Image_By_Name("Helpbox-bottom");
    static const Image *centerBar = g_theMappedImageCollection->Find_Image_By_Name("Helpbox-middle");
    int screen_x;
    int screen_y;
    int width;
    int height;
    window->Win_Get_Screen_Position(&screen_x, &screen_y);
    window->Win_Get_Size(&width, &height);

    if (endBar != nullptr && beginBar != nullptr && centerBar != nullptr) {
        int begin_bar_height = beginBar->Get_Image_Height();
        int end_bar_height = endBar->Get_Image_Height();
        int i1 = height - end_bar_height - begin_bar_height;

        if (i1 > 0) {
            int center_bar_height = centerBar->Get_Image_Height();
            int i2 = screen_y + end_bar_height;
            int i3 = screen_x + width;
            int i4 = i1 / center_bar_height;
            if (i4 > 0) {
                int i5 = i4;
                do {
                    g_theWindowManager->Win_Draw_Image(centerBar, screen_x, i2, i3, i2 + center_bar_height, 0xFFFFFFFF);
                    i2 += center_bar_height;
                    i5--;
                } while (i5 != 0);
            }

            IRegion2D clip_region;
            clip_region.lo.x = screen_x;
            clip_region.hi.x = screen_x + width;
            clip_region.hi.y = screen_y + height - begin_bar_height;
            clip_region.lo.y = i2;

            if (screen_y + height - i2 - begin_bar_height > 0) {
                g_theDisplay->Set_Clip_Region(&clip_region);
                g_theWindowManager->Win_Draw_Image(
                    centerBar, screen_x, i2, i3, i2 + centerBar->Get_Image_Height(), 0xFFFFFFFF);
                g_theDisplay->Enable_Clipping(false);
            }

            g_theWindowManager->Win_Draw_Image(
                beginBar, screen_x, screen_y + height - begin_bar_height, screen_x + width, height + screen_y, 0xFFFFFFFF);
            g_theWindowManager->Win_Draw_Image(
                endBar, screen_x, screen_y, screen_x + width, screen_y + end_bar_height, 0xFFFFFFFF);
        } else {
            g_theWindowManager->Win_Draw_Image(
                beginBar, screen_x, screen_y + height - begin_bar_height, screen_x + width, screen_y + height, 0xFFFFFFFF);
            g_theWindowManager->Win_Draw_Image(endBar,
                screen_x,
                screen_y + height - end_bar_height - begin_bar_height,
                width + screen_x,
                screen_y + height - begin_bar_height,
                0xFFFFFFFF);
        }
    }
}

void W3D_No_Draw(GameWindow *window, WinInstanceData *instance) {}
