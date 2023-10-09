/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Animated Window Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "animatewindowmanager.h"
#include "gamewindow.h"

AnimateWindow::AnimateWindow() :
    m_delay(0),
    m_win(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_animType(WIN_ANIMATION_NONE),
    m_needsToFinish(false),
    m_isFinished(false)
{
    m_startPos.x = 0;
    m_startPos.y = 0;
    m_endPos.x = 0;
    m_endPos.y = 0;
    m_curPos.x = 0;
    m_curPos.y = 0;
    m_restPos.x = 0;
    m_restPos.y = 0;
    m_vel.x = 0.0f;
    m_vel.y = 0.0f;
}

AnimateWindow::~AnimateWindow()
{
    m_win = nullptr;
}

void AnimateWindow::Set_Anim_Data(ICoord2D start_pos,
    ICoord2D end_pos,
    ICoord2D cur_pos,
    ICoord2D rest_pos,
    Coord2D vel,
    unsigned int start_time,
    unsigned int end_time)
{
    m_startPos = start_pos;
    m_endPos = end_pos;
    m_curPos = cur_pos;
    m_restPos = rest_pos;
    m_vel = vel;
    m_startTime = start_time;
    m_endTime = end_time;
}

void Clear_Win_List(std::list<AnimateWindow *> &list)
{
    AnimateWindow *anim_win = nullptr;

    while (!list.empty()) {
        anim_win = *list.begin();
        list.pop_front();

        if (anim_win != nullptr) {
            anim_win->Delete_Instance();
        }

        anim_win = nullptr;
    }
}

AnimateWindowManager::AnimateWindowManager() : m_needsUpdate(false), m_reverse(false)
{
    m_slideFromRight = new ProcessAnimateWindowSlideFromRight();
    m_slideFromRightFast = new ProcessAnimateWindowSlideFromRightFast();
    m_slideFromLeft = new ProcessAnimateWindowSlideFromLeft();
    m_slideFromTop = new ProcessAnimateWindowSlideFromTop();
    m_slideFromTopFast = new ProcessAnimateWindowSlideFromTopFast();
    m_slideFromBottom = new ProcessAnimateWindowSlideFromBottom();
    m_spiral = new ProcessAnimateWindowSpiral();
    m_slideFromBottomTimed = new ProcessAnimateWindowSlideFromBottomTimed();
}

AnimateWindowManager::~AnimateWindowManager()
{
    if (m_slideFromRight != nullptr) {
        delete m_slideFromRight;
    }

    if (m_slideFromRightFast != nullptr) {
        delete m_slideFromRightFast;
    }

    if (m_slideFromLeft != nullptr) {
        delete m_slideFromLeft;
    }

    if (m_slideFromTop != nullptr) {
        delete m_slideFromTop;
    }

    if (m_slideFromTopFast != nullptr) {
        delete m_slideFromTopFast;
    }

    if (m_slideFromBottom != nullptr) {
        delete m_slideFromBottom;
    }

    if (m_spiral != nullptr) {
        delete m_spiral;
    }

    if (m_slideFromBottomTimed != nullptr) {
        delete m_slideFromBottomTimed;
    }

    Reset_To_Rest_Position();
    Clear_Win_List(m_winList);
    Clear_Win_List(m_winMustFinishList);
}

void AnimateWindowManager::Init()
{
    Clear_Win_List(m_winList);
    Clear_Win_List(m_winMustFinishList);
    m_needsUpdate = false;
    m_reverse = false;
}

void AnimateWindowManager::Reset()
{
    Reset_To_Rest_Position();
    Clear_Win_List(m_winList);
    Clear_Win_List(m_winMustFinishList);
    m_needsUpdate = false;
    m_reverse = false;
}

void AnimateWindowManager::Update()
{
    ProcessAnimateWindow *process_anim_win = nullptr;

    if (m_needsUpdate) {
        m_needsUpdate = false;

        for (auto it = m_winMustFinishList.begin(); it != m_winMustFinishList.end(); it++) {
            AnimateWindow *anim_win = *it;

            if (anim_win == nullptr) {
                captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
                return;
            }

            process_anim_win = Get_Process_Animate(anim_win->Get_Anim_Type());

            if (process_anim_win != nullptr) {
                if (m_reverse) {
                    if (!process_anim_win->Reverse_Animate_Window(anim_win)) {
                        m_needsUpdate = true;
                    }
                } else if (!process_anim_win->Update_Animate_Window(anim_win)) {
                    m_needsUpdate = true;
                }
            }
        }

        for (auto it = m_winList.begin(); it != m_winList.end(); it++) {
            AnimateWindow *anim_win = *it;

            if (anim_win == nullptr) {
                captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
                return;
            }

            process_anim_win = Get_Process_Animate(anim_win->Get_Anim_Type());

            if (process_anim_win != nullptr) {
                if (m_reverse) {
                    if (!process_anim_win->Reverse_Animate_Window(anim_win)) {
                        m_needsUpdate = true;
                    }
                } else if (!process_anim_win->Update_Animate_Window(anim_win)) {
                    m_needsUpdate = true;
                }
            }
        }
    }
}

void AnimateWindowManager::Register_Game_Window(
    GameWindow *win, AnimTypes anim_type, bool needs_to_finish, unsigned int ms, unsigned int delay_ms)
{
    if (win != nullptr) {
        if (anim_type > WIN_ANIMATION_NONE && anim_type < WIN_ANIMATION_COUNT) {
            AnimateWindow *anim_win = new AnimateWindow();

            anim_win->Set_Game_Window(win);
            anim_win->Set_Anim_Type(anim_type);
            anim_win->Set_Needs_To_Finish(needs_to_finish);
            anim_win->Set_Delay(delay_ms);
            ProcessAnimateWindow *process_anim_win = Get_Process_Animate(anim_type);

            if (process_anim_win != nullptr) {
                process_anim_win->Set_Max_Duration(ms);
                process_anim_win->Init_Animate_Window(anim_win);
            }

            if (needs_to_finish) {
                m_winMustFinishList.push_back(anim_win);
                m_needsUpdate = true;
            } else {
                m_winList.push_back(anim_win);
            }
        } else {
            captainslog_dbgassert(
                false, "an Invalid WIN_ANIMATION type was passed into Register_Game_Window... please fix me ");
        }
    } else {
        captainslog_dbgassert(false, "Win was NULL as it was passed into Register_Game_Window... not good indeed");
    }
}

ProcessAnimateWindow *AnimateWindowManager::Get_Process_Animate(AnimTypes anim_type)
{
    switch (anim_type) {
        case WIN_ANIMATION_SLIDE_RIGHT:
            return m_slideFromRight;
        case WIN_ANIMATION_SLIDE_RIGHT_FAST:
            return m_slideFromRightFast;
        case WIN_ANIMATION_SLIDE_LEFT:
            return m_slideFromLeft;
        case WIN_ANIMATION_SLIDE_TOP:
            return m_slideFromTop;
        case WIN_ANIMATION_SLIDE_BOTTOM:
            return m_slideFromBottom;
        case WIN_ANIMATION_SPIRAL:
            return m_spiral;
        case WIN_ANIMATION_SLIDE_BOTTOM_TIMED:
            return m_slideFromBottomTimed;
        case WIN_ANIMATION_SLIDE_TOP_FAST:
            return m_slideFromTopFast;
        default:
            return nullptr;
    }
}

void AnimateWindowManager::Reverse_Animate_Window()
{
    m_reverse = true;
    m_needsUpdate = true;
    ProcessAnimateWindow *process_anim_win = nullptr;
    unsigned int delay = 0;

    for (auto it = m_winMustFinishList.begin(); it != m_winMustFinishList.end(); it++) {
        AnimateWindow *anim_win = *it;

        if (anim_win == nullptr) {
            captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
            return;
        }

        if (anim_win->Get_Delay() > delay) {
            delay = anim_win->Get_Delay();
        }
    }

    for (auto it = m_winMustFinishList.begin(); it != m_winMustFinishList.end(); it++) {
        AnimateWindow *anim_win = *it;

        if (anim_win == nullptr) {
            captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
            return;
        }

        process_anim_win = Get_Process_Animate(anim_win->Get_Anim_Type());

        if (process_anim_win != nullptr) {
            process_anim_win->Init_Reverse_Animate_Window(anim_win, delay);
        }

        anim_win->Set_Finished(false);
    }

    for (auto it = m_winList.begin(); it != m_winList.end(); it++) {
        AnimateWindow *anim_win = *it;

        if (anim_win == nullptr) {
            captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
            return;
        }

        process_anim_win = Get_Process_Animate(anim_win->Get_Anim_Type());

        if (process_anim_win != nullptr) {
            process_anim_win->Init_Reverse_Animate_Window(anim_win, 0);
        }

        anim_win->Set_Finished(false);
    }
}

void AnimateWindowManager::Reset_To_Rest_Position()
{
    m_reverse = true;
    m_needsUpdate = true;

    for (auto it = m_winMustFinishList.begin(); it != m_winMustFinishList.end(); it++) {
        AnimateWindow *anim_win = *it;

        if (anim_win == nullptr) {
            captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
            return;
        }

        ICoord2D rest_pos = anim_win->Get_Rest_Pos();
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Set_Position(rest_pos.x, rest_pos.y);
        }
    }

    for (auto it = m_winList.begin(); it != m_winList.end(); it++) {
        AnimateWindow *anim_win = *it;

        if (anim_win == nullptr) {
            captainslog_dbgassert(false, "There's No AnimateWindow in the AnimateWindow List");
            return;
        }

        ICoord2D rest_pos = anim_win->Get_Rest_Pos();
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Set_Position(rest_pos.x, rest_pos.y);
        }
    }
}
