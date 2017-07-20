////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOSETTINGS.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class holding the settings for the audio engine.
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
#include "audiosettings.h"
#include "audiomanager.h"
#include "gamedebug.h"
#include "optionpreferences.h"

FieldParse AudioSettings::s_audioSettingsParseTable[] = {
    { "AudioRoot", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_audioRoot) },
    { "SoundsFolder", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_soundsFolder) },
    { "MusicFolder", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_musicFolder) },
    { "StreamingFolder", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_streamingFolder) },
    { "SoundsExtension", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_soundExtension) },
    { "UseDigital", &INI::Parse_Bool, nullptr, offsetof(AudioSettings, m_useDigital) },
    { "UseMidi", &INI::Parse_Bool, nullptr, offsetof(AudioSettings, m_useMidi) },
    { "OutputRate", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_outputRate) },
    { "OutputBits", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_outputBits) },
    { "OutputChannels", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_outputChannels) },
    { "SampleCount2D", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_sampleCount2D) },
    { "SampleCount3D", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_sampleCount3D) },
    { "StreamCount", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_streamCount) },
    { "Preferred3DHW1", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_preferredDrivers[0]) },
    { "Preferred3DHW2", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_preferredDrivers[1]) },
    { "Preferred3DHW3", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_preferredDrivers[2]) },
    { "Preferred3DHW4", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_preferredDrivers[3]) },
    { "Preferred3DSW", &INI::Parse_AsciiString, nullptr, offsetof(AudioSettings, m_preferredDrivers[4]) },
    { "Default2DSpeakerType", &INI::Parse_Speaker_Type, nullptr, offsetof(AudioSettings, m_default2DSpeakerType) },
    { "Default3DSpeakerType", &INI::Parse_Speaker_Type, nullptr, offsetof(AudioSettings, m_default3DSpeakerType) },
    { "MinSampleVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_minSampleVolume) },
    { "GlobalMinRange", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_globalMinRange) },
    { "GlobalMaxRange", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_globalMaxRange) },
    { "TimeBetweenDrawableSounds", &INI::Parse_Duration_Int, nullptr, offsetof(AudioSettings, m_timeBetweenDrawableSounds) },
    { "TimeToFadeAudio", &INI::Parse_Duration_Int, nullptr, offsetof(AudioSettings, m_timeToFadeAudio) },
    { "AudioFootprintInBytes", &INI::Parse_Unsigned, nullptr, offsetof(AudioSettings, m_audioFootprintInBytes) },
    { "Relative2DVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_relative2DVolume) },
    { "DefaultSoundVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_defaultSoundVolume) },
    { "Default3DSoundVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_default3DSoundVolume) },
    { "DefaultSpeechVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_defaultSpeechVolume) },
    { "DefaultMusicVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_defaultMusicVolume) },
    { "MicrophoneDesiredHeightAboveTerrain", &INI::Parse_Real, nullptr, offsetof(AudioSettings, m_microphoneDesiredHeightAboveTerrain) },
    { "MicrophoneMaxPercentageBetweenGroundAndCamera", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_microphoneMaxPercentBetweenGroundAndCamera) },
    { "ZoomMinDistance", &INI::Parse_Real, nullptr, offsetof(AudioSettings, m_zoomMinDistance) },
    { "ZoomMaxDistance", &INI::Parse_Real, nullptr, offsetof(AudioSettings, m_zoomMaxDistance) },
    { "ZoomSoundVolumePercentageAmount", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_zoomSoundVolumePercentAmount) },
    { nullptr, nullptr, nullptr, 0 }
};

void AudioSettings::Parse_Audio_Settings(INI *ini)
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
