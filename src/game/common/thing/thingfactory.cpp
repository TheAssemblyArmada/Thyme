/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "thingfactory.h"
#include "behaviormodule.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "object.h"
#include <cstring>

#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ThingFactory *g_theThingFactory = nullptr;
#endif

#ifdef GAME_DEBUG_STRUCTS
#ifdef GAME_DLL
Utf8String &s_theThingTemplateBeingParsedName = Make_Global<Utf8String>(0x00E1D4F4);
#else
Utf8String s_theThingTemplateBeingParsedName;
#endif
#endif

ThingFactory::ThingFactory() : m_firstTemplate(nullptr), m_nextTemplateID(1) {}

ThingFactory::~ThingFactory()
{
    Free_Database();
}

ThingTemplate *ThingFactory::New_Template(const Utf8String &name)
{
    ThingTemplate *new_template = NEW_POOL_OBJ(ThingTemplate);
    ThingTemplate *default_template = Find_Template("DefaultThingTemplate", false);

    if (default_template != nullptr) {
        *new_template = *default_template;
        new_template->Set_Copied_From_Default();
    }

    new_template->Friend_Set_Template_ID(m_nextTemplateID);
    m_nextTemplateID++;

    captainslog_dbgassert(m_nextTemplateID != 0, "m_nextTemplateID wrapped to zero");

    new_template->Friend_Set_Template_Name(name);
    Add_Template(new_template);
    return new_template;
}

ThingTemplate *ThingFactory::New_Override(ThingTemplate *thing_template)
{
    captainslog_dbgassert(thing_template, "NULL 'parent' thing template");
    captainslog_dbgassert(Find_Template(thing_template->Get_Name(), true),
        "Thing template '%s' not in master list",
        thing_template->Get_Name().Str());

    ThingTemplate *override_template = static_cast<ThingTemplate *>(thing_template->Friend_Get_Final_Override());
    ThingTemplate *new_template = NEW_POOL_OBJ(ThingTemplate);
    *new_template = *override_template;
    new_template->Set_Copied_From_Default();
    new_template->Set_Is_Allocated();
    override_template->Set_Next(new_template);
    return new_template;
}

void ThingFactory::Free_Database()
{
    while (m_firstTemplate != nullptr) {
        ThingTemplate *t = m_firstTemplate;
        m_firstTemplate = t->Friend_Get_Next_Template();
        t->Delete_Instance();
    }

    m_templateMap.clear();
}

void ThingFactory::PostProcessLoad()
{
    for (ThingTemplate *t = m_firstTemplate; t != nullptr; t = t->Friend_Get_Next_Template()) {
        t->Resolve_Names();
    }
}

void ThingFactory::Reset()
{
    ThingTemplate *t = m_firstTemplate;

    while (t) {
        bool first = false;
        ThingTemplate *next = t->Friend_Get_Next_Template();

        if (t == m_firstTemplate) {
            first = true;
        }

        Utf8String str(t->Get_Name());
        Overridable *o = t->Delete_Overrides();

        if (o == nullptr) {
            if (first) {
                m_firstTemplate = next;
            }
            m_templateMap.erase(str);
        }

        t = next;
    }
}

void ThingFactory::Add_Template(ThingTemplate *tmplate)
{
    if (m_templateMap.find(tmplate->Get_Name()) != m_templateMap.end()) {
        captainslog_dbgassert(0, "Duplicate Thing Template name found: %s", tmplate->Get_Name().Str());
    }

    tmplate->Friend_Set_Next_Template(m_firstTemplate);
    m_firstTemplate = tmplate;
    m_templateMap[tmplate->Get_Name()] = tmplate;
}

ThingTemplate *ThingFactory::Find_Template_By_ID(unsigned short id)
{
    for (ThingTemplate *t = m_firstTemplate; t != nullptr; t = t->Friend_Get_Next_Template()) {
        if (t->Get_Template_ID() == id) {
            return t;
        }
    }

    captainslog_dbgassert(0, "template %d not found", id);
    return nullptr;
}

ThingTemplate *ThingFactory::Find_Template_Internal(const Utf8String &name, bool b)
{
    auto i = m_templateMap.find(name);

    if (i != m_templateMap.end()) {
        return i->second;
    }

    if (strncmp(name.Str(), "***TESTING", strlen("***TESTING")) == 0) {
        ThingTemplate *tmplate = New_Template("Un-namedTemplate");
        tmplate->Init_For_LTA(name);
        m_templateMap.erase("Un-namedTemplate");
        m_templateMap[name] = tmplate;
        return Find_Template_Internal(name, true);
    } else {

        // Thyme specific: Original assert has been demoted to log message because it is a data issue.
        if (b && name.Is_Not_Empty()) {
            captainslog_error(
                "Failed to find thing template %s (case sensitive) This issue has a chance of crashing after you ignore it!",
                name.Str());
        }
        return nullptr;
    }
}

Object *ThingFactory::New_Object(const ThingTemplate *tmplate, Team *team, BitFlags<OBJECT_STATUS_COUNT> status_bits)
{
    if (tmplate == nullptr) {
        throw CODE_03;
    }

    const std::vector<Utf8String> &variations = tmplate->Get_Build_Variations();

    if (!variations.empty()) {
        int random = Get_Logic_Random_Value(0, variations.size() - 1);
        ThingTemplate *variation = Find_Template(variations[random], true);

        if (variation != nullptr) {
            tmplate = variation;
        }
    }

    // Thyme specific: Original assert has been demoted to log message because it is a data issue.
    if (tmplate->Is_KindOf(KINDOF_DRAWABLE_ONLY)) {
        captainslog_error("You may not create Objects with the template %s, only Drawables", tmplate->Get_Name().Str());
    }

    Object *object = g_theGameLogic->Friend_Create_Object(tmplate, status_bits, team);

    for (BehaviorModule **i = object->Get_All_Modules(); *i != nullptr; i++) {
        CreateModuleInterface *create = (*i)->Get_Create();

        if (create != nullptr) {
            create->On_Create();
        }
    }

    g_thePartitionManager->Register_Object(object);
    object->Init_Object();
    return object;
}

Drawable *ThingFactory::New_Drawable(const ThingTemplate *tmplate, DrawableStatus status_bits)
{
    if (tmplate == nullptr) {
        throw CODE_03;
    }

    return g_theGameClient->Create_Drawable(tmplate, status_bits);
}

void ThingFactory::Parse_Object_Definition(INI *ini, const Utf8String &name, const Utf8String &reskin_from)
{
#ifdef GAME_DEBUG_STRUCTS
    s_theThingTemplateBeingParsedName = name;
#endif
    ThingTemplate *tmplate = g_theThingFactory->Find_Template_Internal(name, false);

    if (tmplate) {
        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            tmplate = g_theThingFactory->New_Override(tmplate);
        } else {
            captainslog_debug("[LINE: %d in '%s'] Duplicate factionunit %s found!",
                ini->Get_Line_Number(),
                ini->Get_Filename().Str(),
                name.Str());
        }
    } else {
        tmplate = g_theThingFactory->New_Template(name);

        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            tmplate->Set_Is_Allocated();
        }
    }

    if (reskin_from.Is_Not_Empty()) {
        ThingTemplate *that = g_theThingFactory->Find_Template(reskin_from, true);

        if (that) {
            tmplate->Copy_From(that);
            tmplate->Set_Copied_From_Default();
            tmplate->Friend_Set_Original_Skin_Template(that);
            ini->Init_From_INI(tmplate, ThingTemplate::Get_Reskin_Field_Parse());
        } else {
            captainslog_debug("ObjectReskin must come after the original Object (%s, %s).", reskin_from.Str(), name.Str());
            throw CODE_06;
        }
    } else {
        ini->Init_From_INI(tmplate, ThingTemplate::Get_Field_Parse());
    }

    tmplate->Validate();

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        tmplate->Resolve_Names();
    }

#ifdef GAME_DEBUG_STRUCTS
    s_theThingTemplateBeingParsedName.Clear();
#endif
}
