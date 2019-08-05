/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Video stream implementation using Bink video.
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
#include "videostream.h"

struct Bink;

class BinkVideoStream final : public VideoStream
{
    friend class BinkVideoPlayer;

public:
    BinkVideoStream() : m_binkHandle(nullptr) {}
    virtual ~BinkVideoStream();

    virtual void Update() override;
    virtual bool Is_Frame_Ready() override;
    virtual void Decompress_Frame() override;
    virtual void Render_Frame(VideoBuffer *buffer) override;
    virtual void Next_Frame() override;
    // bool Is_Done();
    virtual int Frame_Index() override;
    virtual int Frame_Count() override;
    virtual void Goto_Frame(int frame) override;
    virtual int Height() override;
    virtual int Width() override;

#ifdef GAME_DLL
    void Hook_Update() { BinkVideoStream::Update(); }
    bool Hook_Is_Frame_Ready() { return BinkVideoStream::Is_Frame_Ready(); }
    void Hook_Decompress_Frame() { BinkVideoStream::Decompress_Frame(); }
    void Hook_Render_Frame(VideoBuffer *buffer) { BinkVideoStream::Render_Frame(buffer); }
    void Hook_Next_Frame() { BinkVideoStream::Next_Frame(); }
    void Hook_Goto_Frame(int frame) { BinkVideoStream::Goto_Frame(frame); }
#endif

private:
    Bink *m_binkHandle;
    uint32_t m_unk;
};
