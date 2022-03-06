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

// clang-format off
namespace
{
constexpr const char *const g_audio_priority_names[] = {
    "LOWEST",
    "LOW",
    "NORMAL",
    "HIGH",
    "CRITICAL",
    nullptr
};

constexpr const char *const g_sound_type_names[] = {
    "UI",
    "WORLD",
    "SHROUDED",
    "GLOBAL",
    "VOICE",
    "PLAYER",
    "ALLIES",
    "ENEMIES",
    "EVERYONE",
    nullptr
};

constexpr const char *const g_audio_control_names[] = {
    "LOOP",
    "RANDOM",
    "ALL",
    "POSTDELAY",
    "INTERRUPT",
    nullptr
};
} // namespace

// wb: 0x00C25790
const FieldParse AudioEventInfo::s_audioEventParseTable[] = {
    FIELD_PARSE_ASCIISTRING("Filename", AudioEventInfo, m_filename),
    FIELD_PARSE_PERCENT_TO_REAL("Volume", AudioEventInfo, m_volume),
    FIELD_PARSE_PERCENT_TO_REAL("VolumeShift", AudioEventInfo, m_volumeShift),
    FIELD_PARSE_PERCENT_TO_REAL("MinVolume", AudioEventInfo, m_minVolume),
    FIELD_PARSE_AUDIO_PITCH_SHIFT("PitchShift", AudioEventInfo),
    FIELD_PARSE_AUDIO_DELAY("Delay", AudioEventInfo),
    FIELD_PARSE_INT("Limit", AudioEventInfo, m_limit),
    FIELD_PARSE_INT("LoopCount", AudioEventInfo, m_loopCount),
    FIELD_PARSE_INDEX_LIST("Priority", g_audio_priority_names, AudioEventInfo, m_priority),
    FIELD_PARSE_BITSTRING32("Type", g_sound_type_names, AudioEventInfo, m_visibility),
    FIELD_PARSE_BITSTRING32("Control", g_audio_control_names, AudioEventInfo, m_control),
    FIELD_PARSE_SOUNDS_LIST("Sounds", AudioEventInfo, m_sounds),
    FIELD_PARSE_SOUNDS_LIST("SoundsNight", AudioEventInfo, m_soundsNight),
    FIELD_PARSE_SOUNDS_LIST("SoundsEvening", AudioEventInfo, m_soundsEvening),
    FIELD_PARSE_SOUNDS_LIST("SoundsMorning", AudioEventInfo, m_soundsMorning),
    FIELD_PARSE_SOUNDS_LIST("Attack", AudioEventInfo, m_attack),
    FIELD_PARSE_SOUNDS_LIST("Decay", AudioEventInfo, m_decay),
    FIELD_PARSE_REAL("MinRange", AudioEventInfo, m_minRange),
    FIELD_PARSE_REAL("MaxRange", AudioEventInfo, m_maxRange),
    FIELD_PARSE_PERCENT_TO_REAL("LowPassCutoff", AudioEventInfo, m_lowPassCutoff),
    FIELD_PARSE_LAST
};
// clang-format on

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
 * zh: 0x0044ED70 wb: 0x00738DB7
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

/**
 * Parses a pitch shift field from an INI file instance.
 *
 * Was originally parsePitchShift
 * zh: 0x0044F290
 */
void AudioEventInfo::Parse_Pitch_Shift(INI *ini, void *formal, void *store, const void *user_data)
{
    AudioEventInfo *info = static_cast<AudioEventInfo *>(store);
    float lo = INI::Scan_Real(ini->Get_Next_Token());
    float hi = INI::Scan_Real(ini->Get_Next_Token());

    captainslog_dbgassert(lo > -100.0f && hi >= lo, "Bad pitch shift values for audio event %s", info->m_eventName.Str());
    info->m_pitchShiftLow = float(lo / 100.0f) + 1.0f;
    info->m_pitchShiftHigh = float(hi / 100.0f) + 1.0f;
}

/**
 * Parses a delay field from an INI file instance.
 *
 * Was originally parseDelay
 * zh: 0x0044F250
 */
void AudioEventInfo::Parse_Delay(INI *ini, void *formal, void *store, const void *user_data)
{
    AudioEventInfo *info = static_cast<AudioEventInfo *>(store);
    float lo = INI::Scan_Real(ini->Get_Next_Token());
    float hi = INI::Scan_Real(ini->Get_Next_Token());

    captainslog_dbgassert(lo >= 0.0f && hi >= lo, "Bad delay values for audio event %s", info->m_eventName.Str());
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
