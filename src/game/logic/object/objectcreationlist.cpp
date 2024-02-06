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

#include "objectcreationlist.h"
#include "object.h"

#ifndef GAME_DLL
ObjectCreationListStore *g_theObjectCreationListStore = nullptr;
std::vector<Utf8String> g_debrisModelNamesGlobalHack;
#endif

// zh: 0x004C1310 wb: 0x0075BB30
ObjectCreationListStore::~ObjectCreationListStore()
{
    for (auto *nugget : m_nuggets) {
        nugget->Delete_Instance();
    }
    Clear();
}

// zh: 0x004C1500 wb: 0x00756CFB
ObjectCreationList *ObjectCreationListStore::Find_Object_Creation_List(const char *name)
{
    if (strcasecmp(name, "None") == 0) {
        return nullptr;
    }

    const auto key = g_theNameKeyGenerator->Name_To_Key(name);

    auto res = m_ocls.find(key);
    if (res == m_ocls.end()) {
        return nullptr;
    }
    return &res->second;
}

// wb: 0x00756D76
void ObjectCreationListStore::Add_Object_Creation_Nugget(ObjectCreationNugget *nugget)
{
    m_nuggets.push_back(nugget);
}

// clang-format off
static const FieldParse s_theObjectCreationListFieldParse[] = {
#ifdef GAME_DLL
    { "CreateObject",       reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004BF350, 0x00759470)) /*&GenericObjectCreationNugget::Parse_Object */,    nullptr,  0 },
    { "CreateDebris",       reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004BF730, 0x00759B60)) /*&GenericObjectCreationNugget::Parse_Debris */,    nullptr,  0 },
    { "ApplyRandomForce",   reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004BEF70, 0x00758F20)) /*&ApplyRandomForceNugget::Parse */,                nullptr,  0 },
    { "DeliverPayload",     reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004BDDD0, 0x00757920)) /*&DeliverPayloadNugget::Parse */,                  nullptr,  0 },
    { "FireWeapon",         reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004BD7E0, 0x00756E00)) /*&FireWeaponNugget::Parse */,                      nullptr,  0 },
    { "Attack",             reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004BD9A0, 0x007572C0)) /*&AttackNugget::Parse */,                          nullptr,  0 },
#endif
    { nullptr,              nullptr, nullptr,  0 },
};
// clang-format on

// zh: 0x004C1570, wb: 0x00756D92
// Was originally called through INI::parseObjectCreationListDefinition
void ObjectCreationListStore::Parse_Object_Creation_List_Definition(INI *ini)
{
    const auto key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());

    auto &ocl = g_theObjectCreationListStore->m_ocls[key];
    ocl.Clear();

    ini->Init_From_INI(&ocl, s_theObjectCreationListFieldParse);
}

// zh: 0x004BD770 wb: 0x00756711
void ObjectCreationNugget::Create(Object *primaryObj, const Object *secondaryObj, uint32_t unk)
{
    const Coord3D *secondary = nullptr;
    if (secondaryObj != nullptr) {
        secondary = secondaryObj->Get_Position();
    }

    const Coord3D *primary = nullptr;
    if (primaryObj != nullptr) {
        primary = primaryObj->Get_Position();
    }

    Create(primaryObj, primary, secondary, -100.0f, unk);
}

// zh: 0x004BD7B0 wb: 0x00756774
void ObjectCreationNugget::Create(Object *primaryObj, const Coord3D *primary, const Coord3D *secondary, bool, uint32_t unk)
{
    Create(primaryObj, primary, secondary, -100.0f, unk);
}

void ObjectCreationList::Create_Internal(const Object *primary, const Object *secondary, uint32_t unk) const
{
#ifdef GAME_DLL
    Call_Method<void, ObjectCreationList const, const Object *, const Object *, uint32_t>(
        PICK_ADDRESS(0x004C1230, 0x00756B17), this, primary, secondary, unk);
#endif
}

void ObjectCreationListStore::Parse_Object_Creation_List(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    ObjectCreationList *list = g_theObjectCreationListStore->Find_Object_Creation_List(name);
    captainslog_dbgassert(list != nullptr || strcasecmp(name, "None") == 0, "ObjectCreationList %s not found!", name);
    *static_cast<ObjectCreationList **>(store) = list;
}
