/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Battle Plan Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "battleplanupdate.h"

BattlePlanStatus BattlePlanUpdate::Get_Active_Battle_Plan()
{
    if (m_transitionStatus == TRANSITION_STATUS_IDLE) {
        return m_activeBattlePlan;
    } else {
        return BATTLE_PLAN_STATUS_NONE;
    }
}
