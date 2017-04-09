////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: FILECLASS.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: Interface for WW3D file io.
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
#include	"fileclass.h"

int FileClass::Tell(void)
{
    return Seek(0, FS_SEEK_CURRENT);
}

time_t FileClass::Get_Date_Time(void)
{
    return 0;
}

bool FileClass::Set_Date_Time(time_t date_time)
{
    return false;
}

int FileClass::Get_File_Handle(void)
{
    return -1;
}
