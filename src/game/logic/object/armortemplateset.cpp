/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Templated information for object armor sets.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "armortemplateset.h"
#include <cstddef>

template<>
const char *BitFlags<ARMORSET_COUNT>::s_bitNamesList[ARMORSET_COUNT + 1] = { "VETERAN",
    "ELITE",
    "HERO",
    "PLAYER_UPGRADE",
    "WEAK_VERSUS_BASEDEFENSES",
    "SECOND_LIFE",
    "CRATE_UPGRADE_ONE",
    "CRATE_UPGRADE_TWO",
    nullptr };

/**
 * @brief Parse a armor template set from an ini file.
 *
 * inlined at 0x0058A877
 */
void ArmorTemplateSet::Parse_Armor_Template_Set(INI *ini)
{
    // TODO Requires ArmorTemplate and DamageFX parsers.
#ifdef GAME_DLL
    static FieldParse _parse_table[] = {
        { "Conditions", BitFlags<ARMORSET_COUNT>::Parse_From_INI, nullptr, offsetof(ArmorTemplateSet, m_conditions) },
        // { "Armor", &INI::parseArmorTemplate, nullptr, offsetof(ArmorTemplateSet, m_template) },
        // { "DamageFX", &INI::parseDamageFX, nullptr, offsetof(ArmorTemplateSet, m_fx) },
        { "Armor", (inifieldparse_t)0x0041CBB0, nullptr, offsetof(ArmorTemplateSet, m_template) },
        { "DamageFX", (inifieldparse_t)0x0041CDD0, nullptr, offsetof(ArmorTemplateSet, m_fx) },
        { nullptr, nullptr, nullptr, 0 }
    };

    ini->Init_From_INI(this, _parse_table);
#endif
}

void ArmorTemplateSet::Clear()
{
    m_conditions.Clear();
    m_template = nullptr;
    m_fx = nullptr;
}
