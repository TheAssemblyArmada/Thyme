/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Progress Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dprogressbar.h"
#include "display.h"
#include "gadgetprogressbar.h"
#include "gamewindowmanager.h"
#include "image.h"

void W3D_Gadget_Progress_Bar_Draw(GameWindow *progress_bar, WinInstanceData *data)
{
    unsigned int progress = 0;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    progress = reinterpret_cast<unsigned int>(progress_bar->Win_Get_User_Data());
#endif

    int screen_x;
    int screen_y;
    int width;
    int height;
    progress_bar->Win_Get_Screen_Position(&screen_x, &screen_y);
    progress_bar->Win_Get_Size(&width, &height);

    int border_color;
    int color;
    int bar_border_color;
    int bar_color;

    if ((progress_bar->Win_Get_Status() & WIN_STATUS_ENABLED) != 0) {
        if ((data->m_state & 2) != 0) {
            border_color = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[0].borderColor;
            color = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[0].color;
            bar_border_color = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[4].borderColor;
            bar_color = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[4].color;
        } else {
            border_color = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[0].borderColor;
            color = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[0].color;
            bar_border_color = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[4].borderColor;
            bar_color = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[4].color;
        }
    } else {
        border_color = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[0].borderColor;
        color = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[0].color;
        bar_border_color = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[4].borderColor;
        bar_color = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[4].color;
    }

    if (border_color != 0xFFFFFF) {
        g_theWindowManager->Win_Open_Rect(border_color, 1.0f, screen_x, screen_y, width + screen_x, screen_y + height);
    }

    if (color != 0xFFFFFF) {
        g_theWindowManager->Win_Fill_Rect(
            color, 1.0f, screen_x + 1, screen_y + 1, width + screen_x + 1 - 2, screen_y + 1 + height - 2);
    }

    if (progress != 0) {
        if (bar_border_color != 0xFFFFFF && progress * width / 100 > 1) {
            g_theWindowManager->Win_Open_Rect(
                bar_border_color, 1.0, screen_x, screen_y, screen_x + progress * width / 100, screen_y + height);
        }

        if (bar_color != 0xFFFFFF) {
            int start_x = screen_x + 1;
            int start_y = screen_y + 1;
            int end_x = progress * width / 100 + screen_x + 1 - 2;
            int end_y = screen_y + 1 + height - 2;

            if (end_x - (screen_x + 1) > 1) {
                g_theWindowManager->Win_Fill_Rect(bar_color, 1.0f, start_x, screen_y + 1, end_x, screen_y + 1 + height - 2);
                g_theWindowManager->Win_Draw_Line(0xFFFFFFFF, 1.0f, start_x, start_y, end_x, start_y);
                g_theWindowManager->Win_Draw_Line(0xFFC8C8C8, 1.0f, start_x, start_y, start_x, end_y);
            }
        }
    }
}

void W3D_Gadget_Progress_Bar_Image_Draw(GameWindow *progress_bar, WinInstanceData *data)
{
    unsigned int progress = 0;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    progress = reinterpret_cast<unsigned int>(progress_bar->Win_Get_User_Data());
#endif

    int screen_x;
    int screen_y;
    int width;
    int height;
    progress_bar->Win_Get_Screen_Position(&screen_x, &screen_y);
    progress_bar->Win_Get_Size(&width, &height);

    int x = data->m_imageOffset.x;
    int y = data->m_imageOffset.y;
    const Image *left_end_image;
    const Image *right_end_image;
    const Image *bar_right_end_image;
    const Image *repeating_center_image;
    const Image *bar_repeating_center_image;

    if ((progress_bar->Win_Get_Status() & WIN_STATUS_ENABLED) != 0) {
        if ((data->m_state & 2) != 0) {
            left_end_image = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[0].image;
            right_end_image = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[1].image;
            bar_right_end_image = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[5].image;
            repeating_center_image = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[2].image;
            bar_repeating_center_image = progress_bar->Win_Get_Instance_Data()->m_hiliteDrawData[6].image;
        } else {
            left_end_image = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[0].image;
            right_end_image = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[1].image;
            bar_right_end_image = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[5].image;
            repeating_center_image = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[2].image;
            bar_repeating_center_image = progress_bar->Win_Get_Instance_Data()->m_enabledDrawData[6].image;
        }
    } else {
        left_end_image = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[0].image;
        right_end_image = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[1].image;
        bar_right_end_image = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[5].image;
        repeating_center_image = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[2].image;
        bar_repeating_center_image = progress_bar->Win_Get_Instance_Data()->m_disabledDrawData[6].image;
    }

    if (left_end_image != nullptr && right_end_image != nullptr && repeating_center_image != nullptr
        && bar_right_end_image != nullptr) {
        int i1 = right_end_image->Get_Image_Width();
        int i2 = screen_x + left_end_image->Get_Image_Width();
        int i3 = screen_y + y + height;
        int i4 = x + screen_x + width - i1;
        int i5 = x + i2;
        int i6 = i1;
        int i7 = y + screen_y;
        int i8 = repeating_center_image->Get_Image_Width();
        int i9 = i5;
        int i10 = y + screen_y + height;

        if ((i4 - i5) / i8 > 0) {
            int i11 = (i4 - i5) / i8;

            do {
                g_theWindowManager->Win_Draw_Image(repeating_center_image, i9, i7, i8 + i9, i10, 0xFFFFFFFF);
                i8 = repeating_center_image->Get_Image_Width();
                i9 += i8;
                i11--;
            } while (i11 != 0);
        }

        IRegion2D clip_region;
        clip_region.lo.x = i9;
        clip_region.lo.y = i7;
        clip_region.hi.x = i4;
        clip_region.hi.y = i10;

        if (i4 - i9 > 0) {
            g_theDisplay->Set_Clip_Region(&clip_region);
            g_theWindowManager->Win_Draw_Image(
                repeating_center_image, i9, i7, i9 + repeating_center_image->Get_Image_Width(), i10, 0xFFFFFFFF);
            g_theDisplay->Enable_Clipping(false);
        }

        g_theWindowManager->Win_Draw_Image(left_end_image, screen_x + x, y + screen_y, i5, i3, 0xFFFFFFFF);
        g_theWindowManager->Win_Draw_Image(right_end_image, i4, i7, i4 + i6, i7 + height, 0xFFFFFFFF);
        int i12 = bar_repeating_center_image->Get_Image_Width();
        int i13 = y + screen_y + 5;
        int i14 = progress * (width - 20) / 100 / i12;
        int i15 = screen_x + 10;
        int i16 = i13 + height - 10;

        if (i14 > 0) {
            int i17 = progress * (width - 20) / 100 / i12;
            do {
                g_theWindowManager->Win_Draw_Image(bar_repeating_center_image, i15, i13, i15 + i12, i16, 0xFFFFFFFF);
                i12 = bar_repeating_center_image->Get_Image_Width();
                i15 += i12;
                i17--;
            } while (i17);
        }

        int i18 = i14 * bar_repeating_center_image->Get_Image_Width() + screen_x + 0xA;

        if ((width - 20) / bar_repeating_center_image->Get_Image_Width() - i14 > 0) {
            int i21 = bar_right_end_image->Get_Image_Width();
            int i19 = (width - 20) / bar_repeating_center_image->Get_Image_Width() - i14;

            do {
                g_theWindowManager->Win_Draw_Image(bar_right_end_image, i18, i13, i21 + i18, i16, 0xFFFFFFFF);
                i21 = bar_right_end_image->Get_Image_Width();
                i18 += i21;
                i19--;
            } while (i19);
        }
    }
}
