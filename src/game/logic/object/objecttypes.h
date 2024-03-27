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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include <vector>

class Player;
class ThingTemplate;

class ObjectTypes : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(ObjectTypes);

public:
    ObjectTypes() {}
    ObjectTypes(const Utf8String &list_name);

#ifdef GAME_DLL
    ObjectTypes *Hook_Ctor() { return new (this) ObjectTypes(); }
    ObjectTypes *Hook_Ctor2(const Utf8String &list_name) { return new (this) ObjectTypes(list_name); }
    void Hook_Dtor() { ObjectTypes::~ObjectTypes(); }
#endif

    virtual ~ObjectTypes() override {}

    // Snapshot virtuals
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() override {}

    unsigned int Get_List_Count() const { return m_objectTypes.size(); }
    Utf8String Get_Nth_In_List(unsigned int i) const;
    void Add_Object_Type(const Utf8String &type);
    void Remove_Object_Type(const Utf8String &type);
    const Utf8String &Get_List_Name() const;
    void Set_List_Name(const Utf8String &name);
    bool Is_In_Set(const Utf8String &type) const;
    bool Is_In_Set(const ThingTemplate *tmplate) const;
    int Prep_For_Player_Counting(std::vector<ThingTemplate *> &types, std::vector<int> &counts);
    bool Can_Build_Any(Player *player);

private:
    Utf8String m_listName;
    std::vector<Utf8String> m_objectTypes;
};
