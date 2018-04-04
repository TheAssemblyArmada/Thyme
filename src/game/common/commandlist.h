/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Command message list handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "gamemessagelist.h"

class CommandList : public GameMessageList
{
public:
    // SubsystemInterface implementations
    virtual ~CommandList();
    virtual void Init() {}
    virtual void Reset();
    virtual void Update() {}

    void Append_Message_List(GameMessage *list);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif
private:
    void Destroy_All_Messages();
};

#ifndef THYME_STANDALONE
extern CommandList *&g_theCommandList;

inline void CommandList::Hook_Me()
{
    // This one actually replaces Reset as it is just inlined Destroy_All_Messages
    Hook_Method(0x0040DD70, &Destroy_All_Messages);
}
#else
extern CommandList *g_theCommandList;
#endif