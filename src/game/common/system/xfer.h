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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "gametype.h"
#include "kindof.h"
#include "science.h"
#include <list>
#include <vector>

template<int bits> class BitFlags;
class Coord3D;
class Coord2D;
class ICoord2D;
class ICoord3D;
class IRegion2D;
class IRegion3D;
class Matrix3D;
class Region2D;
class Region3D;
class RGBColor;
class RGBAColorReal;
class RGBAColorInt;
class Utf16String;
class SnapShot;

struct RealRange
{
    float lo;
    float hi;
};

enum XferType
{
    XFER_INVALID,
    XFER_SAVE,
    XFER_LOAD,
    XFER_CRC,
};

enum XferStatus
{
    XFER_STATUS_INVALID,
    XFER_STATUS_OK,
    XFER_STATUS_EOF,
    XFER_STATUS_FILE_NOT_FOUND,
    XFER_STATUS_FILE_NOT_OPEN,
    XFER_STATUS_FILE_ALREADY_OPEN,
    XFER_STATUS_READ_ERROR,
    XFER_STATUS_WRITE_ERROR,
    XFER_STATUS_UNKNOWN_XFER_MODE,
    XFER_STATUS_FILE_SEEK_ERROR,
    XFER_STATUS_NO_BEGIN_BLOCK,
    XFER_STATUS_UNK11,
    XFER_STATUS_STRING_TOO_LONG,
    XFER_STATUS_UNKNOWN_VERSION,
    XFER_STATUS_INVALID_PARAMETERS,
    XFER_STATUS_NOT_EMPTY,
    XFER_STATUS_NOT_FOUND,
    NUM_XFER_STATUS,
};

class GameClientRandomVariable;
class GameLogicRandomVariable;

class Xfer
{
public:
    Xfer() : m_options(0), m_type(XFER_INVALID), m_filename() {}
    virtual ~Xfer() {}

    virtual XferType Get_Mode() { return m_type; }
    virtual void Set_Options(unsigned options) { m_options |= options; }
    virtual void Clear_Options(unsigned options) { m_options &= ~options; }
    virtual unsigned Get_Options() { return m_options; }

    virtual void Open(Utf8String filename);
    virtual void Close() = 0;
    virtual int Begin_Block() = 0;
    virtual void End_Block() = 0;
    virtual void Skip(int offset) = 0;

    virtual void xferSnapshot(SnapShot *thing) = 0;
    virtual void xferVersion(uint8_t *thing, uint8_t check);
    virtual void xferByte(int8_t *thing);
    virtual void xferUnsignedByte(uint8_t *thing);
    virtual void xferBool(bool *thing);
    virtual void xferInt(int32_t *thing);
    virtual void xferInt64(int64_t *thing);
    virtual void xferUnsignedInt(uint32_t *thing);
    virtual void xferShort(int16_t *thing);
    virtual void xferUnsignedShort(uint16_t *thing);
    virtual void xferReal(float *thing);
    virtual void xferMarkerLabel(Utf8String thing);
    virtual void xferAsciiString(Utf8String *thing);
    virtual void xferUnicodeString(Utf16String *thing);
    virtual void xferCoord3D(Coord3D *thing);
    virtual void xferICoord3D(ICoord3D *thing);
    virtual void xferRegion3D(Region3D *thing);
    virtual void xferIRegion3D(IRegion3D *thing);
    virtual void xferCoord2D(Coord2D *thing);
    virtual void xferICoord2D(ICoord2D *thing);
    virtual void xferRegion2D(Region2D *thing);
    virtual void xferIRegion2D(IRegion2D *thing);
    virtual void xferRealRange(RealRange *thing);
    virtual void xferColor(int32_t *thing);
    virtual void xferRGBColor(RGBColor *thing);
    virtual void xferRGBAColorReal(RGBAColorReal *thing);
    virtual void xferRGBAColorInt(RGBAColorInt *thing);
    virtual void xferObjectID(ObjectID *thing);
    virtual void xferDrawableID(DrawableID *thing);
    virtual void xferSTLObjectIDVector(std::vector<ObjectID> *thing);
    virtual void xferSTLObjectIDList(std::list<ObjectID> *thing);
    virtual void xferSTLIntList(std::list<int32_t> *thing);
    virtual void xferScienceType(ScienceType *thing);
    virtual void xferScienceVec(std::vector<ScienceType> *thing);
    virtual void xferKindOf(KindOfType *thing);
    virtual void xferUpgradeMask(BitFlags<128> *thing);
    virtual void xferUser(void *thing, int size);
    virtual void xferMatrix3D(Matrix3D *thing);
    virtual void xferMapName(Utf8String *thing);
    virtual void xferImplementation(void *thing, int size) = 0;

    void Xfer_Client_Random_Var(GameClientRandomVariable *thing);
    void Xfer_Logic_Random_Var(GameLogicRandomVariable *thing);

protected:
    unsigned m_options;
    XferType m_type;
    Utf8String m_filename;
};

#ifdef GAME_DLL
#include "hooker.h"
#endif