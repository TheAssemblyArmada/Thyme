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
#include "xfercrc.h"
#include "endiantype.h"
#include "snapshot.h"

// \brief Adds val to rotl m_crc
void XferCRC::Add_CRC(uint32_t val)
{
    m_crc = htobe32(val) + (m_crc >> 31) + (m_crc << 1);
}

void XferCRC::Open(Utf8String filename)
{
    Xfer::Open(filename);
    m_crc = 0;
}

void XferCRC::xferSnapshot(SnapShot *thing)
{
    if (thing != nullptr) {
        thing->CRC_Snapshot(this);
    }
}

void XferCRC::xferImplementation(void *thing, int size)
{
    uint32_a *data = static_cast<uint32_a *>(thing);

    if (thing == nullptr || size < 1) {
        return;
    }

    // Use up all the multiples of 4 data.
    for (int i = size / 4; i > 0; --i) {
        Add_CRC(*data++);
    }

    // Use remaining bytes padded with 0
    if (size % 4 > 0) {
        uint32_t tmp = 0;
        uint8_t *cdata = reinterpret_cast<uint8_t *>(data);
        int shift = 0;

        for (int i = 0; i < size % 4; ++i) {
            tmp |= cdata[i] << shift;
            shift += 8;
        }

        Add_CRC(tmp);
    }
}

XferDeepCRC::XferDeepCRC()
{
    m_type = XFER_SAVE;
    m_fileHandle = nullptr;
}

XferDeepCRC::~XferDeepCRC()
{
    if (m_fileHandle != nullptr) {
        captainslog_dbgassert(false, "Warning: Xfer file '%s' was left open", m_filename.Str());
        Close();
    }
}

void XferDeepCRC::Open(Utf8String filename)
{
    m_type = XFER_SAVE;
    captainslog_relassert(m_fileHandle == nullptr,
        XFER_STATUS_FILE_ALREADY_OPEN,
        "Cannot open file '%s' cause we've already got '%s' open",
        filename.Str(),
        m_filename.Str());
    Xfer::Open(filename);
    m_fileHandle = fopen(filename.Str(), "w+b");
    captainslog_relassert(m_fileHandle != nullptr, XFER_STATUS_FILE_NOT_FOUND, "File '%s' not found", filename.Str());
    m_crc = 0;
}

void XferDeepCRC::Close()
{
    captainslog_relassert(m_fileHandle != nullptr, XFER_STATUS_FILE_NOT_OPEN, "Xfer close called, but no file was open");
    fclose(m_fileHandle);
    m_fileHandle = nullptr;
    m_filename.Clear();
}

void XferDeepCRC::xferImplementation(void *thing, int size)
{
    if (thing != nullptr && size >= 1) {
        captainslog_dbgassert(m_fileHandle != nullptr, "XferSave - file pointer for '%s' is NULL", m_filename.Str());
        int ret = fwrite(thing, size, 1, m_fileHandle);
        captainslog_relassert(ret == 1, XFER_STATUS_WRITE_ERROR, "XferSave - Error writing to file '%s'", m_filename.Str());
        XferCRC::xferImplementation(thing, size);
    }
}

void XferDeepCRC::xferMarkerLabel(Utf8String thing) {}

void XferDeepCRC::xferAsciiString(Utf8String *thing)
{
    captainslog_relassert(thing->Get_Length() <= 16385,
        XFER_STATUS_STRING_TOO_LONG,
        "XferSave cannot save this ascii string because it's too long.  Change the size of the length header (but be sure "
        "to preserve save file compatability");
    unsigned short len = thing->Get_Length();
    xferUnsignedShort(&len);

    if (len != 0) {
        xferUser(const_cast<char *>(thing->Str()), len);
    }
}

void XferDeepCRC::xferUnicodeString(Utf16String *thing)
{
    captainslog_relassert(thing->Get_Length() <= 255,
        XFER_STATUS_STRING_TOO_LONG,
        "XferSave cannot save this unicode string because it's too long.  Change the size of the length header (but be sure "
        "to preserve save file compatability");
    int8_t len = thing->Get_Length();
    xferByte(&len);

    if (len != 0) {
        xferUser(const_cast<unichar_t *>(thing->Str()), len * 2);
    }
}
