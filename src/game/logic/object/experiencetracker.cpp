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
#include "experiencetracker.h"
#include "gamelogic.h"
#include "object.h"
#include "xfer.h"

// zh: 0x0061B790 wb: 0x008A35F9
int32_t ExperienceTracker::Get_Experience_Value(const Object *killer) const
{
    if (killer->Get_Relationship(m_parent) == Relationship::ALLIES) {
        return 0;
    }

    return m_parent->Get_Template()->Get_Experience_Value(m_currentLevel);
}

// zh: 0x0061B7D0 wb: 0x008A3637
bool ExperienceTracker::Is_Trainable() const
{
    return m_parent->Get_Template()->Is_Trainable();
}

// zh: 0x0061B7E0 wb: 0x008A3654
bool ExperienceTracker::Is_Accepting_Experience_Points() const
{
    return Is_Trainable() || m_experienceSink != ObjectID::INVALID_OBJECT_ID;
}

// zh: 0x0061B820 wb: 0x008A36A4
void ExperienceTracker::Set_Min_Veterency_Level(VeterancyLevel new_level)
{
    if (m_currentLevel >= new_level) {
        return;
    }

    captainslog_dbgassert(m_parent != nullptr, "Parent must not be nullptr!");

    const auto old_level = m_currentLevel;
    m_currentLevel = new_level;

    m_currentExperience = m_parent->Get_Template()->Get_Experience_Required(m_currentLevel);

    m_parent->On_Veterancy_Level_Changed(old_level, m_currentLevel, true);
}

// zh: 0x0061B860 wb: 0x008A370D
void ExperienceTracker::Set_Veterency_Level(VeterancyLevel new_level, bool unk)
{
    if (m_currentLevel == new_level) {
        return;
    }

    captainslog_dbgassert(m_parent != nullptr, "Parent must not be nullptr!");

    const auto old_level = m_currentLevel;
    m_currentLevel = new_level;

    m_currentExperience = m_parent->Get_Template()->Get_Experience_Required(m_currentLevel);

    m_parent->On_Veterancy_Level_Changed(old_level, m_currentLevel, unk);
}

// zh: 0x0061B8A0 wb: 0x008A377A
bool ExperienceTracker::Gain_Exp_For_Level(int32_t levels_to_gain, bool apply_multiplier)
{
    const auto new_level = std::min<VeterancyLevel>(m_currentLevel + levels_to_gain, VeterancyLevel::VETERANCY_COUNT - 1);

    if (new_level <= m_currentLevel) {
        return false;
    }

    const auto exp_for_level_gain = m_parent->Get_Template()->Get_Experience_Required(new_level) - m_currentExperience;

    Add_Experience_Points(exp_for_level_gain, apply_multiplier);
    return true;
}

// zh: 0x0061B8F0 wb: 0x008A37E4
bool ExperienceTracker::Can_Gain_Exp_For_Level(int32_t levels_to_gain) const
{
    return m_currentLevel != std::min<VeterancyLevel>(m_currentLevel + levels_to_gain, VeterancyLevel::VETERANCY_COUNT - 1);
}

// zh: 0x0061B910 wb: 0x008A381C
void ExperienceTracker::Add_Experience_Points(int32_t experience_gain, bool apply_multiplier)
{
    if (m_experienceSink != ObjectID::INVALID_OBJECT_ID) {
        auto *sink_object = g_theGameLogic->Find_Object_By_ID(m_experienceSink);
        if (sink_object != nullptr) {
            sink_object->Get_Experience_Tracker()->Add_Experience_Points(
                experience_gain * m_experienceMultiplier, apply_multiplier);
            return;
        }
    }

    if (!Is_Trainable()) {
        return;
    }

    const auto old_level = m_currentLevel;
    if (apply_multiplier) {
        experience_gain *= m_experienceMultiplier;
    }

    m_currentExperience += experience_gain;
    VeterancyLevel new_level = VeterancyLevel::VETERANCY_REGULAR;
    // Loop until the penultimate VeterancyLevel as it reaches saturation
    for (; new_level < VeterancyLevel::VETERANCY_COUNT - 1; ++new_level) {
        if (m_currentExperience < m_parent->Get_Template()->Get_Experience_Required(new_level + 1)) {
            break;
        }
    }

    m_currentLevel = new_level;
    if (old_level != m_currentLevel) {
        m_parent->On_Veterancy_Level_Changed(old_level, m_currentLevel, true);
    }
}

// zh: 0x0061B9D0 wb: 0x008A3934
void ExperienceTracker::Set_Experience_And_Level(int32_t experience_gain, bool unk)
{
    if (m_experienceSink != ObjectID::INVALID_OBJECT_ID) {
        auto *sink_object = g_theGameLogic->Find_Object_By_ID(m_experienceSink);
        if (sink_object != nullptr) {
            sink_object->Get_Experience_Tracker()->Set_Experience_And_Level(experience_gain, unk);
            return;
        }
    }

    if (!Is_Trainable()) {
        return;
    }

    const auto old_level = m_currentLevel;

    m_currentExperience = experience_gain;

    VeterancyLevel new_level = VeterancyLevel::VETERANCY_REGULAR;
    // Loop until the penultimate VeterancyLevel as it reaches saturation
    for (; new_level < VeterancyLevel::VETERANCY_COUNT - 1; ++new_level) {
        if (m_currentExperience < m_parent->Get_Template()->Get_Experience_Required(new_level + 1)) {
            break;
        }
    }

    m_currentLevel = new_level;
    if (old_level != m_currentLevel) {
        m_parent->On_Veterancy_Level_Changed(old_level, m_currentLevel, unk);
    }
}

// zh: 0x0061BA70 wb: 0x008A3A1B
void ExperienceTracker::CRC_Snapshot(Xfer *xfer)
{
    xfer->xferInt(&m_currentExperience);
    xfer->xferUser(&m_currentLevel, sizeof(m_currentLevel));
}

// zh: 0x0061BAA0 wb: 0x008A3A51
void ExperienceTracker::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUser(&m_currentLevel, sizeof(m_currentLevel));
    xfer->xferInt(&m_currentExperience);
    xfer->xferObjectID(&m_experienceSink);
    xfer->xferReal(&m_experienceMultiplier);
}
