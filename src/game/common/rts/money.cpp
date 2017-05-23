////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MONEY.CPP
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
#include "money.h"
#include "ini.h"

void Money::Xfer_Snapshot(Xfer *xfer)
{
}

void Money::Parse_Money_Amount(INI *ini, void *formal, void *store, void const *user_data)
{
    INI::Parse_Unsigned(ini, formal, &(static_cast<Money*>(store)->m_money), user_data);
}
