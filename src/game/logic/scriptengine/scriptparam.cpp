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
#include "kindof.h"
#include <captainslog.h>

Parameter::Parameter(ParameterType type, int intval) :
    m_type(type), m_initialized(false), m_int(intval), m_real(0.0f), m_string(), m_objStatus()
{
    m_coord.Zero();
}

/**
 * @brief Alters the stored string of some types of paramter.
 *
 * 0x0051E990, also inlined example at around 0x00520376
 */
void Parameter::Qualify(const Utf8String &suffix, const Utf8String &side_test, const Utf8String &side_replacemet)
{
    // captainslog_trace("Qualifying parameter string '%s' with suffix '%s' against '%s' to replace with '%s'.",
    // m_string.Str(), suffix.Str(), side_test.Str(), side_replacemet.Str());
    switch (m_type) {
        case SIDE:
            if (m_string + suffix == side_test) {
                m_string = side_replacemet;
            }

            break;
        case TEAM:
            if (m_string != "<This Team>") {
                m_string += suffix;
            }

            break;
        case SCRIPT: // Fallthrough
        case COUNTER: // Fallthrough
        case FLAG: // Fallthrough
        case SCRIPT_SUBROUTINE:
            m_string += suffix;
            break;
        default:
            break;
    };
}

/**
 * @brief Get the UI string for this parameter.
 *
 * 0x0051EB90
 */
Utf8String Parameter::Get_UI_Text()
{
    // Dunno where this is supposed to be from, only appears used here.
    static BorderColor _border_colors[] = { { "Orange", 0xFFFF8700 },
        { "Green", 0xFF00FF00 },
        { "Blue", 0xFF0000FF },
        { "Cyan", 0xFF00FFFF },
        { "Magenta", 0xFFFF00FF },
        { "Yellow", 0xFFFFFF00 },
        { "Purple", 0xFF9E00FF },
        { "Pink", 0xFFFF8670 } };

    Utf8String ui_string = m_string.Is_Empty() ? "?" : m_string;
    Utf8String ui_text;

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
                    captainslog_dbgassert(false, "Unknown comparison.");
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
                    captainslog_dbgassert(false, "Unknown relationship.");
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
                    captainslog_dbgassert(false, "Unknown AI Mood.");
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
            if (m_int < KINDOF_OBSTACLE || m_int >= KINDOF_COUNT) {
                ui_text.Format("Kind is 'unknown kind'", ui_string.Str());
            } else {
                ui_text.Format("Kind is '%s'", BitFlags<KINDOF_COUNT>::Bit_As_String(m_int));
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
                ui_text.Format("Buildable (?)");
            } else {
                ui_text.Format("Buildable (%s)", g_buildableStatusNames[m_int]);
            }

            break;
        case SURFACES_ALLOWED:
            if (m_int - 1 < SURFACE_GROUND || m_int - 1 >= SURFACE_COUNT) {
                ui_text.Format("Surfaces Allowed: ?");
            } else {
                ui_text.Format("Surfaces Allowed: %s", g_surfaceNames[m_int - 1]);
            }

            break;
        case SHAKE_INTENSITY:
            if (m_int < SHAKE_SUBTLE || m_int >= SHAKE_COUNT) {
                ui_text.Format("Shake Intensity: ?");
            } else {
                ui_text.Format("Shake Intensity: %s", g_shakeIntensityNames[m_int]);
            }
            break;
        case COMMANDBUTTON:
            ui_text.Format("Command button: '%s'", ui_string.Str());
            break;
        case FONT:
            ui_text.Format("Font: '%s'", ui_string.Str());
            break;
        case OBJECT_STATUS:
            if (m_string.Is_Empty()) {
                ui_text.Format("Object Status is '?'");
            } else {
                ui_text.Format("Object Status is '%s'", ui_string.Str());
            }

            break;
        case TEAM_COMMANDBUTTON_ABILITY:
            ui_text.Format("Ability '%s'", ui_string.Str());
            break;
        case SKIRMISH_APPROACH_PATH:
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
        case OBJECT_TYPE_LIST:
            ui_text.Format("'%s'", ui_string.Str());
            break;
        case REVEAL_NAME:
            ui_text.Format("Reveal Name: '%s'", ui_string.Str());
            break;
        case SCIENCE_AVAILABILITY:
            ui_text.Format("Science availablity '%s'", ui_string.Str());
            break;
        case EVACUATE_CONTAINER_SIDE:
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
    captainslog_dbgassert(m_type == COORD3D, "Parameter is not a Coord3D.");
    coord->Zero();

    if (m_type == COORD3D) {
        *coord = m_coord;
    }
}

void Parameter::Set_Coord3D(Coord3D *coord)
{
    captainslog_dbgassert(m_type == COORD3D, "Parameter is not a Coord3D.");

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
    Parameter *param = NEW_POOL_OBJ(Parameter, type);

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
            strlcpy_tpl(old_name, param->m_string.Str());
            strlcpy_tpl(new_name, "GLA");
            strlcat_tpl(new_name, &old_name[strlen("Fundamentalist")]);
            param->m_string = new_name;
            captainslog_trace("Fixed up script reference from '%s' to '%s'.", old_name, new_name);
        }
    }

    // Fix up old side specific capture building upgrade references.
    if (param->m_type == UPGRADE) {
        if (param->m_string == "Upgrade_AmericaRangerCaptureBuilding"
            || param->m_string == "Upgrade_ChinaRedguardCaptureBuilding"
            || param->m_string == "Upgrade_GLARebelCaptureBuilding") {
            param->m_string = "Upgrade_InfantryCaptureBuilding";
            captainslog_trace("Fixed up script reference for 'Upgrade_InfantryCaptureBuilding'.");
        }
    }

    // Read the object status into our bitflag struct.
    if (param->m_type == OBJECT_STATUS) {
        for (unsigned bit = 0; bit < OBJECT_STATUS_COUNT; ++bit) {
            if (param->m_string.Compare_No_Case(BitFlags<OBJECT_STATUS_COUNT>::Get_Bit_Names_List()[bit]) == 0) {
                param->Set_Status_Bits(BitFlags<OBJECT_STATUS_COUNT>(BitFlags<OBJECT_STATUS_COUNT>::kInit, bit));
                break;
            }
        }
    }

    if (param->m_type == KIND_OF_PARAM) {
        const char **list = BitFlags<KINDOF_COUNT>::Get_Bit_Names_List();

        if (param->m_string.Is_Not_Empty()) {
            bool found = false; // For assertion check.

            for (int i = 0; list[i] != nullptr; ++i) {
                if (param->m_string.Compare_No_Case(list[i]) == 0) {
                    param->m_int = i;

                    return param;
                }

                // These additional checks look like they are intended to fix up old entries. Probably don't need to be
                // checked every loop though... Need to identify maps/scripts that require these fixes.
                if (param->m_string.Compare_No_Case("CRUSHER") == 0) {
                    captainslog_trace("Parameter KindOF Matched CRUSHER.");
                    param->m_int = i;

                    return param;
                }

                if (param->m_string.Compare_No_Case("CRUSHABLE") == 0) {
                    captainslog_trace("Parameter KindOF Matched CRUSHABLE.");
                    param->m_int = i;

                    return param;
                }

                if (param->m_string.Compare_No_Case("OVERLAPPABLE") == 0) {
                    captainslog_trace("Parameter KindOF Matched OVERLAPPABLE.");
                    param->m_int = i;

                    return param;
                }

                // This is a very odd condition, it basically sets it to SMALL_MISSILE, sets the m_int to 0 (because the
                // compare will always hit and it has reset the loop counter) and then continues the loop though
                // until SMALL_MISSLE is encountered in the list...
                if (param->m_string.Compare_No_Case("MISSILE") == 0) {
                    // param->m_string.Format("SMALL_MISSLE");
                    // for (i = 0; list[i] != nullptr; ++i) {
                    //    if (param->m_string.Compare_No_Case("SMALL_MISSILE")) {
                    //        param->m_int = i;
                    //        found = true;
                    //        break;
                    //    }
                    //}
                    // Outcome should basically be this I believe:
                    captainslog_trace("Parameter KindOF Matched MISSILE.");
                    param->m_string = "SMALL_MISSILE";
                    param->m_int = KINDOF_SMALL_MISSILE;
                    found = true;
                    break;
                }
            }

            captainslog_relassert(found, 0xDEAD0001, "Did not find parameter kind from string.");
        } else {
            param->m_string = list[param->m_int];
        }
    }

    return param;
}
