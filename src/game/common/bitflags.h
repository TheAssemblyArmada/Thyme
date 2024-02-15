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
#include "xfer.h"
#include <bitset>

template<int> class BitFlags;

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

    OBJECT_STATUS_COUNT
};

using ObjectStatusBitFlags = BitFlags<OBJECT_STATUS_COUNT>;

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
    MODELCONDITION_INVALID = -1,
};

using ModelConditionBitFlags = BitFlags<MODELCONDITION_COUNT>;

template<int bits> class BitFlags
{
public:
    enum BogusInitType
    {
        kInit = 0,
    };

    BitFlags() {}
    BitFlags(BogusInitType type, int flag) { m_bits.set(flag); }
    BitFlags(BogusInitType type, int flag1, int flag2)
    {
        m_bits.set(flag1);
        m_bits.set(flag2);
    }

    BitFlags(BogusInitType type, int flag1, int flag2, int flag3)
    {
        m_bits.set(flag1);
        m_bits.set(flag2);
        m_bits.set(flag3);
    }

    BitFlags(BogusInitType type, int flag1, int flag2, int flag3, int flag4)
    {
        m_bits.set(flag1);
        m_bits.set(flag2);
        m_bits.set(flag3);
        m_bits.set(flag4);
    }

    BitFlags(BogusInitType type, int flag1, int flag2, int flag3, int flag4, int flag5)
    {
        m_bits.set(flag1);
        m_bits.set(flag2);
        m_bits.set(flag3);
        m_bits.set(flag4);
        m_bits.set(flag5);
    }

    bool operator==(const BitFlags &that) const { return m_bits == that.m_bits; }
    bool operator!=(const BitFlags &that) const { return m_bits != that.m_bits; }

    void Clear() { m_bits.reset(); }
    void Set(int bit, bool value) { m_bits.set(bit, value); }
    bool Test(int bit) const { return m_bits.test(bit); }
    bool Any() const { return m_bits.any(); }
    void Flip() { m_bits.flip(); }
    int Count() const { return m_bits.count(); }
    int Size() const { return m_bits.size(); }

    void Clear(BitFlags const &clear) { m_bits &= ~clear.m_bits; }
    void Set(BitFlags const &set) { m_bits |= set.m_bits; }
    void Set_All()
    {
        Clear();
        Flip();
    }

    void Clear_And_Set(BitFlags const &clear, BitFlags const &set)
    {
        m_bits &= ~clear.m_bits;
        m_bits |= set.m_bits;
    }

    int Count_Intersection(BitFlags const &count) const
    {
        std::bitset<bits> temp = m_bits;
        temp &= count.m_bits;
        return temp.count();
    }

    int Count_Inverse_Intersection(BitFlags const &count) const
    {
        std::bitset<bits> temp = m_bits;
        temp.flip();
        temp &= count.m_bits;
        return temp.count();
    }

    bool Test_Set_And_Clear(BitFlags const &set, BitFlags const &clear) const
    {
        std::bitset<bits> temp = m_bits;
        temp &= clear.m_bits;

        if (temp.any()) {
            return false;
        }

        temp = m_bits;
        temp.flip();
        temp &= set.m_bits;
        return temp.any() == false;
    }

    bool Any_Intersection_With(BitFlags const &set) const
    {
        std::bitset<bits> temp = m_bits;
        temp &= set.m_bits;
        return temp.any();
    }

    bool No_Intersection_With(BitFlags const &set) const
    {
        std::bitset<bits> temp = m_bits;
        temp &= set.m_bits;
        return !temp.any();
    }

    void Parse(INI *ini, Utf8String *string = nullptr);

    void Xfer(Xfer *xfer)
    {
        uint8_t version = 1;
        xfer->xferVersion(&version, 1);

        if (xfer->Get_Mode() == XFER_SAVE) {
            int count = Count();
            xfer->xferInt(&count);

            for (int i = 0; i < Size(); i++) {
                const char *str = Get_Name_For_Bit(i);

                if (str != nullptr) {
                    Utf8String s(str);
                    xfer->xferAsciiString(&s);
                }
            }
        } else if (xfer->Get_Mode() == XFER_LOAD) {
            Clear();
            int count;
            xfer->xferInt(&count);
            Utf8String str;

            for (int i = 0; i < count; i++) {
                xfer->xferAsciiString(&str);

                if (!Set_Bit_By_Name(str.Str())) {
                    captainslog_error("invalid bit name %s", str.Str());
                    throw XFER_STATUS_READ_ERROR;
                }
            }
        } else if (xfer->Get_Mode() == XFER_CRC) {
            xfer->xferUser(this, 4);
        } else {
            captainslog_error("BitFlagsXfer - Unknown xfer mode '%d'", xfer->Get_Mode());
            throw XFER_STATUS_UNKNOWN_XFER_MODE;
        }
    }

    bool Test_For_All(BitFlags const &set) const
    {
        captainslog_dbgassert(
            set.Any(), "BitFlags::testForAll is always true if you ask about zero flags.  Did you mean that?");
        std::bitset<bits> temp = m_bits;
        temp.flip();
        temp &= set.m_bits;
        return !temp.any();
    }

    static int Get_Single_Bit_From_Name(const char *name);
    bool Set_Bit_By_Name(const char *name);

    const char *Get_Name_For_Bit(int bit) const
    {
        if (Test(bit)) {
            return s_bitNamesList[bit];
        }

        return nullptr;
    }

    void Get_Name_For_Bits(Utf8String *str) const
    {
        if (str != nullptr) {
            for (int i = 0; i < Size(); i++) {
                const char *name = Get_Name_For_Bit(i);

                if (name != nullptr) {
                    str->Concat(name);
                    str->Concat(",\n");
                }
            }
        }
    }

    static void Parse_From_INI(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Single_Bit_From_INI(INI *ini, void *formal, void *store, const void *user_data);
    static const char **Get_Bit_Names_List() { return s_bitNamesList; }

    static BitFlags Set_Bit(BitFlags &flags, int bit)
    {
        flags.Set(bit, 1);
        return flags;
    }

    static BitFlags Clear_Bit(BitFlags &flags, int bit)
    {
        flags.Set(bit, 0);
        return flags;
    }

    static const char *Bit_As_String(int bit)
    {
        if (bit < 0 || bit >= bits) {
            return nullptr;
        } else {
            return s_bitNamesList[bit];
        }
    }

private:
    static const char *s_bitNamesList[bits + 1];
    std::bitset<bits> m_bits;
};

// template<int bits>
// const char *BitFlags<bits>::s_bitNamesList[bits + 1];

template<int bits> void BitFlags<bits>::Parse(INI *ini, Utf8String *string)
{
    if (string != nullptr) {
        string->Clear();
    }

    bool set = false;
    bool adjust = false;

    for (const char *token = ini->Get_Next_Token_Or_Null(); token != nullptr; token = ini->Get_Next_Token_Or_Null()) {
        if (string != nullptr) {
            if (string->Is_Not_Empty()) {
                string->Concat(" ");
            }

            string->Concat(token);
        }

        if (!strcasecmp(token, "NONE")) {
            if (set || adjust) {
                captainslog_error("you may not mix normal and +- ops in bitstring lists");
                throw 0xDEAD0006;
            }

            Clear();
            return;
        }

        // If we have a plus or minus as first char, then that decides if bit is set or cleared.
        if (*token == '+') {
            if (set) {
                captainslog_error("you may not mix normal and +- ops in bitstring lists");
                throw 0xDEAD0006;
            }

            Set(INI::Scan_IndexList(token + 1, s_bitNamesList), 1);
            adjust = true;
        } else if (*token == '-') {
            if (set) {
                captainslog_error("you may not mix normal and +- ops in bitstring lists");
                throw 0xDEAD0006;
            }

            Set(INI::Scan_IndexList(token + 1, s_bitNamesList), 0);
            adjust = true;
        } else {
            if (adjust) {
                captainslog_error("you may not mix normal and +- ops in bitstring lists");
                throw 0xDEAD0006;
            }

            if (!set) {
                Clear();
            }

            Set(INI::Scan_IndexList(token, s_bitNamesList), 1);
            set = true;
        }
    }
}

template<int bits> void BitFlags<bits>::Parse_Single_Bit_From_INI(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<int *>(store) = INI::Scan_IndexList(ini->Get_Next_Token(), s_bitNamesList);
}

template<int bits> int BitFlags<bits>::Get_Single_Bit_From_Name(const char *name)
{
    for (int i = 0; s_bitNamesList[i] != nullptr; i++) {
        if (!strcasecmp(s_bitNamesList[i], name)) {
            return i;
        }
    }

    return -1;
}

template<int bits> bool BitFlags<bits>::Set_Bit_By_Name(const char *name)
{
    int bit = Get_Single_Bit_From_Name(name);
    if (bit < 0) {
        return false;
    }

    Set(bit, true);
    return true;
}

template<int bits> void BitFlags<bits>::Parse_From_INI(INI *ini, void *formal, void *store, const void *user_data)
{
    static_cast<BitFlags<bits> *>(store)->Parse(ini, nullptr);
}
