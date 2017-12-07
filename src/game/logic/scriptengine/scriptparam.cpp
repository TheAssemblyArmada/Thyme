/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing action and condition parameters.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptparam.h"
#include "gamemath.h"
#include "gametype.h"
#include "stringex.h"

// TODO move this somewhere more appropriate?
const char *BitFlags<OBJECT_STATUS_COUNT>::s_bitNamesList[] = {"NONE",
    "DESTROYED",
    "CAN_ATTACK",
    "UNDER_CONSTRUCTION",
    "UNSELECTABLE",
    "NO_COLLISIONS",
    "NO_ATTACK",
    "AIRBORNE_TARGET",
    "PARACHUTING",
    "REPULSOR",
    "HIJACKED",
    "AFLAME",
    "BURNED",
    "WET",
    "IS_FIRING_WEAPON",
    "IS_BRAKING",
    "STEALTHED",
    "DETECTED",
    "CAN_STEALTH",
    "SOLD",
    "UNDERGOING_REPAIR",
    "RECONSTRUCTING",
    "MASKED",
    "IS_ATTACKING",
    "USING_ABILITY",
    "IS_AIMING_WEAPON",
    "NO_ATTACK_FROM_AI",
    "IGNORING_STEALTH",
    "IS_CARBOMB",
    "DECK_HEIGHT_OFFSET",
    "STATUS_RIDER1",
    "STATUS_RIDER2",
    "STATUS_RIDER3",
    "STATUS_RIDER4",
    "STATUS_RIDER5",
    "STATUS_RIDER6",
    "STATUS_RIDER7",
    "STATUS_RIDER8",
    "FAERIE_FIRE",
    "KILLING_SELF",
    "REASSIGN_PARKING",
    "BOOBY_TRAPPED",
    "IMMOBILE",
    "DISGUISED",
    "DEPLOYED",
    nullptr};

const char *BitFlags<KIND_OF_COUNT>::s_bitNamesList[] = {"OBSTACLE",
    "SELECTABLE",
    "IMMOBILE",
    "CAN_ATTACK",
    "STICK_TO_TERRAIN_SLOPE",
    "CAN_CAST_REFLECTIONS",
    "SHRUBBERY",
    "STRUCTURE",
    "INFANTRY",
    "VEHICLE",
    "AIRCRAFT",
    "HUGE_VEHICLE",
    "DOZER",
    "HARVESTER",
    "COMMANDCENTER",
    "LINEBUILD",
    "SALVAGER",
    "WEAPON_SALVAGER",
    "TRANSPORT",
    "BRIDGE",
    "LANDMARK_BRIDGE",
    "BRIDGE_TOWER",
    "PROJECTILE",
    "PRELOAD",
    "NO_GARRISON",
    "WAVEGUIDE",
    "WAVE_EFFECT",
    "NO_COLLIDE",
    "REPAIR_PAD",
    "HEAL_PAD",
    "STEALTH_GARRISON",
    "CASH_GENERATOR",
    "DRAWABLE_ONLY",
    "MP_COUNT_FOR_VICTORY",
    "REBUILD_HOLE",
    "SCORE",
    "SCORE_CREATE",
    "SCORE_DESTROY",
    "NO_HEAL_ICON",
    "CAN_RAPPEL",
    "PARACHUTABLE",
    "CAN_BE_REPULSED",
    "MOB_NEXUS",
    "IGNORED_IN_GUI",
    "CRATE",
    "CAPTURABLE",
    "CLEARED_BY_BUILD",
    "SMALL_MISSILE",
    "ALWAYS_VISIBLE",
    "UNATTACKABLE",
    "MINE",
    "CLEANUP_HAZARD",
    "PORTABLE_STRUCTURE",
    "ALWAYS_SELECTABLE",
    "ATTACK_NEEDS_LINE_OF_SIGHT",
    "WALK_ON_TOP_OF_WALL",
    "DEFENSIVE_WALL",
    "FS_POWER",
    "FS_FACTORY",
    "FS_BASE_DEFENSE",
    "FS_TECHNOLOGY",
    "AIRCRAFT_PATH_AROUND",
    "LOW_OVERLAPPABLE",
    "FORCEATTACKABLE",
    "AUTO_RALLYPOINT",
    "TECH_BUILDING",
    "POWERED",
    "PRODUCED_AT_HELIPAD",
    "DRONE",
    "CAN_SEE_THROUGH_STRUCTURE",
    "BALLISTIC_MISSILE",
    "CLICK_THROUGH",
    "SUPPLY_SOURCE_ON_PREVIEW",
    "PARACHUTE",
    "GARRISONABLE_UNTIL_DESTROYED",
    "BOAT",
    "IMMUNE_TO_CAPTURE",
    "HULK",
    "SHOW_PORTRAIT_WHEN_CONTROLLED",
    "SPAWNS_ARE_THE_WEAPONS",
    "CANNOT_BUILD_NEAR_SUPPLIES",
    "SUPPLY_SOURCE",
    "REVEAL_TO_ALL",
    "DISGUISER",
    "INERT",
    "HERO",
    "IGNORES_SELECT_ALL",
    "DONT_AUTO_CRUSH_INFANTRY",
    "CLIFF_JUMPER",
    "FS_SUPPLY_DROPZONE",
    "FS_SUPERWEAPON",
    "FS_BLACK_MARKET",
    "FS_SUPPLY_CENTER",
    "FS_STRATEGY_CENTER",
    "MONEY_HACKER",
    "ARMOR_SALVAGER",
    "REVEALS_ENEMY_PATHS",
    "BOOBY_TRAP",
    "FS_FAKE",
    "FS_INTERNET_CENTER",
    "BLAST_CRATER",
    "PROP",
    "OPTIMIZED_TREE",
    "FS_ADVANCED_TECH",
    "FS_BARRACKS",
    "FS_WARFACTORY",
    "FS_AIRFIELD",
    "AIRCRAFT_CARRIER",
    "NO_SELECT",
    "REJECT_UNMANNED",
    "CANNOT_RETALIATE",
    "TECH_BASE_DEFENSE",
    "EMP_HARDENED",
    "DEMOTRAP",
    "CONSERVATIVE_BUILDING",
    "IGNORE_DOCKING_BONES",
    nullptr};

Parameter::Parameter(ParameterType type) :
    m_type(type),
    m_initialized(false),
    m_int(0),
    m_real(0.0f),
    m_string(),
    m_coord{0.0f, 0.0f, 0.0f},
    m_objStatus()
{
}

/**
 * @brief Get the UI string for this parameter.
 *
 * 0x0051EB90
 */
AsciiString Parameter::Get_UI_Text()
{
    // Dunno where this is supposed to be from, only appears used here.
    static BorderColor _border_colors[] = {
        { "Orange", 0xFFFF8700 },
        { "Green", 0xFF00FF00 },
        { "Blue", 0xFF0000FF },
        { "Cyan", 0xFF00FFFF },
        { "Magenta", 0xFFFF00FF },
        { "Yellow", 0xFFFFFF00 },
        { "Purple", 0xFF9E00FF },
        { "Pink", 0xFFFF8670 }
    };

    AsciiString ui_string = m_string.Is_Empty() ? "???" : m_string;
    AsciiString ui_text;

    switch (m_type) {
        case INT:
            ui_text.Format(" %d ", m_int);
            break;
        case REAL:
            ui_text.Format("%.2f", m_real);
            break;
        case SCRIPT:
            ui_text.Format("Script '%s'", ui_string.Str());
            break;
        case TEAM:
            ui_text.Format("Team '%s'", ui_string.Str());
            break;
        case COUNTER:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case FLAG:
            ui_text.Format("Flag named '%s'", ui_string.Str());
            break;
        case COMPARISON:
            switch (m_int) {
                case LESS_THAN:
                    ui_text = "Less Than";
                    break;
                case LESS_THAN_EQUAL:
                    ui_text = "Less Than or Equal";
                    break;
                case EQUAL:
                    ui_text = "Equal To";
                    break;
                case GREATER_THAN_EQUAL:
                    ui_text = "Greater Than or Equal To";
                    break;
                case GREATER_THAN:
                    ui_text = "Greater Than";
                    break;
                case NOT_EQUAL:
                    ui_text = "Not Equal To";
                    break;
                default:
                    DEBUG_ASSERT_PRINT(false, "Unknown comparison.\n");
                    break;
            };

            break;
        case WAYPOINT:
            ui_text.Format("Waypoint '%s'", ui_string.Str());
            break;
        case BOOLEAN:
            if (m_int == 0) {
                ui_text = "FALSE";
            } else {
                ui_text = "TRUE";
            }
            break;
        case TRIGGER_AREA:
            ui_text.Format(" area '%s'", ui_string.Str());
            break;
        case TEXT_STRING:
            ui_text.Format("String: '%s'", ui_string.Str());
            break;
        case SIDE:
            ui_text.Format("Player '%s'", ui_string.Str());
            break;
        case SOUND:
            ui_text.Format("Sound '%s'", ui_string.Str());
            break;
        case SCRIPT_SUBROUTINE:
            ui_text.Format("Subroutine '%s'", ui_string.Str());
            break;
        case UNIT:
            ui_text.Format("Unit '%s'", ui_string.Str());
            break;
        case OBJECT_TYPE:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case COORD3D:
            ui_text.Format("(%.2f,%.2f,%.2f)", m_coord.x, m_coord.y, m_coord.z);
            break;
        case ANGLE:
            ui_text.Format("%.2f degrees", m_real * 180.0f / GAMEMATH_PI); // convert radians to degrees
            break;
        case TEAM_STATE:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case RELATION:
            switch (m_int) {
                case ENEMIES:
                    ui_text = "Enemy";
                    break;
                case NEUTRAL:
                    ui_text = "Neutral";
                    break;
                case ALLIES:
                    ui_text = "Friend";
                    break;
                default:
                    DEBUG_ASSERT_PRINT(false, "Unknown relationship.\n");
                    break;
            }

            break;
        case AI_MOOD:
            switch (m_int) {
                case AI_MOOD_SLEEP:
                    ui_text = "Sleep";
                    break;
                case AI_MOOD_PASSIVE:
                    ui_text = "Passive";
                    break;
                case AI_MOOD_NORMAL:
                    ui_text = "Normal";
                    break;
                case AI_MOOD_ALERT:
                    ui_text = "Alert";
                    break;
                case AI_MOOD_AGGRESSIVE:
                    ui_text = "Aggressive";
                    break;
                default:
                    DEBUG_ASSERT_PRINT(false, "Unknown AI Mood.\n");
                    break;
            }

            break;
        case DIALOG:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case MUSIC:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case MOVIE:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case WAYPOINT_PATH:
            ui_text.Format("Waypoint Path '%s'", ui_string.Str());
            break;
        case LOCALIZED_TEXT:
            ui_text.Format("Localized String: '%s'", ui_string.Str());
            break;
        case BRIDGE:
            ui_text.Format("Bridge '%s'", ui_string.Str());
            break;
        case KIND_OF_PARAM:
            if (m_int < KIND_OF_OBSTACLE || m_int >= KIND_OF_COUNT) {
                ui_text.Format("Kind is 'unknown kind'", ui_string.Str());
            } else {
                ui_text.Format("Kind is '%s'", BitFlags<KIND_OF_COUNT>::s_bitNamesList[m_int]);
            }
            break;
        case ATTACK_PRIORITY_SET:
            ui_text.Format("Attack priority set '%s'", ui_string.Str());
            break;
        case RADAR_EVENT_TYPE:
            switch (m_int) {
                case RADAR_EVENT_INVALID: // fallthrough
                case RADAR_EVENT_CONSTRUCTION:
                    ui_text = "Construction";
                    break;
                case RADAR_EVENT_UPGRADE:
                    ui_text = "Upgrade";
                    break;
                case RADAR_EVENT_UNDER_ATTACK:
                    ui_text = "Under Attack";
                    break;
                case RADAR_EVENT_INFORMATION:
                    ui_text = "Information";
                    break;
                case RADAR_EVENT_INFILTRATION:
                    ui_text = "Infiltration";
                    break;
                default:
                    break;
            }

            break;
        case SPECIAL_POWER:
            ui_text.Format("Special power '%s'", ui_string.Str());
            break;
        case SCIENCE:
            ui_text.Format("Science '%s'", ui_string.Str());
            break;
        case UPGRADE:
            ui_text.Format("Upgrade '%s'", ui_string.Str());
            break;
        case COMMANDBUTTON_ABILITY:
            ui_text.Format("Ability '%s'", ui_string.Str());
            break;
        case BOUNDARY:
            ui_text.Format("Boundary '%s'", _border_colors[m_int % 8].color_name);
            break;
        case BUILDABLE:
            if (m_int < BSTATUS_YES || m_int >= BSTATUS_COUNT) {
                ui_text.Format("Buildable (???)");
            } else {
                ui_text.Format("Buildable (%s)", g_buildableStatusNames[m_int]);
            }

            break;
        case SURFACES_ALLOWED:
            if (m_int - 1 < SURFACE_GROUND || m_int - 1 >= SURFACE_COUNT) {
                ui_text.Format("Surfaces Allowed: ???");
            } else {
                ui_text.Format("Surfaces Allowed: %s", g_surfaceNames[m_int - 1]);
            }

            break;
        case SHAKE_INTENSITY:
            if (m_int < SHAKE_SUBTLE || m_int >= SHAKE_COUNT) {
                ui_text.Format("Shake Intensity: ???");
            } else {
                ui_text.Format("Shake Intensity: %s", g_shakeIntensityNames[m_int]);
            }
            break;
        case UNK_COMMANDBUTTON_RELATED:
            ui_text.Format("Command button: '%s'", ui_string.Str());
            break;
        case FONT:
            ui_text.Format("Font: '%s'", ui_string.Str());
            break;
        case OBJECT_STATUS:
            if (m_string.Is_Empty()) {
                ui_text.Format("Object Status is '???'");
            } else {
                ui_text.Format("Object Status is '%s'", ui_string.Str());
            }

            break;
        case UNK_ABILITY_RELATED:
            ui_text.Format("Ability '%s'", ui_string.Str());
            break;
        case UNK_STRING_RELATED:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case COLOR:
            ui_text.Format(" R:%d G:%d B:%d ", (m_int & 0xFF0000) >> 16, (m_int & 0xFF00) >> 8, m_int & 0xFF);
            break;
        case EMOTICON:
            ui_text.Format("Emoticon '%s'", ui_string.Str());
            break;
        case OBJECT_FLAG:
            ui_text.Format("Object Flag: '%s'", ui_string.Str());
            break;
        case FACTION_NAME:
            ui_text.Format("Faction Name: '%s'", ui_string.Str());
            break;
        case UNK_INT_VALUE_RELATED:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case REVEAL_NAME:
            ui_text.Format("Reveal Name: '%s'", ui_string.Str());
            break;
        case SCIENCE_AVAILABILITY:
            ui_text.Format("Science availablity '%s'", ui_string.Str());
            break;
        case UNK_POSITON_REALATED:
            switch (m_int) {
                case 1:
                    ui_text = "left";
                    break;
                case 2:
                    ui_text = "right";
                    break;
                case 3:
                    ui_text = "normal (burst from center)";
                    break;
                default:
                    ui_text = "unspecified";
                    break;
            }

            break;
        case PERCENT:
            ui_text.Format("%.2f%%", m_real * 100.0f);
            break;
        default:
            break;
    }

    return ui_text;
}

void Parameter::Get_Coord3D(Coord3D *coord)
{
    DEBUG_ASSERT_PRINT(m_type == COORD3D, "Parameter is not a Coord3D.\n");

    if (m_type == COORD3D) {
        *coord = m_coord;
    }
}

void Parameter::Set_Coord3D(Coord3D *coord)
{
    DEBUG_ASSERT_PRINT(m_type == COORD3D, "Parameter is not a Coord3D.\n");

    if (m_type == COORD3D) {
        m_coord = *coord;
    }
}

/**
 * @brief Read a parameter out of a data chunk file.
 *
 * 0x0051F760
 */
Parameter *Parameter::Read_Parameter(DataChunkInput &input)
{
    ParameterType type = ParameterType(input.Read_Int32());
    Parameter *param = new Parameter(type);

    if (param->m_type == COORD3D) {
        Coord3D tmp;
        tmp.x = input.Read_Real32();
        tmp.y = input.Read_Real32();
        tmp.z = input.Read_Real32();
        param->Set_Coord3D(&tmp);
    } else {
        param->m_int = input.Read_Int32();
        param->m_real = input.Read_Real32();
        param->m_string = input.Read_AsciiString();
    }

    // Some fixup for script parameters referring to Fundamentalist rather than GLAFundamentalist.
    if (param->m_type == OBJECT_TYPE) {
        if (param->m_string.Starts_With("Fundamentalist")) {
            char new_name[256];
            char old_name[256];
            strlcpy(old_name, param->m_string.Str(), sizeof(old_name));
            strlcpy(new_name, "GLA", sizeof(new_name));
            strlcat(new_name, &old_name[strlen("Fundamentalist")], sizeof(new_name));
            param->m_string = new_name;
            DEBUG_LOG("Fixed up script reference from '%s' to '%s'.\n", old_name, new_name);
        }
    }

    // Fix up old side specific capture building upgrade references.
    if (param->m_type == UPGRADE) {
        if (param->m_string == "Upgrade_AmericaRangerCaptureBuilding"
            || param->m_string == "Upgrade_ChinaRedguardCaptureBuilding"
            || param->m_string == "Upgrade_GLARebelCaptureBuilding") {
            param->m_string = "Upgrade_InfantryCaptureBuilding";
            DEBUG_LOG("Fixed up script reference for 'Upgrade_InfantryCaptureBuilding'.\n");
        }
    }

    // Read the object status into our bitflag struct.
    if (param->m_type == OBJECT_STATUS) {
        for (unsigned bit = 0; bit < OBJECT_STATUS_COUNT; ++bit) {
            if (strcasecmp(param->m_string.Str(), BitFlags<OBJECT_STATUS_COUNT>::s_bitNamesList[bit]) == 0) {
                param->m_objStatus.Set(bit);
                break;
            }
        }
    }

    if (param->m_type == KIND_OF_PARAM) {
        if (param->m_string.Is_Not_Empty()) {
            bool found = false; // For assertion check.

            for (int i = 0; BitFlags<KIND_OF_COUNT>::s_bitNamesList[i] != nullptr; ++i) {
                if (param->m_string.Compare_No_Case(BitFlags<KIND_OF_COUNT>::s_bitNamesList[i])) {
                    param->m_int = i;
                    found = true;
                    break;
                }

                // These additional checks look like they are intended to fix up old entries. Probably don't need to be
                // checked every loop though... Need to identify maps/scripts that require these fixes.
                if (param->m_string.Compare_No_Case("CRUSHER")) {
                    DEBUG_LOG("Parameter KindOF Matched CRUSHER.");
                    param->m_int = i;
                    found = true;
                    break;
                }

                if (param->m_string.Compare_No_Case("CRUSHABLE")) {
                    DEBUG_LOG("Parameter KindOF Matched CRUSHABLE.");
                    param->m_int = i;
                    found = true;
                    break;
                }

                if (param->m_string.Compare_No_Case("OVERLAPPABLE")) {
                    DEBUG_LOG("Parameter KindOF Matched OVERLAPPABLE.");
                    param->m_int = i;
                    found = true;
                    break;
                }

                // This is a very odd condition, it basically sets it to SMALL_MISSILE, sets the m_int to 0 (because the
                // compare will always hit and it has reset the loop counter) and then continues the loop though
                // until SMALL_MISSLE is encountered in the list...
                if (param->m_string.Compare_No_Case("MISSILE")) {
                    // param->m_string.Format("SMALL_MISSLE");
                    // for (i = 0; BitFlags<KIND_OF_COUNT>::s_bitNamesList[i] != nullptr; ++i) {
                    //    if (param->m_string.Compare_No_Case("SMALL_MISSILE")) {
                    //        param->m_int = i;
                    //        found = true;
                    //        break;
                    //    }
                    //}
                    // Outcome should basically be this I believe:
                    DEBUG_LOG("Parameter KindOF Matched MISSILE.");
                    param->m_string = "SMALL_MISSILE";
                    param->m_int = KIND_OF_SMALL_MISSILE;
                    found = true;
                    break;
                }
            }

            ASSERT_THROW_PRINT(found, 0xDEAD0001, "Did not find parameter kind from string.\n");
        } else {
            param->m_string = BitFlags<KIND_OF_COUNT>::s_bitNamesList[param->m_int];
        }
    }

    return param;
}
