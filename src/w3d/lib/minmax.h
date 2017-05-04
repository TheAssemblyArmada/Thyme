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

#ifndef MINMAX_H
#define MINMAX_H

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

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- Clamp() -->
///
/// \brief
///     Returns the maximum of 'minval' and 'maxval'.
///
/// \param
///     original    The original value to clamp.
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
template<class T>
T Clamp(T original, T minval, T maxval)
{
    if ( original < minval ) {
        return minval;
    }

    if ( original > maxval ) {
        return maxval;
    }

    return original;
};

#endif // _MINMAX_H
