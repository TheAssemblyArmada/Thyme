/**
 * @file ObjectCreationList
 *
 * @author Duncans_pumpkin
 *
 * @brief ObjectCreationNugget, ObjectCreationList, ObjectCreationListStore
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
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "subsysteminterface.h"
#include <map>
#include <vector>

class INI;
class Object;
class Coord3D;

class ObjectCreationNugget : public MemoryPoolObject
{
    IMPLEMENT_ABSTRACT_POOL(ObjectCreationNugget);

public:
    virtual ~ObjectCreationNugget() {}
    virtual void Create(Object *primaryObj, const Coord3D *primary, const Coord3D *secondary, float unk1, uint32_t unk2) = 0;
    virtual void Create(Object *primary, const Object *secondary, uint32_t unk);
    virtual void Create(Object *primaryObj, const Coord3D *primary, const Coord3D *secondary, bool, uint32_t unk);
};

class ObjectCreationList
{
public:
    ObjectCreationList(){};
    ~ObjectCreationList(){};

    void Clear() { m_nuggets.clear(); }
    void Add_Object_Creation_Nugget(ObjectCreationNugget *nugget);
    void Create_Internal(const Object *primary, const Object *secondary, uint32_t unk) const;

    void Create(const Object *primary, const Object *secondary, uint32_t unk) const
    {
        return Create_Internal(primary, secondary, unk);
    }

private:
    std::vector<ObjectCreationNugget *> m_nuggets;
};

class ObjectCreationListStore : public SubsystemInterface
{
public:
    ObjectCreationListStore() {}

    virtual ~ObjectCreationListStore() override;
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    // zh: 0x004C1280 wb: 0x00756BBD
    ObjectCreationListStore *Hook_Ctor() { return new (this) ObjectCreationListStore; }

    ObjectCreationList *Find_Object_Creation_List(const char *name);
    void Add_Object_Creation_Nugget(ObjectCreationNugget *nugget);

    static void Parse_Object_Creation_List_Definition(INI *ini);
    static void Parse_Object_Creation_List(INI *ini, void *formal, void *store, const void *user_data);

private:
    void Clear() { m_nuggets.clear(); }

    std::map<NameKeyType, ObjectCreationList> m_ocls;
    std::vector<ObjectCreationNugget *> m_nuggets;
};

#ifdef GAME_DLL
extern ObjectCreationListStore *&g_theObjectCreationListStore;
extern std::vector<Utf8String> &g_debrisModelNamesGlobalHack;
#else
extern ObjectCreationListStore *g_theObjectCreationListStore;
extern std::vector<Utf8String> g_debrisModelNamesGlobalHack;
#endif
