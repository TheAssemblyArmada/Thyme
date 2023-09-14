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
const char *g_audio_priority_names[] = { "LOWEST", "LOW", "NORMAL", "HIGH", "CRITICAL", nullptr };
const char *g_sound_type_names[] = {
    "UI", "WORLD", "SHROUDED", "GLOBAL", "VOICE", "PLAYER", "ALLIES", "ENEMIES", "EVERYONE", nullptr
};
const char *g_audio_control_names[] = { "LOOP", "RANDOM", "ALL", "POSTDELAY", "INTERRUPT", nullptr };
} // namespace

const FieldParse AudioEventInfo::s_audioEventParseTable[] = {
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
    { "Sounds", &INI::Parse_Sounds_List, nullptr, offsetof(AudioEventInfo, m_sounds) },
    { "SoundsNight", &INI::Parse_Sounds_List, nullptr, offsetof(AudioEventInfo, m_soundsNight) },
    { "SoundsEvening", &INI::Parse_Sounds_List, nullptr, offsetof(AudioEventInfo, m_soundsEvening) },
    { "SoundsMorning", &INI::Parse_Sounds_List, nullptr, offsetof(AudioEventInfo, m_soundsMorning) },
    { "Attack", &INI::Parse_Sounds_List, nullptr, offsetof(AudioEventInfo, m_attack) },
    { "Decay", &INI::Parse_Sounds_List, nullptr, offsetof(AudioEventInfo, m_decay) },
    { "MinRange", &INI::Parse_Real, nullptr, offsetof(AudioEventInfo, m_minRange) },
    { "MaxRange", &INI::Parse_Real, nullptr, offsetof(AudioEventInfo, m_maxRange) },
    { "LowPassCutoff", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioEventInfo, m_lowPassCutoff) },
    { nullptr, nullptr, nullptr, 0 }
};

// wb: 0x006E6800
AudioEventInfo::AudioEventInfo() :
    // #BUGFIX Initialize all members
    m_eventName(),
    m_filename(),
    m_volume{},
    m_volumeShift{},
    m_minVolume{},
    m_pitchShiftLow{},
    m_pitchShiftHigh{},
    m_delayLow{},
    m_delayHigh{},
    m_limit{},
    m_loopCount{},
    m_priority{},
    m_visibility{},
    m_control{},
    m_soundsMorning(),
    m_sounds(),
    m_soundsNight(),
    m_soundsEvening(),
    m_attack{},
    m_decay{},
    m_lowPassCutoff{},
    m_minRange{},
    m_maxRange{},
    m_eventType{}
{
}

/**
 * Parses audio event definition information from an INI instance.
 *
 * 0x0044ED70
 * Was originally INI::parseAudioEventDefintion
 */
void AudioEventInfo::Parse_Audio_Event_Definition(INI *ini)
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

void AudioEventInfo::Parse_Music_Track_Definition(INI *ini)
{
    Utf8String track;
    track.Set(ini->Get_Next_Token());
    AudioEventInfo *new_audio_event = g_theAudio->New_Audio_Event_Info(track);

    if (new_audio_event != nullptr) {
        AudioEventInfo *default_audio_event = g_theAudio->Find_Audio_Event_Info("DefaultMusicTrack");

        if (default_audio_event != nullptr) {
            *new_audio_event = *default_audio_event;
            g_theAudio->Add_Track_Name(track);
        }

        new_audio_event->m_eventName = track;
        new_audio_event->m_eventType = EVENT_MUSIC;
        ini->Init_From_INI(new_audio_event, s_audioEventParseTable);
    }
}

void AudioEventInfo::Parse_Dialog_Definition(INI *ini)
{
    Utf8String track;
    track.Set(ini->Get_Next_Token());
    AudioEventInfo *new_audio_event = g_theAudio->New_Audio_Event_Info(track);

    if (new_audio_event != nullptr) {
        AudioEventInfo *default_audio_event = g_theAudio->Find_Audio_Event_Info("DefaultDialog");

        if (default_audio_event != nullptr) {
            *new_audio_event = *default_audio_event;
        }

        new_audio_event->m_eventName = track;
        new_audio_event->m_eventType = EVENT_SPEECH;
        ini->Init_From_INI(new_audio_event, s_audioEventParseTable);
    }
}

/**
 * Parses a pitch shift field from an INI file instance.
 *
 * Was originally parsePitchShift
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
 * Was originally parseDelay
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

void DynamicAudioEventInfo::Override_Audio_Name(const Utf8String &name)
{
    m_overrideName = m_eventName;
    m_overrideFlags.Set(OVERRIDE_NAME, 1);
    m_eventName = name;
}

void DynamicAudioEventInfo::Override_Loop_Flag(bool loop)
{
    m_overrideFlags.Set(OVERRIDE_LOOP_FLAG, 1);

    if (loop) {
        m_control |= CONTROL_LOOP;
    } else {
        m_control &= ~CONTROL_LOOP;
    }
}

void DynamicAudioEventInfo::Override_Loop_Count(int loop_count)
{
    m_overrideFlags.Set(OVERRIDE_LOOP_COUNT, 1);
    m_loopCount = loop_count;
}

void DynamicAudioEventInfo::Override_Volume(float volume)
{
    m_overrideFlags.Set(OVERRIDE_VOLUME, 1);
    m_volume = volume;
}

void DynamicAudioEventInfo::Override_Min_Volume(float volume)
{
    m_overrideFlags.Set(OVERRIDE_MIN_VOLUME, 1);
    m_minVolume = volume;
}

void DynamicAudioEventInfo::Override_Max_Range(float range)
{
    m_overrideFlags.Set(OVERRIDE_MAX_RANGE, 1);
    m_maxRange = range;
}

void DynamicAudioEventInfo::Override_Min_Range(float range)
{
    m_overrideFlags.Set(OVERRIDE_MIN_RANGE, 1);
    m_minRange = range;
}

void DynamicAudioEventInfo::Override_Priority(int priority)
{
    m_overrideFlags.Set(OVERRIDE_PRIORITY, 1);
    m_priority = priority;
}

void DynamicAudioEventInfo::Xfer_No_Name(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);

    if (xfer->Get_Mode() == XFER_LOAD) {
        unsigned char flags;
        xfer->xferUnsignedByte(&flags);

        for (int i = 0; i < OVERRIDE_COUNT; i++) {
            m_overrideFlags.Set(i, ((1 << i) & flags) != 0);
        }
    } else {

        unsigned char flags = 0;
        for (int i = 0; i < OVERRIDE_COUNT; i++) {
            if (m_overrideFlags.Test(i)) {
                flags |= 1 << i;
            }
        }

        xfer->xferUnsignedByte(&flags);
    }

    if (Loop_Flag_Overridden()) {
        bool b = (m_control & CONTROL_LOOP) != 0;
        xfer->xferBool(&b);

        if (b) {
            m_control |= CONTROL_LOOP;
        } else {
            m_control &= ~CONTROL_LOOP;
        }
    }

    if (Loop_Count_Overridden()) {
        xfer->xferInt(&m_loopCount);
    }

    if (Volume_Overridden()) {
        xfer->xferReal(&m_volume);
    }

    if (Min_Volume_Overridden()) {
        xfer->xferReal(&m_minVolume);
    }

    if (Min_Range_Overridden()) {
        xfer->xferReal(&m_minRange);
    }

    if (Max_Range_Overridden()) {
        xfer->xferReal(&m_maxRange);
    }

    if (Priority_Overridden()) {
        unsigned char priority = m_priority;
        xfer->xferUnsignedByte(&priority);
        m_priority = priority;
    }
}
