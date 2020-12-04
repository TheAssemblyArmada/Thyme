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

#include "always.h"
#include "asciistring.h"
#include "ini.h"
#include <bitset>

// TODO move this somewhere more appropriate?
// Don't add or remove anything to these until all code accessing the BitFlags type
// for these enums have been reimplemented.
enum ObjectStatusTypes
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

enum DisabledType
{
    DISABLED_TYPE_DEFAULT,
    DISABLED_TYPE_DISABLED_HACKED,
    DISABLED_TYPE_DISABLED_EMP,
    DISABLED_TYPE_DISABLED_HELD,
    DISABLED_TYPE_DISABLED_PARALYZED,
    DISABLED_TYPE_DISABLED_UNMANNED,
    DISABLED_TYPE_DISABLED_UNDERPOWERED,
    DISABLED_TYPE_DISABLED_FREEFALL,
    DISABLED_TYPE_DISABLED_AWESTRUCK,
    DISABLED_TYPE_DISABLED_BRAINWASHED,
    DISABLED_TYPE_DISABLED_SUBDUED,
    DISABLED_TYPE_DISABLED_SCRIPT_DISABLED,
    DISABLED_TYPE_DISABLED_SCRIPT_UNDERPOWERED,
    DISABLED_TYPE_COUNT,
};

enum ModelConditionFlagType
{
    MODELCONDITION_TOPPLED,
    MODELCONDITION_FRONTCRUSHED,
    MODELCONDITION_BACKCRUSHED,
    MODELCONDITION_DAMAGED,
    MODELCONDITION_REALLYDAMAGED,
    MODELCONDITION_RUBBLE,
    MODELCONDITION_SPECIAL_DAMAGED,
    MODELCONDITION_NIGHT,
    MODELCONDITION_SNOW,
    MODELCONDITION_PARACHUTING,
    MODELCONDITION_GARRISONED,
    MODELCONDITION_ENEMYNEAR,
    MODELCONDITION_WEAPONSET_VETERAN,
    MODELCONDITION_WEAPONSET_ELITE,
    MODELCONDITION_WEAPONSET_HERO,
    MODELCONDITION_WEAPONSET_CRATEUPGRADE_ONE,
    MODELCONDITION_WEAPONSET_CRATEUPGRADE_TWO,
    MODELCONDITION_WEAPONSET_PLAYER_UPGRADE,
    MODELCONDITION_DOOR_1_OPENING,
    MODELCONDITION_DOOR_1_CLOSING,
    MODELCONDITION_DOOR_1_WAITING_OPEN,
    MODELCONDITION_DOOR_1_WAITING_TO_CLOSE,
    MODELCONDITION_DOOR_2_OPENING,
    MODELCONDITION_DOOR_2_CLOSING,
    MODELCONDITION_DOOR_2_WAITING_OPEN,
    MODELCONDITION_DOOR_2_WAITING_TO_CLOSE,
    MODELCONDITION_DOOR_3_OPENING,
    MODELCONDITION_DOOR_3_CLOSING,
    MODELCONDITION_DOOR_3_WAITING_OPEN,
    MODELCONDITION_DOOR_3_WAITING_TO_CLOSE,
    MODELCONDITION_DOOR_4_OPENING,
    MODELCONDITION_DOOR_4_CLOSING,
    MODELCONDITION_DOOR_4_WAITING_OPEN,
    MODELCONDITION_DOOR_4_WAITING_TO_CLOSE,
    MODELCONDITION_ATTACKING,
    MODELCONDITION_PREATTACK_A,
    MODELCONDITION_FIRING_A,
    MODELCONDITION_BETWEEN_FIRING_SHOTS_A,
    MODELCONDITION_RELOADING_A,
    MODELCONDITION_PREATTACK_B,
    MODELCONDITION_FIRING_B,
    MODELCONDITION_BETWEEN_FIRING_SHOTS_B,
    MODELCONDITION_RELOADING_B,
    MODELCONDITION_PREATTACK_C,
    MODELCONDITION_FIRING_C,
    MODELCONDITION_BETWEEN_FIRING_SHOTS_C,
    MODELCONDITION_RELOADING_C,
    MODELCONDITION_TURRET_ROTATE,
    MODELCONDITION_POST_COLLAPSE,
    MODELCONDITION_MOVING,
    MODELCONDITION_DYING,
    MODELCONDITION_AWAITING_CONSTRUCTION,
    MODELCONDITION_PARTIALLY_CONSTRUCTED,
    MODELCONDITION_ACTIVELY_BEING_CONSTRUCTED,
    MODELCONDITION_PRONE,
    MODELCONDITION_FREEFALL,
    MODELCONDITION_ACTIVELY_CONSTRUCTING,
    MODELCONDITION_CONSTRUCTION_COMPLETE,
    MODELCONDITION_RADAR_EXTENDING,
    MODELCONDITION_RADAR_UPGRADED,
    MODELCONDITION_PANICKING,
    MODELCONDITION_AFLAME,
    MODELCONDITION_SMOLDERING,
    MODELCONDITION_BURNED,
    MODELCONDITION_DOCKING,
    MODELCONDITION_DOCKING_BEGINNING,
    MODELCONDITION_DOCKING_ACTIVE,
    MODELCONDITION_DOCKING_ENDING,
    MODELCONDITION_CARRYING,
    MODELCONDITION_FLOODED,
    MODELCONDITION_LOADED,
    MODELCONDITION_JETAFTERBURNER,
    MODELCONDITION_JETEXHAUST,
    MODELCONDITION_PACKING,
    MODELCONDITION_UNPACKING,
    MODELCONDITION_DEPLOYED,
    MODELCONDITION_OVER_WATER,
    MODELCONDITION_POWER_PLANT_UPGRADED,
    MODELCONDITION_CLIMBING,
    MODELCONDITION_SOLD,
    MODELCONDITION_RAPPELLING,
    MODELCONDITION_ARMED,
    MODELCONDITION_POWER_PLANT_UPGRADING,
    MODELCONDITION_SPECIAL_CHEERING,
    MODELCONDITION_CONTINUOUS_FIRE_SLOW,
    MODELCONDITION_CONTINUOUS_FIRE_MEAN,
    MODELCONDITION_CONTINUOUS_FIRE_FAST,
    MODELCONDITION_RAISING_FLAG,
    MODELCONDITION_CAPTURED,
    MODELCONDITION_EXPLODED_FLAILING,
    MODELCONDITION_EXPLODED_BOUNCING,
    MODELCONDITION_SPLATTED,
    MODELCONDITION_USING_WEAPON_A,
    MODELCONDITION_USING_WEAPON_B,
    MODELCONDITION_USING_WEAPON_C,
    MODELCONDITION_PREORDER,
    MODELCONDITION_CENTER_TO_LEFT,
    MODELCONDITION_LEFT_TO_CENTER,
    MODELCONDITION_CENTER_TO_RIGHT,
    MODELCONDITION_RIGHT_TO_CENTER,
    MODELCONDITION_RIDER1,
    MODELCONDITION_RIDER2,
    MODELCONDITION_RIDER3,
    MODELCONDITION_RIDER4,
    MODELCONDITION_RIDER5,
    MODELCONDITION_RIDER6,
    MODELCONDITION_RIDER7,
    MODELCONDITION_RIDER8,
    MODELCONDITION_STUNNED_FLAILING,
    MODELCONDITION_STUNNED,
    MODELCONDITION_SECOND_LIFE,
    MODELCONDITION_JAMMED,
    MODELCONDITION_ARMORSET_CRATEUPGRADE_ONE,
    MODELCONDITION_ARMORSET_CRATEUPGRADE_TWO,
    MODELCONDITION_USER_1,
    MODELCONDITION_USER_2,
    MODELCONDITION_DISGUISED,
    MODELCONDITION_COUNT,
    MODELCONDITION_INVALID = 0xFF,
};

template<int bits> class BitFlags
{
public:
    bool operator==(BitFlags &that) const { return m_bits == that.m_bits; }
    bool operator!=(BitFlags &that) const { return m_bits != that.m_bits; }

    void Clear() { m_bits.reset(); }
    void Set(unsigned bit) { m_bits.set(bit); }
    bool Get(unsigned bit) const { return m_bits.test(bit); }
    bool Any() const { return m_bits.any(); }

    void Parse(INI *ini, Utf8String *string = nullptr);
    static void Parse_INI(INI *ini, void *formal, void *store, const void *user_data);

    static const char *s_bitNamesList[bits + 1];

private:
    std::bitset<bits> m_bits;
};

// template<int bits>
// const char *BitFlags<bits>::s_bitNamesList[bits + 1];

template<int bits> void BitFlags<bits>::Parse(INI *ini, Utf8String *string)
{
    if (string != nullptr) {
        string->Clear();
    }

    const char *token = ini->Get_Next_Token_Or_Null();
    bool adjust = false;
    bool set = false;

    if (token != nullptr) {
        while (strcasecmp(token, "NONE") != 0) {
            // If we have a passed in Utf8String, add the tokens to it as we parse them.
            if (string != nullptr) {
                if (string->Is_Not_Empty()) {
                    *string += " ";
                }

                *string += token;
            }

            // If we have a plus or minus as first char, then that decides if bit is set or cleared.
            // Otherwise just set.
            if (*token == '+') {
                captainslog_relassert(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                m_bits.set(INI::Scan_IndexList(token + 1, s_bitNamesList));
                adjust = true;
            } else if (*token == '-') {
                captainslog_relassert(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                m_bits.reset(INI::Scan_IndexList(token + 1, s_bitNamesList));
                adjust = true;
            } else {
                captainslog_relassert(!adjust,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to set when we already adjusted.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                // If we haven't started setting yet, 0 it out.
                if (!set) {
                    m_bits.reset();
                }

                m_bits.set(INI::Scan_IndexList(token, s_bitNamesList));
                set = true;
            }

            token = ini->Get_Next_Token_Or_Null();

            if (token == nullptr) {
                return;
            }
        }

        // If we encounter a "NONE" entry, set all bits to 0.
        // We should never reach here if something has already been set.
        captainslog_relassert(!adjust && !set,
            0xDEAD0006,
            "File: '%s', Line: %d Trying to clear when we already set or adjusted.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        m_bits.reset();
    }
}

template<int bits> void BitFlags<bits>::Parse_INI(INI *ini, void *formal, void *store, const void *user_data)
{
    static_cast<BitFlags<bits> *>(store)->Parse(ini, nullptr);
}
