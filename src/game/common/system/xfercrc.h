/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief CRC data being transferred.
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
#include "xfer.h"

class XferCRC : public Xfer
{
public:
    XferCRC() : m_crc(0) { m_type = XFER_CRC; }
    virtual ~XferCRC() {}

    virtual void Open(Utf8String filename);
    virtual void Close() {}
    virtual int Begin_Block() { return 0; }
    virtual void End_Block() {}
    virtual void Skip(int offset) {}

    virtual void xferSnapshot(SnapShot *thing);
    virtual void xferImplementation(void *thing, int size);
    virtual uint32_t Get_CRC() { return m_crc; }

protected:
    void Add_CRC(uint32_t val);

    uint32_t m_crc;
};

class XferDeepCRC : public XferCRC
{
public:
    XferDeepCRC();
    virtual ~XferDeepCRC() override;
    virtual void Open(Utf8String filename) override;
    virtual void Close() override;
    virtual void xferMarkerLabel(Utf8String thing) override;
    virtual void xferAsciiString(Utf8String *thing) override;
    virtual void xferUnicodeString(Utf16String *thing) override;
    virtual void xferImplementation(void *thing, int size) override;

private:
    FILE *m_fileHandle;
};
