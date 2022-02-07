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
    m_repairable(false),
    m_sellable(true),
    m_autoBuild(true),
    m_renderObj(nullptr),
    m_shadowObj(nullptr),
    m_selected(false),
    m_objectID(OBJECT_UNK),
    m_objectTimestamp(0),
    m_underConstruction(false),
    m_unkbool3(false),
    m_unkint1(0),
    m_unkint2(0),
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
    xfer->xferBool(&m_repairable);
    xfer->xferBool(&m_sellable);
    xfer->xferBool(&m_autoBuild);
    xfer->xferObjectID(&m_objectID);
    xfer->xferUnsignedInt(&m_objectTimestamp);
    xfer->xferBool(&m_underConstruction);
    xfer->xferUser(m_unkArray, sizeof(m_unkArray));
    xfer->xferBool(&m_unkbool3);
    xfer->xferInt(&m_unkint1);
    xfer->xferBool(&m_unkbool4);

    if (version >= 2) {
        xfer->xferInt(&m_unkint2);
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
        m_repairable = that.m_repairable;
        m_sellable = that.m_sellable;
        m_autoBuild = that.m_autoBuild;
        m_renderObj = that.m_renderObj;
        m_shadowObj = that.m_shadowObj;
        m_selected = that.m_selected;
        m_objectID = that.m_objectID;
        m_objectTimestamp = that.m_objectTimestamp;
        m_underConstruction = that.m_underConstruction;
        memcpy(m_unkArray, that.m_unkArray, sizeof(m_unkArray));
        m_unkbool3 = that.m_unkbool3;
        m_unkint1 = that.m_unkint1;
        m_unkint2 = that.m_unkint2;
        m_unkbool4 = that.m_unkbool4;
    }

    return *this;
}

/**
 * @brief Parses data from the current data chunk into this object.
 */
void BuildListInfo::Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info)
{
    m_buildingName = input.Read_AsciiString();
    m_templateName = input.Read_AsciiString();
    m_location.x = input.Read_Real32();
    m_location.y = input.Read_Real32();
    m_location.z = input.Read_Real32();
    m_angle = input.Read_Real32();
    m_isInitiallyBuilt = input.Read_Byte() != 0;
    m_numRebuilds = input.Read_Int32();

    if (info->version >= 3) {
        m_script = input.Read_AsciiString();
        m_health = input.Read_Int32();
        m_whiner = input.Read_Byte() != 0;
        m_repairable = input.Read_Byte() != 0;
        m_sellable = input.Read_Byte() != 0;
    }
}
