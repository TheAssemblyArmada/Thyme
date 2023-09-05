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

#include "xfer.h"

class XferCRC : public Xfer
{
public:
    XferCRC() : m_crc(0) { m_type = XFER_CRC; }
    ~XferCRC() override {}

    void Open(Utf8String filename) override;
    void Close() override {}
    int Begin_Block() override { return 0; }
    void End_Block() override {}
    void Skip(int offset) override {}

    void xferSnapshot(SnapShot *thing) override;
    void xferImplementation(void *thing, int size) override;
    virtual uint32_t Get_CRC() { return m_crc; }

protected:
    void Add_CRC(uint32_t val);

    uint32_t m_crc;
};

class XferDeepCRC : public XferCRC
{
public:
    XferDeepCRC();
    ~XferDeepCRC() override;
    void Open(Utf8String filename) override;
    void Close() override;
    void xferMarkerLabel(Utf8String thing) override;
    void xferAsciiString(Utf8String *thing) override;
    void xferUnicodeString(Utf16String *thing) override;
    void xferImplementation(void *thing, int size) override;

private:
    FILE *m_fileHandle;
};
