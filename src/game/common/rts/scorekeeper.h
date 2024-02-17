/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Class for keeping score of a player.
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
#include "bitflags.h"
#include "gametype.h"
#include "snapshot.h"
#include <map>

class Object;
class ThingTemplate;

typedef std::map<const ThingTemplate *, int32_t> ScoreKeeperCountMap;

class ScoreKeeper : public SnapShot
{
public:
    enum
    {
        UNITS_BUILT_MULTIPLIER = 100,
        UNITS_DESTROYED_MULTIPLIER = 100,
        BUILDINGS_BUILT_MULTIPLIER = 100,
        BUILDINGS_DESTROYED_MULTIPLIER = 100,
        SUPPLIES_COLLECTED_MULTIPLIER = 1,
    };

    ScoreKeeper();
    ~ScoreKeeper() {}

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Reset(int player_index);

    int Calculate_Score();

    void Add_Money_Earned(int earned) { m_totalMoneyEarned += earned; }
    void Add_Money_Spent(int spent) { m_totalMoneySpent += spent; }

    void Add_Object_Built(const Object *object);
    void Remove_Object_Built(const Object *object);

    int Get_Total_Units_Built(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);
    int Get_Total_Objects_Built(const ThingTemplate *tmpl);

    void Add_Object_Captured(const Object *object);
    void Add_Object_Destroyed(const Object *object);
    void Add_Object_Lost(const Object *object);

    int Get_Total_Money_Earned() const { return m_totalMoneyEarned; }
    int Get_Total_Money_Spent() const { return m_totalMoneySpent; }

    int Get_Total_Units_Destroyed() const;
    int Get_Total_Units_Built() const { return m_totalUnitsBuilt; }
    int Get_Total_Units_Lost() const { return m_totalUnitsLost; }

    int Get_Total_Buildings_Destroyed() const;
    int Get_Total_Buildings_Built() const { return m_totalBuildingsBuilt; }
    int Get_Total_Buildings_Lost() const { return m_totalBuildingsLost; }

    int Get_Total_Tech_Buildings_Captured() const { return m_totalTechBuildingsCaptured; }
    int Get_Total_Scored_Buildings_Captured() const { return m_totalScoredBuildingsCaptured; }

private:
    void Xfer_Object_Count_Map(Xfer *xfer, ScoreKeeperCountMap *count_map);

private:
    int32_t m_totalMoneyEarned;
    int32_t m_totalMoneySpent;

    int32_t m_totalUnitsDestroyed[MAX_PLAYER_COUNT];
    int32_t m_totalUnitsBuilt;
    int32_t m_totalUnitsLost;

    int32_t m_totalBuildingsDestroyed[MAX_PLAYER_COUNT];
    int32_t m_totalBuildingsBuilt;
    int32_t m_totalBuildingsLost;

    int32_t m_totalTechBuildingsCaptured;
    int32_t m_totalScoredBuildingsCaptured;

    int32_t m_currentScore;

    int32_t m_playerIndex;

    ScoreKeeperCountMap m_objectsBuilt;
    ScoreKeeperCountMap m_objectsDestroyed[MAX_PLAYER_COUNT];
    ScoreKeeperCountMap m_objectsLost;
    ScoreKeeperCountMap m_objectsCaptured;
};
