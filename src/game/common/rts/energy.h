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
#pragma once

#include "always.h"
#include "snapshot.h"
#include <new>

class Object;
class Player;

class Energy : public SnapShot
{
public:
    Energy();

    virtual ~Energy() {}

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Init(Player *player);

    int Get_Production() const;

    float Get_Energy_Supply_Ratio() const;

    bool Has_Sufficient_Power() const;

    void Adjust_Power(int amount, bool positive);

    void Object_Entering_Influence(Object *obj);
    void Object_Leaving_Influence(Object *obj);

    void Add_Power_Bonus(Object *obj);
    void Remove_Power_Bonus(Object *obj);

    void Add_Production(int amount);
    void Add_Consumption(int amount);

    int Get_Consumption() const { return m_energyConsumption; }

    void Set_Frame(unsigned int frame) { m_frame = frame; }
    unsigned int Get_Frame() const { return m_frame; }

#ifdef GAME_DLL
    Energy *Hook_Ctor() { return new (this) Energy; }
#endif

private:
    int m_energyProduction;
    int m_energyConsumption;
    unsigned int m_frame;
    Player *m_player;
};
