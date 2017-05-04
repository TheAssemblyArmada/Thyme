////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: CONFIG.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Basic header files and defines that are always needed.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef BASE_CONFIG_H
#define BASE_CONFIG_H

////////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
////////////////////////////////////////////////////////////////////////////////
#include "platform.h"
#include "compiler.h"


//Forces use of INT_MAX macros where from stdint.h on some compilers
#ifndef __STDC_LIMIT_MACROS
    #define __STDC_LIMIT_MACROS 1
#endif // __STDC_LIMIT_MACROS

#endif // _BASE_CONFIG_H_
