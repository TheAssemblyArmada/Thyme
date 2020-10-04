/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Audio event information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "audioeventinfo.h"
#include "audiomanager.h"
#include <captainslog.h>
#include <cstddef>

namespace
{
const char *g_audio_priority_names[] = { "LOWEST", "LOW", "NORMAL", "HIGH", "CRITICAL" };
const char *g_sound_type_names[] = {
    "UI", "WORLD", "SHROUDED", "GLOBAL", "VOICE", "PLAYER", "ALLIES", "ENEMIES", "EVERYONE"
};
const char *g_audio_control_names[] = { "LOOP", "RANDOM", "ALL", "POSTDELAY", "INTERRUPT" };
} // namespace

FieldParse AudioEventInfo::s_audioEventParseTable[] = {
    { "Filename", &INI::Parse_AsciiString, nullptr, offsetof(AudioEventInfo, m_filename) },
    { "Volume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_volume) },
    { "VolumeShift", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_volumeShift) },
    { "MinVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_minVolume) },
    { "PitchShift", &AudioEventInfo::Parse_Pitch_Shift, nullptr, 0 },
    { "Delay", &AudioEventInfo::Parse_Delay, nullptr, 0 },
    { "Limit", &INI::Parse_Int, nullptr, offsetof(AudioEventInfo, m_limit) },
    { "LoopCount", &INI::Parse_Int, nullptr, offsetof(AudioEventInfo, m_loopCount) },
    { "Priority", &INI::Parse_Index_List, g_audio_priority_names, offsetof(AudioEventInfo, m_priority) },
    { "Type", &INI::Parse_Bitstring32, g_sound_type_names, offsetof(AudioEventInfo, m_visibility) },
    { "Control", &INI::Parse_Bitstring32, g_audio_control_names, offsetof(AudioEventInfo, m_control) },
    { "Sounds", &INI::Parse_Sounds_Vector, nullptr, offsetof(AudioEventInfo, m_sounds) },
    { "SoundsNight", &INI::Parse_Sounds_Vector, nullptr, offsetof(AudioEventInfo, m_soundsNight) },
    { "SoundsEvening", &INI::Parse_Sounds_Vector, nullptr, offsetof(AudioEventInfo, m_soundsEvening) },
    { "SoundsMorning", &INI::Parse_Sounds_Vector, nullptr, offsetof(AudioEventInfo, m_soundsMorning) },
    { "Attack", &INI::Parse_Sounds_Vector, nullptr, offsetof(AudioEventInfo, m_attack) },
    { "Decay", &INI::Parse_Sounds_Vector, nullptr, offsetof(AudioEventInfo, m_decay) },
    { "MinRange", &INI::Parse_Real, nullptr, offsetof(AudioEventInfo, m_minRange) },
    { "MaxRange", &INI::Parse_Real, nullptr, offsetof(AudioEventInfo, m_maxRange) },
    { "LowPassCutoff", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_lowPassCutoff) },
    { nullptr, nullptr, nullptr, 0 }
};

/**
 * Parses audio event definition information from an INI instance.
 *
 * 0x0044ED70
 */
void AudioEventInfo::Parse_Audio_Event(INI *ini)
{
    Utf8String name = ini->Get_Next_Token();
    AudioEventInfo *new_event = g_theAudio->New_Audio_Event_Info(name);

    if (new_event != nullptr) {
        AudioEventInfo *def_event = g_theAudio->Find_Audio_Event_Info("DefaultSoundEffect");
        if (def_event != nullptr) {
            *new_event = *def_event;
        }

        new_event->m_eventName = name;
        new_event->m_eventType = EVENT_SOUND;
        ini->Init_From_INI(new_event, s_audioEventParseTable);
    }
}

/**
 * Parses a pitch shift field from an INI file instance.
 *
 * 0x0044F290
 */
void AudioEventInfo::Parse_Pitch_Shift(INI *ini, void *formal, void *store, const void *user_data)
{
    AudioEventInfo *info = static_cast<AudioEventInfo *>(store);
    float lo = INI::Scan_Real(ini->Get_Next_Token());
    float hi = INI::Scan_Real(ini->Get_Next_Token());

    captainslog_assert(lo > -100.0f && hi >= lo);
    info->m_pitchShiftLow = float(lo / 100.0f) + 1.0f;
    info->m_pitchShiftHigh = float(hi / 100.0f) + 1.0f;
}

/**
 * Parses a delay field from an INI file instance.
 *
 * 0x0044F250
 */
void AudioEventInfo::Parse_Delay(INI *ini, void *formal, void *store, const void *user_data)
{
    AudioEventInfo *info = static_cast<AudioEventInfo *>(store);
    float lo = INI::Scan_Real(ini->Get_Next_Token());
    float hi = INI::Scan_Real(ini->Get_Next_Token());

    captainslog_assert(lo >= 0.0f && hi >= lo);
    info->m_delayLow = lo;
    info->m_delayHigh = hi;
}
