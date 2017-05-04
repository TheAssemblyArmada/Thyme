////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MONEY.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base class for handling in game money tracking.
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

#ifndef MONEY_H
#define MONEY_H

#include "snapshot.h"

class Money : public SnapShot
{
public:
    Money() : m_money(0), m_playerIndex(0) {}
    virtual ~Money() {}

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}
protected:
    unsigned int m_money;
    int m_playerIndex;
};

#endif