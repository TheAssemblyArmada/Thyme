////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: STRINGEX.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Some extra string manipulation functions not present in
//                 standard CRTs
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

#ifndef _STRINGEX_H_
#define _STRINGEX_H_

#ifdef __cplusplus
extern "C" {
#endif

inline char16_t *strcpy16(char16_t *dst, char16_t const *src)
{
    char16_t *tmp = dst;
    while ( (*tmp++ = *src++) );

    return dst;
}

inline size_t strlen16(char16_t const *str)
{
    if ( !str ) {
        return 0;
    }

    size_t len = 0;
    while ( str[len] ) {
        ++len;
    }

    return len;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif
