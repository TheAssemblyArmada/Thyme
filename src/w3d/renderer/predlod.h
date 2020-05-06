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
#pragma once

#include "always.h"

class PredictiveLODOptimizerClass
{
public:
    static void Add_Cost(float cost) { s_TotalCost += cost; }

private:
#ifdef GAME_DLL
    static float &s_TotalCost;
#else
    static float s_TotalCost;
#endif
};