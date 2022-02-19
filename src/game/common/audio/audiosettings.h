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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "gametype.h"
#include "ini.h"

enum PreferredDrivers
{
    DRIVER_HW1,
    DRIVER_HW2,
    DRIVER_HW3,
    DRIVER_HW4,
    DRIVER_SOFTWARE,
    DRIVER_COUNT,
};

class AudioSettings
{
public:
    AudioSettings() {}
    ~AudioSettings() {}

    const Utf8String &Get_Audio_Root() const { return m_audioRoot; }
    const Utf8String &Get_Sound_Folder() const { return m_soundsFolder; }
    const Utf8String &Get_Music_Folder() const { return m_musicFolder; }
    const Utf8String &Get_Streaming_Folder() const { return m_streamingFolder; }
    const Utf8String &Get_Extension() const { return m_soundExtension; }
    Utf8String Get_Preferred_Driver(int32_t index) const { return m_preferredDrivers[index]; }
    SpeakerType Get_Default_2D_Speaker() const { return m_default2DSpeakerType; }
    float Get_Default_Sound_Volume() const { return m_defaultSoundVolume; }
    float Get_Default_3D_Sound_Volume() const { return m_default3DSoundVolume; }
    float Get_Default_Music_Volume() const { return m_defaultMusicVolume; }
    float Get_Default_Speech_Volume() const { return m_defaultSpeechVolume; }
    float Get_Relative_Volume() const { return m_relative2DVolume; }
    SpeakerType Get_Default_2D_Speaker_Type() const { return m_default2DSpeakerType; }
    SpeakerType Get_Default_3D_Speaker_Type() const { return m_default3DSpeakerType; }
    float Get_Min_Sample_Vol() const { return m_minSampleVolume; }
    float Get_Sound_Volume() const { return m_soundVolume; }
    float Get_3D_Sound_Volume() const { return m_3dSoundVolume; }
    float Get_Music_Volume() const { return m_musicVolume; }
    float Get_Speech_Volume() const { return m_speechVolume; }
    int32_t Get_Audio_Footprint() const { return m_audioFootprintInBytes; }
    bool Use_Digital() const { return m_useDigital; }
    bool Use_Midi() const { return m_useMidi; }
    int32_t Output_Rate() const { return m_outputRate; }
    int32_t Output_Bits() const { return m_outputBits; }
    int32_t Output_Channels() const { return m_outputChannels; }
    int32_t Global_Min_Range() const { return m_globalMinRange; }
    int32_t Global_Max_Range() const { return m_globalMaxRange; }
    int32_t Get_2D_Sample_Count() const { return m_sampleCount2D; }
    int32_t Get_3D_Sample_Count() const { return m_sampleCount3D; }
    int32_t Get_Stream_Count() const { return m_streamCount; }
    int32_t Get_Time_To_Fade() const { return m_timeToFadeAudio; }

    static void Parse_Audio_Settings_Definition(INI *ini);

private:
    Utf8String m_audioRoot;
    Utf8String m_soundsFolder;
    Utf8String m_musicFolder;
    Utf8String m_streamingFolder;
    Utf8String m_soundExtension;
    bool m_useDigital;
    bool m_useMidi;
    int32_t m_outputRate;
    int32_t m_outputBits;
    int32_t m_outputChannels;
    int32_t m_sampleCount2D;
    int32_t m_sampleCount3D;
    int32_t m_streamCount;
    int32_t m_globalMinRange;
    int32_t m_globalMaxRange;
    int32_t m_timeBetweenDrawableSounds;
    int32_t m_timeToFadeAudio;
    int32_t m_audioFootprintInBytes;
    float m_minSampleVolume;
    Utf8String m_preferredDrivers[DRIVER_COUNT];
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