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
#include "gadgetstatictext.h"
#include "gametext.h"
#include "gamewindowmanager.h"

void ControlBar::Update_Context_Under_Construction()
{
    Object *object = m_currentSelectedDrawable->Get_Object();

    if (object->Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
        if (object->Get_Construction_Percent() != m_displayedConstructPercent) {
            Update_Construction_Text_Display(object);
        }
    } else {
        Evaluate_Context_UI();
    }
}

void ControlBar::Update_Construction_Text_Display(Object *obj)
{
    Utf16String text;
    static NameKeyType desc_id = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:UnderConstructionDesc");
    GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(nullptr, desc_id);
    captainslog_dbgassert(window != nullptr, "Under construction window not found");
    text.Format(g_theGameText->Fetch("CONTROLBAR:UnderConstructionDesc"), obj->Get_Construction_Percent());
    Gadget_Static_Text_Set_Text(window, text);
    m_displayedConstructPercent = obj->Get_Construction_Percent();
}

void ControlBar::Populate_Under_Construction(Object *object_under_construction)
{
    if (object_under_construction != nullptr) {
        const CommandButton *button = Find_Command_Button("Command_CancelConstruction");
        GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(
            m_contextParent[5], g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonCancelConstruction"));
        Set_Control_Command(window, button);
        window->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
        Update_Construction_Text_Display(object_under_construction);
        ExitInterface *exit = object_under_construction->Get_Object_Exit_Interface();

        if (exit != nullptr) {
            Show_Rally_Point(exit->Get_Rally_Point());
        }
    }
}
