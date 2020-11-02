/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Predictive LOD Optimizer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "predlod.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
float PredictiveLODOptimizerClass::s_TotalCost;
#endif

// 0x00808FE0
void PredictiveLODOptimizerClass::Free()
{
#ifdef GAME_DLL
    Call_Function<void>(0x00808FE0);
#endif
}
