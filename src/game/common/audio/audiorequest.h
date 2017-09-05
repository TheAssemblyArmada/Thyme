////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOREQUEST.H
//
//        Author:: OmniBlade
//
//  Contributors::
//
//   Description:: Object representing a request to the audio system.
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

#ifndef AUDIOREQUEST_H
#define AUDIOREQUEST_H

#include "always.h"
#include "audioeventrts.h"
#include "mempoolobj.h"

union reqevent_t
{
    unsigned int handle;
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

public:
    virtual ~AudioRequest() {}

    void Set_Music_Event_Object(AudioEventRTS *object) { m_event.object = object; m_requestType = REQUEST_MUSIC_ADD; }
    void Set_Event_Handle(unsigned int handle) { m_event.handle = handle; m_requestType = REQUEST_REMOVE; }

private:
    AudioRequest(bool is_add) : m_isAdding(is_add), m_isProcessed(false) {}

private:
    int m_requestType;
    reqevent_t m_event;
    bool m_isAdding;
    bool m_isProcessed;
};

#endif // AUDIOREQUEST_H
