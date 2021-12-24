/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "energy.h"
#include "audiomanager.h"
#include "gamelogic.h"
#include "ini.h"
#include "object.h"
#include "player.h"
#include "playerlist.h"

Energy::Energy() : m_energyProduction(0), m_energyConsumption(0), m_frame(0), m_player(nullptr) {}

void Energy::Init(Player *player)
{
    m_energyProduction = 0;
    m_energyConsumption = 0;
    m_player = player;
    // TODO investigate, should this also clear m_frame?
}

int Energy::Get_Production() const
{
    if (g_theGameLogic->Get_Frame() < m_frame) {
        return 0;
    }
    return m_energyProduction;
}

float Energy::Get_Energy_Supply_Ratio() const
{
    captainslog_dbgassert(m_energyProduction >= 0 && m_energyConsumption >= 0, "Negative Energy numbers!");

    if (g_theGameLogic->Get_Frame() < m_frame) {
        return 0.0f;
    }

    if (m_energyConsumption != 0) {
        return (float)m_energyProduction / (float)m_energyConsumption;
    }

    return (float)m_energyProduction;
}

bool Energy::Has_Sufficient_Power() const
{
    if (g_theGameLogic->Get_Frame() < m_frame) {
        return false;
    }

    return m_energyProduction >= m_energyConsumption;
}

void Energy::Adjust_Power(int amount, bool positive)
{
    if (amount != 0) {

        if (amount > 0) {

            if (positive) {
                Add_Production(amount);
            } else {
                Add_Production(-amount);
            }

        } else {

            if (positive) {
                Add_Consumption(-amount);
            } else {
                Add_Consumption(amount);
            }
        }
    }
}

void Energy::Object_Entering_Influence(Object *obj)
{
    if (obj != nullptr) {

        int amount = obj->Get_Template()->Get_Energy_Production();

        if (amount < 0) {
            Add_Consumption(-amount);
        } else if (amount > 0) {
            Add_Production(amount);
        }

        captainslog_dbgassert(m_energyProduction >= 0 && m_energyConsumption >= 0,
            "Energy - Negative Energy numbers, Production = %d Consumption = %d",
            m_energyProduction,
            m_energyConsumption);
    }
}

void Energy::Object_Leaving_Influence(Object *obj)
{
    if (obj != nullptr) {

        int amount = obj->Get_Template()->Get_Energy_Production();

        if (amount < 0) {
            Add_Consumption(amount);
        } else if (amount > 0) {
            Add_Production(-amount);
        }

        captainslog_dbgassert(m_energyProduction >= 0 && m_energyConsumption >= 0,
            "Energy - Negative Energy numbers, Production = %d Consumption = %d",
            m_energyProduction,
            m_energyConsumption);
    }
}

void Energy::Add_Power_Bonus(Object *obj)
{
    if (obj != nullptr) {

        int bonus = obj->Get_Template()->Get_Energy_Bonus();

        Add_Production(bonus);

        captainslog_dbgassert(m_energyProduction >= 0 && m_energyConsumption >= 0,
            "Energy - Negative Energy numbers, Production = %d Consumption = %d",
            m_energyProduction,
            m_energyConsumption);
    }
}

void Energy::Remove_Power_Bonus(Object *obj)
{
    if (obj != nullptr) {

        int bonus = obj->Get_Template()->Get_Energy_Bonus();

        Add_Production(-bonus);

        captainslog_dbgassert(m_energyProduction >= 0 && m_energyConsumption >= 0,
            "Energy - Negative Energy numbers, Production = %d Consumption = %d",
            m_energyProduction,
            m_energyConsumption);
    }
}

void Energy::Add_Production(int amount)
{
    m_energyProduction += amount;

    if (m_player != nullptr) {
        m_player->On_Power_Brown_Out_Change(Has_Sufficient_Power() == 0);
    }
}

void Energy::Add_Consumption(int amount)
{
    m_energyConsumption += amount;

    if (m_player != nullptr) {
        m_player->On_Power_Brown_Out_Change(Has_Sufficient_Power() == 0);
    }
}

void Energy::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 3;
    xfer->xferVersion(&version, 3);

    if (version < 2) {
        xfer->xferInt(&m_energyProduction);
        xfer->xferInt(&m_energyConsumption);
    }

    int index;

    if (xfer->Get_Mode() == XFER_SAVE) {
        index = m_player->Get_Player_Index();
    }

    xfer->xferInt(&index);

    m_player = g_thePlayerList->Get_Nth_Player(index);

    if (version >= 3) {
        xfer->xferUnsignedInt(&m_frame);
    }
}
