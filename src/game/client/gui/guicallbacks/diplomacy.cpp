/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Diplomacy
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "diplomacy.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

void Toggle_Diplomacy(bool immediate)
{
#ifdef GAME_DLL
    Call_Function<void, bool>(PICK_ADDRESS(0x005EEF60, 0x00A11E14), immediate);
#endif
}

void Update_Diplomacy_Briefing_Text(Utf8String text, bool clear)
{
#ifdef GAME_DLL
    Call_Function<void, Utf8String, bool>(PICK_ADDRESS(0x005EE4D0, 0x00A11298), text, clear);
#endif
}

std::list<Utf8String> *Get_Briefing_Text_List()
{
#ifdef GAME_DLL
    return Call_Function<std::list<Utf8String> *>(PICK_ADDRESS(0x005EE4C0, 0x00A1128E));
#else
    return nullptr;
#endif
}
