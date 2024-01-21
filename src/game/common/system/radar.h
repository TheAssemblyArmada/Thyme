/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radar
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
#include "color.h"
#include "coord.h"
#include "gametype.h"
#include "scriptparam.h"
#include "snapshot.h"
#include "subsysteminterface.h"

class TerrainLogic;
class RadarObject;
class GameWindow;
class Object;

class Radar : public SnapShot, public SubsystemInterface
{
public:
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual void Refresh_Terrain(TerrainLogic *logic);
    virtual void Queue_Terrain_Refresh();
    virtual void New_Map(TerrainLogic *logic);
    virtual void Draw(int i1, int i2, int i3, int i4) = 0;
    virtual void Clear_Shroud() = 0;
    virtual void Set_Shroud_Level(int i1, int i2, CellShroudStatus status) = 0;

    virtual ~Radar() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    void Remove_Object(Object *obj);
    void Add_Object(Object *obj);
    void Try_Under_Attack_Event(Object *obj);
    void Try_Infiltration_Event(Object *obj);
    void Try_Event(RadarEventType event, const Coord3D *pos);
    bool Local_Pixel_To_Radar(const ICoord2D *pixel, ICoord2D *radar);
    bool Radar_To_World(const ICoord2D *radar, Coord3D *world);
    bool Screen_Pixel_To_World(const ICoord2D *screen, Coord3D *world);

    void Force_On(bool on) { m_radarForceOn = on; }
    bool Is_Radar_Forced() const { return m_radarForceOn; }
    bool Is_Radar_Hidden() const { return m_radarHidden; }
    bool Is_Radar_Window(GameWindow *window) const { return m_radarWindow == window && m_radarWindow != nullptr; }

private:
    enum
    {
        MAX_RADAR_EVENTS = 64
    };

    struct RadarEvent
    {
        int type;
        bool active;
        int create_frame;
        int die_frame;
        int fade_frame;
        RGBAColorInt color1;
        RGBAColorInt color2;
        Coord3D world_loc;
        ICoord2D radar_loc;
        bool sound_played;
    };

    bool m_radarHidden;
    bool m_radarForceOn;
    RadarObject *m_objectList;
    RadarObject *m_localObjectList;
    float m_terrainAverageZ;
    float m_waterAverageZ;
    float m_xSample;
    float m_ySample;
    RadarEvent m_event[MAX_RADAR_EVENTS];
    int m_nextFreeRadarEvent;
    int m_lastRadarEvent;
    GameWindow *m_radarWindow;
    Region3D m_mapExtent;
    unsigned int m_refreshTerrain;
};

#ifdef GAME_DLL
extern Radar *&g_theRadar;
#else
extern Radar *g_theRadar;
#endif
