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
#include "videoplayer.h"
#include "videostream.h"
#include <cstddef>

#ifndef GAME_DLL
VideoPlayer *g_theVideoPlayer = nullptr;
#endif

const FieldParse VideoPlayer::s_videoFieldParseTable[] = {
    { "Filename", &INI::Parse_AsciiString, nullptr, offsetof(Video, file_name) },
    { "Comment", &INI::Parse_AsciiString, nullptr, offsetof(Video, world_builder_comment) },
    { nullptr, nullptr, nullptr, 0 }
};

/**
 * 0x0051AC00
 */
VideoPlayer::~VideoPlayer()
{
    if (g_theVideoPlayer == this) {
        g_theVideoPlayer = nullptr;
    }
}

/**
 * @brief Initialises the subsystem.
 *
 * 0x0051AD00
 */
void VideoPlayer::Init()
{
    INI ini;
    ini.Load("Data/INI/Default/Video.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Video.ini", INI_LOAD_OVERWRITE, nullptr);
}

/**
 * @brief Resets the subsystem.
 *
 * 0x00630E10
 */
void VideoPlayer::Reset()
{
    Close_All_Streams();
}

/**
 * @brief Updates the subsystem.
 *
 * 0x0051ADA0
 */
void VideoPlayer::Update()
{
    for (auto stream = First_Stream(); stream != nullptr; stream = stream->Next()) {
        stream->Update();
    }
}

/**
 * @brief Closes all streams being tracked by this video player.
 *
 * 0x0051AE10
 */
void VideoPlayer::Close_All_Streams()
{
    for (auto stream = First_Stream(); stream != nullptr; stream = First_Stream()) {
        stream->Close();
    }
}

/**
 * @brief Adds a video to the players internal list.
 *
 * 0x0051AE30
 */
void VideoPlayer::Add_Video(Video *video)
{
    for (auto it = m_videosAvailableToPlay.begin(); it != m_videosAvailableToPlay.end(); ++it) {
        if (it->internal_name == video->internal_name) {
            *it = *video;

            return;
        }
    }

    m_videosAvailableToPlay.push_back(*video);
}

/**
 * @brief Removes a video from the players internal list.
 *
 * 0x0051AF80
 */
void VideoPlayer::Remove_Video(Video *video)
{
    for (auto it = m_videosAvailableToPlay.begin(); it != m_videosAvailableToPlay.end(); ++it) {
        if (it->internal_name == video->internal_name) {
            m_videosAvailableToPlay.erase(it);
            break;
        }
    }
}

/**
 * @brief Gets the number of videos this player knows about.
 *
 * 0x0051B070
 */
int VideoPlayer::Get_Video_Count()
{
    return m_videosAvailableToPlay.size();
}

/**
 * @brief Gets a video from its index.
 *
 * 0x0051B170
 */
Video *VideoPlayer::Get_Video(int index)
{
    return &m_videosAvailableToPlay[index];
}

/**
 * @brief Gets a video from its name.
 *
 * 0x0051B090
 */
Video *VideoPlayer::Get_Video(Utf8String name)
{
    for (auto it = m_videosAvailableToPlay.begin(); it != m_videosAvailableToPlay.end(); ++it) {
        if (it->internal_name == name) {
            return &(*it);
        }
    }

    return nullptr;
}

/**
 * @brief Removes a stream from the internal list.
 */
void VideoPlayer::Remove_Stream(VideoStream *stream)
{
    VideoStream *list_stream = First_Stream();

    if (list_stream == nullptr) {
        return;
    }

    if (stream == list_stream) {
        m_firstStream = stream->m_next;
    } else {
        VideoStream *prev_stream = list_stream;

        while ((list_stream = list_stream->Next()) != nullptr) {
            if (stream == list_stream) {
                prev_stream->m_next = stream->m_next;

                return;
            }

            prev_stream = list_stream;
        }
    }
}

void VideoPlayer::Parse(INI *ini)
{
    Video video;
    video.internal_name = ini->Get_Next_Token();
    ini->Init_From_INI(&video, g_theVideoPlayer->Get_Field_Parse());
    g_theVideoPlayer->Add_Video(&video);
}
