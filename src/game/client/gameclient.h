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
class Object;
struct RayEffectData;
class ThingTemplate;

enum Scorches
{
    SCORCHES_UNK,
};

// Temp until class properly implemented.
class CommandTranslator : public GameMessageTranslator
{
public:
    CommandTranslator() :
        m_objective(0), m_teamExists(false), m_clickDownCoord(), m_clickUpCoord(), m_clickDownTime(0), m_clickUpTime(0)
    {
    }
    virtual ~CommandTranslator() override {}

    GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;

    enum CommandEvaluateType
    {
        DO_COMMAND,
        DO_HINT,
        EVALUATE_ONLY,
    };

private:
    int m_objective;
    bool m_teamExists;
    ICoord2D m_clickDownCoord;
    ICoord2D m_clickUpCoord;
    int m_clickDownTime;
    int m_clickUpTime;
};

class GameClient : public SubsystemInterface, public SnapShot
{
    struct DrawableTOCEntry
    {
        Utf8String name;
        uint16_t id;
    };

    enum
    {
        MAX_CLIENT_TRANSLATORS = 32,
    };

public:
    GameClient();
    virtual ~GameClient();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

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
    virtual Drawable *Create_Drawable(const ThingTemplate *temp, DrawableStatus status) = 0;
    virtual void Destroy_Drawable(Drawable *drawable);
    virtual void Set_Time_Of_Day(TimeOfDayType time);
    virtual void Select_Drawables_In_Group(int group) {}
    virtual void Assign_Selected_Drawables_To_Group(int group) {}

    virtual uint32_t Get_Frame() { return m_frame; }

    virtual void Set_Team_Color(int red, int blue, int green) = 0;

    virtual void Adjust_LOD(int lod) = 0;

    virtual void Release_Shadows();
    virtual void Allocate_Shadows();

    virtual void Preload_Assets(TimeOfDayType tod);

    virtual Drawable *Get_Drawable_List() { return m_drawableList; }

    virtual int Notify_Terrain_Object_Moved(Object *obj) = 0;

    virtual void *Create_GameDisplay() = 0;
    virtual void *Create_InGameUI() = 0;
    virtual void *Create_WindowManager() = 0;
    virtual void *Create_FontLibrary() = 0;
    virtual void *Create_DisplayStringManager() = 0;
    virtual void *Create_VideoPlayer() = 0;
    virtual void *Create_TerrainVisual() = 0;
    virtual void *Create_Keyboard() = 0;
    virtual void *Create_Mouse() = 0;
    virtual void *Create_SnowManager() = 0;

    virtual void Set_Frame_Rate(float fps) = 0;

    DrawableID Alloc_Drawable_ID()
    {
        DrawableID id = m_nextDrawableID;
        m_nextDrawableID = DrawableID(id + 1);
        return id;
    }

    DrawableID Get_Next_DrawableID() { return m_nextDrawableID; }
    void Set_Next_DrawableID(DrawableID id) { m_nextDrawableID = id; }

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
    std::list<void *> m_drawableTB; // Text Bearing drawables.
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
