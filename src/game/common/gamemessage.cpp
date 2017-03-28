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
#include "playerlist.h"

GameMessage::GameMessage(MessageType type) :
    m_next(nullptr),
    m_prev(nullptr),
    m_list(nullptr),
    m_type(MSG_INVALID),
    m_playerIndex(ThePlayerList->Get_Local_Player()->Get_Player_Index()),  //ThePlayerList->m_local->m_playerIndex
    m_argCount(0),
    m_argList(nullptr),
    m_argTail(nullptr)
{
    
}

GameMessage::~GameMessage()
{
    GameMessageArgument *argobj = m_argList;

    while ( argobj != nullptr ) {
        GameMessageArgument *tmp = argobj;
        argobj = argobj->m_next;
        Delete_Instance(tmp);
    }

    if ( m_list != nullptr ) {
        m_list->Remove_Message(this);
    }
}

GameMessageArgument *GameMessage::Allocate_Arg()
{
    GameMessageArgument *arg = new GameMessageArgument;

    if ( m_argTail != nullptr ) {
        m_argTail->m_next = arg; 
    } else {
        m_argList = arg;
    }

    arg->m_next = nullptr;
    m_argTail = arg;
    ++m_argCount;

    return arg;
}

ArgumentType *GameMessage::Get_Argument(int arg)
{
    static ArgumentType junkconst;

    GameMessageArgument *argobj = m_argList;
    int i = 0;

    while ( argobj != nullptr ) {
        if ( i == arg ) {
            return &argobj->m_data;
        }
        
        ++i;
        argobj = argobj->m_next;
    }

    return &junkconst;
}

ArgumentDataType GameMessage::Get_Argument_Type(int arg)
{
    if ( arg >= m_argCount ) {
        return ARGUMENTDATATYPE_UNKNOWN;
    }

    GameMessageArgument *argobj = m_argList;

    for ( int i = 0; i < arg; ++i ) {
        if ( argobj == nullptr ) {
            return ARGUMENTDATATYPE_UNKNOWN;
        }

        argobj = argobj->m_next;
    }

    if ( argobj == nullptr ) {
        return ARGUMENTDATATYPE_UNKNOWN;
    }

    return argobj->m_type;
}

void GameMessage::Append_Int_Arg(int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.integer = arg;
    argobj->m_type = ARGUMENTDATATYPE_INTEGER;
}

void GameMessage::Append_Real_Arg(float arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.real = arg;
    argobj->m_type = ARGUMENTDATATYPE_REAL;
}

void GameMessage::Append_Bool_Arg(bool arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.boolean = arg;
    argobj->m_type = ARGUMENTDATATYPE_BOOLEAN;
}

void GameMessage::Append_ObjectID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.objectID = arg;
    argobj->m_type = ARGUMENTDATATYPE_OBJECTID;
}

void GameMessage::Append_DrawableID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.drawableID = arg;
    argobj->m_type = ARGUMENTDATATYPE_DRAWABLEID;
}

void GameMessage::Append_TeamID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.teamID = arg;
    argobj->m_type = ARGUMENTDATATYPE_TEAMID;
}

void GameMessage::Append_Location_Arg(Coord3D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.position = arg;
    argobj->m_type = ARGUMENTDATATYPE_LOCATION;
}

void GameMessage::Append_Pixel_Arg(ICoord2D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.pixel = arg;
    argobj->m_type = ARGUMENTDATATYPE_PIXEL;
}

void GameMessage::Append_Region_Arg(IRegion2D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.region = arg;
    argobj->m_type = ARGUMENTDATATYPE_PIXELREGION;
}

void GameMessage::Append_Time_Stamp_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.timestamp = arg;
    argobj->m_type = ARGUMENTDATATYPE_TIMESTAMP;
}

void GameMessage::Append_Wide_Char_Arg(wchar_t arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.widechar = arg;
    argobj->m_type = ARGUMENTDATATYPE_WIDECHAR;
}
