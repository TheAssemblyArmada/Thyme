/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Some data transfer interface?
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bitflags.h"
#include "color.h"
#include "endiantype.h"
#include "gamestate.h"
#include "matrix3d.h"
#include "randomvalue.h"
#include "science.h"
#include "upgrade.h"
#include <captainslog.h>

void Xfer::Open(Utf8String filename)
{
    m_filename = filename;
}

void Xfer::xferVersion(uint8_t *thing, uint8_t check)
{
    xferImplementation(thing, sizeof(*thing));

    captainslog_relassert(
        *thing <= check, XFER_STATUS_UNKNOWN_VERSION, "Xfer version %d greater than expected, %d.", *thing, check);
}

void Xfer::xferByte(int8_t *thing)
{
    xferImplementation(thing, sizeof(*thing));
}

void Xfer::xferUnsignedByte(uint8_t *thing)
{
    xferImplementation(thing, sizeof(*thing));
}

void Xfer::xferBool(bool *thing)
{
    xferImplementation(thing, sizeof(*thing));
}

void Xfer::xferInt(int32_t *thing)
{
    int32_t temp = htole32(*thing);
    xferImplementation(&temp, sizeof(temp));
    *thing = le32toh(temp);
}

void Xfer::xferInt64(int64_t *thing)
{
    int64_t temp = htole64(*thing);
    xferImplementation(&temp, sizeof(temp));
    *thing = le64toh(temp);
}

void Xfer::xferUnsignedInt(uint32_t *thing)
{
    uint32_t temp = htole32(*thing);
    xferImplementation(&temp, sizeof(temp));
    *thing = le32toh(temp);
}

void Xfer::xferShort(int16_t *thing)
{
    int16_t temp = htole16(*thing);
    xferImplementation(&temp, sizeof(temp));
    *thing = le16toh(temp);
}

void Xfer::xferUnsignedShort(uint16_t *thing)
{
    int16_t temp = htole16(*thing);
    xferImplementation(&temp, sizeof(temp));
    *thing = le16toh(temp);
}

void Xfer::xferReal(float *thing)
{
    static_assert(sizeof(float_int_tp) == sizeof(uint32_t), "Type punning union size is incorrect.");
    float_int_tp temp;
    temp.real = *thing;
    temp.integer = htole32(temp.integer);
    xferImplementation(&temp, sizeof(temp));
    temp.integer = le32toh(temp.integer);
    *thing = temp.real;
}

void Xfer::xferMarkerLabel(Utf8String thing)
{
    // Empty
}

void Xfer::xferAsciiString(Utf8String *thing)
{
    xferImplementation(const_cast<char *>(thing->Str()), thing->Get_Length());
}

void Xfer::xferUnicodeString(Utf16String *thing)
{
    xferImplementation(const_cast<unichar_t *>(thing->Str()), thing->Get_Length() * 2);
}

void Xfer::xferCoord3D(Coord3D *thing)
{
    xferReal(&thing->x);
    xferReal(&thing->y);
    xferReal(&thing->z);
}

void Xfer::xferICoord3D(ICoord3D *thing)
{
    xferInt(&thing->x);
    xferInt(&thing->y);
    xferInt(&thing->z);
}

void Xfer::xferRegion3D(Region3D *thing)
{
    xferCoord3D(&thing->lo);
    xferCoord3D(&thing->hi);
}

void Xfer::xferIRegion3D(IRegion3D *thing)
{
    xferICoord3D(&thing->lo);
    xferICoord3D(&thing->hi);
}

void Xfer::xferCoord2D(Coord2D *thing)
{
    xferReal(&thing->x);
    xferReal(&thing->y);
}

void Xfer::xferICoord2D(ICoord2D *thing)
{
    xferInt(&thing->x);
    xferInt(&thing->y);
}

void Xfer::xferRegion2D(Region2D *thing)
{
    xferCoord2D(&thing->lo);
    xferCoord2D(&thing->hi);
}

void Xfer::xferIRegion2D(IRegion2D *thing)
{
    xferICoord2D(&thing->lo);
    xferICoord2D(&thing->hi);
}

void Xfer::xferRealRange(RealRange *thing)
{
    xferReal(&thing->lo);
    xferReal(&thing->hi);
}

void Xfer::xferColor(int32_t *thing)
{
    xferInt(thing);
}

void Xfer::xferRGBColor(RGBColor *thing)
{
    xferReal(&thing->red);
    xferReal(&thing->green);
    xferReal(&thing->blue);
}

void Xfer::xferRGBAColorReal(RGBAColorReal *thing)
{
    xferReal(&thing->red);
    xferReal(&thing->green);
    xferReal(&thing->blue);
    xferReal(&thing->alpha);
}

void Xfer::xferRGBAColorInt(RGBAColorInt *thing)
{
    xferUnsignedInt(&thing->red);
    xferUnsignedInt(&thing->green);
    xferUnsignedInt(&thing->blue);
    xferUnsignedInt(&thing->alpha);
}

void Xfer::xferObjectID(ObjectID *thing)
{
    xferInt(reinterpret_cast<int32_t *>(thing));
}

void Xfer::xferDrawableID(DrawableID *thing)
{
    xferInt(reinterpret_cast<int32_t *>(thing));
}

void Xfer::xferSTLObjectIDVector(std::vector<ObjectID> *thing)
{
    uint8_t ver = 1;
    xferVersion(&ver, 1);

    uint16_t count = (uint16_t)thing->size();
    xferUnsignedShort(&count);

    if (Get_Mode() == XFER_SAVE || Get_Mode() == XFER_CRC) {
        for (auto it = thing->begin(); it != thing->end(); ++it) {
            xferObjectID(&(*it));
        }
    } else {
        captainslog_relassert(Get_Mode() == XFER_LOAD, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
        captainslog_relassert(thing->size() == 0, XFER_STATUS_NOT_EMPTY, "Trying to xfer load to none empty vector.");

        ObjectID val;

        for (int i = 0; i < count; ++i) {
            xferObjectID(&val);
            thing->insert(thing->end(), val);
        }
    }
}

void Xfer::xferSTLObjectIDList(std::list<ObjectID> *thing)
{
    uint8_t ver = 1;
    xferVersion(&ver, 1);

    uint16_t count = (uint16_t)thing->size();
    xferUnsignedShort(&count);

    if (Get_Mode() == XFER_SAVE || Get_Mode() == XFER_CRC) {
        for (auto it = thing->begin(); it != thing->end(); ++it) {
            xferObjectID(&(*it));
        }
    } else {
        captainslog_relassert(Get_Mode() == XFER_LOAD, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
        captainslog_relassert(thing->size() == 0, XFER_STATUS_NOT_EMPTY, "Trying to xfer load to none empty vector.");

        ObjectID val;

        for (int i = 0; i < count; ++i) {
            xferObjectID(&val);
            thing->insert(thing->end(), val);
        }
    }
}

void Xfer::xferSTLIntList(std::list<int32_t> *thing)
{
    if (thing == nullptr) {
        return;
    }

    uint8_t ver = 1;
    xferVersion(&ver, 1);

    uint16_t count = (uint16_t)thing->size();
    xferUnsignedShort(&count);

    if (Get_Mode() == XFER_SAVE || Get_Mode() == XFER_CRC) {
        for (auto it = thing->begin(); it != thing->end(); ++it) {
            xferInt(&(*it));
        }
    } else {
        captainslog_relassert(Get_Mode() == XFER_LOAD, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
        captainslog_relassert(thing->size() == 0, XFER_STATUS_NOT_EMPTY, "Trying to xfer load to none empty vector.");

        int32_t val;

        for (int i = 0; i < count; ++i) {
            xferInt(&val);
            thing->insert(thing->end(), val);
        }
    }
}

void Xfer::xferScienceType(ScienceType *thing)
{
    captainslog_dbgassert(thing != nullptr, "xferScienceType - Invalid parameters");

    Utf8String name;

    switch (Get_Mode()) {
        case XFER_SAVE:
            name = g_theScienceStore->Get_Internal_Name_From_Science(*thing);
            xferAsciiString(&name);
            break;
        case XFER_LOAD:
            xferAsciiString(&name);

            *thing = g_theScienceStore->Get_Science_From_Internal_Name(name);

            captainslog_relassert(
                *thing != SCIENCE_INVALID, XFER_STATUS_NOT_FOUND, "xferScienceType - Unknown science '%s'", name.Str());
            break;
        case XFER_CRC:
            xferImplementation(thing, sizeof(*thing));
            break;
        default:
            captainslog_relassert(false, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
    }
}

void Xfer::xferScienceVec(std::vector<ScienceType> *thing)
{
    captainslog_dbgassert(thing != nullptr, "xferScienceVec - Invalid parameters");

    uint8_t ver = 1;
    xferVersion(&ver, 1);
    unsigned short count = static_cast<unsigned short>(thing->size());
    xferUnsignedShort(&count);

    switch (Get_Mode()) {
        case XFER_SAVE:
            for (auto it = thing->begin(); it != thing->end(); it++) {
                ScienceType science = *it;
                xferScienceType(&science);
                break;
            }
        case XFER_LOAD:
            if (!thing->empty()) {
                thing->clear();
            }

            for (unsigned short j = 0; j < count; j++) {
                ScienceType t;
                xferScienceType(&t);
                thing->push_back(t);
            }
            break;
        case XFER_CRC:
            for (auto it = thing->begin(); it != thing->end(); it++) {
                ScienceType science = *it;
                xferImplementation(&science, sizeof(science));
            }
            break;
        default:
            captainslog_relassert(false, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
    }
}

void Xfer::xferKindOf(KindOfType *thing)
{
    uint8_t ver = 1;
    xferVersion(&ver, 1);

    Utf8String kind;
    const char *kindc;

    switch (Get_Mode()) {
        case XFER_SAVE:
            if (*thing >= KINDOF_FIRST && *thing < KINDOF_COUNT) {
                kind = BitFlags<KINDOF_COUNT>::Bit_As_String(*thing);
            }

            xferAsciiString(&kind);

            break;
        case XFER_LOAD: {
            xferAsciiString(&kind);
            kindc = kind.Str();

            int i = BitFlags<KINDOF_COUNT>::Get_Single_Bit_From_Name(kindc);

            if (i != -1) {
                *thing = (KindOfType)i;
            }

            break;
        }
        case XFER_CRC:
            xferImplementation(thing, sizeof(*thing));
            break;
        default:
            captainslog_relassert(false, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
            break;
    }
}

void Xfer::xferUpgradeMask(BitFlags<128> *thing)
{
    uint8_t ver = 1;
    xferVersion(&ver, 1);

    switch (Get_Mode()) {
        case XFER_SAVE: {
            Utf8String name;
            unsigned short count = 0;

            for (UpgradeTemplate *tmplate = g_theUpgradeCenter->Get_Upgrade_List(); tmplate != nullptr;
                 tmplate = tmplate->Friend_Get_Next()) {
                BitFlags<128> mask = tmplate->Get_Upgrade_Mask();
                if (thing->Test_For_All(mask)) {
                    count++;
                }
            }

            xferUnsignedShort(&count);

            for (UpgradeTemplate *tmplate = g_theUpgradeCenter->Get_Upgrade_List(); tmplate != nullptr;
                 tmplate = tmplate->Friend_Get_Next()) {
                BitFlags<128> mask = tmplate->Get_Upgrade_Mask();
                if (thing->Test_For_All(mask)) {
                    name = tmplate->Get_Name();
                    xferAsciiString(&name);
                }
            }

            break;
        }
        case XFER_LOAD: {
            Utf8String name;
            unsigned short count;
            xferUnsignedShort(&count);
            thing->Clear();

            for (int i = 0; i < count; i++) {
                xferAsciiString(&name);
                const UpgradeTemplate *tmplate = g_theUpgradeCenter->Find_Upgrade(name);
                captainslog_relassert(
                    tmplate != nullptr, XFER_STATUS_NOT_FOUND, "Xfer::xferUpgradeMask - Unknown upgrade '%s'", name.Str());
                thing->Set(tmplate->Get_Upgrade_Mask());
            }

            break;
        }
        case XFER_CRC:
            xferImplementation(thing, sizeof(*thing));
            break;
        default:
            captainslog_relassert(false, XFER_STATUS_UNKNOWN_XFER_MODE, "Xfer mode unknown.");
    }
}

void Xfer::xferUser(void *thing, int size)
{
    xferImplementation(thing, size);
}

void Xfer::xferMatrix3D(Matrix3D *thing)
{
    xferReal(&(*thing)[0][0]);
    xferReal(&(*thing)[0][1]);
    xferReal(&(*thing)[0][2]);
    xferReal(&(*thing)[0][3]);
    xferReal(&(*thing)[1][0]);
    xferReal(&(*thing)[1][1]);
    xferReal(&(*thing)[1][2]);
    xferReal(&(*thing)[1][3]);
    xferReal(&(*thing)[2][0]);
    xferReal(&(*thing)[2][1]);
    xferReal(&(*thing)[2][2]);
    xferReal(&(*thing)[2][3]);
}

void Xfer::xferMapName(Utf8String *thing)
{
    Utf8String map;

    if (Get_Mode() == XFER_SAVE) {
        map = g_theGameState->Real_To_Portable_Map_Path(*thing);
        xferAsciiString(&map);
    } else if (Get_Mode() == XFER_LOAD) {
        xferAsciiString(&map);
        *thing = g_theGameState->Portable_To_Real_Map_Path(map);
    }
}

void Xfer::Xfer_Client_Random_Var(GameClientRandomVariable *thing)
{
    xferInt(reinterpret_cast<int32_t *>(&thing->m_type));
    xferReal(&thing->m_low);
    xferReal(&thing->m_high);
}

void Xfer::Xfer_Logic_Random_Var(GameLogicRandomVariable *thing)
{
    xferInt(reinterpret_cast<int32_t *>(&thing->m_type));
    xferReal(&thing->m_low);
    xferReal(&thing->m_high);
}
