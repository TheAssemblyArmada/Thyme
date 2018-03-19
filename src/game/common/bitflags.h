/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper around std::bitset.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <bitset>

 // TODO move this somewhere more appropriate?
 // Don't add or remove anything to these until all code accessing the BitFlags type
 // for these enums have been reimplemented.
enum ObjectStatuses
{
    OBJECT_STATUS_NONE,
    OBJECT_STATUS_DESTROYED,
    OBJECT_STATUS_CAN_ATTACK,
    OBJECT_STATUS_UNDER_CONSTRUCTION,
    OBJECT_STATUS_UNSELECTABLE,
    OBJECT_STATUS_NO_COLLISIONS,
    OBJECT_STATUS_NO_ATTACK,
    OBJECT_STATUS_AIRBORNE_TARGET,
    OBJECT_STATUS_PARACHUTING,
    OBJECT_STATUS_REPULSOR,
    OBJECT_STATUS_HIJACKED,
    OBJECT_STATUS_AFLAME,
    OBJECT_STATUS_BURNED,
    OBJECT_STATUS_WET,
    OBJECT_STATUS_IS_FIRING_WEAPON,
    OBJECT_STATUS_IS_BRAKING,
    OBJECT_STATUS_STEALTHED,
    OBJECT_STATUS_DETECTED,
    OBJECT_STATUS_CAN_STEALTH,
    OBJECT_STATUS_SOLD,
    OBJECT_STATUS_UNDERGOING_REPAIR,
    OBJECT_STATUS_RECONSTRUCTING,
    OBJECT_STATUS_MASKED,
    OBJECT_STATUS_IS_ATTACKING,
    OBJECT_STATUS_USING_ABILITY,
    OBJECT_STATUS_IS_AIMING_WEAPON,
    OBJECT_STATUS_NO_ATTACK_FROM_AI,
    OBJECT_STATUS_IGNORING_STEALTH,
    OBJECT_STATUS_IS_CARBOMB,
    OBJECT_STATUS_DECK_HEIGHT_OFFSET,
    OBJECT_STATUS_STATUS_RIDER1,
    OBJECT_STATUS_STATUS_RIDER2,
    OBJECT_STATUS_STATUS_RIDER3,
    OBJECT_STATUS_STATUS_RIDER4,
    OBJECT_STATUS_STATUS_RIDER5,
    OBJECT_STATUS_STATUS_RIDER6,
    OBJECT_STATUS_STATUS_RIDER7,
    OBJECT_STATUS_STATUS_RIDER8,
    OBJECT_STATUS_FAERIE_FIRE,
    OBJECT_STATUS_KILLING_SELF,
    OBJECT_STATUS_REASSIGN_PARKING,
    OBJECT_STATUS_BOOBY_TRAPPED,
    OBJECT_STATUS_IMMOBILE,
    OBJECT_STATUS_DISGUISED,
    OBJECT_STATUS_DEPLOYED,
    OBJECT_STATUS_COUNT,
};

template <int bits>
class BitFlags
{
public:
    bool operator==(BitFlags &that) const { return m_bits == that.m_bits; }
    bool operator!=(BitFlags &that) const { return m_bits != that.m_bits; }

    void Set(unsigned bit) { m_bits.set(bit); }
    bool Get(unsigned bit) { return m_bits.test(bit); }

    static const char *s_bitNamesList[];
private:
    std::bitset<bits> m_bits;
};