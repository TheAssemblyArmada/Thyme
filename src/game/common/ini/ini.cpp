////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: INI.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Parser for SAGE engine configuration files.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "ini.h"
#include "file.h"
#include "gamedebug.h"

// Parsing tables, remove hook when table can be populated correctly
// Can use Make_Function_Ptr for unimplemented.
#if 0
BlockParse TheTypeTable[] =
{
    { "AIData", &INI::parseAIDataDefinition },
    { "Animation", &INI::parseAnim2DDefinition },
    { "Armor", &INI::parseArmorDefinition },
    { "AudioEvent", &INI::parseAudioEventDefinition },
    { "AudioSettings", &INI::parseAudioSettingsDefinition },
    { "Bridge", &INI::parseTerrainBridgeDefinition },
    { "Campaign", &INI::parseCampaignDefinition },
    { "ChallengeGenerals", &INI::parseChallengeModeDefinition },
    { "CommandButton", &INI::parseCommandButtonDefinition },
    { "CommandMap", &INI::parseMetaMapDefinition },
    { "CommandSet", &INI::parseCommandSetDefinition },
    { "ControlBarScheme", &INI::parseControlBarSchemeDefinition },
    { "ControlBarResizer", &INI::parseControlBarResizerDefinition },
    { "CrateData", &INI::parseCrateTemplateDefinition },
    { "Credits", &INI::parseCredits },
    { "WindowTransition", &INI::parseWindowTransitions },
    { "DamageFX", &INI::parseDamageFXDefinition },
    { "DialogEvent", &INI::parseDialogDefinition },
    { "DrawGroupInfo", &INI::parseDrawGroupNumberDefinition },
    { "EvaEvent", &INI::parseEvaEvent },
    { "FXList", &INI::parseFXListDefinition },
    { "GameData", &INI::parseGameDataDefinition },
    { "InGameUI", &INI::parseInGameUIDefinition },
    { "Locomotor", &INI::parseLocomotorTemplateDefinition },
    { "Language", &INI::parseLanguageDefinition },
    { "MapCache", &INI::parseMapCacheDefinition },
    { "MapData", &INI::parseMapDataDefinition },
    { "MappedImage", &INI::parseMappedImageDefinition },
    { "MiscAudio", &INI::parseMiscAudio },
    { "Mouse", &INI::parseMouseDefinition },
    { "MouseCursor", &INI::parseMouseCursorDefinition },
    { "MultiplayerColor", &INI::parseMultiplayerColorDefinition },
    { "MultiplayerStartingMoneyChoice", &INI::parseMultiplayerStartingMoneyChoiceDefinition },
    { "OnlineChatColors", &INI::parseOnlineChatColorDefinition },
    { "MultiplayerSettings", &INI::parseMultiplayerSettingsDefinition },
    { "MusicTrack", &INI::parseMusicTrackDefinition },
    { "Object", &INI::parseObjectDefinition },
    { "ObjectCreationList", &INI::parseObjectCreationListDefinition },
    { "ObjectReskin", &INI::parseObjectReskinDefinition },
    { "ParticleSystem", &INI::parseParticleSystemDefinition },
    { "PlayerTemplate", &INI::parsePlayerTemplateDefinition },
    { "Road", &INI::parseTerrainRoadDefinition },
    { "Science", &INI::parseScienceDefinition },
    { "Rank", &INI::parseRankDefinition },
    { "SpecialPower", &INI::parseSpecialPowerDefinition },
    { "ShellMenuScheme", &INI::parseShellMenuSchemeDefinition },
    { "Terrain", &INI::parseTerrainDefinition },
    { "Upgrade", &INI::parseUpgradeDefinition },
    { "Video", &INI::parseVideoDefinition },
    { "WaterSet", &INI::parseWaterSettingDefinition },
    { "WaterTransparency", &INI::parseWaterTransparencyDefinition },
    { "Weather", &INI::parseWeatherDefinition },
    { "Weapon", &INI::parseWeaponTemplateDefinition },
    { "WebpageURL", &INI::parseWebpageURL },
    { "HeaderTemplate", &INI::parseHeaderTemplateDefinition },
    { "StaticGameLOD", &INI::parseStaticGameLODDefinition },
    { "DynamicGameLOD", &INI::parseDynamicGameLODDefinition },
    { "LODPreset", &INI::parseLODPreset },
    { "BenchProfile", &INI::parseBenchProfile },
    { "ReallyLowMHz", &parseReallyLowMHz },
    { "ScriptAction", &ScriptEngine::parseScriptAction },
    { "ScriptCondition", &ScriptEngine::parseScriptCondition },
    { nullptr, nullptr }
};
#else
#define TheTypeTable (Make_Global<BlockParse*>(0x0093B670))
#endif

void MultiIniFieldParse::Add(FieldParse *field_parse, unsigned int extra_offset)
{
    THROW_ASSERT(Count < MAX_MULTI_FIELDS, 0xDEAD0001);

    FieldParsers[Count] = field_parse;
    ExtraOffsets[Count] = extra_offset;
    ++Count;
}

INI::INI() :
    BackingFile(nullptr),
    BufferReadPos(0),
    BufferData(0),
    FileName("None"),
    LoadType(INI_LOAD_INVALID),
    Seps(" \n\r\t="),
    SepsPercent(" \n\r\t=%%"),
    SepsColon(" \n\r\t=:"),
    SepsQuote(" \"\n="),
    EndToken("END"),
    EndOfFile(false)
{
}

INI::~INI()
{
}

void INI::Load(AsciiString filename, INILoadType type, Xfer *xfer)
{
}

void INI::Load_Directory(AsciiString dir, bool search_subdirs, INILoadType type, Xfer *xfer)
{
}

void INI::Prep_File(AsciiString filename, INILoadType type)
{
}

void INI::Unprep_File()
{
}

void INI::Init_From_INI(void *what, FieldParse *parse_table)
{
    MultiIniFieldParse p;

    p.Add(parse_table, 0);
    Init_From_INI_Multi(what, p);
}

void INI::Init_From_INI_Multi(void *what, MultiIniFieldParse const &parse_table_list)
{
    bool done = false;

    while ( !done ) {
        Read_Line();
        char *token = strtok(CurrentBlock, Seps);

        if ( token == nullptr ) {
            break;
        }

        if ( strcasecmp(token, EndToken) == 0 ) {
            done = true;
        } else {
            iniparsefunc_t parsefunc;
            int offset;
            void const *data;
            int exoffset = 0;

            // Find an appropriate parser function from the parse table
            for ( int i = 0; ; ++i ) {
                ASSERT_PRINT(
                    i < parse_table_list.Count,
                    "[LINE: %d - FILE: '%s'] Unknown field '%s' in block '%s'\n",
                    LineNumber,
                    FileName.Str(),
                    token,
                    CurrentBlock
                );

                parsefunc = Find_Field_Parse(parse_table_list.FieldParsers[i], token, offset, data);
                
                if ( parsefunc != nullptr ) {
                    exoffset = parse_table_list.ExtraOffsets[i];

                    break;
                }
            }

            parsefunc(this, what, static_cast<char*>(what) + offset + exoffset, data);
        }

        ASSERT_PRINT(
            done && !EndOfFile,
            "Error parsing block '%s', in INI file '%s'.  Missing '%s' token\n",
            CurrentBlock,
            FileName.Str(),
            EndToken
        );
    }
}

void INI::Init_From_INI_Multi_Proc(void *what, void(*proc)(MultiIniFieldParse *))
{
    MultiIniFieldParse p;

    proc(&p);
    Init_From_INI_Multi(what, p);
}

bool INI::Read_Line()
{
    return false;
}
