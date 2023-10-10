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
#pragma once
#include "always.h"
#include "gamewindow.h"
#include "videobuffer.h"
#include "videoplayer.h"
#include "videostream.h"

enum WindowVideoPlayType
{
    WINDOW_PLAY_MOVIE_ONCE,
    WINDOW_PLAY_MOVIE_LOOP,
    WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME,
    WINDOW_PLAY_MOVIE_COUNT,
};

enum WindowVideoStates
{
    WINDOW_VIDEO_STATE_START,
    WINDOW_VIDEO_STATE_STOP,
    WINDOW_VIDEO_STATE_PAUSE,
    WINDOW_VIDEO_STATE_PLAY,
    WINDOW_VIDEO_STATE_HIDDEN,
    WINDOW_VIDEO_STATE_COUNT,
};

class WindowVideo
{
public:
    WindowVideo();
    ~WindowVideo();

    WindowVideoPlayType Get_Play_Type() const { return m_playType; }
    WindowVideoStates Get_State() const { return m_state; }
    VideoBuffer *Get_Video_Buffer() const { return m_videoBuffer; }
    VideoStream *Get_Video_Stream() const { return m_videoStream; }
    GameWindow *Get_Win() const { return m_win; }

    void Init(GameWindow *win,
        Utf8String movie_name,
        WindowVideoPlayType play_type,
        VideoBuffer *video_buffer,
        VideoStream *video_stream);

    void Set_Window_State(WindowVideoStates state);

private:
    WindowVideoPlayType m_playType;
    GameWindow *m_win;
    VideoBuffer *m_videoBuffer;
    VideoStream *m_videoStream;
    Utf8String m_movieName;
    WindowVideoStates m_state;
};

class WindowVideoManager : public SubsystemInterface
{
public:
    WindowVideoManager();
    virtual ~WindowVideoManager() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    WindowVideoStates Get_Win_State(GameWindow *win);
    void Hide_Movie(GameWindow *win);
    void Load_Movie(GameWindow *win, Utf8String movie_name, WindowVideoPlayType play_type);
    void Pause_All_Movies();
    void Pause_Movie(GameWindow *win);
    void Resume_All_Movies();
    void Resume_Movie(GameWindow *win);
    void Stop_All_Movies();
    void Stop_And_Remove_Movie(GameWindow *win);
    void Stop_Movie(GameWindow *win);

#ifdef GAME_DLL
    WindowVideoManager *Hook_Ctor() { return new (this) WindowVideoManager; }
#endif

private:
    struct HashConstGameWindowPtr
    {
        size_t operator()(const GameWindow *object) const { return reinterpret_cast<size_t>(object); }
    };

#ifdef THYME_USE_STLPORT
    std::hash_map<const GameWindow *, WindowVideo *, HashConstGameWindowPtr> m_playingVideos;
#else
    std::unordered_map<const GameWindow *, WindowVideo *, HashConstGameWindowPtr> m_playingVideos;
#endif
    bool m_stopAllMovies;
    bool m_pauseAllMovies;
};
