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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "gametype.h"
#include "ini.h"

class AudioSettings
{
public:
    AudioSettings() {}
    ~AudioSettings() {}

    Utf8String Get_Preferred_Driver(int index) const { return m_preferredDrivers[index]; }
    SpeakerType Get_Default_2D_Speaker() const { return m_default2DSpeakerType; }
    float Get_Default_Sound_Volume() const { return m_defaultSoundVolume; }
    float Get_Default_3D_Sound_Volume() const { return m_default3DSoundVolume; }
    float Get_Default_Music_Volume() const { return m_defaultMusicVolume; }
    float Get_Default_Speech_Volume() const { return m_defaultSpeechVolume; }
    float Get_Relative_Volume() const { return m_relative2DVolume; }
    SpeakerType Get_Default_2D_Speaker_Type() const { return m_default2DSpeakerType; }
    SpeakerType Get_Default_3D_Speaker_Type() const { return m_default3DSpeakerType; }
    float Get_Min_Sample_Vol() const { return m_minSampleVolume; }
    
    static void Parse_Audio_Settings(INI *ini);

private:
    Utf8String m_audioRoot;
    Utf8String m_soundsFolder;
    Utf8String m_musicFolder;
    Utf8String m_streamingFolder;
    Utf8String m_soundExtension;
    bool m_useDigital;
    bool m_useMidi;
    int m_outputRate;
    int m_outputBits;
    int m_outputChannels;
    int m_sampleCount2D;
    int m_sampleCount3D;
    int m_streamCount;
    int m_globalMinRange;
    int m_globalMaxRange;
    int m_timeBetweenDrawableSounds;
    int m_timeToFadeAudio;
    int m_audioFootprintInBytes;
    float m_minSampleVolume;
    Utf8String m_preferredDrivers[5];
    float m_relative2DVolume;
    float m_defaultSoundVolume;
    float m_default3DSoundVolume;
    float m_defaultSpeechVolume;
    float m_defaultMusicVolume;
    SpeakerType m_default2DSpeakerType;
    SpeakerType m_default3DSpeakerType;
    float m_soundVolume;
    float m_3dSoundVolume;
    float m_speechVolume;
    float m_musicVolume;
    float m_microphoneDesiredHeightAboveTerrain;
    float m_microphoneMaxPercentBetweenGroundAndCamera;
    float m_zoomMinDistance;
    float m_zoomMaxDistance;
    float m_zoomSoundVolumePercentAmount;

    static FieldParse s_audioSettingsParseTable[];
};