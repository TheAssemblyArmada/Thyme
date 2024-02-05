/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for managing build information.
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
#include "datachunk.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"

class RenderObjClass;
class Shadow;

class BuildListInfo : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(BuildListInfo);

    enum
    {
        UNLIMITED_REBUILDS = -1,
    };

protected:
    virtual ~BuildListInfo() override;

public:
    BuildListInfo();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    BuildListInfo &operator=(const BuildListInfo &that);

    BuildListInfo *Duplicate();

    void Decrement_Num_Rebuilds()
    {
        if (m_numRebuilds != 0) {
            if (m_numRebuilds != -1) {
                m_numRebuilds--;
            }
        }
    }

    void Increment_Num_Rebuilds()
    {
        if (m_numRebuilds != -1) {
            m_numRebuilds++;
        }
    }

    BuildListInfo *Get_Next() const { return m_nextBuildList; }
    Utf8String Get_Building_Name() const { return m_buildingName; }
    Utf8String Get_Template_Name() const { return m_templateName; }
    Utf8String Get_Script() const { return m_script; }
    unsigned int Get_Num_Rebuilds() const { return m_numRebuilds; }
    const Coord3D *Get_Location() const { return &m_location; }
    float Get_Angle() const { return m_angle; }
    int Get_Health() const { return m_health; }
    bool Get_Sellable() const { return m_sellable; }
    bool Get_Whiner() const { return m_whiner; }
    bool Get_Repairable() const { return m_repairable; }
    RenderObjClass *Get_Render_Obj() const { return m_renderObj; }
    ObjectID Get_Object_ID() const { return m_objectID; }
    int Get_Max_Resource_Gatherers() const { return m_maxResourceGatherers; }
    int Get_Resource_Gatherers() const { return m_resourceGatherers; }
    const Coord2D *Get_Rally_Point_Offset() const { return &m_rallyPointOffset; }
    unsigned int Get_Object_Timestamp() const { return m_objectTimestamp; }
    bool Get_Unk4() const { return m_unkbool4; }
    bool Get_Auto_Build() const { return m_autoBuild; }

    bool Is_Initially_Built() const { return m_isInitiallyBuilt; }
    bool Is_Selected() const { return m_selected; }
    bool Is_Supply_Center() const { return m_isSupplyCenter; }
    bool Is_Buildable() const { return Get_Num_Rebuilds() > 0 || Get_Num_Rebuilds() == -1; }

    void Set_Next_Build_List(BuildListInfo *next) { m_nextBuildList = next; }
    void Set_Building_Name(Utf8String name) { m_buildingName = name; }
    void Set_Template_Name(Utf8String name) { m_templateName = name; }
    void Set_Script(Utf8String script) { m_script = script; }
    void Set_Location(const Coord3D &location) { m_location = location; }
    void Set_Angle(float angle) { m_angle = angle; }
    void Set_Initially_Built(bool built) { m_isInitiallyBuilt = built; }
    void Set_Object_ID(ObjectID id) { m_objectID = id; }
    void Set_Num_Rebuilds(unsigned int num) { m_numRebuilds = num; }
    void Set_Unk4() { m_unkbool4 = true; }
    void Set_Selected(bool set) { m_selected = set; }
    void Set_Sellable(bool set) { m_sellable = set; }
    void Set_Repairable(bool set) { m_repairable = set; }
    void Set_Whiner(bool set) { m_whiner = set; }
    void Set_Health(int health) { m_health = health; }
    void Set_Render_Obj(RenderObjClass *robj) { m_renderObj = robj; }
    void Set_Shadow_Obj(Shadow *shadow) { m_shadowObj = shadow; }
    void Set_Under_Construction(bool set) { m_underConstruction = set; }
    void Set_Is_Supply_Center(bool set) { m_isSupplyCenter = set; }
    void Set_Max_Resource_Gatherers(int max) { m_maxResourceGatherers = max; }
    void Set_Resource_Gatherers(int max) { m_resourceGatherers = max; }
    void Set_Object_Timestamp(unsigned int time) { m_objectTimestamp = time; }

    void Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info);
    static void Parse_Structure(INI *ini, void *formal, void *store, const void *user_data);

private:
    Utf8String m_buildingName;
    Utf8String m_templateName;
    Coord3D m_location;
    Coord2D m_rallyPointOffset;
    float m_angle;
    bool m_isInitiallyBuilt;
    unsigned int m_numRebuilds;
    BuildListInfo *m_nextBuildList;
    Utf8String m_script;
    int m_health;
    bool m_whiner;
    bool m_sellable;
    bool m_repairable;
    bool m_autoBuild;
    RenderObjClass *m_renderObj;
    Shadow *m_shadowObj;
    bool m_selected;
    ObjectID m_objectID;
    unsigned int m_objectTimestamp;
    bool m_underConstruction;
    int m_unkArray[10];
    bool m_isSupplyCenter;
    int m_maxResourceGatherers;
    int m_resourceGatherers;
    bool m_unkbool4;
};
