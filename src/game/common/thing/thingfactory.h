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
#pragma once

#include "always.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include "thingtemplate.h"

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class Team;
class Object;
class Drawable;

class ThingFactory : public SubsystemInterface
{
public:
    ThingFactory();
    virtual ~ThingFactory() override;
    virtual void Init() override {}
    virtual void PostProcessLoad() override;
    virtual void Reset() override;
    virtual void Update() override {}

    ThingTemplate *New_Template(const Utf8String &name);
    ThingTemplate *New_Override(ThingTemplate *thing_template);
    ThingTemplate *First_Template() { return m_firstTemplate; }
    ThingTemplate *Find_Template_Internal(const Utf8String &name, bool b);
    ThingTemplate *Find_Template(const Utf8String &name, bool b) { return Find_Template_Internal(name, b); }
    static void Parse_Object_Definition(INI *ini, const Utf8String &name, const Utf8String &reskin_from);
    void Add_Template(ThingTemplate *tmplate);
    ThingTemplate *Find_Template_By_ID(unsigned short id);
    Object *New_Object(const ThingTemplate *tmplate, Team *team, BitFlags<OBJECT_STATUS_COUNT> status_bits);
    Drawable *New_Drawable(const ThingTemplate *tmplate, DrawableStatus status_bits);
    void Free_Database();

private:
    ThingTemplate *m_firstTemplate;
    unsigned short m_nextTemplateID;
#ifdef THYME_USE_STLPORT
    std::hash_map<const Utf8String, ThingTemplate *, rts::hash<Utf8String>, std::equal_to<Utf8String>> m_templateMap;
#else
    std::unordered_map<const Utf8String, ThingTemplate *, rts::hash<Utf8String>, std::equal_to<Utf8String>> m_templateMap;
#endif
};

class W3DThingFactory : public ThingFactory
{
#ifdef GAME_DLL
    W3DThingFactory *Hook_Ctor() { return new (this) W3DThingFactory; }
#endif
};

#ifdef GAME_DLL
#include "hooker.h"
extern ThingFactory *&g_theThingFactory;
#else
extern ThingFactory *g_theThingFactory;
#endif
