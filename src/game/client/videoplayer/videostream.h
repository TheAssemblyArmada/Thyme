/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Virtual interface for video streams.
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

class VideoPlayer;
class VideoBuffer;

class VideoStream
{
    friend class VideoPlayer;

public:
    VideoStream() : m_player(nullptr), m_next(nullptr) {}
    virtual ~VideoStream();

    virtual VideoStream *Next() { return m_next; };
    virtual void Update() {}
    virtual void Close() { delete this; }
    virtual bool Is_Frame_Ready() { return true; }
    virtual void Decompress_Frame() {}
    virtual void Render_Frame(VideoBuffer *buffer) {}
    virtual void Next_Frame() {}
    // virtual bool Is_Done() { return true; } // New virtual in Mac build
    virtual int Frame_Index() { return 0; }
    virtual int Frame_Count() { return 0; }
    virtual void Goto_Frame(int frame) {} // Removed in Mac build
    virtual int Height() { return 0; }
    virtual int Width() { return 0; }

protected:
    VideoPlayer *m_player;
    VideoStream *m_next;
};
