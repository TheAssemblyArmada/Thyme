/**
 * @file
 *
 * @author xezon
 *
 * @brief Specter Gunship Deployment Update Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "spectregunshipdeploymentupdate.h"

#include "terrainlogic.h"
#include "thing.h"

// wb: 0096E380
SpectreGunshipDeploymentUpdateModuleData::SpectreGunshipDeploymentUpdateModuleData()
    : ModuleData()
    , m_gunshipTemplateName()
    , m_unk_0018()
    , m_radiusDecal1()
    , m_radiusDecal2()
{
    m_specialPowerTemplate = nullptr;
    m_requiredScience = SCIENCE_INVALID;
    m_attackAreaRadius = 200.0f;
    m_createLocType = GUNSHIP_CREATE_AT_EDGE_FARTHEST_FROM_TARGET;
}

SpectreGunshipDeploymentUpdateModuleData::~SpectreGunshipDeploymentUpdateModuleData()
{
    // #TODO
}

// wb: 0096E42C
void SpectreGunshipDeploymentUpdateModuleData::Build_Field_Parse(MultiIniFieldParse &parse)
{
    // wb: 00D54820
    static const char *const _TheGunshipCreateLocTypeNames[GUNSHIP_CREATE_LOC_COUNT + 1] = {
        "CREATE_AT_EDGE_NEAR_SOURCE",
        "CREATE_AT_EDGE_FARTHEST_FROM_SOURCE",
        "CREATE_AT_EDGE_NEAR_TARGET",
        "CREATE_AT_EDGE_FARTHEST_FROM_TARGET",
        nullptr
    };

    // wb: 00C38E08
    static const FieldParse dataFieldParse[] = {
        { "GunshipTemplateName",
            &INI::Parse_AsciiString,
            nullptr,
            offsetof(SpectreGunshipDeploymentUpdateModuleData, m_gunshipTemplateName) },
        { "RequiredScience",
            &INI::Parse_Science,
            nullptr,
            offsetof(SpectreGunshipDeploymentUpdateModuleData, m_requiredScience) },
        { "SpecialPowerTemplate",
            &INI::Parse_SpecialPowerTemplate,
            nullptr,
            offsetof(SpectreGunshipDeploymentUpdateModuleData, m_specialPowerTemplate) },
        { "AttackAreaRadius",
            &INI::Parse_Real,
            nullptr,
            offsetof(SpectreGunshipDeploymentUpdateModuleData, m_attackAreaRadius) },
        { "CreateLocation",
            &INI::Parse_Index_List,
            _TheGunshipCreateLocTypeNames,
            offsetof(SpectreGunshipDeploymentUpdateModuleData, m_createLocType) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ModuleData::Build_Field_Parse(parse);
    parse.Add(dataFieldParse, 0);
}

SpectreGunshipDeploymentUpdate::SpectreGunshipDeploymentUpdate(Thing *thing, const ModuleData *module_data) :
    SpecialPowerUpdateModule(thing, module_data)
{
    // TODO
}

// wb: 007872B0
SpectreGunshipDeploymentUpdate *SpectreGunshipDeploymentUpdate::Friend_New_Module_Instance(
    Thing *thing, const ModuleData *module_data)
{
    return new SpectreGunshipDeploymentUpdate(thing, module_data);
}

// wb: 00787520
SpectreGunshipDeploymentUpdateModuleData *SpectreGunshipDeploymentUpdate::Friend_New_Module_Data(INI *ini)
{
    SpectreGunshipDeploymentUpdateModuleData *data = new SpectreGunshipDeploymentUpdateModuleData();

    if (ini != nullptr) {
        ini->Init_From_INI_Multi_Proc(data, SpectreGunshipDeploymentUpdateModuleData::Build_Field_Parse);
    }

    return data;
}

NameKeyType SpectreGunshipDeploymentUpdate::Get_Module_Name_Key(void) const
{
    static const NameKeyType s_key = Name_To_Key("SpectreGunshipDeploymentUpdate");
    return s_key;
}

void SpectreGunshipDeploymentUpdate::On_Object_Created()
{
    // TODO
}

void SpectreGunshipDeploymentUpdate::CRC_Snapshot(Xfer *xfer)
{
    // TODO
}

void SpectreGunshipDeploymentUpdate::Xfer_Snapshot(Xfer *xfer)
{
    // TODO
}

void SpectreGunshipDeploymentUpdate::Load_Post_Process()
{
    // TODO
}

SpecialPowerUpdateInterface *SpectreGunshipDeploymentUpdate::Get_Special_Power_Update_Interface()
{
    // TODO
    return nullptr;
}

UpdateSleepTime SpectreGunshipDeploymentUpdate::Update()
{
    // TODO
    return UpdateSleepTime();
}

DisabledBitFlags SpectreGunshipDeploymentUpdate::Get_Disabled_Types_To_Process() const
{
    // TODO
    return DisabledBitFlags();
}

bool SpectreGunshipDeploymentUpdate::Does_Special_Power_Update_Pass_Science_Test() const
{
    // TODO
    return false;
}

ScienceType SpectreGunshipDeploymentUpdate::Get_Extra_Required_Science() const
{
    // TODO
    return SCIENCE_INVALID;
}

void SpectreGunshipDeploymentUpdate::Initiate_Intent_To_Do_Special_Power(
    const SpecialPowerTemplate *, const Object *, const Coord3D *, const Waypoint *, unsigned int)
{
    // TODO
}

bool SpectreGunshipDeploymentUpdate::Is_Special_Ability() const
{
    return false;
}

bool SpectreGunshipDeploymentUpdate::Is_Special_Power() const
{
    return true;
}

bool SpectreGunshipDeploymentUpdate::Is_Active() const
{
    return false;
}

CommandOption SpectreGunshipDeploymentUpdate::Get_Command_Option() const
{
    // TODO
    return CommandOption::COMMAND_OPTION_0;
}

bool SpectreGunshipDeploymentUpdate::Does_Special_Power_Have_Overridable_Destination_Active() const
{
    // TODO
    return false;
}

bool SpectreGunshipDeploymentUpdate::Does_Special_Power_Have_Overridable_Destination() const
{
    // TODO
    return false;
}

void SpectreGunshipDeploymentUpdate::Set_Special_Power_Overridable_Destination(const Coord3D *)
{
    // TODO
}

bool SpectreGunshipDeploymentUpdate::Is_Power_Currently_In_Use(const CommandButton *) const
{
    // TODO
    return false;
}
