/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding the settings for the audio engine.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "audiosettings.h"
#include "audiomanager.h"
#include "optionpreferences.h"
#include <cstddef>

// clang-format off
const FieldParse AudioSettings::s_audioSettingsParseTable[] = {
    FIELD_PARSE_ASCIISTRING("AudioRoot", AudioSettings, m_audioRoot),
    FIELD_PARSE_ASCIISTRING("SoundsFolder", AudioSettings, m_soundsFolder),
    FIELD_PARSE_ASCIISTRING("MusicFolder", AudioSettings, m_musicFolder),
    FIELD_PARSE_ASCIISTRING("StreamingFolder", AudioSettings, m_streamingFolder),
    FIELD_PARSE_ASCIISTRING("SoundsExtension", AudioSettings, m_soundExtension),
    FIELD_PARSE_BOOL("UseDigital", AudioSettings, m_useDigital),
    FIELD_PARSE_BOOL("UseMidi", AudioSettings, m_useMidi),
    FIELD_PARSE_INT("OutputRate", AudioSettings, m_outputRate),
    FIELD_PARSE_INT("OutputBits", AudioSettings, m_outputBits),
    FIELD_PARSE_INT("OutputChannels", AudioSettings, m_outputChannels),
    FIELD_PARSE_INT("SampleCount2D", AudioSettings, m_sampleCount2D),
    FIELD_PARSE_INT("SampleCount3D", AudioSettings, m_sampleCount3D),
    FIELD_PARSE_INT("StreamCount", AudioSettings, m_streamCount),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW1", AudioSettings, m_preferredDrivers[0]),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW2", AudioSettings, m_preferredDrivers[1]),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW3", AudioSettings, m_preferredDrivers[2]),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW4", AudioSettings, m_preferredDrivers[3]),
    FIELD_PARSE_ASCIISTRING("Preferred3DSW", AudioSettings, m_preferredDrivers[4]),
    { "Default2DSpeakerType", &INI::Parse_Speaker_Type, nullptr, offsetof(AudioSettings, m_default2DSpeakerType) },
    { "Default3DSpeakerType", &INI::Parse_Speaker_Type, nullptr, offsetof(AudioSettings, m_default3DSpeakerType) },
    FIELD_PARSE_PERCENT_TO_REAL("MinSampleVolume", AudioSettings, m_minSampleVolume),
    FIELD_PARSE_INT("GlobalMinRange", AudioSettings, m_globalMinRange),
    FIELD_PARSE_INT("GlobalMaxRange", AudioSettings, m_globalMaxRange),
    FIELD_PARSE_DURATION_UNSIGNED_INT("TimeBetweenDrawableSounds", AudioSettings, m_timeBetweenDrawableSounds),
    FIELD_PARSE_DURATION_UNSIGNED_INT("TimeToFadeAudio", AudioSettings, m_timeToFadeAudio),
    { "AudioFootprintInBytes", &INI::Parse_Unsigned_Int, nullptr, offsetof(AudioSettings, m_audioFootprintInBytes) },
    FIELD_PARSE_PERCENT_TO_REAL("Relative2DVolume", AudioSettings, m_relative2DVolume),
    FIELD_PARSE_PERCENT_TO_REAL("DefaultSoundVolume", AudioSettings, m_defaultSoundVolume),
    FIELD_PARSE_PERCENT_TO_REAL("Default3DSoundVolume", AudioSettings, m_default3DSoundVolume),
    FIELD_PARSE_PERCENT_TO_REAL("DefaultSpeechVolume", AudioSettings, m_defaultSpeechVolume),
    FIELD_PARSE_PERCENT_TO_REAL("DefaultMusicVolume", AudioSettings, m_defaultMusicVolume),
    FIELD_PARSE_REAL("MicrophoneDesiredHeightAboveTerrain", AudioSettings, m_microphoneDesiredHeightAboveTerrain),
    FIELD_PARSE_PERCENT_TO_REAL("MicrophoneMaxPercentageBetweenGroundAndCamera", AudioSettings, m_microphoneMaxPercentBetweenGroundAndCamera),
    FIELD_PARSE_REAL("ZoomMinDistance", AudioSettings, m_zoomMinDistance),
    FIELD_PARSE_REAL("ZoomMaxDistance", AudioSettings, m_zoomMaxDistance),
    FIELD_PARSE_PERCENT_TO_REAL("ZoomSoundVolumePercentageAmount", AudioSettings, m_zoomSoundVolumePercentAmount),
    FIELD_PARSE_LAST
};
// clang-format on

// wb: 0x006E5C20
AudioSettings::AudioSettings() :
    // #BUGFIX Initialize all members
    m_audioRoot(),
    m_soundsFolder(),
    m_musicFolder(),
    m_streamingFolder(),
    m_soundExtension(),
    m_useDigital{},
    m_useMidi{},
    m_outputRate{},
    m_outputBits{},
    m_outputChannels{},
    m_sampleCount2D{},
    m_sampleCount3D{},
    m_streamCount{},
    m_globalMinRange{},
    m_globalMaxRange{},
    m_timeBetweenDrawableSounds{},
    m_timeToFadeAudio{},
    m_audioFootprintInBytes{},
    m_minSampleVolume{},
    m_preferredDrivers(),
    m_relative2DVolume{},
    m_defaultSoundVolume{},
    m_default3DSoundVolume{},
    m_defaultSpeechVolume{},
    m_defaultMusicVolume{},
    m_default2DSpeakerType{},
    m_default3DSpeakerType{},
    m_soundVolume{},
    m_3dSoundVolume{},
    m_speechVolume{},
    m_musicVolume{},
    m_microphoneDesiredHeightAboveTerrain{},
    m_microphoneMaxPercentBetweenGroundAndCamera{},
    m_zoomMinDistance{},
    m_zoomMaxDistance{},
    m_zoomSoundVolumePercentAmount{}
{
}

// was originally INI::parseAudioSettingsDefinition
void AudioSettings::Parse_Audio_Settings_Definition(INI *ini)
{
    AudioSettings *as = g_theAudio->Get_Audio_Settings();
    ini->Init_From_INI(as, s_audioSettingsParseTable);
    OptionPreferences prefs;

    g_theAudio->Set_Preferred_3D_Provider(prefs.Get_Preferred_3D_Provider());
    g_theAudio->Set_Preferred_Speaker(prefs.Get_Speaker_Type());
    as->m_soundVolume = prefs.Get_Sound_Volume() / 100.0f;
    as->m_3dSoundVolume = prefs.Get_3DSound_Volume() / 100.0f;
    as->m_speechVolume = prefs.Get_Speech_Volume() / 100.0f;
    as->m_musicVolume = prefs.Get_Music_Volume() / 100.0f;
}
