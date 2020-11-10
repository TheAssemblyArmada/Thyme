/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Base client object providing overall control of client IO and rendering.
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
#include "coord.h"
#include "gametype.h"
#include "messagestream.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>
#include <vector>

class Drawable;
struct RayEffectData;
class ThingTemplate;

enum Scorches
{
    SCORCHES_UNK,
};

enum DrawableStatus
{
    DRAWABLE_STATUS_UNK,
};

// Temp until class properly implemented.
class CommandTranslator
{
public:
    enum CommandEvaluateType
    {
        UNK_TYPE,
    };
};

class GameClient : public SubsystemInterface, public SnapShot
{
    struct DrawableTOCEntry
    {
        Utf8String name;
        uint16_t number;
    };

    enum
    {
        MAX_CLIENT_TRANSLATORS = 32,
    };

public:
    GameClient();
    virtual ~GameClient();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void Set_Frame(uint32_t frame) { m_frame = frame; }
    virtual void Register_Drawable(Drawable *drawable);
    virtual Drawable *Find_Drawable_By_ID(DrawableID id);
    virtual Drawable *First_Drawable();
    virtual void Evaluate_Context_Command(
        Drawable *drawable, const Coord3D *pos, CommandTranslator::CommandEvaluateType type);
    virtual void Remove_From_Ray_Effects(Drawable *drawable);
    virtual void Get_Ray_Effect_Data(Drawable *drawable, RayEffectData *data);
    virtual RayEffectData *Create_Ray_Effect_From_Template(
        const Coord3D *src, const Coord3D *dst, const ThingTemplate *temp) = 0;
    virtual void Add_Scorch(Coord3D *pos, float scale, Scorches scorch) = 0;
    virtual bool Load_Map(Utf8String name) { return name.Is_Not_Empty() ? true : false; }
    virtual void Unload_Map(Utf8String name) {}
    virtual void Iterate_Drawables_In_Region(Region3D *region, void (*func)(Drawable *, void *), void *data);
    virtual Drawable *Create_Drawable(ThingTemplate *temp, DrawableStatus status) = 0;
    virtual void Destroy_Drawable(Drawable *drawable);
    virtual void Set_Time_Of_Day(TimeOfDayType time);
    virtual void Select_Drawables_In_Group(int group) {}
    virtual void Assign_Selected_Drawables_To_Group(int group) {}
    virtual uint32_t Get_Frame() { return m_frame; }

protected:
    uint32_t m_frame;
    Drawable *m_drawableList;
    std::vector<Drawable *> m_drawableLUT;
    DrawableID m_nextDrawableID;
    uint32_t m_translators[MAX_CLIENT_TRANSLATORS];
    uint32_t m_translatorCount;
    CommandTranslator *m_commandTranslator;
    int field_AC; // Dunno what this actually is yet.
    std::list<DrawableTOCEntry> m_drawableTOC;
    std::vector<void *> m_unkVector; // Presumed, needs confirming.
};

#ifdef GAME_DLL
extern GameClient *&g_theGameClient;
#else
extern GameClient *g_theGameClient;
#endif

class GameClientMessageDispatcher : public GameMessageTranslator
{
public:
    GameClientMessageDispatcher() {}
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg);
    virtual ~GameClientMessageDispatcher() {}
};
