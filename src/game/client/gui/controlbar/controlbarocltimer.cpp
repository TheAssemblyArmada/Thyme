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
#include "controlbar.h"
#include "gadgetprogressbar.h"
#include "gadgetstatictext.h"
#include "gametext.h"
#include "gamewindow.h"
#include "gamewindowmanager.h"
#include "oclupdate.h"

void ControlBar::Update_Context_OCL_Timer()
{
    Object *object = m_currentSelectedDrawable->Get_Object();
    static const NameKeyType key_OCLUpdate = g_theNameKeyGenerator->Name_To_Key("OCLUpdate");
    OCLUpdate *ocl = static_cast<OCLUpdate *>(object->Find_Update_Module(key_OCLUpdate));
    unsigned int remaining_frames = ocl->Get_Remaining_Frames() / 30;
    float countdown = ocl->Get_Countdown_Percent();

    if (m_oclTimerFrame != remaining_frames) {
        Update_OCL_Timer_Text_Display(remaining_frames, countdown);
    }
}

void ControlBar::Populate_OCL_Timer(Object *obj)
{
    if (obj != nullptr) {
        GameWindow *button = g_theWindowManager->Win_Get_Window_From_Id(
            m_contextParent[8], g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:OCLTimerSellButton"));

        if (obj->Is_KindOf(KINDOF_TECH_BUILDING)) {
            if (obj->Is_KindOf(KINDOF_TECH_BUILDING) && obj->Is_KindOf(KINDOF_AUTO_RALLYPOINT)) {
                const CommandButton *command = Find_Command_Button("Command_SetRallyPoint");
                Set_Control_Command(button, command);
                button->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
                ExitInterface *exit = obj->Get_Object_Exit_Interface();

                if (exit != nullptr) {
                    Show_Rally_Point(exit->Get_Rally_Point());
                }
            } else {
                button->Win_Hide(true);
            }
        } else {
            const CommandButton *command = Find_Command_Button("Command_Sell");
            Set_Control_Command(button, command);
            button->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
        }

        Update_Context_OCL_Timer();
        Set_Portrait_By_Object(obj);
    }
}

void ControlBar::Update_OCL_Timer_Text_Display(unsigned int remaining_frames, float progress)
{
    Utf16String str;
    static const NameKeyType descID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:OCLTimerStaticText");
    GameWindow *desc = g_theWindowManager->Win_Get_Window_From_Id(nullptr, descID);
    static const NameKeyType barID = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:OCLTimerProgressBar");
    GameWindow *bar = g_theWindowManager->Win_Get_Window_From_Id(nullptr, barID);
    captainslog_dbgassert(desc != nullptr, "Under construction window not found");

    if ((remaining_frames % 60) >= 10) {
        str.Format(g_theGameText->Fetch("CONTROLBAR:OCLTimerDesc"), remaining_frames / 60, remaining_frames % 60);
    } else {
        str.Format(g_theGameText->Fetch("CONTROLBAR:OCLTimerDescWithPadding"), remaining_frames / 60, remaining_frames % 60);
    }

    Gadget_Static_Text_Set_Text(desc, str);
    Gadget_Progress_Bar_Set_Progress(bar, progress * 100.0f);
    m_oclTimerFrame = remaining_frames;
}
