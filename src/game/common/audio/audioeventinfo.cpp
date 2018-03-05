////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOEVENTINFO.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Audio event information.
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
#include "audioeventinfo.h"
#include <cstddef>

FieldParse AudioEventInfo::s_audioEventParseTable[] =
{
    { "Filename", &INI::Parse_AsciiString, nullptr, offsetof(AudioEventInfo, m_filename) },
    { "Volume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_volume) },
    { "VolumeShift", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_volumeShift) },
    { "MinVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_minVolume) },
    //{ "PitchShift", &INI::parsePitchShift, nullptr, 0 },
    //{ "Delay", &INI::parseDelay, nullptr, 0 },
    { "Limit", &INI::Parse_Int, nullptr, offsetof(AudioEventInfo, m_limit) },
    { "LoopCount", &INI::Parse_Int, nullptr, offsetof(AudioEventInfo, m_loopCount) },
    //{ "Priority", &INI::Parse_Index_List, &theAudioPriorityNames, offsetof(AudioEventInfo, m_priority) },
    //{ "Type", &INI::Parse_Bitstring32, &theSoundTypeNames, offsetof(AudioEventInfo, m_type) },
    //{ "Control", &INI::Parse_Bitstring32, &theAudioControlNames, offsetof(AudioEventInfo, m_control) },
    //{ "Sounds", &INI::parseSoundsList, nullptr, offsetof(AudioEventInfo, m_sounds) },
    //{ "SoundsNight", &INI::parseSoundsList, nullptr, offsetof(AudioEventInfo, m_soundsNight) },
    //{ "SoundsEvening", &INI::parseSoundsList, nullptr, offsetof(AudioEventInfo, m_soundsEvening) },
    //{ "SoundsMorning", &INI::parseSoundsList, nullptr, offsetof(AudioEventInfo, m_soundsMorning) },
    //{ "Attack", &INI::parseSoundsList, nullptr, offsetof(AudioEventInfo, m_attack) },
    //{ "Decay", &INI::parseSoundsList, nullptr, offsetof(AudioEventInfo, m_decay) },
    { "MinRange", &INI::Parse_Real, nullptr, offsetof(AudioEventInfo, m_minRange) },
    { "MaxRange", &INI::Parse_Real, nullptr, offsetof(AudioEventInfo, m_maxRange) },
    { "LowPassCutoff", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_lowPassCutoff) }
};

void AudioEventInfo::Parse_Audio_Event(INI * ini)
{
    // TODO
}
