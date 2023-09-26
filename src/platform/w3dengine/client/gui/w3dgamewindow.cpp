/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Game Window
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dgamewindow.h"
#include "displaystring.h"
#include "gadgetlistbox.h"
#include "gamewindowmanager.h"
#include "image.h"
#include "w3ddisplay.h"

W3DGameWindow::W3DGameWindow() :
    m_xTextLoc(0), m_yTextLoc(0), m_color(0xFFFFFF), m_textChanged(false), m_textLocChanged(false)
{
    Win_Set_Draw_Func(g_theWindowManager->Get_Default_Draw());
}

W3DGameWindow::~W3DGameWindow() {}

void W3DGameWindow::Win_Draw_Border()
{
    bool done = false;
    int x;
    int y;
    Win_Get_Screen_Position(&x, &y);

    for (int i = 0; i < 32 && !done; i++) {
        unsigned int style = (1 << i) & Win_Get_Style();

        if (style == 0) {
            continue;
        }

        if (style == GWS_ENTRY_FIELD) {
            int xsize = m_size.x;
            int xpos = x;
            int ypos = y;
            int text_length = Win_Get_Text_Length();

            if (text_length != 0) {
                int width = 0;
                g_theWindowManager->Win_Get_Text_Size(Win_Get_Font(), Win_Get_Text(), &width, nullptr, 0);
                xsize -= width + 6;
                xpos += width + 6;
            }

            Blit_Border_Rect(xpos, ypos, xsize, m_size.y);
            done = true;
        } else if (style >= GWS_ENTRY_FIELD) {
            if (style == GWS_USER_WINDOW) {
                Blit_Border_Rect(x, y, m_size.x, m_size.y);
                done = true;
                continue;
            }

            if (style >= GWS_USER_WINDOW) {
                if (style == GWS_TAB_CONTROL) {
                    Blit_Border_Rect(x, y, m_size.x, m_size.y);
                    done = true;
                    continue;
                }
            } else if (style == GWS_PROGRESS_BAR || (style < GWS_PROGRESS_BAR && style == GWS_STATIC_TEXT)) {
                Blit_Border_Rect(x, y, m_size.x, m_size.y);
                done = true;
                continue;
            }
        } else {
            if (style == GWS_VERT_SLIDER) {
                done = true;
                continue;
            }

            if (style >= GWS_VERT_SLIDER) {
                if (style == GWS_SCROLL_LISTBOX) {
                    _ListboxData *data = static_cast<_ListboxData *>(Win_Get_User_Data());
                    int x_size_offset = 0;
                    int y_offset = 0;

                    if (data->m_scrollBar) {
                        int child_width;
                        int child_height;
                        data->m_slider->Win_Get_Child()->Win_Get_Size(&child_width, &child_height);
                        x_size_offset = child_height;
                    }

                    int text_length = Win_Get_Text_Length();

                    if (text_length != 0) {
                        y_offset = 4;
                    }

                    Blit_Border_Rect(x - 3, y - (y_offset + 3), m_size.x + 3 - x_size_offset, m_size.y + 6);
                    done = true;
                } else if (style < GWS_SCROLL_LISTBOX && style == GWS_HORZ_SLIDER) {
                    done = true;
                    continue;
                }
            } else {
                if (style == (GWS_PUSH_BUTTON | GWS_RADIO_BUTTON)) {
                    continue;
                }

                if (style >= (GWS_PUSH_BUTTON | GWS_RADIO_BUTTON)) {
                    if (style < (GWS_PUSH_BUTTON | GWS_CHECK_BOX)) {
                        done = true;
                    }
                } else if (style >= GWS_PUSH_BUTTON) {
                    Blit_Border_Rect(x, y, m_size.x, m_size.y);
                    done = true;
                    continue;
                }
            }
        }
    }
}

int W3DGameWindow::Win_Set_Text(Utf16String new_text)
{
    GameWindow::Win_Set_Text(new_text);
    m_text.Build_Sentence(Win_Get_Text().Str());
    m_textChanged = true;
    return 0;
}

void W3DGameWindow::Win_Set_Font(GameFont *font)
{
    GameWindow::Win_Set_Font(font);
    m_text.Set_Font(font->m_fontData);
    m_textChanged = true;
}

enum
{
    BORDER_CORNER_UL,
    BORDER_CORNER_UR,
    BORDER_CORNER_LL,
    BORDER_CORNER_LR,
    BORDER_LEFT,
    BORDER_LEFT_SHORT,
    BORDER_TOP,
    BORDER_TOP_SHORT,
    BORDER_RIGHT,
    BORDER_RIGHT_SHORT,
    BORDER_BOTTOM,
    BORDER_BOTTOM_SHORT,
    NUM_BORDER_PIECES,
};

bool g_bordersInit = false;
const Image *g_borderPieces[NUM_BORDER_PIECES];

void Init_Borders()
{
    g_borderPieces[BORDER_CORNER_UL] = g_theMappedImageCollection->Find_Image_By_Name("BorderCornerUL");
    g_borderPieces[BORDER_CORNER_UR] = g_theMappedImageCollection->Find_Image_By_Name("BorderCornerUR");
    g_borderPieces[BORDER_CORNER_LL] = g_theMappedImageCollection->Find_Image_By_Name("BorderCornerLL");
    g_borderPieces[BORDER_CORNER_LR] = g_theMappedImageCollection->Find_Image_By_Name("BorderCornerLR");
    g_borderPieces[BORDER_LEFT] = g_theMappedImageCollection->Find_Image_By_Name("BorderLeft");
    g_borderPieces[BORDER_LEFT_SHORT] = g_theMappedImageCollection->Find_Image_By_Name("BorderLeftShort");
    g_borderPieces[BORDER_TOP] = g_theMappedImageCollection->Find_Image_By_Name("BorderTop");
    g_borderPieces[BORDER_TOP_SHORT] = g_theMappedImageCollection->Find_Image_By_Name("BorderTopShort");
    g_borderPieces[BORDER_RIGHT] = g_theMappedImageCollection->Find_Image_By_Name("BorderRight");
    g_borderPieces[BORDER_RIGHT_SHORT] = g_theMappedImageCollection->Find_Image_By_Name("BorderRightShort");
    g_borderPieces[BORDER_BOTTOM] = g_theMappedImageCollection->Find_Image_By_Name("BorderBottom");
    g_borderPieces[BORDER_BOTTOM_SHORT] = g_theMappedImageCollection->Find_Image_By_Name("BorderBottomShort");
    g_bordersInit = true;
}

void W3DGameWindow::Blit_Border_Rect(int left, int top, int width, int height)
{
    if (!g_bordersInit) {
        Init_Borders();
    }

    int right = left + width;
    int bottom = top + height;
    int bottom_pad = top + height - 5;
    int i;

    for (i = left + 5; i <= right - 25; i += 20) {
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_RIGHT], i, top - 15, i + 20, top + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_BOTTOM], i, bottom_pad, i + 20, bottom_pad + 20, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    }

    int right_pad = right - 5;

    if (right - 5 - i >= 10) {
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_RIGHT_SHORT], i, top - 15, i + 10, top + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(g_borderPieces[BORDER_BOTTOM_SHORT],
            i,
            bottom_pad,
            i + 10,
            bottom_pad + 20,
            0xFFFFFFFF,
            Display::DRAWIMAGE_ADDITIVE);
    }

    if (i < right_pad) {
        int i5 = i - (10 - ((right_pad - i + 1) & 0xFFFFFFFE));
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_RIGHT_SHORT], i5, top - 15, i5 + 10, top + 5, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(g_borderPieces[BORDER_BOTTOM_SHORT],
            i5,
            bottom_pad,
            i5 + 10,
            bottom_pad + 20,
            0xFFFFFFFF,
            Display::DRAWIMAGE_ADDITIVE);
    }

    int left_pad = left - 15;
    int right_pad2 = right - 5;
    int j;

    for (j = top + 5; j <= bottom - 25; j += 20) {
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_LEFT], left - 15, j, left_pad + 20, j + 20, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_TOP], right - 5, j, right_pad2 + 20, j + 20, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
    }

    int bottom_pad2 = bottom - 5;

    if (bottom - 5 - j >= 10) {
        g_theDisplay->Draw_Image(
            g_borderPieces[BORDER_LEFT_SHORT], left - 15, j, left_pad + 20, j + 10, 0xFFFFFFFF, Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(g_borderPieces[BORDER_TOP_SHORT],
            right - 5,
            j,
            right_pad2 + 20,
            j + 10,
            0xFFFFFFFF,
            Display::DRAWIMAGE_ADDITIVE);
    }

    if (j < bottom_pad2) {
        int i9 = j - (10 - ((bottom_pad2 - j + 1) & 0xFFFFFFFE));
        g_theDisplay->Draw_Image(g_borderPieces[BORDER_LEFT_SHORT],
            left - 15,
            i9,
            left_pad + 20,
            i9 + 10,
            0xFFFFFFFF,
            Display::DRAWIMAGE_ADDITIVE);
        g_theDisplay->Draw_Image(g_borderPieces[BORDER_TOP_SHORT],
            right - 5,
            i9,
            right_pad2 + 20,
            i9 + 10,
            0xFFFFFFFF,
            Display::DRAWIMAGE_ADDITIVE);
    }

    g_theDisplay->Draw_Image(g_borderPieces[BORDER_CORNER_UL],
        left - 15,
        top - 15,
        left - 15 + 20,
        top - 15 + 20,
        0xFFFFFFFF,
        Display::DRAWIMAGE_ADDITIVE);
    g_theDisplay->Draw_Image(g_borderPieces[BORDER_CORNER_UR],
        right - 5,
        top - 15,
        right - 5 + 20,
        top - 15 + 20,
        0xFFFFFFFF,
        Display::DRAWIMAGE_ADDITIVE);
    g_theDisplay->Draw_Image(g_borderPieces[BORDER_CORNER_LL],
        left - 15,
        bottom - 5,
        left - 15 + 20,
        bottom - 5 + 20,
        0xFFFFFFFF,
        Display::DRAWIMAGE_ADDITIVE);
    g_theDisplay->Draw_Image(g_borderPieces[BORDER_CORNER_LR],
        right - 5,
        bottom - 5,
        right - 5 + 20,
        bottom - 5 + 20,
        0xFFFFFFFF,
        Display::DRAWIMAGE_ADDITIVE);
}

void W3D_Game_Win_Default_Draw(GameWindow *window, WinInstanceData *instance)
{
    int x;
    int y;
    int width;
    int height;
    window->Win_Get_Screen_Position(&x, &y);
    window->Win_Get_Size(&width, &height);

    if ((window->Win_Get_Status() & WIN_STATUS_IMAGE) != 0) {
        const Image *image;

        if ((window->Win_Get_Status() & WIN_STATUS_ENABLED) != 0) {
            if ((instance->m_state & 2) != 0) {
                image = window->Win_Get_Instance_Data()->m_hiliteDrawData[0].image;
            } else {
                image = window->Win_Get_Instance_Data()->m_enabledDrawData[0].image;
            }
        } else {
            image = window->Win_Get_Instance_Data()->m_disabledDrawData[0].image;
        }

        if (image != nullptr) {
            g_theWindowManager->Win_Draw_Image(image,
                x + instance->m_imageOffset.x,
                y + instance->m_imageOffset.y,
                x + instance->m_imageOffset.x + width,
                y + instance->m_imageOffset.y + height,
                0xFFFFFFFF);
        }
    } else {
        int color;
        int border_color;

        if ((window->Win_Get_Status() & WIN_STATUS_ENABLED) != 0) {
            if ((instance->m_state & 2) != 0) {
                color = window->Win_Get_Instance_Data()->m_hiliteDrawData[0].color;
                border_color = window->Win_Get_Instance_Data()->m_hiliteDrawData[0].borderColor;
            } else {
                color = window->Win_Get_Instance_Data()->m_enabledDrawData[0].color;
                border_color = window->Win_Get_Instance_Data()->m_enabledDrawData[0].borderColor;
            }
        } else {
            color = window->Win_Get_Instance_Data()->m_disabledDrawData[0].color;
            border_color = window->Win_Get_Instance_Data()->m_disabledDrawData[0].borderColor;
        }

        if (border_color != 0xFFFFFF) {
            g_theWindowManager->Win_Open_Rect(border_color, 1.0f, x, y, x + width, y + height);
        }

        if (color != 0xFFFFFF) {
            g_theWindowManager->Win_Fill_Rect(color, 1.0f, x + 1.0f, y + 1.0f, (x + width) - 1.0f, (y + height) - 1.0f);
        }
    }

    if (instance->m_videoBuffer != nullptr) {
        int video_x;
        int video_y;
        int video_width;
        int video_height;
        window->Win_Get_Screen_Position(&video_x, &video_y);
        window->Win_Get_Size(&video_width, &video_height);
        g_theDisplay->Draw_VideoBuffer(
            instance->m_videoBuffer, video_x, video_y, video_x + video_width, video_y + video_height);
    }
}
