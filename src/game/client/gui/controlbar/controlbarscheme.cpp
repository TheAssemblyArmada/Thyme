/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar Scheme
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "controlbarscheme.h"
#include "colorspace.h"
#include "controlbar.h"
#include "display.h"
#include "gadgetpushbutton.h"
#include "gamewindowmanager.h"
#include "player.h"
#include "playertemplate.h"
#include "recorder.h"

ControlBarSchemeImage::ControlBarSchemeImage() : m_image(nullptr), m_layer(0)
{
    m_position.x = 0;
    m_position.y = 0;
    m_size.x = 0;
    m_size.y = 0;
}

ControlBarSchemeImage::~ControlBarSchemeImage()
{
    m_image = nullptr;
}

ControlBarSchemeAnimation::ControlBarSchemeAnimation() :
    m_animType(0), m_animImage(nullptr), m_animDuration(0), m_currentFrame(0)
{
    m_finalPos.x = 0;
    m_finalPos.y = 0;
    m_startPos.x = 0;
    m_startPos.y = 0;
}

ControlBarSchemeAnimation::~ControlBarSchemeAnimation()
{
    m_animImage = nullptr;
}

ControlBarScheme::ControlBarScheme() :
    m_buttonQueueImage(nullptr),
    m_rightHUDImage(nullptr),
    m_buildUpClockColor(Make_Color(0, 0, 0, 100)),
    m_borderBuildColor(0xFFFFFFFF),
    m_borderActionColor(0xFFFFFFFF),
    m_borderUpgradeColor(0xFFFFFFFF),
    m_borderSystemColor(0xFFFFFFFF),
    m_commandBarBorderColor(0),
    m_optionsButtonEnableImage(nullptr),
    m_optionsButtonHilitedImage(nullptr),
    m_optionsButtonPushedImage(nullptr),
    m_optionsButtonDisabledImage(nullptr),
    m_idleWorkerButtonEnableImage(nullptr),
    m_idleWorkerButtonHilitedImage(nullptr),
    m_idleWorkerButtonPushedImage(nullptr),
    m_idleWorkerButtonDisabledImage(nullptr),
    m_buddyButtonEnableImage(nullptr),
    m_buddyButtonHilitedImage(nullptr),
    m_buddyButtonPushedImage(nullptr),
    m_buddyButtonDisabledImage(nullptr),
    m_beaconButtonEnableImage(nullptr),
    m_beaconButtonHilitedImage(nullptr),
    m_beaconButtonPushedImage(nullptr),
    m_beaconButtonDisabledImage(nullptr),
    m_genBarButtonInImage(nullptr),
    m_genBarButtonOnImage(nullptr),
    m_toggleButtonUpInImage(nullptr),
    m_toggleButtonUpOnImage(nullptr),
    m_toggleButtonUpPushedImage(nullptr),
    m_toggleButtonDownInImage(nullptr),
    m_toggleButtonDownOnImage(nullptr),
    m_toggleButtonDownPushedImage(nullptr),
    m_generalButtonEnableImage(nullptr),
    m_generalButtonHilitedImage(nullptr),
    m_generalButtonPushedImage(nullptr),
    m_generalButtonDisabledImage(nullptr),
    m_uAttackButtonEnableImage(nullptr),
    m_uAttackButtonHilitedImage(nullptr),
    m_uAttackButtonPushedImage(nullptr),
    m_minMaxButtonEnableImage(nullptr),
    m_minMaxButtonHilitedImage(nullptr),
    m_minMaxButtonPushedImage(nullptr),
    m_genArrowImage(nullptr),
    m_expBarForegroundImage(nullptr),
    m_commandMarkerImage(nullptr),
    m_powerPurchaseImage(nullptr)
{
    m_screenCreationRes.x = 0;
    m_screenCreationRes.y = 0;
    m_minMaxUL.x = 0;
    m_minMaxLR.x = 0;
    m_generalUL.x = 0;
    m_generalLR.x = 0;
    m_uAttackUL.x = 0;
    m_uAttackLR.x = 0;
    m_optionsUL.x = 0;
    m_optionsLR.x = 0;
    m_workerUL.x = 0;
    m_workerLR.x = 0;
    m_chatUL.x = 0;
    m_chatLR.x = 0;
    m_beaconUL.x = 0;
    m_beaconLR.x = 0;
    m_powerBarUL.x = 0;
    m_powerBarLR.x = 0;
    m_moneyUL.x = 0;
    m_moneyLR.x = 0;
    m_minMaxUL.y = 0;
    m_minMaxLR.y = 0;
    m_generalUL.y = 0;
    m_generalLR.y = 0;
    m_uAttackUL.y = 0;
    m_uAttackLR.y = 0;
    m_optionsUL.y = 0;
    m_optionsLR.y = 0;
    m_workerUL.y = 0;
    m_workerLR.y = 0;
    m_chatUL.y = 0;
    m_chatLR.y = 0;
    m_beaconUL.y = 0;
    m_beaconLR.y = 0;
    m_powerBarUL.y = 0;
    m_powerBarLR.y = 0;
    m_moneyUL.y = 0;
    m_moneyLR.y = 0;
}

ControlBarScheme::~ControlBarScheme()
{
    Reset();
}

void ControlBarScheme::Init()
{
    if (g_theControlBar != nullptr) {
        g_theControlBar->Switch_Control_Bar_Stage(CONTROL_BAR_STAGE_DEFAULT);
        g_theControlBar->Update_Build_Queue_Disabled_Images(m_buttonQueueImage);
        g_theControlBar->Update_Right_HUD_Image(m_rightHUDImage);
        g_theControlBar->Update_Build_Up_Clock_Color(m_buildUpClockColor);
        g_theControlBar->Update_Command_Bar_Border_Colors(
            m_borderBuildColor, m_borderActionColor, m_borderUpgradeColor, m_borderSystemColor);
        g_theControlBar->Set_Command_Bar_Border_Color(m_commandBarBorderColor);
        g_theControlBar->Update_Slot_Exit_Image(m_commandMarkerImage);
        g_theControlBar->Update_Up_Down_Images(m_toggleButtonUpInImage,
            m_toggleButtonUpOnImage,
            m_toggleButtonUpPushedImage,
            m_toggleButtonDownInImage,
            m_toggleButtonDownOnImage,
            m_toggleButtonDownPushedImage,
            m_generalButtonEnableImage,
            m_generalButtonHilitedImage);
        g_theControlBar->Set_Gen_Arrow_Image(m_genArrowImage);
    }

    GameWindow *window = nullptr;
    float width = (float)g_theDisplay->Get_Width() / (float)m_screenCreationRes.x;
    float height = (float)g_theDisplay->Get_Height() / (float)m_screenCreationRes.y;
    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PopupCommunicator"));

    if (window != nullptr) {
        Gadget_Button_Set_Enabled_Image(window, m_buddyButtonEnableImage);
        Gadget_Button_Set_Hilite_Image(window, m_buddyButtonHilitedImage);
        Gadget_Button_Set_Hilite_Selected_Image(window, m_buddyButtonPushedImage);
        Gadget_Button_Set_Disabled_Image(window, m_buddyButtonDisabledImage);

        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_chatUL.x * width - (float)parent_x;
            y_pos = (float)m_chatUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_chatUL.x * width;
            y_pos = (float)m_chatUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_chatLR.x - m_chatUL.x) * width + 0.0f, (float)(m_chatLR.y - m_chatUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonIdleWorker"));

    if (window != nullptr) {
        Gadget_Button_Set_Enabled_Image(window, m_idleWorkerButtonEnableImage);
        Gadget_Button_Set_Hilite_Image(window, m_idleWorkerButtonHilitedImage);
        Gadget_Button_Set_Hilite_Selected_Image(window, m_idleWorkerButtonPushedImage);
        Gadget_Button_Set_Disabled_Image(window, m_idleWorkerButtonDisabledImage);

        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_workerUL.x * width - (float)parent_x;
            y_pos = (float)m_workerUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_workerUL.x * width;
            y_pos = (float)m_workerUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_workerLR.x - m_workerUL.x) * width + 0.0f, (float)(m_workerLR.y - m_workerUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ExpBarForeground"));

    if (window != nullptr) {
        window->Win_Set_Enabled_Image(0, m_expBarForegroundImage);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonOptions"));

    if (window != nullptr) {
        Gadget_Button_Set_Enabled_Image(window, m_optionsButtonEnableImage);
        Gadget_Button_Set_Hilite_Image(window, m_optionsButtonHilitedImage);
        Gadget_Button_Set_Hilite_Selected_Image(window, m_optionsButtonPushedImage);
        Gadget_Button_Set_Disabled_Image(window, m_optionsButtonDisabledImage);

        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_optionsUL.x * width - (float)parent_x;
            y_pos = (float)m_optionsUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_optionsUL.x * width;
            y_pos = (float)m_optionsUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_optionsLR.x - m_optionsUL.x) * width + 0.0f, (float)(m_optionsLR.y - m_optionsUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonPlaceBeacon"));

    if (window != nullptr) {
        Gadget_Button_Set_Enabled_Image(window, m_beaconButtonEnableImage);
        Gadget_Button_Set_Hilite_Image(window, m_beaconButtonHilitedImage);
        Gadget_Button_Set_Hilite_Selected_Image(window, m_beaconButtonPushedImage);
        Gadget_Button_Set_Disabled_Image(window, m_beaconButtonDisabledImage);

        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_beaconUL.x * width - (float)parent_x;
            y_pos = (float)m_beaconUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_beaconUL.x * width;
            y_pos = (float)m_beaconUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_beaconLR.x - m_beaconUL.x) * width + 0.0f, (float)(m_beaconLR.y - m_beaconUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:MoneyDisplay"));

    if (window != nullptr) {
        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_moneyUL.x * width - (float)parent_x;
            y_pos = (float)m_moneyUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_moneyUL.x * width;
            y_pos = (float)m_moneyUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_moneyLR.x - m_moneyUL.x) * width + 0.0f, (float)(m_moneyLR.y - m_moneyUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PowerWindow"));

    if (window != nullptr) {
        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_powerBarUL.x * width - (float)parent_x;
            y_pos = (float)m_powerBarUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_powerBarUL.x * width;
            y_pos = (float)m_powerBarUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size((float)(m_powerBarLR.x - m_powerBarUL.x) * width + 0.0f,
            (float)(m_powerBarLR.y - m_powerBarUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonGeneral"));

    if (window != nullptr) {
        Gadget_Button_Set_Enabled_Image(window, m_generalButtonEnableImage);
        Gadget_Button_Set_Hilite_Image(window, m_generalButtonHilitedImage);
        Gadget_Button_Set_Hilite_Selected_Image(window, m_generalButtonPushedImage);
        Gadget_Button_Set_Disabled_Image(window, m_generalButtonDisabledImage);

        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_generalUL.x * width - (float)parent_x;
            y_pos = (float)m_generalUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_generalUL.x * width;
            y_pos = (float)m_generalUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_generalLR.x - m_generalUL.x) * width + 0.0f, (float)(m_generalLR.y - m_generalUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonLarge"));

    if (window != nullptr) {
        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_minMaxUL.x * width - (float)parent_x;
            y_pos = (float)m_minMaxUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_minMaxUL.x * width;
            y_pos = (float)m_minMaxUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_minMaxLR.x - m_minMaxUL.x) * width + 0.0f, (float)(m_minMaxLR.y - m_minMaxUL.y) * height + 0.0f);
    }

    window =
        g_theWindowManager->Win_Get_Window_From_Id(nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:WinUAttack"));

    if (window != nullptr) {
        window->Win_Set_Enabled_Image(0, m_uAttackButtonEnableImage);
        window->Win_Set_Disabled_Image(0, m_uAttackButtonHilitedImage);

        GameWindow *parent = window->Win_Get_Parent();
        int x_pos;
        int y_pos;

        if (parent != nullptr) {
            int parent_x;
            int parent_y;
            parent->Win_Get_Screen_Position(&parent_x, &parent_y);
            x_pos = (float)m_uAttackUL.x * width - (float)parent_x;
            y_pos = (float)m_uAttackUL.y * height - (float)parent_y;
        } else {
            x_pos = (float)m_uAttackUL.x * width;
            y_pos = (float)m_uAttackUL.y * height;
        }

        window->Win_Set_Position(x_pos, y_pos);
        window->Win_Set_Size(
            (float)(m_uAttackLR.x - m_uAttackUL.x) * width + 0.0f, (float)(m_uAttackLR.y - m_uAttackUL.y) * height + 0.0f);
    }

    window = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("GeneralsExpPoints.wnd:GenExpParent"));

    if (window != nullptr) {
        window->Win_Set_Enabled_Image(0, m_powerPurchaseImage);

        if (m_powerPurchaseImage != nullptr) {
            window->Win_Set_Size(
                m_powerPurchaseImage->Get_Image_Width() * width, m_powerPurchaseImage->Get_Image_Height() * height);
        }
    }
}

void ControlBarScheme::Add_Animation(ControlBarSchemeAnimation *scheme_anim)
{
    if (scheme_anim != nullptr) {
        m_animations.push_back(scheme_anim);
    } else {
        captainslog_dbgassert(false, "Trying to add a null animation to the controlbarscheme");
    }
}

void ControlBarScheme::Add_Image(ControlBarSchemeImage *scheme_image)
{
    if (scheme_image != nullptr) {
        if (scheme_image->m_layer >= LAYER_COUNT) {
            captainslog_dbgassert(false,
                "SchemeImage %s attempted to be added to layer %d which is not between %d, %d",
                scheme_image->m_name.Str(),
                scheme_image->m_layer,
                0,
                LAYER_COUNT);
            scheme_image->m_layer = 0;
        }

        m_layer[scheme_image->m_layer].push_back(scheme_image);
    } else {
        captainslog_dbgassert(false, "Trying to add a null image to the controlbarscheme");
    }
}

void ControlBarScheme::Draw_Background(Coord2D multi, ICoord2D offset)
{
    for (int i = 5; i >= 3; i--) {
        for (auto it = m_layer[i].begin(); it != m_layer[i].end(); it++) {
            ControlBarSchemeImage *image = *it;

            if (image != nullptr) {
                if (image->m_image != nullptr) {
                    g_theDisplay->Draw_Image(image->m_image,
                        (float)image->m_position.x * multi.x + (float)offset.x,
                        (float)image->m_position.y * multi.y + (float)offset.y,
                        (float)(image->m_size.x + image->m_position.x) * multi.x + (float)offset.x,
                        (float)(image->m_size.y + image->m_position.y) * multi.y + (float)offset.y,
                        0xFFFFFFFF,
                        Display::DRAWIMAGE_ADDITIVE);
                }
            } else {
                captainslog_dbgassert(false, "There is no ControlBarSchemeImage found in the m_layer list");
            }
        }
    }
}

void ControlBarScheme::Draw_Foreground(Coord2D multi, ICoord2D offset)
{
    for (int i = 2; i >= 0; i--) {
        for (auto it = m_layer[i].begin(); it != m_layer[i].end(); it++) {
            ControlBarSchemeImage *image = *it;

            if (image != nullptr) {
                if (image->m_image != nullptr) {
                    g_theDisplay->Draw_Image(image->m_image,
                        (float)image->m_position.x * multi.x + (float)offset.x,
                        (float)image->m_position.y * multi.y + (float)offset.y,
                        (float)(image->m_size.x + image->m_position.x) * multi.x + (float)offset.x,
                        (float)(image->m_size.y + image->m_position.y) * multi.y + (float)offset.y,
                        0xFFFFFFFF,
                        Display::DRAWIMAGE_ADDITIVE);
                }
            } else {
                captainslog_dbgassert(false, "There is no ControlBarSchemeImage found in the m_layer list");
            }
        }
    }
}

void ControlBarScheme::Reset()
{
    for (int i = 0; i < LAYER_COUNT; i++) {
        for (auto it = m_layer[i].begin(); it != m_layer[i].end(); it++) {
            if (*it != nullptr) {
                delete *it;
            }
        }

        m_layer[i].clear();
    }

    for (auto it = m_animations.begin(); it != m_animations.end(); it++) {
        if (*it != nullptr) {
            delete *it;
        }
    }

    m_animations.clear();
    m_name.Clear();
    m_screenCreationRes.x = 0.0f;
    m_screenCreationRes.y = 0.0f;
    m_side.Clear();
    m_buttonQueueImage = nullptr;
    m_rightHUDImage = nullptr;
    m_optionsButtonEnableImage = nullptr;
    m_optionsButtonHilitedImage = nullptr;
    m_optionsButtonPushedImage = nullptr;
    m_optionsButtonDisabledImage = nullptr;
    m_idleWorkerButtonEnableImage = nullptr;
    m_idleWorkerButtonHilitedImage = nullptr;
    m_idleWorkerButtonPushedImage = nullptr;
    m_idleWorkerButtonDisabledImage = nullptr;
    m_buddyButtonEnableImage = nullptr;
    m_buddyButtonHilitedImage = nullptr;
    m_buddyButtonPushedImage = nullptr;
    m_buddyButtonDisabledImage = nullptr;
    m_beaconButtonEnableImage = nullptr;
    m_beaconButtonHilitedImage = nullptr;
    m_beaconButtonPushedImage = nullptr;
    m_beaconButtonDisabledImage = nullptr;
    m_genBarButtonInImage = nullptr;
    m_genBarButtonOnImage = nullptr;
    m_toggleButtonUpInImage = nullptr;
    m_toggleButtonUpOnImage = nullptr;
    m_toggleButtonUpPushedImage = nullptr;
    m_toggleButtonDownInImage = nullptr;
    m_toggleButtonDownOnImage = nullptr;
    m_toggleButtonDownPushedImage = nullptr;
    m_commandMarkerImage = nullptr;
    m_expBarForegroundImage = nullptr;
    m_powerPurchaseImage = nullptr;
}

void ControlBarScheme::Update()
{
    for (auto it = m_animations.begin(); it != m_animations.end(); it++) {
        ControlBarSchemeAnimation *anim = *it;

        if (anim == nullptr) {
            captainslog_dbgassert(false, "There's no Animation in the ControlBarSchemeAnimationList:m_animations");
            return;
        }

        Update_Anim(anim);
    }
}

void Anim_Slide_Right(ControlBarSchemeAnimation *anim)
{
    if (anim->m_animImage != nullptr && anim->m_animDuration != 0) {
        unsigned int frame = anim->Get_Current_Frame();
        ICoord2D pos = anim->Get_Start_Pos();

        if (frame == anim->m_animDuration) {
            anim->m_animImage->m_position = pos;
            anim->Set_Current_Frame(0);
        } else {
            if (frame == 0) {
                pos = anim->m_animImage->m_position;
                anim->Set_Start_Pos(pos);
            }

            frame++;
            anim->Set_Current_Frame(frame);
            anim->m_animImage->m_position.x = frame * (anim->m_finalPos.x - pos.x) / anim->m_animDuration + pos.x;
        }
    }
}

void ControlBarScheme::Update_Anim(ControlBarSchemeAnimation *anim)
{
    if (anim->m_animType != ControlBarSchemeAnimation::CB_ANIM_SLIDE_RIGHT) {
        captainslog_dbgassert(false, "We tried to animate but not animate function was found %d", anim->m_animType);
    } else {
        Anim_Slide_Right(anim);
    }
}

FieldParse ControlBarSchemeManager::m_controlBarSchemeFieldParseTable[] = {
    { "ImagePart", &ControlBarSchemeManager::Parse_Image_Part, nullptr, 0 },
    { "AnimatingPart", &ControlBarSchemeManager::Parse_Animating_Part, nullptr, 0 },
    { "ScreenCreationRes", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_screenCreationRes) },
    { "Side", &INI::Parse_AsciiString, nullptr, offsetof(ControlBarScheme, m_side) },
    { "QueueButtonImage", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarScheme, m_buttonQueueImage) },
    { "RightHUDImage", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarScheme, m_rightHUDImage) },
    { "BuildUpClockColor", &INI::Parse_Color_Int, nullptr, offsetof(ControlBarScheme, m_buildUpClockColor) },
    { "ButtonBorderBuildColor", &INI::Parse_Color_Int, nullptr, offsetof(ControlBarScheme, m_borderBuildColor) },
    { "CommandBarBorderColor", &INI::Parse_Color_Int, nullptr, offsetof(ControlBarScheme, m_commandBarBorderColor) },
    { "ButtonBorderActionColor", &INI::Parse_Color_Int, nullptr, offsetof(ControlBarScheme, m_borderActionColor) },
    { "ButtonBorderUpgradeColor", &INI::Parse_Color_Int, nullptr, offsetof(ControlBarScheme, m_borderUpgradeColor) },
    { "ButtonBorderSystemColor", &INI::Parse_Color_Int, nullptr, offsetof(ControlBarScheme, m_borderSystemColor) },
    { "OptionsButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_optionsButtonEnableImage) },
    { "OptionsButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_optionsButtonHilitedImage) },
    { "OptionsButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_optionsButtonPushedImage) },
    { "OptionsButtonDisabled",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_optionsButtonDisabledImage) },
    { "IdleWorkerButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_idleWorkerButtonEnableImage) },
    { "IdleWorkerButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_idleWorkerButtonHilitedImage) },
    { "IdleWorkerButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_idleWorkerButtonPushedImage) },
    { "IdleWorkerButtonDisabled",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_idleWorkerButtonDisabledImage) },
    { "BuddyButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_buddyButtonEnableImage) },
    { "BuddyButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_buddyButtonHilitedImage) },
    { "BuddyButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_buddyButtonPushedImage) },
    { "BuddyButtonDisabled",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_buddyButtonDisabledImage) },
    { "BeaconButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_beaconButtonEnableImage) },
    { "BeaconButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_beaconButtonHilitedImage) },
    { "BeaconButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_beaconButtonPushedImage) },
    { "BeaconButtonDisabled",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_beaconButtonDisabledImage) },
    { "GenBarButtonIn", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarScheme, m_genBarButtonInImage) },
    { "GenBarButtonOn", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarScheme, m_genBarButtonOnImage) },
    { "ToggleButtonUpIn",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_toggleButtonUpInImage) },
    { "ToggleButtonUpOn",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_toggleButtonUpOnImage) },
    { "ToggleButtonUpPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_toggleButtonUpPushedImage) },
    { "ToggleButtonDownIn",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_toggleButtonDownInImage) },
    { "ToggleButtonDownOn",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_toggleButtonDownOnImage) },
    { "ToggleButtonDownPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_toggleButtonDownPushedImage) },
    { "GeneralButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_generalButtonEnableImage) },
    { "GeneralButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_generalButtonHilitedImage) },
    { "GeneralButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_generalButtonPushedImage) },
    { "GeneralButtonDisabled",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_generalButtonDisabledImage) },
    { "UAttackButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_uAttackButtonEnableImage) },
    { "UAttackButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_uAttackButtonHilitedImage) },
    { "UAttackButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_uAttackButtonPushedImage) },
    { "GenArrow", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarScheme, m_genArrowImage) },
    { "MinMaxButtonEnable",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_minMaxButtonEnableImage) },
    { "MinMaxButtonHightlited",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_minMaxButtonHilitedImage) },
    { "MinMaxButtonPushed",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_minMaxButtonPushedImage) },
    { "MinMaxUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_minMaxUL) },
    { "MinMaxLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_minMaxLR) },
    { "GeneralUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_generalUL) },
    { "GeneralLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_generalLR) },
    { "UAttackUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_uAttackUL) },
    { "UAttackLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_uAttackLR) },
    { "OptionsUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_optionsUL) },
    { "OptionsLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_optionsLR) },
    { "WorkerUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_workerUL) },
    { "WorkerLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_workerLR) },
    { "ChatUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_chatUL) },
    { "ChatLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_chatLR) },
    { "BeaconUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_beaconUL) },
    { "BeaconLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_beaconLR) },
    { "PowerBarUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_powerBarUL) },
    { "PowerBarLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_powerBarLR) },
    { "MoneyUL", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_moneyUL) },
    { "MoneyLR", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarScheme, m_moneyLR) },
    { "CommandMarkerImage",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_commandMarkerImage) },
    { "ExpBarForegroundImage",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_expBarForegroundImage) },
    { "PowerPurchaseImage",
        &ImageCollection::Parse_Mapped_Image,
        nullptr,
        offsetof(ControlBarScheme, m_powerPurchaseImage) },
    { nullptr, nullptr, nullptr, 0 }
};

ControlBarSchemeManager::ControlBarSchemeManager() : m_currentScheme(nullptr)
{
    m_multiplyer.x = 1.0f;
    m_multiplyer.y = 1.0f;
}

ControlBarSchemeManager::~ControlBarSchemeManager()
{
    for (auto it = m_schemeList.begin(); it != m_schemeList.end(); it++) {
        if (*it != nullptr) {
            delete *it;
        }
    }

    m_schemeList.clear();
    m_currentScheme = nullptr;
}

void ControlBarSchemeManager::Parse_Image_Part(INI *ini, void *instance, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = {
        { "Position", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarSchemeImage, m_position) },
        { "Size", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarSchemeImage, m_size) },
        { "ImageName", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarSchemeImage, m_image) },
        { "Layer", &INI::Parse_Int, nullptr, offsetof(ControlBarSchemeImage, m_layer) },
        { nullptr, nullptr, nullptr, 0 }
    };

    ControlBarSchemeImage *image = new ControlBarSchemeImage;
    ini->Init_From_INI(image, myFieldParse);
    static_cast<ControlBarScheme *>(instance)->Add_Image(image);
}

void ControlBarSchemeManager::Parse_Animating_Part_Image(INI *ini, void *instance, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = {
        { "Position", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarSchemeImage, m_position) },
        { "Size", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarSchemeImage, m_size) },
        { "ImageName", &ImageCollection::Parse_Mapped_Image, nullptr, offsetof(ControlBarSchemeImage, m_image) },
        { "Layer", &INI::Parse_Int, nullptr, offsetof(ControlBarSchemeImage, m_layer) },
        { nullptr, nullptr, nullptr, 0 }
    };

    ControlBarSchemeImage *image = new ControlBarSchemeImage;
    ini->Init_From_INI(image, myFieldParse);
    static_cast<ControlBarSchemeAnimation *>(instance)->m_animImage = image;
}

static const LookupListRec s_animTypeNames[] = { { "SLIDE_RIGHT", ControlBarSchemeAnimation::CB_ANIM_SLIDE_RIGHT },
    { nullptr, 0 } };

void ControlBarSchemeManager::Parse_Animating_Part(INI *ini, void *instance, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = {
        { "Name", &INI::Parse_AsciiString, nullptr, offsetof(ControlBarSchemeAnimation, m_name) },
        { "Animation", &INI::Parse_Lookup_List, s_animTypeNames, offsetof(ControlBarSchemeAnimation, m_animType) },
        { "Duration", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(ControlBarSchemeAnimation, m_animDuration) },
        { "FinalPos", &INI::Parse_ICoord2D, nullptr, offsetof(ControlBarSchemeAnimation, m_finalPos) },
        { "ImagePart", &ControlBarSchemeManager::Parse_Animating_Part_Image, nullptr, 0 },
        { nullptr, nullptr, nullptr, 0 }
    };

    ControlBarSchemeAnimation *anim = new ControlBarSchemeAnimation;
    ini->Init_From_INI(anim, myFieldParse);
    static_cast<ControlBarScheme *>(instance)->Add_Animation(anim);
    static_cast<ControlBarScheme *>(instance)->Add_Image(anim->m_animImage);
}

ControlBarScheme *ControlBarSchemeManager::New_Control_Bar_Scheme(Utf8String name)
{
    ControlBarScheme *scheme = Find_Control_Bar_Scheme(name);

    if (scheme != nullptr) {
        captainslog_dbgassert("We're overwriting a previous control bar scheme %s", name.Str());
        scheme->Reset();
        scheme->m_name.Set(name);
        scheme->m_name.To_Lower();
        return scheme;
    } else {
        scheme = new ControlBarScheme();

        if (scheme != nullptr && !name.Is_Empty()) {
            scheme->m_name.Set(name);
            scheme->m_name.To_Lower();
            m_schemeList.push_back(scheme);
            return scheme;
        } else {
            captainslog_dbgassert(false, "Could not create controlbar %s", name.Str());
        }

        return nullptr;
    }
}

ControlBarScheme *ControlBarSchemeManager::Find_Control_Bar_Scheme(Utf8String name)
{
    name.To_Lower();

    for (auto it = m_schemeList.begin(); it != m_schemeList.end(); it++) {
        ControlBarScheme *scheme = *it;

        if (scheme == nullptr) {
            break;
        }

        if (scheme->m_name.Compare_No_Case(name) == 0) {
            return scheme;
        }
    }

    captainslog_debug("There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList");
    return nullptr;
}

void ControlBarSchemeManager::Preload_Assets(TimeOfDayType time_of_day)
{
    for (auto it = m_schemeList.begin(); it != m_schemeList.end(); it++) {
        ControlBarScheme *scheme = *it;

        if (scheme != nullptr) {
            if (scheme->m_buttonQueueImage != nullptr) {
                g_theDisplay->Preload_Texture_Assets(scheme->m_buttonQueueImage->Get_File_Name());
            }

            if (scheme->m_rightHUDImage != nullptr) {
                g_theDisplay->Preload_Texture_Assets(scheme->m_rightHUDImage->Get_File_Name());
            }

            for (int i = 0; i < LAYER_COUNT; i++) {
                for (auto it2 = scheme->m_layer[i].begin(); it2 != scheme->m_layer[i].end(); it2++) {
                    ControlBarSchemeImage *scheme_image = *it2;

                    if (scheme_image != nullptr) {
                        Image *image = g_theMappedImageCollection->Find_Image_By_Name(scheme_image->m_name);

                        if (image != nullptr) {
                            g_theDisplay->Preload_Texture_Assets(image->Get_File_Name());
                        }
                    }
                }
            }
        } else {
            captainslog_dbgassert(false, "There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList");
        }
    }
}

void ControlBarSchemeManager::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Default\\ControlBarScheme.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data\\INI\\ControlBarScheme.ini", INI_LOAD_OVERWRITE, nullptr);
    captainslog_dbgassert(m_schemeList.size() != 0,
        "There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList that was just read from the INI file");
}

void ControlBarSchemeManager::Set_Control_Bar_Scheme(Utf8String scheme_name)
{
    ControlBarScheme *scheme = Find_Control_Bar_Scheme(scheme_name);

    if (scheme != nullptr) {
        m_multiplyer.x = g_theDisplay->Get_Width() / scheme->m_screenCreationRes.x;
        m_multiplyer.y = g_theDisplay->Get_Width() / scheme->m_screenCreationRes.y;
        m_currentScheme = scheme;
    } else {
        captainslog_dbgassert(false, "There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList");
    }

    if (m_currentScheme != nullptr) {
        m_currentScheme->Init();
    }
}

void ControlBarSchemeManager::Update()
{
    if (m_currentScheme != nullptr) {
        m_currentScheme->Update();
    }
}

void ControlBarSchemeManager::Draw_Background(ICoord2D offset)
{
    if (m_currentScheme != nullptr) {
        m_currentScheme->Draw_Background(m_multiplyer, offset);
    }
}

void ControlBarSchemeManager::Draw_Foreground(ICoord2D offset)
{
    if (m_currentScheme != nullptr) {
        m_currentScheme->Draw_Foreground(m_multiplyer, offset);
    }
}

void ControlBarSchemeManager::Set_Control_Bar_Scheme_By_Player_Template(const PlayerTemplate *player_template, bool small)
{
    if (player_template != nullptr) {
        Utf8String side = player_template->Get_Side();

        if (small) {
            side.Concat("Small");
        }

        if (m_currentScheme == nullptr || m_currentScheme->m_side.Compare(side) != 0) {
            if (side.Is_Empty()) {
                side.Set("Observer");
            }

            captainslog_debug("Set_Control_Bar_Scheme_By_Player_Template used %s as its side", side.Str());
            ControlBarScheme *new_scheme = nullptr;

            for (auto it = m_schemeList.begin(); it != m_schemeList.end(); it++) {
                ControlBarScheme *scheme = *it;

                if (scheme != nullptr) {
                    if (scheme->m_side.Compare_No_Case(side) == 0
                        && (new_scheme == nullptr || new_scheme->m_screenCreationRes.x < scheme->m_screenCreationRes.x)) {
                        new_scheme = scheme;
                    }
                } else {
                    captainslog_dbgassert(false, "There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList");
                }
            }

            if (new_scheme != nullptr) {
                m_multiplyer.x = (float)g_theDisplay->Get_Width() / (float)new_scheme->m_screenCreationRes.x;
                m_multiplyer.y = (float)g_theDisplay->Get_Height() / (float)new_scheme->m_screenCreationRes.y;
                m_currentScheme = new_scheme;
            } else {
                m_currentScheme = Find_Control_Bar_Scheme("Default");
                captainslog_debug("There's no ControlBarScheme with a side of %s", side.Str());
            }

            if (m_currentScheme != nullptr) {
                m_currentScheme->Init();
            }
        } else {
            m_currentScheme->Init();
            captainslog_debug("Set_Control_Bar_Scheme_By_Player_Template already is using %s as its side", side.Str());
        }
    }
}

void ControlBarSchemeManager::Set_Control_Bar_Scheme_By_Player(Player *player)
{
    GameWindow *button = g_theWindowManager->Win_Get_Window_From_Id(
        nullptr, g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:PopupCommunicator"));

    if (button != nullptr && g_theControlBar != nullptr) {
        if (g_theRecorder->Is_Multiplayer()) {
            g_theControlBar->Set_Control_Command(button, g_theControlBar->Find_Command_Button("NonCommand_Communicator"));
        } else {
            g_theControlBar->Set_Control_Command(button, g_theControlBar->Find_Command_Button("NonCommand_BriefingHistory"));
        }
    }

    if (player != nullptr) {
        Utf8String side = player->Get_Side();

        if (m_currentScheme == nullptr || m_currentScheme->m_side.Compare(side) != 0) {
            if (side.Is_Empty()) {
                side.Set("Observer");
            }

            captainslog_debug("Set_Control_Bar_Scheme_By_Player used %s as its side", side.Str());
            ControlBarScheme *new_scheme = nullptr;

            for (auto it = m_schemeList.begin(); it != m_schemeList.end(); it++) {
                ControlBarScheme *scheme = *it;

                if (scheme != nullptr) {
                    if (scheme->m_side.Compare_No_Case(side) == 0
                        && (new_scheme == nullptr || new_scheme->m_screenCreationRes.x < scheme->m_screenCreationRes.x)) {
                        new_scheme = scheme;
                    }
                } else {
                    captainslog_dbgassert(false, "There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList");
                }
            }

            if (new_scheme != nullptr) {
                m_multiplyer.x = (float)g_theDisplay->Get_Width() / (float)new_scheme->m_screenCreationRes.x;
                m_multiplyer.y = (float)g_theDisplay->Get_Height() / (float)new_scheme->m_screenCreationRes.y;
                m_currentScheme = new_scheme;
            } else {
                m_currentScheme = Find_Control_Bar_Scheme("Default");
                captainslog_debug("There's no ControlBarScheme with a side of %s", side.Str());
            }

            if (m_currentScheme != nullptr) {
                m_currentScheme->Init();
            }
        } else {
            m_currentScheme->Init();
            captainslog_debug("Set_Control_Bar_Scheme_By_Player already is using %s as its side", side.Str());
        }
    }
}
