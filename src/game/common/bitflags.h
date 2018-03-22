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

template<int bits>
class BitFlags
{
public:
    bool operator==(BitFlags &that) const { return m_bits == that.m_bits; }
    bool operator!=(BitFlags &that) const { return m_bits != that.m_bits; }

    void Clear() { m_bits.reset(); }
    void Set(unsigned bit) { m_bits.set(bit); }
    bool Get(unsigned bit) { return m_bits.test(bit); }

    void Parse(INI *ini, AsciiString *string = nullptr);
    static void Parse_INI(INI *ini, void *formal, void *store, const void *user_data);

    static const char *s_bitNamesList[bits + 1];

private:
    std::bitset<bits> m_bits;
};

template<int bits>
const char *BitFlags<bits>::s_bitNamesList[bits + 1];

template<int bits>
void BitFlags<bits>::Parse(INI *ini, AsciiString *string)
{
    if (string != nullptr) {
        string->Clear();
    }

    const char *token = ini->Get_Next_Token_Or_Null();
    bool adjust = false;
    bool set = false;

    if (token != nullptr) {
        while (strcasecmp(token, "NONE") != 0) {
            // If we have a passed in AsciiString, add the tokens to it as we parse them.
            if (string != nullptr) {
                if (string->Is_Not_Empty()) {
                    *string += " ";
                }

                *string += token;
            }

            // If we have a plus or minus as first char, then that decides if bit is set or cleared.
            // Otherwise just set.
            if (*token == '+') {
                ASSERT_THROW_PRINT(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                m_bits.set(INI::Scan_IndexList(token + 1, s_bitNamesList));
                adjust = true;
            } else if (*token == '-') {
                ASSERT_THROW_PRINT(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                m_bits.reset(INI::Scan_IndexList(token + 1, s_bitNamesList));
                adjust = true;
            } else {
                ASSERT_THROW_PRINT(!adjust,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to set when we already adjusted.\n",
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
        ASSERT_THROW_PRINT(!adjust && !set,
            0xDEAD0006,
            "File: '%s', Line: %d Trying to clear when we already set or adjusted.\n",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        m_bits.reset();
    }
}

template<int bits>
void BitFlags<bits>::Parse_INI(INI *ini, void *formal, void *store, const void *user_data)
{
    static_cast<BitFlags<bits> *>(store)->Parse(ini, nullptr);
}
