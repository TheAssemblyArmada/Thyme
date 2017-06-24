////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMESTATE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Game state tracker.
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
#include "gamestate.h"
#include "filetransfer.h"
#include "globaldata.h"
#include "maputil.h"

GameState::GameState()
{

}

GameState::~GameState()
{
}

void GameState::Init()
{
}

void GameState::Reset()
{
}

void GameState::Xfer_Snapshot(Xfer *xfer)
{
}

AsciiString GameState::Get_Save_Dir()
{
    AsciiString ret = g_theWriteableGlobalData->m_userDataDirectory;
    ret += "Save/";

    return ret;
}

AsciiString GameState::Real_To_Portable_Map_Path(AsciiString &path)
{
    AsciiString ret;

    path.Fix_Path();  

    if ( path.Starts_With_No_Case(Get_Save_Dir().Str()) ) {
        ret = "Save/";
        ret += Get_File_From_Path(path);
    } else {
        if ( path.Starts_With_No_Case(TheMapCache->Get_Map_Dir().Str()) ) {
            ret = "Maps/";
            ret += Get_Leaf_And_Dir_Name(path);
        } else {
            if ( path.Starts_With_No_Case(TheMapCache->Get_User_Map_Dir().Str()) ) {
                ret = "UserData/Maps/";
                ret += Get_Leaf_And_Dir_Name(path);
            } else {
                ret = path;
            }
        }
    }

    ret.To_Lower();

    return ret;
}

AsciiString GameState::Portable_To_Real_Map_Path(AsciiString &path)
{
    AsciiString ret;

    path.Fix_Path();

    if ( path.Starts_With_No_Case("Save/") ) {
        ret = Get_Save_Dir();
        ret += Get_File_From_Path(path);
    } else if ( path.Starts_With_No_Case("Maps/") ) {
        ret = TheMapCache->Get_Map_Dir();
        ret += "/";
        ret += Get_Leaf_And_Dir_Name(path);
    } else if ( path.Starts_With_No_Case("UserData/Maps/") ) {
        ret = TheMapCache->Get_User_Map_Dir();
        ret += "/";
        ret += Get_Leaf_And_Dir_Name(path);
    } else {
        ret = path;
    }

    ret.To_Lower();

    return ret;
}

// Returns file and last two containing directories from path in the form "path/to/file"
AsciiString Get_Leaf_And_Dir_Name(AsciiString const &path)
{
    int len = path.Get_Length();
    const char *str = path.Str();
    const char *leaf = &str[len - 1];

    // Search backwards for first directory
    while ( leaf > str ) {
        if ( *leaf == '\\' || *leaf == '/' ) {
            break;
        }

        --leaf;
    }

    if ( leaf == nullptr || leaf <= str ) {
        return path;
    }

    // Search backwards from last pos for the second directory.
    while ( leaf > str ) {
        if ( *leaf == '\\' || *leaf == '/' ) {
            break;
        }

        --leaf;
    }

    if ( leaf == nullptr || leaf <= str ) {
        return path;
    }

    return AsciiString(++leaf);
}
