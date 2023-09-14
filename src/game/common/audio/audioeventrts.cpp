/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Audio event class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "audioeventrts.h"
#include "audiomanager.h"
#include "drawable.h"
#include "filesystem.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "object.h"
#include "randomvalue.h"
#include "registryget.h"

/**
 * 0x004446B0
 */
AudioEventRTS::AudioEventRTS() :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_soundListPos(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
    m_positionOfAudio.Zero();
    // #BUGFIX Initialize all members
    m_objectID = INVALID_OBJECT_ID;
}

/**
 * 0x00444B30
 */
AudioEventRTS::AudioEventRTS(const AudioEventRTS &that) :
    m_filename(that.m_filename),
    m_eventInfo(that.m_eventInfo),
    m_playingHandle(that.m_playingHandle),
    m_handleToKill(that.m_handleToKill),
    m_eventName(that.m_eventName),
    m_filenameAttack(that.m_filenameAttack),
    m_filenameDecay(that.m_filenameDecay),
    m_priority(that.m_priority),
    m_volumeAdjustFactor(that.m_volumeAdjustFactor),
    m_timeOfDay(that.m_timeOfDay),
    m_eventType(that.m_eventType),
    m_shouldFade(that.m_shouldFade),
    m_isLogical(that.m_isLogical),
    m_shouldPlayLocally(that.m_shouldPlayLocally),
    m_pitchShift(that.m_pitchShift),
    m_volumeShift(that.m_volumeShift),
    m_delay(that.m_delay),
    m_loopCount(that.m_loopCount),
    m_currentSoundIndex(that.m_currentSoundIndex),
    m_soundListPos(that.m_soundListPos),
    m_playerIndex(that.m_playerIndex),
    m_nextPlayPortion(that.m_nextPlayPortion)
{
    if (m_eventType == EVENT_3D || m_eventType == EVENT_3D_DEAD) {
        m_positionOfAudio.Set(&that.m_positionOfAudio);
        m_objectID = INVALID_OBJECT_ID; // #BUGFIX Initialize all members
    } else if (m_eventType == EVENT_3D_DRAWABLE) {
        m_drawableID = that.m_drawableID;
        m_positionOfAudio.Zero(); // #BUGFIX Initialize all members
    } else if (m_eventType == EVENT_3D_OBJECT) {
        m_objectID = that.m_objectID;
        m_positionOfAudio.Zero(); // #BUGFIX Initialize all members
    }
}

/**
 * 0x004447E0
 */
AudioEventRTS::AudioEventRTS(const Utf8String &name) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_soundListPos(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
    m_positionOfAudio.Zero();
    // #BUGFIX Initialize all members
    m_objectID = INVALID_OBJECT_ID;
}

/**
 * 0x004448F0
 */
AudioEventRTS::AudioEventRTS(const Utf8String &name, ObjectID id) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_objectID(id),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_soundListPos(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
    if (m_objectID != INVALID_OBJECT_ID) {
        m_eventType = EVENT_3D_OBJECT;
    }

    // #BUGFIX Initialize all members
    m_positionOfAudio.Zero();
}

AudioEventRTS::AudioEventRTS(const Utf8String &name, DrawableID id) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_drawableID(id),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_soundListPos(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
    if (m_drawableID != INVALID_DRAWABLE_ID) {
        m_eventType = EVENT_3D_DRAWABLE;
    }
    // #BUGFIX Initialize all members
    m_positionOfAudio.Zero();
}

/**
 * 0x00444A10
 */
AudioEventRTS::AudioEventRTS(const Utf8String &name, const Coord3D *pos) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_eventType(EVENT_3D),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_soundListPos(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
    m_positionOfAudio.Set(pos);
    // #BUGFIX Initialize all members
    m_objectID = INVALID_OBJECT_ID;
}

/**
 * 0x00444D10
 */
AudioEventRTS &AudioEventRTS::operator=(const AudioEventRTS &that)
{
    if (this != &that) {
        m_filename = that.m_filename;
        m_eventInfo = that.m_eventInfo;
        m_playingHandle = that.m_playingHandle;
        m_handleToKill = that.m_handleToKill;
        m_eventName = that.m_eventName;
        m_filenameAttack = that.m_filenameAttack;
        m_filenameDecay = that.m_filenameDecay;
        m_priority = that.m_priority;
        m_volumeAdjustFactor = that.m_volumeAdjustFactor;
        m_timeOfDay = that.m_timeOfDay;
        m_eventType = that.m_eventType;
        m_shouldFade = that.m_shouldFade;
        m_isLogical = that.m_isLogical;
        m_shouldPlayLocally = that.m_shouldPlayLocally;
        m_pitchShift = that.m_pitchShift;
        m_volumeShift = that.m_volumeShift;
        m_delay = that.m_delay;
        m_loopCount = that.m_loopCount;
        m_currentSoundIndex = that.m_currentSoundIndex;
        m_soundListPos = that.m_soundListPos;
        m_playerIndex = that.m_playerIndex;
        m_nextPlayPortion = that.m_nextPlayPortion;

        if (m_eventType == EVENT_3D || m_eventType == EVENT_3D_DEAD) {
            m_positionOfAudio.Set(&that.m_positionOfAudio);
        } else if (m_eventType == EVENT_3D_DRAWABLE) {
            m_drawableID = that.m_drawableID;
        } else if (m_eventType == EVENT_3D_OBJECT) {
            m_objectID = that.m_objectID;
        }
    }

    return *this;
}

/**
 * Generates the filename for the current event information. Uses localised path if available.
 *
 * 0x00445080
 */
void AudioEventRTS::Generate_Filename()
{
    if (m_eventInfo == nullptr) {
        return;
    }

    m_filename = Generate_Filename_Prefix(m_eventInfo->Get_Event_Type(), false);

    if (m_eventInfo->Get_Event_Type() == EVENT_MUSIC || m_eventInfo->Get_Event_Type() == EVENT_SPEECH) {
        m_filename += m_eventInfo->Get_File_Name();
        Adjust_For_Localization(m_filename);
    } else {
        unsigned count = m_eventInfo->Sound_Count();

        if (count > 0) {
            unsigned index;

            if (m_eventInfo->Get_Control() & CONTROL_RANDOM) {
                index = m_isLogical ? Get_Logic_Random_Value(0, count - 1) : Get_Audio_Random_Value(0, count - 1);

                if (m_currentSoundIndex == index && count > 2) {
                    index = (index + 1) % count;
                }

                m_currentSoundIndex = index;
            } else {
                ++m_currentSoundIndex;
                index = m_currentSoundIndex % count;
            }

            m_filename += m_eventInfo->Get_Sound(index);
            m_filename += Generate_Filename_Extension(m_eventInfo->Get_Event_Type());
            Adjust_For_Localization(m_filename);
            m_delay = m_eventInfo->Get_Delay();
        } else {
            m_filename = "";
        }
    }
}

/**
 * Generates the information needed to play this event from the event info.
 *
 * 0x00445380
 */
void AudioEventRTS::Generate_Play_Info()
{
    m_pitchShift = m_eventInfo->Get_Pitch_Shift();
    m_volumeShift = m_eventInfo->Get_Volume_Shift();
    m_loopCount = m_eventInfo->Get_Loop_Count();
    m_nextPlayPortion = 0;

    unsigned count = m_eventInfo->Attack_Count();

    if (count <= 0) {
        m_nextPlayPortion = 1;
    } else {
        m_filenameAttack = Generate_Filename_Prefix(m_eventInfo->Get_Event_Type(), false);
        int index = m_isLogical ? Get_Logic_Random_Value(0, count - 1) : Get_Audio_Random_Value(0, count - 1);
        m_filenameAttack += m_eventInfo->Get_Attack(index);
        m_filenameAttack += Generate_Filename_Extension(m_eventInfo->Get_Event_Type());
        Adjust_For_Localization(m_filenameAttack);
    }

    count = m_eventInfo->Decay_Count();

    if (count > 0) {
        m_filenameDecay = Generate_Filename_Prefix(m_eventInfo->Get_Event_Type(), false);
        int index = m_isLogical ? Get_Logic_Random_Value(0, count - 1) : Get_Audio_Random_Value(0, count - 1);
        m_filenameDecay += m_eventInfo->Get_Decay(index);
        m_filenameDecay += Generate_Filename_Extension(m_eventInfo->Get_Event_Type());
        Adjust_For_Localization(m_filenameDecay);
    }

    m_isLogical = false;
}

/**
 * Decrements the events loop count.
 *
 * 0x00445740
 */
void AudioEventRTS::Decrease_Loop_Count()
{
    if (m_loopCount == 1) {
        m_loopCount = -1;
    } else if (m_loopCount > 1) {
        --m_loopCount;
    }
}

/**
 * Advances the next play portion.
 *
 * 0x004456C0
 */
void AudioEventRTS::Advance_Next_Play_Portion()
{
    switch (m_nextPlayPortion) {
        case 0:
            m_nextPlayPortion = 1;
            break;
        case 1:
            if (m_eventInfo != nullptr && m_eventInfo->Get_Control() & CONTROL_ALL) {
                if (m_soundListPos == m_eventInfo->Sound_Count()) {
                    // Unclear why this is set here as its changed immediately after.
                    // Possible inlined function or incorrect fallthrough?
                    m_nextPlayPortion = 2;
                }

                ++m_soundListPos;
            }

            if (m_filenameDecay.Is_Empty()) {
                m_nextPlayPortion = 3;
            } else {
                m_nextPlayPortion = 2;
            }

            break;
        case 2:
            m_nextPlayPortion = 3;
            break;
        default:
            break;
    }
}

/**
 * Generates the filename prefix path based on the event type. Optionally adds localized path.
 *
 * 0x00445A20
 */
Utf8String AudioEventRTS::Generate_Filename_Prefix(AudioType type, bool localize)
{
    Utf8String prefix = g_theAudio->Get_Audio_Settings()->Get_Audio_Root();
    prefix += "/";

    switch (type) {
        case EVENT_MUSIC:
            prefix += g_theAudio->Get_Audio_Settings()->Get_Music_Folder();
            break;
        case EVENT_SPEECH:
            prefix += g_theAudio->Get_Audio_Settings()->Get_Streaming_Folder();
            break;
        case EVENT_SOUND:
            prefix += g_theAudio->Get_Audio_Settings()->Get_Sound_Folder();
            break;
        default:
            break;
    }

    prefix += "/";

    if (localize) {
        prefix += Get_Registry_Language();
        prefix += "/";
    }

    return prefix;
}

/**
 * Generates the filename extension for the event type.
 *
 * 0x00445C30
 */
Utf8String AudioEventRTS::Generate_Filename_Extension(AudioType type)
{
    Utf8String ext;

    if (type != EVENT_MUSIC) {
        ext += ".";
        ext += g_theAudio->Get_Audio_Settings()->Get_Extension();
    }

    return ext;
}

/**
 * Adjusts a path to add a localization directory to it.
 *
 * 0x00445D80
 */
void AudioEventRTS::Adjust_For_Localization(Utf8String &filename)
{
    // Search for both posix and windows path separator, original only searches windows.
    const char *tmp = filename.Reverse_Find('/');

    if (tmp == nullptr) {
        tmp = filename.Reverse_Find('\\');
    }

    if (tmp != nullptr) {
        Utf8String localised_name = Generate_Filename_Prefix(m_eventInfo->Get_Event_Type(), true);
        localised_name += tmp;

        if (g_theFileSystem->Does_File_Exist(localised_name.Str())) {
            filename = localised_name;
        }
    }
}

/**
 * Sets the events name.
 *
 * 0x00444F90
 */
void AudioEventRTS::Set_Event_Name(Utf8String name)
{
    if (strcmp(name.Str(), m_eventName.Str()) != 0 && m_eventInfo != nullptr) {
        m_eventInfo = nullptr;
    }

    m_eventName = name;
}

Coord3D *AudioEventRTS::Get_Current_Pos()
{
    switch (m_eventType) {
        case EVENT_MUSIC:
            return &m_positionOfAudio;
        case EVENT_SOUND: {
            Object *obj = g_theGameLogic->Find_Object_By_ID(m_objectID);

            if (obj != nullptr) {
                m_positionOfAudio.Set(obj->Get_Position());
            } else {
                m_eventType = EVENT_UNKVAL3;
            }
            return &m_positionOfAudio;
        }
        case EVENT_SPEECH: {
            Drawable *draw = g_theGameClient->Find_Drawable_By_ID(m_drawableID);

            if (draw != nullptr) {
                m_positionOfAudio.Set(draw->Get_Position());
            } else {
                m_eventType = EVENT_UNKVAL3;
            }
            return &m_positionOfAudio;
        }
        case EVENT_UNKVAL3:
            return &m_positionOfAudio;
        default:
            return nullptr;
    }
}

/**
 * Checks if the event relates to positional audio.
 *
 * 0x00445900
 */
bool AudioEventRTS::Is_Positional_Audio() const
{
    if (m_eventInfo != nullptr && !(m_eventInfo->Get_Visibility() & VISIBILITY_WORLD)) {
        return false;
    }

    return m_eventType != EVENT_UNKVAL4 && (m_objectID != INVALID_OBJECT_ID || m_eventType == EVENT_3D);
}

/**
 * Gets the volume to play this event at.
 *
 * 0x00445950
 */
float AudioEventRTS::Get_Volume() const
{
    if (m_volumeAdjustFactor == -1.0f) {
        if (m_eventInfo != nullptr) {
            return m_eventInfo->Get_Volume();
        }

        return 0.5f;
    }

    return m_volumeAdjustFactor;
}

bool AudioEventRTS::Is_Currently_Playing()
{
    return g_theAudio->Is_Currently_Playing(m_playingHandle);
}

void AudioEventRTS::Set_Object_ID(ObjectID id)
{
    if (m_eventType == EVENT_3D_OBJECT || m_eventType == EVENT_UNKVAL4) {
        m_objectID = id;
        m_eventType = EVENT_3D_OBJECT;
    }
}

void AudioEventRTS::Set_Drawable_ID(DrawableID id)
{
    if (m_eventType == EVENT_3D_DRAWABLE || m_eventType == EVENT_UNKVAL4) {
        m_drawableID = id;
        m_eventType = EVENT_3D_DRAWABLE;
    }
}
