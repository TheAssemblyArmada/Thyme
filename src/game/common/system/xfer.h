////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: XFER.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Some transfer thing interface.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _XFER_H_
#define _XFER_H_

#include "asciistring.h"
#include "bitflags.h"
#include "color.h"
#include "coord.h"
#include "kindof.h"
#include "snapshot.h"
#include "unicodestring.h"
#include <list>
#include <vector>

struct RealRange
{
    float lo;
    float hi;
};

class Matrix3D;

typedef int ObjectID;
typedef int DrawableID;

enum ScienceType
{
    SCIENCE_INVALID = -1,
    SCIENCE_BOGUS = 0,
};

enum XferType
{
    XFER_INVALID = 0,
    XFER_SAVE    = 1,
    XFER_LOAD    = 2,
    XFER_CRC     = 3,
};

class Xfer
{
public:
    Xfer() : m_options(0), m_type(XFER_INVALID), m_name() {}
    virtual ~Xfer() {}

    virtual XferType Get_Mode() { return m_type; }
    virtual void Set_Options(unsigned int options) { m_options |= options; }
    virtual void Clear_Options(unsigned int options) { m_options &= ~options; }
    virtual unsigned int Get_Options(void) { return m_options; }

    virtual void Open(AsciiString filename) = 0;
    virtual void Close() = 0;
    virtual int Begin_Block() = 0;
    virtual int End_Block() = 0;
    virtual int Skip(int offset) = 0;

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
    virtual void xferMarkerLabel(AsciiString thing);
    virtual void xferAsciiString(AsciiString *thing);
    virtual void xferUnicodeString(UnicodeString *thing);
    virtual void xferCoord3D(Coord3D *thing);
    virtual void xferICoord3D(ICoord3D *thing);
    virtual void xferRegion3D(Region3D *thing);
    virtual void xferIRegion3D(IRegion3D *thing);
    virtual void xferCoord2D(Coord2D *thing);
    virtual void xferICoord2D(ICoord2D *thing);
    virtual void xferRegion2D(Region2D *thing);
    virtual void xferIRegion2D(IRegion2D *thing);
    virtual void xferRealRange(RealRange *thing);
    virtual void xferColor(int32_t thing);
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
    virtual void xferMapName(AsciiString *thing);
    virtual void xferImplementation(void *thing, int size) = 0;

protected:
    unsigned int m_options;
    XferType m_type;
    AsciiString m_name;
};

#endif // _XFER_H_
