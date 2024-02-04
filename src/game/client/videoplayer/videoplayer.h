/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Virtual interface for video players.
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
#include "ini.h"
#include "subsysteminterface.h"
#include <vector>

class VideoStream;

struct Video
{
    Utf8String file_name;
    Utf8String internal_name;
    Utf8String world_builder_comment;
};

class VideoPlayer : public SubsystemInterface
{
public:
    VideoPlayer() : m_videosAvailableToPlay(), m_firstStream(nullptr) {}
    virtual ~VideoPlayer();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void Deinit() {}
    virtual void Lose_Focus() {}
    virtual void Regain_Focus() {}
    virtual VideoStream *Open(Utf8String title) { return nullptr; }
    virtual VideoStream *Load(Utf8String title) { return nullptr; }
    virtual VideoStream *First_Stream() { return m_firstStream; }
    virtual void Close_All_Streams();
    virtual void Add_Video(Video *video);
    virtual void Remove_Video(Video *video);
    virtual int Get_Video_Count();
    virtual Video *Get_Video(int index);
    virtual Video *Get_Video(Utf8String name);
    virtual const FieldParse *Get_Field_Parse() { return s_videoFieldParseTable; }
    virtual void Notify_Player_Of_New_Provider(bool unk) {}

    void Remove_Stream(VideoStream *stream);
    static void Parse(INI *ini);

protected:
    std::vector<Video> m_videosAvailableToPlay;
    VideoStream *m_firstStream;

    static const FieldParse s_videoFieldParseTable[];
};

#ifdef GAME_DLL
extern VideoPlayer *&g_theVideoPlayer;
#else
extern VideoPlayer *g_theVideoPlayer;
#endif
