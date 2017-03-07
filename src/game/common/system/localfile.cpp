////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: LOCALFILE.CPP
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
#include "localfile.h"
#include "ramfile.h"

int LocalFile::TotalOpen = 0;

void *LocalFile::Read_All_And_Close()
{
    int size = Size();
    uint8_t *data;

    if ( size > 0 ) {
        DEBUG_LOG("Reading %s and closing.\n", FileName.Str());
        data = new uint8_t[size];

        Read(data, size);
        Close();
    } else {
        // Calling function is responsible for delete so just alloc
        // 1 byte and return if no data.
        data = new uint8_t[1];
    }

    return data;
}

File *LocalFile::Convert_To_RAM()
{
    DEBUG_LOG("Converting %s to RAMFile.\n", FileName.Str());
    RAMFile *ramfile = new RAMFile;

    if ( ramfile->Open(this) ) {
        if ( DeleteOnClose ) {
            ramfile->Set_Del_On_Close(true);
            Close();

            return ramfile;
        } else {
            Close();
            Delete_Instance(this);

            return ramfile;
        }
    }
        
    ramfile->Close();
    Delete_Instance(ramfile);

    return this;
}
