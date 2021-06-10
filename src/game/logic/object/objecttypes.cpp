/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief List of objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "objecttypes.h"
#include "xfer.h"
#include <captainslog.h>

ObjectTypes::ObjectTypes(const Utf8String &list_name) : m_listName(list_name) {}

void ObjectTypes::Xfer_Snapshot(Xfer *xfer)
{
    static constexpr uint8_t CURRENT_VERSION = 1;
    uint8_t version = CURRENT_VERSION;

    xfer->xferVersion(&version, CURRENT_VERSION);
    xfer->xferAsciiString(&m_listName);

    uint16_t count = uint16_t(m_objectTypes.size());
    xfer->xferUnsignedShort(&count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = m_objectTypes.begin(); it != m_objectTypes.end(); ++it) {
            xfer->xferAsciiString(&(*it));
        }
    } else {
        captainslog_relassert(
            m_objectTypes.empty(), "SaveCode", "ObjectTypes::xfer - m_objectTypes vector should be emtpy but is not !");
        Utf8String string;

        for (unsigned i = 0; i < count; ++i) {
            xfer->xferAsciiString(&string);
            m_objectTypes.push_back(string);
        }
    }
}
