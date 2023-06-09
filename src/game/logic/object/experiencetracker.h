/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief ExperienceTracker
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"

class Object;

class ExperienceTracker : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(ExperienceTracker, ExperienceTrackerPool);

public:
    // zh: 0x0061B620 wb: 0x008A34F0
    ExperienceTracker(Object *parent) :
        m_parent(parent),
        m_currentLevel(VeterancyLevel::VETERANCY_REGULAR),
        m_currentExperience(0),
        m_experienceSink(ObjectID::INVALID_OBJECT_ID),
        m_experienceMultiplier(1.0f)
    {
    }

    // zh: 0x0061B730 wb: 0x008A3584
    virtual ~ExperienceTracker() override {}

    int32_t Get_Experience_Value(const Object *killer) const;
    bool Is_Trainable() const;
    bool Is_Accepting_Experience_Points() const;
    // zh: 0x0061B810 wb: 0x008A368E
    void Set_Experience_Sink(ObjectID sink) { m_experienceSink = sink; }
    void Set_Min_Veterency_Level(VeterancyLevel new_level);
    void Set_Veterency_Level(VeterancyLevel new_level, bool unk);
    bool Gain_Exp_For_Level(int32_t levels_to_gain, bool apply_multiplier);
    bool Can_Gain_Exp_For_Level(int32_t levels_to_gain) const;
    void Add_Experience_Points(int32_t experience_gain, bool apply_multiplier);
    void Set_Experience_And_Level(int32_t experience_gain, bool unk);
    VeterancyLevel Get_Current_Level() const { return m_currentLevel; }

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    Object *m_parent;
    VeterancyLevel m_currentLevel;
    int32_t m_currentExperience;
    ObjectID m_experienceSink;
    float m_experienceMultiplier; // ExperienceScalarUpgrade
};
