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
#pragma once

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

class Xfer;

enum SnapshotCode
{
    SNAPSHOT_NONE = 0,
    SNAPSHOT_OK = 1,
    NUM_SNAPSHOT_CODES,
};

class SnapShot
{
public:
    virtual void CRC_Snapshot(Xfer *xfer) = 0;
    virtual void Xfer_Snapshot(Xfer *xfer) = 0;
    virtual void Load_Post_Process() = 0;
};

#endif
