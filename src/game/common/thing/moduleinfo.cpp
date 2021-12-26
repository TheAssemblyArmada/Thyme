/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for tracking module information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "moduleinfo.h"
#include "module.h"
#include "thingtemplate.h"

ModuleInfo::Nugget *ModuleInfo::Try_Find_Nugget(Utf8String const &name)
{
    for (auto &i : m_info) {
        if (i.tag_name == name) {
            return &i;
        }
    }

    return nullptr;
}

void ModuleInfo::Add_Module_Info(ThingTemplate *thing_template,
    const Utf8String &module_name,
    const Utf8String &tag_name,
    ModuleData *mod_data,
    int interface_mask,
    bool inherited,
    bool like_kind_override)
{
    const ModuleInfo *info = thing_template->Get_Body_Modules();
    const Nugget *nugget = Try_Find_Nugget(tag_name);

    if (nugget) {
        captainslog_relassert(nugget->tag_name != tag_name,
            CODE_06,
            "addModuleInfo - ERROR defining module '%s' on thing template '%s'.  The module '%s' has the tag '%s' which "
            "must be unique among all modules for this object, but the tag '%s' is also already on module '%s' within this "
            "object.\n\nPlease make unique tag names within an object definition\n",
            module_name.Str(),
            thing_template->Get_Name().Str(),
            module_name.Str(),
            tag_name.Str(),
            tag_name.Str(),
            nugget->name.Str());
    }

    info = thing_template->Get_Draw_Modules();
    nugget = Try_Find_Nugget(tag_name);

    if (nugget) {
        captainslog_relassert(nugget->tag_name != tag_name,
            CODE_06,
            "addModuleInfo - ERROR defining module '%s' on thing template '%s'.  The module '%s' has the tag '%s' which "
            "must be unique among all modules for this object, but the tag '%s' is also already on module '%s' within this "
            "object.\n\nPlease make unique tag names within an object definition\n",
            module_name.Str(),
            thing_template->Get_Name().Str(),
            module_name.Str(),
            tag_name.Str(),
            tag_name.Str(),
            nugget->name.Str());
    }

    info = thing_template->Get_Client_Update_Modules();
    nugget = Try_Find_Nugget(tag_name);

    if (nugget) {
        captainslog_relassert(nugget->tag_name != tag_name,
            CODE_06,
            "addModuleInfo - ERROR defining module '%s' on thing template '%s'.  The module '%s' has the tag '%s' which "
            "must be unique among all modules for this object, but the tag '%s' is also already on module '%s' within this "
            "object.\n\nPlease make unique tag names within an object definition\n",
            module_name.Str(),
            thing_template->Get_Name().Str(),
            module_name.Str(),
            tag_name.Str(),
            tag_name.Str(),
            nugget->name.Str());
    }

    m_info.push_back(Nugget(module_name, tag_name, mod_data, interface_mask, inherited, like_kind_override));
}

bool ModuleInfo::Clear_Module_Data_With_Tag(const Utf8String &tag, Utf8String &name)
{
    bool ret = false;
    auto nugget = m_info.begin();

    while (nugget != m_info.end()) {
        if (nugget->tag_name == tag) {
            captainslog_dbgassert(
                !ret, "Hmm, multiple clears in ModuleInfo::clearModuleDataWithTag, should this be possible?");
            name = nugget->name;
            nugget = m_info.erase(nugget);
            ret = true;
        } else {
            nugget++;
        }
    }

    return ret;
}

bool ModuleInfo::Clear_Copied_From_Default_Entries(int interface_mask, Utf8String const &name, ThingTemplate *tmplate)
{
    static BitFlags<KINDOF_COUNT> ImmuneToGPSScramblerMask;
    ImmuneToGPSScramblerMask.Set(KINDOF_AIRCRAFT, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_SHRUBBERY, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_OPTIMIZED_TREE, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_STRUCTURE, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_DRAWABLE_ONLY, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_MOB_NEXUS, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_IGNORED_IN_GUI, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_CLEARED_BY_BUILD, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_DEFENSIVE_WALL, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_BALLISTIC_MISSILE, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_SUPPLY_SOURCE, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_BOAT, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_INERT, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_BRIDGE, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_LANDMARK_BRIDGE, true);
    ImmuneToGPSScramblerMask.Set(KINDOF_BRIDGE_TOWER, true);
    bool immunetogps = tmplate->Is_Any_KindOf(ImmuneToGPSScramblerMask);

    static BitFlags<KINDOF_COUNT> CandidateForGPSScramblerMask;
    CandidateForGPSScramblerMask.Set(KINDOF_SCORE, true);
    CandidateForGPSScramblerMask.Set(KINDOF_VEHICLE, true);
    CandidateForGPSScramblerMask.Set(KINDOF_INFANTRY, true);
    CandidateForGPSScramblerMask.Set(KINDOF_PORTABLE_STRUCTURE, true);
    bool candidateforgps = tmplate->Is_Any_KindOf(CandidateForGPSScramblerMask);

    bool ret = false;
    auto nugget = m_info.begin();

    for (;;) {
        if (nugget == m_info.end()) {
            return ret;
        }

        if ((nugget->interface_mask & interface_mask) && nugget->copied_from_default) {
            if (nugget->inherited) {
                if (nugget->tag_name.Compare("ModuleTag_DefaultAutoHealBehavior") || tmplate->Is_Trainable()) {
                    nugget++;
                } else {
                    nugget = m_info.erase(nugget);
                    ret = true;
                }
            } else if (nugget->like_kind_override) {
                Utf8String str(nugget->name);

                if (str == name || immunetogps || !candidateforgps) {
                    nugget = m_info.erase(nugget);
                    ret = true;
                } else {
                    nugget++;
                }
            } else {
                nugget = m_info.erase(nugget);
                ret = true;
            }
        } else {
            nugget++;
        }
    }

    return ret;
}

bool ModuleInfo::Clear_Ai_Module_Info()
{
    bool clear = false;
    auto nugget = m_info.begin();

    while (nugget != m_info.end()) {
        if (nugget->data->Is_AI_Module_Data()) {
            nugget = m_info.erase(nugget);
            clear = true;
        } else {
            nugget++;
        }
    }

    return clear;
}
