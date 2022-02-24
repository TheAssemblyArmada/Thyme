/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object representing a request to the audio system.
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
#include "audioeventrts.h"
#include "mempoolobj.h"

union reqevent_t
{
    uintptr_t handle;
    AudioEventRTS *object;
};

enum AudioRequestType
{
    AR_PLAY,
    AR_PAUSE,
    AR_STOP,
};

namespace Thyme
{
class ALAudioManager;
}

class AudioRequest : public MemoryPoolObject
{
    IMPLEMENT_POOL(AudioRequest);
    friend class AudioManager;
    friend class MilesAudioManager;
    friend class Thyme::ALAudioManager;

protected:
    virtual ~AudioRequest() override {}

public:
    void Request_Play(AudioEventRTS *object)
    {
        m_event.object = object;
        m_requestType = AR_PLAY;
    }
    void Request_Stop(uintptr_t handle)
    {
        m_event.handle = handle;
        m_requestType = AR_STOP;
    }
    void Set_Type(AudioRequestType type) { m_requestType = type; }
    AudioRequestType Request_Type() const { return m_requestType; }
    uintptr_t Event_Handle() const { return m_event.handle; }
    AudioEventRTS *Event_Object() const { return m_event.object; }
    bool Is_Adding() const { return m_isAdding; }

private:
    AudioRequest(bool is_add) : m_isAdding(is_add), m_isProcessed(false)
    {
        // #BUGFIX Initialize all members
        m_requestType = AR_PLAY;
        m_event.handle = 0;
    }

private:
    AudioRequestType m_requestType;
    reqevent_t m_event;
    bool m_isAdding;
    bool m_isProcessed;
};