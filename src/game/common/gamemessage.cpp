////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMESSAGE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Message object.
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
#include "gamemessage.h"
#include "gamemessagelist.h"

GameMessage::GameMessage(MessageType type) :
    Next(nullptr),
    Prev(nullptr),
    List(nullptr),
    Type(MSG_INVALID),
    PlayerIndex(0),  //ThePlayerList->m_local->m_playerIndex
    ArgCount(0),
    ArgList(nullptr),
    ArgTail(nullptr)
{
}

GameMessage::~GameMessage()
{
    GameMessageArgument *argobj = ArgList;

    while ( argobj != nullptr ) {
        GameMessageArgument *tmp = argobj;
        argobj = argobj->Next;
        Delete_Instance(tmp);
    }

    if ( List != nullptr ) {
        List->Remove_Message(this);
    }
}

GameMessageArgument *GameMessage::Allocate_Arg()
{
    GameMessageArgument *arg = new GameMessageArgument;

    if ( ArgTail != nullptr ) {
        ArgTail->Next = arg; 
    } else {
        ArgList = arg;
    }

    arg->Next = nullptr;
    ArgTail = arg;
    ++ArgCount;

    return arg;
}

ArgumentType *GameMessage::Get_Argument(int arg)
{
    static ArgumentType junkconst;

    GameMessageArgument *argobj = ArgList;
    int i = 0;

    while ( argobj != nullptr ) {
        if ( i == arg ) {
            return &argobj->Data;
        }
        
        ++i;
        argobj = argobj->Next;
    }

    return &junkconst;
}

ArgumentDataType GameMessage::Get_Argument_Type(int arg)
{
    if ( arg >= ArgCount ) {
        return ARGUMENTDATATYPE_UNKNOWN;
    }

    GameMessageArgument *argobj = ArgList;

    for ( int i = 0; i < arg; ++i ) {
        if ( argobj == nullptr ) {
            return ARGUMENTDATATYPE_UNKNOWN;
        }

        argobj = argobj->Next;
    }

    if ( argobj == nullptr ) {
        return ARGUMENTDATATYPE_UNKNOWN;
    }

    return argobj->Type;
}

void GameMessage::Append_Int_Arg(int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.integer = arg;
    argobj->Type = ARGUMENTDATATYPE_INTEGER;
}

void GameMessage::Append_Real_Arg(float arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.real = arg;
    argobj->Type = ARGUMENTDATATYPE_REAL;
}

void GameMessage::Append_Bool_Arg(bool arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.boolean = arg;
    argobj->Type = ARGUMENTDATATYPE_BOOLEAN;
}

void GameMessage::Append_ObjectID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.objectID = arg;
    argobj->Type = ARGUMENTDATATYPE_OBJECTID;
}

void GameMessage::Append_DrawableID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.drawableID = arg;
    argobj->Type = ARGUMENTDATATYPE_DRAWABLEID;
}

void GameMessage::Append_TeamID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.teamID = arg;
    argobj->Type = ARGUMENTDATATYPE_TEAMID;
}

void GameMessage::Append_Location_Arg(Coord3D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.position = arg;
    argobj->Type = ARGUMENTDATATYPE_LOCATION;
}

void GameMessage::Append_Pixel_Arg(ICoord2D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.pixel = arg;
    argobj->Type = ARGUMENTDATATYPE_PIXEL;
}

void GameMessage::Append_Region_Arg(IRegion2D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.region = arg;
    argobj->Type = ARGUMENTDATATYPE_PIXELREGION;
}

void GameMessage::Append_Time_Stamp_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.timestamp = arg;
    argobj->Type = ARGUMENTDATATYPE_TIMESTAMP;
}

void GameMessage::Append_Wide_Char_Arg(wchar_t arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->Data.widechar = arg;
    argobj->Type = ARGUMENTDATATYPE_WIDECHAR;
}
