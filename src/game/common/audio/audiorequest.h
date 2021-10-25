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

enum RequestType
{
    REQUEST_MUSIC_ADD,
    REQUEST_SOUND_ADD,
    REQUEST_REMOVE,
};

class AudioRequest : public MemoryPoolObject
{
    IMPLEMENT_POOL(AudioRequest);
    friend class AudioManager;
    friend class MilesAudioManager;

protected:
    virtual ~AudioRequest() override {}

public:
    void Set_Music_Event_Object(AudioEventRTS *object)
    {
        m_event.object = object;
        m_requestType = REQUEST_MUSIC_ADD;
    }
    void Set_Event_Handle(uintptr_t handle)
    {
        m_event.handle = handle;
        m_requestType = REQUEST_REMOVE;
    }
    void Set_Type(RequestType type) { m_requestType = type; }
    RequestType Request_Type() const { return m_requestType; }
    uintptr_t Event_Handle() const { return m_event.handle; }
    AudioEventRTS *Event_Object() const { return m_event.object; }
    bool Is_Adding() const { return m_isAdding; }

private:
    // #TODO Add better constructor.
    AudioRequest(bool is_add) : m_isAdding(is_add), m_isProcessed(false) {}

private:
    RequestType m_requestType;
    reqevent_t m_event;
    bool m_isAdding;
    bool m_isProcessed;
};