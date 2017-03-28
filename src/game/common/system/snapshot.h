////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SNAPSHOT.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: TODO
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

#ifndef _SNAPSHOT_H_
#define _SNAPSHOT_H_

#include "xfer.h"

class SnapShot
{
    virtual void CRC(Xfer *xfer) = 0;
    virtual void Xfer(Xfer *xfer) = 0;
    virtual void Load_Post_Process() = 0;
};

#endif
