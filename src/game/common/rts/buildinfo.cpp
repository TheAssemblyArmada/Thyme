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
#include "buildinfo.h"
#include "ai.h"
#include "xfer.h"

BuildListInfo::BuildListInfo() :
    m_buildingName(),
    m_templateName(),
    m_angle(0.0f),
    m_isInitiallyBuilt(false),
    m_numRebuilds(0),
    m_nextBuildList(nullptr),
    m_script(),
    m_health(100),
    m_whiner(true),
    m_sellable(false),
    m_repairable(true),
    m_autoBuild(true),
    m_renderObj(nullptr),
    m_shadowObj(nullptr),
    m_selected(false),
    m_objectID(INVALID_OBJECT_ID),
    m_objectTimestamp(0),
    m_underConstruction(false),
    m_isSupplyCenter(false),
    m_maxResourceGatherers(0),
    m_resourceGatherers(0),
    m_unkbool4(false)
{
    m_location.Zero();
    m_rallyPointOffset.x = 0.0f;
    m_rallyPointOffset.y = 0.0f;
    memset(m_unkArray, 0, sizeof(m_unkArray));
}

BuildListInfo::~BuildListInfo()
{
    for (BuildListInfo *next = m_nextBuildList, *saved = nullptr; next != nullptr; next = saved) {
        saved = next->m_nextBuildList;
        next->m_nextBuildList = nullptr;
        next->Delete_Instance();
    }
}

/**
 * @brief Uses the passed Xfer object to perform transfer of this build info list.
 *
 * 0x004D8E50
 */
void BuildListInfo::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 2;
    xfer->xferVersion(&version, 2);
    xfer->xferAsciiString(&m_buildingName);
    xfer->xferAsciiString(&m_templateName);
    xfer->xferCoord3D(&m_location);
    xfer->xferCoord2D(&m_rallyPointOffset);
    xfer->xferReal(&m_angle);
    xfer->xferBool(&m_isInitiallyBuilt);
    xfer->xferUnsignedInt(&m_numRebuilds);
    xfer->xferAsciiString(&m_script);
    xfer->xferInt(&m_health);
    xfer->xferBool(&m_whiner);
    xfer->xferBool(&m_sellable);
    xfer->xferBool(&m_repairable);
    xfer->xferBool(&m_autoBuild);
    xfer->xferObjectID(&m_objectID);
    xfer->xferUnsignedInt(&m_objectTimestamp);
    xfer->xferBool(&m_underConstruction);
    xfer->xferUser(m_unkArray, sizeof(m_unkArray));
    xfer->xferBool(&m_isSupplyCenter);
    xfer->xferInt(&m_maxResourceGatherers);
    xfer->xferBool(&m_unkbool4);

    if (version >= 2) {
        xfer->xferInt(&m_resourceGatherers);
    }
}

BuildListInfo &BuildListInfo::operator=(const BuildListInfo &that)
{
    if (this != &that) {
        m_buildingName = that.m_buildingName;
        m_templateName = that.m_templateName;
        m_location = that.m_location;
        m_rallyPointOffset = that.m_rallyPointOffset;
        m_angle = that.m_angle;
        m_isInitiallyBuilt = that.m_isInitiallyBuilt;
        m_numRebuilds = that.m_numRebuilds;
        m_nextBuildList = that.m_nextBuildList;
        m_script = that.m_script;
        m_health = that.m_health;
        m_whiner = that.m_whiner;
        m_sellable = that.m_sellable;
        m_repairable = that.m_repairable;
        m_autoBuild = that.m_autoBuild;
        m_renderObj = that.m_renderObj;
        m_shadowObj = that.m_shadowObj;
        m_selected = that.m_selected;
        m_objectID = that.m_objectID;
        m_objectTimestamp = that.m_objectTimestamp;
        m_underConstruction = that.m_underConstruction;
        memcpy(m_unkArray, that.m_unkArray, sizeof(m_unkArray));
        m_isSupplyCenter = that.m_isSupplyCenter;
        m_maxResourceGatherers = that.m_maxResourceGatherers;
        m_resourceGatherers = that.m_resourceGatherers;
        m_unkbool4 = that.m_unkbool4;
    }

    return *this;
}

/**
 * @brief Parses data from the current data chunk into this object.
 */
void BuildListInfo::Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info)
{
    Set_Building_Name(input.Read_AsciiString());
    Set_Template_Name(input.Read_AsciiString());
    Coord3D location;
    location.x = input.Read_Real32();
    location.y = input.Read_Real32();
    input.Read_Real32();
    location.z = 0; // Confirmed that this throws away the return value of Read_Real
    Set_Location(location);
    Set_Angle(input.Read_Real32());
    Set_Initially_Built(input.Read_Byte() != 0);
    Set_Num_Rebuilds(input.Read_Int32());

    if (info->version >= 3) {
        Set_Script(input.Read_AsciiString());
        Set_Health(input.Read_Int32());
        Set_Whiner(input.Read_Byte() != 0);
        Set_Sellable(input.Read_Byte() != 0);
        Set_Repairable(input.Read_Byte() != 0);
    }
}

void BuildListInfo::Parse_Structure(INI *ini, void *formal, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = {
        { "Name", &INI::Parse_AsciiString, nullptr, offsetof(BuildListInfo, m_buildingName) },
        { "Location", &INI::Parse_Coord2D, nullptr, offsetof(BuildListInfo, m_location) },
        { "Rebuilds", &INI::Parse_Int, nullptr, offsetof(BuildListInfo, m_numRebuilds) },
        { "Angle", &INI::Parse_Angle_Real, nullptr, offsetof(BuildListInfo, m_angle) },
        { "InitiallyBuilt", &INI::Parse_Bool, nullptr, offsetof(BuildListInfo, m_isInitiallyBuilt) },
        { "RallyPointOffset", &INI::Parse_Coord2D, nullptr, offsetof(BuildListInfo, m_rallyPointOffset) },
        { "AutomaticallyBuild", &INI::Parse_Bool, nullptr, offsetof(BuildListInfo, m_autoBuild) },
        { nullptr, nullptr, nullptr, 0 }
    };

    Utf8String name(ini->Get_Next_Token());
    BuildListInfo *info = new BuildListInfo();
    info->Set_Template_Name(name);
    ini->Init_From_INI(info, myFieldParse);
    static_cast<AISideBuildList *>(formal)->Add_Info(info);
}

BuildListInfo *BuildListInfo::Duplicate()
{
    BuildListInfo *head = new BuildListInfo();
    *head = *this;
    head->m_nextBuildList = nullptr;
    BuildListInfo *list = m_nextBuildList;
    BuildListInfo *tail = head;

    while (list != nullptr) {
        BuildListInfo *next = new BuildListInfo();
        *next = *list;
        next->m_nextBuildList = nullptr;
        tail->m_nextBuildList = next;
        tail = next;
        list = list->m_nextBuildList;
    }

    return head;
}
