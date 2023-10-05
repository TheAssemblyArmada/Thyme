/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Static Text
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dstatictext.h"
#include "displaystring.h"
#include "gadgetstatictext.h"
#include "gamewindowmanager.h"
#include "globaldata.h"

void Draw_Static_Text_Text(GameWindow *static_text, WinInstanceData *data, int text_color, int border_color)
{
    _TextData *st_data = static_cast<_TextData *>(static_text->Win_Get_User_Data());
    DisplayString *text = st_data->m_text;

    if (text != nullptr && text->Get_Text_Length() != 0) {
        int screen_x;
        int screen_y;
        int width;
        int height;
        static_text->Win_Get_Screen_Position(&screen_x, &screen_y);
        static_text->Win_Get_Size(&width, &height);
        text->Set_Word_Wrap(width - 10);

        if ((static_text->Win_Get_Status() & WIN_STATUS_WRAP_CENTERED) != 0) {
            text->Set_Word_Wrap_Centered(true);
        } else {
            text->Set_Word_Wrap_Centered(false);
        }

        if ((static_text->Win_Get_Status() & WIN_STATUS_HOTKEY_TEXT) != 0 && g_theWriteableGlobalData != nullptr) {
            text->Set_Use_Hotkey(true, g_theWriteableGlobalData->m_hotKeytextColor);
        } else {
            text->Set_Use_Hotkey(false, 0);
        }

        int text_x;
        int text_y;
        text->Get_Size(&text_x, &text_y);
        IRegion2D clip_region;
        clip_region.lo.x = screen_x;
        clip_region.lo.y = screen_y;
        clip_region.hi.x = screen_x + width;
        clip_region.hi.y = screen_y + height;
        int x_offset;

        if (st_data->m_centered) {
            x_offset = width / 2 - text_x / 2;
        } else {
            x_offset = st_data->m_xOffset;
        }

        int x = screen_x + x_offset;
        int y_offset;

        if (st_data->m_vertCentered) {
            y_offset = height / 2 - text_y / 2;
        } else {
            y_offset = st_data->m_yOffset;
        }

        int y = screen_y + y_offset;
        text->Set_Clip_Region(&clip_region);
        text->Draw(x, y, text_color, border_color);
    }
}

void W3D_Gadget_Static_Text_Draw(GameWindow *static_text, WinInstanceData *data)
{
    _TextData *st_data = static_cast<_TextData *>(static_text->Win_Get_User_Data());
    int screen_x;
    int screen_y;
    int width;
    int height;
    static_text->Win_Get_Screen_Position(&screen_x, &screen_y);
    static_text->Win_Get_Size(&width, &height);

    int border_color;
    int color;
    int text_color;
    int text_border_color;

    if ((static_text->Win_Get_Status() & WIN_STATUS_ENABLED) != 0) {
        border_color = static_text->Win_Get_Instance_Data()->m_enabledDrawData[0].borderColor;
        color = static_text->Win_Get_Instance_Data()->m_enabledDrawData[0].color;
        text_color = static_text->Win_Get_Enabled_Text_Color();
        text_border_color = static_text->Win_Get_Enabled_Text_Border_Color();
    } else {
        border_color = static_text->Win_Get_Instance_Data()->m_disabledDrawData[0].borderColor;
        color = static_text->Win_Get_Instance_Data()->m_disabledDrawData[0].color;
        text_color = static_text->Win_Get_Disabled_Text_Color();
        text_border_color = static_text->Win_Get_Disabled_Text_Border_Color();
    }

    if (border_color != 0xFFFFFF) {
        g_theWindowManager->Win_Open_Rect(border_color, 1.0f, screen_x, screen_y, width + screen_x, screen_y + height);
    }

    if (color != 0xFFFFFF) {
        g_theWindowManager->Win_Fill_Rect(
            color, 1.0f, screen_x + 1, screen_y + 1, width + screen_x + 1 - 2, screen_y + 1 + height - 2);
    }

    if (st_data->m_text != nullptr) {
        if (text_color != 0xFFFFFF) {
            Draw_Static_Text_Text(static_text, data, text_color, text_border_color);
        }
    }
}

void W3D_Gadget_Static_Text_Image_Draw(GameWindow *static_text, WinInstanceData *data)
{
    _TextData *st_data = static_cast<_TextData *>(static_text->Win_Get_User_Data());
    int screen_x;
    int screen_y;
    int width;
    int height;
    static_text->Win_Get_Screen_Position(&screen_x, &screen_y);
    static_text->Win_Get_Size(&width, &height);

    const Image *image;
    int text_color;
    int text_border_color;

    if ((static_text->Win_Get_Status() & WIN_STATUS_ENABLED) != 0) {
        image = static_text->Win_Get_Instance_Data()->m_enabledDrawData[0].image;
        text_color = static_text->Win_Get_Enabled_Text_Color();
        text_border_color = static_text->Win_Get_Enabled_Text_Border_Color();
    } else {
        image = static_text->Win_Get_Instance_Data()->m_disabledDrawData[0].image;
        text_color = static_text->Win_Get_Disabled_Text_Color();
        text_border_color = static_text->Win_Get_Disabled_Text_Border_Color();
    }

    if (image != nullptr) {
        g_theWindowManager->Win_Draw_Image(image,
            screen_x + data->m_imageOffset.x,
            screen_y + data->m_imageOffset.y,
            screen_x + data->m_imageOffset.x + width,
            screen_y + data->m_imageOffset.y + height,
            0xFFFFFFFF);
    }

    if (st_data->m_text != nullptr) {
        if (text_color != 0xFFFFFF) {
            Draw_Static_Text_Text(static_text, data, text_color, text_border_color);
        }
    }
}
