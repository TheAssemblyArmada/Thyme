/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Window Video Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "windowvideomanager.h"
#include "display.h"

WindowVideo::WindowVideo() :
    m_playType(WINDOW_PLAY_MOVIE_ONCE),
    m_win(nullptr),
    m_videoBuffer(nullptr),
    m_videoStream(nullptr),
    m_state(WINDOW_VIDEO_STATE_STOP)
{
}

WindowVideo::~WindowVideo()
{
    if (m_win != nullptr) {
        m_win->Win_Get_Instance_Data()->Set_VideoBuffer(nullptr);
    }

    m_win = nullptr;
    delete m_videoBuffer;
    delete m_videoStream;
}

void WindowVideo::Init(GameWindow *win,
    Utf8String movie_name,
    WindowVideoPlayType play_type,
    VideoBuffer *video_buffer,
    VideoStream *video_stream)
{
    m_win = win;
    m_movieName = movie_name;
    m_playType = play_type;
    m_videoBuffer = video_buffer;
    m_videoStream = video_stream;
    m_state = WINDOW_VIDEO_STATE_PLAY;

    if (m_win != nullptr) {
        m_win->Win_Get_Instance_Data()->Set_VideoBuffer(m_videoBuffer);
    }
}

void WindowVideo::Set_Window_State(WindowVideoStates state)
{
    m_state = state;

    if (m_state == WINDOW_VIDEO_STATE_STOP && m_win != nullptr) {
        m_win->Win_Get_Instance_Data()->Set_VideoBuffer(nullptr);
    }

    if (m_state == WINDOW_VIDEO_STATE_PLAY || m_state == WINDOW_VIDEO_STATE_PAUSE) {
        if (m_win != nullptr) {
            m_win->Win_Get_Instance_Data()->Set_VideoBuffer(m_videoBuffer);
        }
    }
}

WindowVideoManager::WindowVideoManager() : m_stopAllMovies(false), m_pauseAllMovies(false) {}

WindowVideoManager::~WindowVideoManager()
{
    for (auto it = m_playingVideos.begin(); it != m_playingVideos.end(); it++) {
        delete it->second;
    }

    m_playingVideos.clear();
}

void WindowVideoManager::Init()
{
    m_playingVideos.clear();
    m_stopAllMovies = false;
    m_pauseAllMovies = false;
}

void WindowVideoManager::Reset()
{
    for (auto it = m_playingVideos.begin(); it != m_playingVideos.end(); it++) {
        delete it->second;
    }

    m_playingVideos.clear();
    m_stopAllMovies = false;
    m_pauseAllMovies = false;
}

void WindowVideoManager::Update()
{
    if (!m_pauseAllMovies && !m_stopAllMovies) {
        for (auto it = m_playingVideos.begin(); it != m_playingVideos.end(); it++) {
            WindowVideo *win_video = it->second;

            if (win_video == nullptr) {
                captainslog_dbgassert(false, "There's No WindowVideo in the m_playingVideos list");
                return;
            }

            GameWindow *win = win_video->Get_Win();

            if (win_video->Get_State() == WINDOW_VIDEO_STATE_HIDDEN && !win->Win_Is_Hidden()) {
                Resume_Movie(win);
            }

            if (win_video->Get_State() == WINDOW_VIDEO_STATE_PLAY && win->Win_Is_Hidden()) {
                Hide_Movie(win);
            }

            if (win_video->Get_State() == WINDOW_VIDEO_STATE_PLAY) {
                VideoStream *stream = win_video->Get_Video_Stream();
                VideoBuffer *buffer = win_video->Get_Video_Buffer();

                if (stream != nullptr) {
                    if (buffer != nullptr) {
                        if (stream->Is_Frame_Ready()) {
                            stream->Decompress_Frame();
                            stream->Render_Frame(buffer);
                            stream->Next_Frame();

                            if (stream->Frame_Index() == 0) {
                                if (win_video->Get_Play_Type() != WINDOW_PLAY_MOVIE_ONCE) {
                                    if (win_video->Get_Play_Type() == WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME) {
                                        Pause_Movie(win);
                                    }
                                } else {
                                    Stop_Movie(win);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void WindowVideoManager::Load_Movie(GameWindow *win, Utf8String movie_name, WindowVideoPlayType play_type)
{
    Stop_And_Remove_Movie(win);
    VideoStream *stream = g_theVideoPlayer->Open(movie_name);

    if (stream != nullptr) {
        VideoBuffer *buffer = g_theDisplay->Create_VideoBuffer();

        if (buffer != nullptr && buffer->Allocate(stream->Width(), stream->Height())) {
            WindowVideo *win_video = new WindowVideo();
            win_video->Init(win, movie_name, play_type, buffer, stream);
            m_playingVideos[win] = win_video;
            m_pauseAllMovies = false;
            m_stopAllMovies = false;
        } else {
            if (buffer != nullptr) {
                delete buffer;
            }

            if (stream != nullptr) {
                stream->Close();
            }
        }
    }
}

void WindowVideoManager::Pause_Movie(GameWindow *win)
{
    auto it = m_playingVideos.find(win);

    if (it != m_playingVideos.end()) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_PAUSE);
        }
    }
}

void WindowVideoManager::Hide_Movie(GameWindow *win)
{
    auto it = m_playingVideos.find(win);

    if (it != m_playingVideos.end()) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_HIDDEN);
        }
    }
}

void WindowVideoManager::Resume_Movie(GameWindow *win)
{
    auto it = m_playingVideos.find(win);

    if (it != m_playingVideos.end()) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_PLAY);
        }
    }

    m_pauseAllMovies = false;
    m_stopAllMovies = false;
}

void WindowVideoManager::Stop_Movie(GameWindow *win)
{
    auto it = m_playingVideos.find(win);

    if (it != m_playingVideos.end()) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_STOP);
        }
    }
}

void WindowVideoManager::Stop_And_Remove_Movie(GameWindow *win)
{
    auto it = m_playingVideos.find(win);

    if (it != m_playingVideos.end()) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            delete win_video;
        }

        m_playingVideos.erase(it);
    }
}

void WindowVideoManager::Stop_All_Movies()
{
    for (auto it = m_playingVideos.begin(); it != m_playingVideos.end(); it++) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_STOP);
        }
    }

    m_stopAllMovies = true;
    m_pauseAllMovies = false;
}

void WindowVideoManager::Pause_All_Movies()
{
    for (auto it = m_playingVideos.begin(); it != m_playingVideos.end(); it++) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_PAUSE);
        }
    }

    m_pauseAllMovies = true;
    m_stopAllMovies = false;
}

void WindowVideoManager::Resume_All_Movies()
{
    for (auto it = m_playingVideos.begin(); it != m_playingVideos.end(); it++) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            win_video->Set_Window_State(WINDOW_VIDEO_STATE_PLAY);
        }
    }

    m_pauseAllMovies = false;
    m_stopAllMovies = false;
}

WindowVideoStates WindowVideoManager::Get_Win_State(GameWindow *win)
{
    auto it = m_playingVideos.find(win);

    if (it != m_playingVideos.end()) {
        WindowVideo *win_video = it->second;

        if (win_video != nullptr) {
            return win_video->Get_State();
        }
    }

    return WINDOW_VIDEO_STATE_STOP;
}
