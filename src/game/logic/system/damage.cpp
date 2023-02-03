/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Damage
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "damage.h"
#include "thingfactory.h"

#ifndef GAME_DLL
BitFlags<DAMAGE_NUM_TYPES> DAMAGE_TYPE_FLAGS_ALL;
#endif

BitFlags<DAMAGE_NUM_TYPES> DAMAGE_TYPE_FLAGS_NONE;

void DamageInfoInput::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char current_version = 3;
    unsigned char version = 3;
    xfer->xferVersion(&version, current_version);
    xfer->xferObjectID(&m_sourceID);
    xfer->xferUser(&m_playerMask, sizeof(m_playerMask));
    xfer->xferUser(&m_damageType, sizeof(m_damageType));

    if (version >= 2) {
        xfer->xferUser(&m_unk1, sizeof(m_unk1));
    }

    xfer->xferUser(&m_deathType, sizeof(m_deathType));
    xfer->xferReal(&m_amount);

    if (current_version >= 2) {
        xfer->xferBool(&m_unk2);
    }

    xfer->xferUser(&m_damageStatusType, sizeof(m_damageStatusType));
    xfer->xferCoord3D(&m_shockWavePos);
    xfer->xferReal(&m_shockWaveAmount);
    xfer->xferReal(&m_shockWaveRadius);
    xfer->xferReal(&m_shockWaveTaperOff);

    if (version >= 3) {
        Utf8String name = Utf8String::s_emptyString;

        if (m_thingTemplate != nullptr) {
            name = m_thingTemplate->Get_Name();
        }

        xfer->xferAsciiString(&name);

        if (xfer->Get_Mode() == XFER_LOAD) {
            m_thingTemplate = g_theThingFactory->Find_Template(name, true);
        }
    }
}

void DamageInfoOutput::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferReal(&m_actualDamageDealt);
    xfer->xferReal(&m_actualDamageClipped);
    xfer->xferBool(&m_noEffect);
}

void DamageInfo::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferSnapshot(&m_in);
    xfer->xferSnapshot(&m_out);
}

void Init_Damage_Type_Masks()
{
    DAMAGE_TYPE_FLAGS_ALL.Set_All();
}
