////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: COMMANDLIST.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Command message list handling.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _COMMANDLIST_H_
#define _COMMANDLIST_H_

#include "gamemessagelist.h"

class CommandList : public GameMessageList
{
public:
    // SubsystemInterface implementations
    virtual ~CommandList();
    virtual void Init() {}
    virtual void Reset();
    virtual void Update() {}

    static void Hook_Me();
private:
    void Destroy_All_Messages();
    void Append_Message_List(GameMessage *list);
};

inline void CommandList::Hook_Me()
{
    // This one actually replaces Reset as it is just inlined Destroy_All_Messages
    Hook_Method((Make_Method_Ptr<void, CommandList>(0x0040DD70)), &Destroy_All_Messages);
}

#endif
