////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MINMAX.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: 
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
#ifdef COMPILER_MSVC
#pragma once
#endif // COMPILER_MSVC

#ifndef _MINMAX_H_
#define _MINMAX_H_

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- MIN() -->
///
/// \brief
///     Returns the minimum of 'minval' and 'maxval'.
///
/// \param
///     minval      The lowest value to compare.
///
/// \param
///     maxval      The largest value to compare.
///
/// \return
///     value       The lesser of the values passed as arguments.
///
/// \warning
///     None
///
////////////////////////////////////////////////////////////////////////////////
template<typename T>
T MIN(T minval, T maxval)
{
    return minval < maxval ? minval : maxval;
}


////////////////////////////////////////////////////////////////////////////////
///
/// <!-- MAX() -->
///
/// \brief
///     Returns the maximum of 'minval' and 'maxval'.
///
/// \param
///     minval      The lowest value to compare.
///
/// \param
///     maxval      The largest value to compare.
///
/// \return
///     value       The largest of the values passed as arguments.
///
/// \warning
///     None
///
////////////////////////////////////////////////////////////////////////////////
template<typename T>
T MAX(T minval, T maxval)
{
    return maxval < minval ? minval : maxval;
}

#endif // _MINMAX_H_
