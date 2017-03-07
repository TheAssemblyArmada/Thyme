////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: LOCALFILE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Cross platform class for file IO.
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

#ifndef _LOCALFILE_H_
#define _LOCALFILE_H_

#include "always.h"
#include "file.h"

class LocalFile : public File
{
public:
    virtual void *Read_All_And_Close();
    virtual File *Convert_To_RAM();

protected:
    static int TotalOpen;
};

#endif // _LOCALFILE_H_
