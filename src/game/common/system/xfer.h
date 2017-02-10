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
#include <list>
#include <vector>

class SnapShot;
class UnicodeString;
class Coord3D;
class ICoord3D;
class Region3D;
class IRegion3D;
class Coord2D;
class ICoord2D;
class Region2D;
class IRegion2D;
class RGBColor;
class RGBAColorInt;
class RGBAColorReal;
class ObjectID;
class RealRange;
class DrawableID;
class ScienceType;
class KindOf;
template <int bits> struct BitFlags;
class Matrix3D;

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
        Xfer();
        virtual ~Xfer();

        virtual XferType Get_Mode() { return Type; }
        virtual void Set_Options(unsigned int options) { Options |= options; }
        virtual void Clear_Options(unsigned int options) { Options &= ~options; }
        virtual unsigned int Get_Options(void) { return Options; }

        virtual void Open(AsciiString filename) = 0;
        virtual void Close() = 0;
        virtual int Begin_Block() = 0;
        virtual int End_Block() = 0;
        virtual int Skip(int offset) = 0;

        virtual bool xferSnapshot(SnapShot *thing) = 0;
        virtual bool xferVersion(uint8_t *thing, uint8_t check);
        virtual bool xferByte(int8_t thing);
        virtual bool xferUnsignedByte(uint8_t thing);
        virtual bool xferBool(bool thing);
        virtual bool xferInt(int32_t thing);
        virtual bool xferInt64(int64_t thing);
        virtual bool xferUnsignedInt(uint32_t thing);
        virtual bool xferShort(int16_t thing);
        virtual bool xferUnsignedShort(uint16_t thing);
        virtual bool xferReal(float thing);
        virtual bool xferMarkerLabel(AsciiString thing);
        virtual bool xferAsciiString(AsciiString *thing);
        virtual bool xferUnicodeString(UnicodeString *thing);
        virtual bool xferCoord3D(Coord3D *thing);
        virtual bool xferICoord3D(ICoord3D *thing);
        virtual bool xferRegion3D(Region3D *thing);
        virtual bool xferIRegion3D(IRegion3D *thing);
        virtual bool xferCoord2D(Coord2D *thing);
        virtual bool xferICoord2D(ICoord2D *thing);
        virtual bool xferRegion2D(Region2D *thing);
        virtual bool xferIRegion2d(IRegion2D *thing);
        virtual bool xferRealRange(RealRange *thing);
        virtual bool xferColor(int32_t thing);
        virtual bool xferRGBColor(RGBColor *thing);
        virtual bool xferRGBAColorReal(RGBAColorReal *thing);
        virtual bool xferRGBAColorInt(RGBAColorInt *thing);
        virtual bool xferObjectID(ObjectID *thing);
        virtual bool xferDrawableID(DrawableID *thing);
        virtual bool xferSTLObjectIDVector(std::vector<ObjectID> *thing);
        virtual bool xferSTLObjectIDList(std::list<ObjectID> *thing);
        virtual bool xferSTLIntList(std::list<int32_t> *thing);
        virtual bool xferScienceType(ScienceType *thing);
        virtual bool xferScienceVec(std::vector<ScienceType> *thing);
        virtual bool xferKindOf(KindOf *thing);
        virtual bool xferUpgradeMask(BitFlags<128> *thing);
        virtual bool xferUser(void *thing);
        virtual bool xferMatrix3D(Matrix3D *thing);
        virtual bool xferMapName(AsciiString *thing);
        virtual bool xferImplementation(void *thing, int size) = 0;

    protected:
        unsigned int Options;
        XferType Type;
        AsciiString Name;
};

#endif
