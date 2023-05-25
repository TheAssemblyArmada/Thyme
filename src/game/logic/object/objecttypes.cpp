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
#include "player.h"
#include "thingfactory.h"
#include "thingtemplate.h"
#include "xfer.h"
#include <algorithm>
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
        for (auto &it : m_objectTypes) {
            xfer->xferAsciiString(&it);
        }
    } else {
        captainslog_relassert(
            m_objectTypes.empty(), 6, "ObjectTypes::xfer - m_objectTypes vector should be emtpy but is not !");
        Utf8String string;

        for (unsigned i = 0; i < count; ++i) {
            xfer->xferAsciiString(&string);
            m_objectTypes.push_back(string);
        }
    }
}

Utf8String ObjectTypes::Get_Nth_In_List(unsigned int i) const
{
    if (i >= Get_List_Count()) {
        return Utf8String::s_emptyString;
    } else {
        return m_objectTypes[i];
    }
}

void ObjectTypes::Add_Object_Type(const Utf8String &type)
{
    if (!Is_In_Set(type)) {
        m_objectTypes.push_back(type);
    }
}

void ObjectTypes::Remove_Object_Type(const Utf8String &type)
{
    if (Is_In_Set(type)) {
        m_objectTypes.erase(std::find(m_objectTypes.begin(), m_objectTypes.end(), type));
    } else {
        captainslog_dbgassert(
            false, "Attempted to remove '%s' from '%s', but it wasn't there.", type.Str(), m_listName.Str());
    }
}

const Utf8String &ObjectTypes::Get_List_Name() const
{
    return m_listName;
}

void ObjectTypes::Set_List_Name(const Utf8String &name)
{
    m_listName = name;
}

bool ObjectTypes::Is_In_Set(const Utf8String &type) const
{
    return std::find(m_objectTypes.begin(), m_objectTypes.end(), type) != m_objectTypes.end();
}

bool ObjectTypes::Is_In_Set(const ThingTemplate *tmplate) const
{
    if (tmplate == nullptr) {
        return false;
    }

    return Is_In_Set(tmplate->Get_Name());
}

int ObjectTypes::Prep_For_Player_Counting(std::vector<ThingTemplate *> &types, std::vector<int> &counts)
{
    for (auto it = m_objectTypes.begin(); it != m_objectTypes.end(); it++) {
        ThingTemplate *tmplate = g_theThingFactory->Find_Template(*it, true);

        if (tmplate != nullptr) {
            types.push_back(tmplate);
        }
    }

    counts.resize(types.size());
    return types.size();
}

bool ObjectTypes::Can_Build_Any(Player *player)
{
    for (auto it = m_objectTypes.begin(); it != m_objectTypes.end(); it++) {
        ThingTemplate *tmplate = g_theThingFactory->Find_Template(*it, true);

        if (tmplate != nullptr && player->Can_Build(tmplate)) {
            return true;
        }
    }

    return false;
}
