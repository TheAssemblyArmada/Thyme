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
#include "processanimatewindow.h"
#include "animatewindowmanager.h"
#include "display.h"
#include "gamewindow.h"
#include "rtsutils.h"

ProcessAnimateWindowSlideFromRight::ProcessAnimateWindowSlideFromRight() :
    m_slowDownThreshold(80), m_slowDownRatio(0.67f), m_speedUpRatio(2.0f - m_slowDownRatio)
{
    m_maxVel.x = -40.0f;
    m_maxVel.y = 0.0f;
}

ProcessAnimateWindowSlideFromRight::~ProcessAnimateWindowSlideFromRight() {}

void ProcessAnimateWindowSlideFromRight::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;

    if (anim_win != nullptr) {
        anim_win->Set_Finished(false);
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            end_pos = rest_pos;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = width + rest_pos.x;
            start_pos.x = width + rest_pos.x;
            cur_pos.y = rest_pos.y;
            start_pos.y = rest_pos.y;
            window->Win_Set_Position(width + rest_pos.x, rest_pos.y);
            vel.x = m_maxVel.x;
            vel.y = 0;
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromRight::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay() != 0) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        anim_win->Set_Vel(anim_win->Get_Vel() * -1.0f);
        anim_win->Set_Finished(false);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromRight::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x += vel.x;
                    if (cur_pos.x >= end_pos.x) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);

                        if (cur_pos.x - end_pos.x <= m_slowDownThreshold) {
                            vel.x = vel.x * m_slowDownRatio;
                        }
                        if (vel.x >= -1.0f) {
                            vel.x = -1.0f;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.x = end_pos.x;
                        anim_win->Set_Finished(true);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromRight::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x += vel.x;

                    if (cur_pos.x <= start_pos.x) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        ICoord2D end_pos = anim_win->Get_End_Pos();

                        if (cur_pos.x - end_pos.x > m_slowDownThreshold) {
                            vel.x = -m_maxVel.x;
                        } else {
                            vel.x = vel.x * m_speedUpRatio;
                        }

                        if (-m_maxVel.x < vel.x) {
                            vel.x = -m_maxVel.x;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.x = start_pos.x;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

ProcessAnimateWindowSlideFromLeft::ProcessAnimateWindowSlideFromLeft() :
    m_slowDownThreshold(80), m_slowDownRatio(0.67f), m_speedUpRatio(2.0f - m_slowDownRatio)
{
    m_maxVel.x = 40.0f;
    m_maxVel.y = 0.0f;
}

ProcessAnimateWindowSlideFromLeft::~ProcessAnimateWindowSlideFromLeft() {}

void ProcessAnimateWindowSlideFromLeft::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            end_pos = rest_pos;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = rest_pos.x - width;
            start_pos.x = rest_pos.x - width;
            cur_pos.y = rest_pos.y;
            start_pos.y = rest_pos.y;
            window->Win_Set_Position(rest_pos.x - width, rest_pos.y);
            vel = m_maxVel;
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromLeft::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay()) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        anim_win->Set_Vel(anim_win->Get_Vel() * -1.0f);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromLeft::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x += vel.x;

                    if (cur_pos.x <= end_pos.x) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);

                        if (end_pos.x - cur_pos.x <= m_slowDownThreshold) {
                            vel.x = vel.x * m_slowDownRatio;
                        }

                        if (vel.x < 1.0f) {
                            vel.x = 1.0f;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.x = end_pos.x;
                        anim_win->Set_Finished(true);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromLeft::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x += vel.x;

                    if (cur_pos.x >= start_pos.x) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        ICoord2D end_pos = anim_win->Get_End_Pos();

                        if (end_pos.x - cur_pos.x > m_slowDownThreshold) {
                            vel.x = -m_maxVel.x;
                        } else {
                            vel.x = vel.x * m_speedUpRatio;
                        }

                        if (-m_maxVel.x > vel.x) {
                            vel.x = -m_maxVel.x;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.x = start_pos.x;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

ProcessAnimateWindowSlideFromTop::ProcessAnimateWindowSlideFromTop() :
    m_slowDownThreshold(80), m_slowDownRatio(0.67f), m_speedUpRatio(2.0f - m_slowDownRatio)
{
    m_maxVel.y = 40.0f;
    m_maxVel.x = 0.0f;
}

ProcessAnimateWindowSlideFromTop::~ProcessAnimateWindowSlideFromTop() {}

void ProcessAnimateWindowSlideFromTop::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            end_pos = rest_pos;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = rest_pos.x;
            start_pos.x = rest_pos.x;
            cur_pos.y = rest_pos.y - width;
            start_pos.y = rest_pos.y - width;
            window->Win_Set_Position(rest_pos.x, rest_pos.y - width);
            vel = m_maxVel;
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromTop::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay()) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        anim_win->Set_Vel(anim_win->Get_Vel() * -1.0f);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromTop::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.y += vel.y;

                    if (cur_pos.y <= end_pos.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);

                        if (end_pos.y - cur_pos.y <= m_slowDownThreshold) {
                            vel.y = vel.y * m_slowDownRatio;
                        }

                        if (vel.y < 1.0f) {
                            vel.y = 1.0f;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.y = end_pos.y;
                        window->Win_Set_Position(cur_pos.x, end_pos.y);
                        anim_win->Set_Finished(true);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromTop::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.y += vel.y;

                    if (cur_pos.y >= start_pos.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        ICoord2D end_pos = anim_win->Get_End_Pos();

                        if (end_pos.y - cur_pos.y > m_slowDownThreshold) {
                            vel.y = -m_maxVel.y;
                        } else {
                            vel.y = vel.y * m_speedUpRatio;
                        }

                        if (-m_maxVel.y > vel.y) {
                            vel.y = -m_maxVel.y;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.y = start_pos.y;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

ProcessAnimateWindowSlideFromBottom::ProcessAnimateWindowSlideFromBottom() :
    m_slowDownThreshold(80), m_slowDownRatio(0.67f), m_speedUpRatio(2.0f - m_slowDownRatio)
{
    m_maxVel.y = -40.0f;
    m_maxVel.x = 0.0f;
}

ProcessAnimateWindowSlideFromBottom::~ProcessAnimateWindowSlideFromBottom() {}

void ProcessAnimateWindowSlideFromBottom::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            end_pos = rest_pos;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = rest_pos.x;
            start_pos.x = rest_pos.x;
            cur_pos.y = width + rest_pos.y;
            start_pos.y = width + rest_pos.y;
            window->Win_Set_Position(rest_pos.x, width + rest_pos.y);
            vel = m_maxVel;
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromBottom::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay()) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        anim_win->Set_Vel(anim_win->Get_Vel() * -1.0f);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromBottom::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.y += vel.y;

                    if (cur_pos.y >= end_pos.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);

                        if (cur_pos.y - end_pos.y <= m_slowDownThreshold) {
                            vel.y = vel.y * m_slowDownRatio;
                        }

                        if (vel.y >= -1.0f) {
                            vel.y = -1.0f;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.y = end_pos.y;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromBottom::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.y += vel.y;

                    if (cur_pos.y <= start_pos.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        ICoord2D end_pos = anim_win->Get_End_Pos();

                        if (cur_pos.y - end_pos.y > m_slowDownThreshold) {
                            vel.y = -m_maxVel.y;
                        } else {
                            vel.y = vel.y * m_speedUpRatio;
                        }

                        if (-m_maxVel.y < vel.y) {
                            vel.y = -m_maxVel.y;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.y = start_pos.y;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

ProcessAnimateWindowSlideFromBottomTimed::ProcessAnimateWindowSlideFromBottomTimed() : m_maxDuration(1000) {}

ProcessAnimateWindowSlideFromBottomTimed::~ProcessAnimateWindowSlideFromBottomTimed() {}

void ProcessAnimateWindowSlideFromBottomTimed::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            end_pos = rest_pos;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = rest_pos.x;
            start_pos.x = rest_pos.x;
            cur_pos.y = width + rest_pos.y;
            start_pos.y = width + rest_pos.y;
            window->Win_Set_Position(rest_pos.x, width + rest_pos.y);
            unsigned int time = rts::Get_Time();
            unsigned int delay = anim_win->Get_Delay();
            captainslog_debug("Init_Animate_Window at %d (%d->%d)", time, delay + time, delay + m_maxDuration + time);
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, delay + time, delay + m_maxDuration + time);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromBottomTimed::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();
        if (window) {
            rest_pos = anim_win->Get_Rest_Pos();
            start_pos = rest_pos;
            cur_pos.y = start_pos.y;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = rest_pos.x;
            end_pos.x = rest_pos.x;
            end_pos.y = width + rest_pos.y;
            window->Win_Set_Position(start_pos.x, start_pos.y);
            unsigned int time = rts::Get_Time();
            captainslog_debug("initReverseAnimateWindow at %d (%d->%d)", time, time, m_maxDuration + time);
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, time, m_maxDuration + time);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromBottomTimed::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    unsigned int time = rts::Get_Time();
                    unsigned int start_time = anim_win->Get_Start_Time();
                    unsigned int end_time = anim_win->Get_End_Time();

                    if (time >= start_time) {
                        float f1 = 1.0f - (float)(end_time - time) / (float)m_maxDuration;
                        if (time < end_time) {
                            cur_pos.y = (float)start_pos.y + (float)(end_pos.y - start_pos.y) * f1;
                            captainslog_debug("(%d,%d) -> (%d,%d) -> (%d,%d) at %g",
                                start_pos.x,
                                start_pos.y,
                                cur_pos.x,
                                cur_pos.y,
                                end_pos.x,
                                end_pos.y,
                                f1);
                            window->Win_Set_Position(cur_pos.x, cur_pos.y);
                            anim_win->Set_Cur_Pos(cur_pos);
                            return false;
                        } else {
                            cur_pos.y = end_pos.y;
                            anim_win->Set_Finished(true);
                            window->Win_Set_Position(cur_pos.x, cur_pos.y);
                            captainslog_debug("window finished animating at %d (%d->%d)", time, start_time, end_time);
                            return true;
                        }
                    } else {
                        return false;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromBottomTimed::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    return Update_Animate_Window(anim_win);
}

void ProcessAnimateWindowSlideFromBottomTimed::Set_Max_Duration(unsigned int max_duration)
{
    m_maxDuration = max_duration;
}

ProcessAnimateWindowSpiral::ProcessAnimateWindowSpiral() : m_deltaTheta(0.33f), m_maxR(g_theDisplay->Get_Width() >> 1) {}

ProcessAnimateWindowSpiral::~ProcessAnimateWindowSpiral() {}

void ProcessAnimateWindowSpiral::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;
    int win_size_x = 0;
    int win_size_y = 0;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            window->Win_Get_Size(&win_size_x, &win_size_y);
            end_pos = rest_pos;
            vel.x = 0.0f;
            vel.y = m_maxR;
            cur_pos.x = GameMath::Cos(0.0f) * m_maxR + (float)end_pos.x;
            start_pos.x = cur_pos.x;
            cur_pos.y = GameMath::Sin(vel.x) * vel.y + (double)end_pos.y;
            start_pos.y = cur_pos.y;
            window->Win_Set_Position(start_pos.x, cur_pos.y);
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSpiral::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay()) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        Coord2D vel = anim_win->Get_Vel();
        vel.x = 0.0f;
        vel.y = 0.0f;
        anim_win->Set_Vel(vel);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSpiral::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x = GameMath::Cos(vel.x) * vel.y + (float)end_pos.x;
                    cur_pos.y = GameMath::Sin(vel.x) * vel.y + (float)end_pos.y;
                    vel.x = vel.x + m_deltaTheta;
                    vel.y = vel.y - 5.0f;

                    int win_size_x;
                    int win_size_y;
                    window->Win_Get_Size(&win_size_x, &win_size_y);

                    if (GameMath::Min(win_size_x / 2, win_size_y / 2) <= vel.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        ICoord2D rest_pos = anim_win->Get_Rest_Pos();
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(rest_pos.x, rest_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSpiral::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            GameWindow *window = anim_win->Get_Game_Window();

            if (window != nullptr) {
                ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                ICoord2D end_pos = anim_win->Get_End_Pos();
                Coord2D vel = anim_win->Get_Vel();
                cur_pos.x = GameMath::Cos(vel.x) * vel.y + (float)end_pos.x;
                cur_pos.y = GameMath::Sin(vel.x) * vel.y + (float)end_pos.y;
                vel.x = vel.x - m_deltaTheta;
                vel.y = vel.y + 5.0f;

                int win_size_x;
                int win_size_y;
                window->Win_Get_Size(&win_size_x, &win_size_y);

                if (m_maxR >= vel.y) {
                    window->Win_Set_Position(cur_pos.x, cur_pos.y);
                    anim_win->Set_Cur_Pos(cur_pos);
                    anim_win->Set_Vel(vel);
                    return false;
                } else {
                    anim_win->Set_Finished(true);
                    return true;
                }
            } else {
                captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                return true;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

ProcessAnimateWindowSlideFromTopFast::ProcessAnimateWindowSlideFromTopFast() :
    m_slowDownThreshold(40), m_slowDownRatio(0.67f), m_speedUpRatio(4.0f - m_slowDownRatio)
{
    m_maxVel.y = 60.0f;
    m_maxVel.x = 0.0f;
}

ProcessAnimateWindowSlideFromTopFast::~ProcessAnimateWindowSlideFromTopFast() {}

void ProcessAnimateWindowSlideFromTopFast::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;
    int win_size_x = 0;
    int win_size_y = 0;

    if (anim_win != nullptr) {
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            end_pos = rest_pos;
            window->Win_Get_Size(&win_size_x, &win_size_y);
            cur_pos.x = rest_pos.x;
            start_pos.x = rest_pos.x;
            cur_pos.y = -win_size_y;
            start_pos.y = -win_size_y;
            window->Win_Set_Position(rest_pos.x, -win_size_y);
            vel = m_maxVel;
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromTopFast::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay()) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        anim_win->Set_Vel(anim_win->Get_Vel() * -1.0f);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromTopFast::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.y += vel.y;

                    if (cur_pos.y <= end_pos.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);

                        if (end_pos.y - cur_pos.y <= m_slowDownThreshold) {
                            vel.y = vel.y * m_slowDownRatio;
                        }

                        if (vel.y < 1.0f) {
                            vel.y = 1.0f;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.y = end_pos.y;
                        window->Win_Set_Position(cur_pos.x, end_pos.y);
                        anim_win->Set_Finished(true);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromTopFast::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.y += vel.y;

                    if (cur_pos.y >= start_pos.y) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        ICoord2D end_pos = anim_win->Get_End_Pos();

                        if (end_pos.y - cur_pos.y > m_slowDownThreshold) {
                            vel.y = -m_maxVel.y;
                        } else {
                            vel.y = vel.y * m_speedUpRatio;
                        }

                        if (-m_maxVel.y > vel.y) {
                            vel.y = -m_maxVel.y;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.y = start_pos.y;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

ProcessAnimateWindowSlideFromRightFast::ProcessAnimateWindowSlideFromRightFast() :
    m_slowDownThreshold(60), m_slowDownRatio(0.77f), m_speedUpRatio(3.0f - m_slowDownRatio)
{
    m_maxVel.x = -80.0f;
    m_maxVel.y = 0.0f;
}

ProcessAnimateWindowSlideFromRightFast::~ProcessAnimateWindowSlideFromRightFast() {}

void ProcessAnimateWindowSlideFromRightFast::Init_Animate_Window(AnimateWindow *anim_win)
{
    ICoord2D rest_pos;
    ICoord2D start_pos;
    ICoord2D cur_pos;
    ICoord2D end_pos;
    Coord2D vel;

    rest_pos.x = 0;
    rest_pos.y = 0;
    start_pos.x = 0;
    start_pos.y = 0;
    cur_pos.x = 0;
    cur_pos.y = 0;
    end_pos.x = 0;
    end_pos.y = 0;
    vel.x = 0.0f;
    vel.y = 0.0f;
    int win_size_x = 0;
    int win_size_y = 0;

    if (anim_win != nullptr) {
        anim_win->Set_Finished(false);
        GameWindow *window = anim_win->Get_Game_Window();

        if (window != nullptr) {
            window->Win_Get_Position(&rest_pos.x, &rest_pos.y);
            window->Win_Get_Size(&win_size_x, &win_size_y);
            end_pos = rest_pos;
            unsigned int width = g_theDisplay->Get_Width();
            cur_pos.x = win_size_x + width;
            start_pos.x = win_size_x + width;
            cur_pos.y = rest_pos.y;
            start_pos.y = rest_pos.y;
            window->Win_Set_Position(win_size_x + width, rest_pos.y);
            vel.x = m_maxVel.x;
            vel.y = 0.0f;
            anim_win->Set_Anim_Data(start_pos, end_pos, cur_pos, rest_pos, vel, anim_win->Get_Delay() + rts::Get_Time(), 0);
        } else {
            captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Init_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

void ProcessAnimateWindowSlideFromRightFast::Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay)
{
    if (anim_win != nullptr) {
        if (anim_win->Get_Delay()) {
            anim_win->Set_Start_Time(max_delay - anim_win->Get_Delay() + rts::Get_Time());
        }

        anim_win->Set_Vel(anim_win->Get_Vel() * -1.0f);
        anim_win->Set_Finished(false);
        GameWindow *window = anim_win->Get_Game_Window();

        int win_pos_x;
        int win_pos_y;
        window->Win_Get_Position(&win_pos_x, &win_pos_y);
        ICoord2D pos;
        pos = anim_win->Get_Cur_Pos();
        pos.y = win_pos_y;
        anim_win->Set_Cur_Pos(pos);
        pos = anim_win->Get_End_Pos();
        pos.y = win_pos_y;
        anim_win->Set_End_Pos(pos);
        pos = anim_win->Get_Start_Pos();
        pos.y = win_pos_y;
        anim_win->Set_Start_Pos(pos);
    } else {
        captainslog_dbgassert(
            false, "anim_win was passed into Init_Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
    }
}

bool ProcessAnimateWindowSlideFromRightFast::Update_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D end_pos = anim_win->Get_End_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x += vel.x;

                    if (cur_pos.x >= end_pos.x) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);

                        if (cur_pos.x - end_pos.x <= m_slowDownThreshold) {
                            vel.x = vel.x * m_slowDownRatio;
                        }

                        if (vel.x >= -1.0f) {
                            vel.x = -1.0f;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.x = end_pos.x;
                        anim_win->Set_Finished(true);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Update_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}

bool ProcessAnimateWindowSlideFromRightFast::Reverse_Animate_Window(AnimateWindow *anim_win)
{
    if (anim_win != nullptr) {
        if (anim_win->Is_Finished()) {
            return true;
        } else {
            if (rts::Get_Time() >= anim_win->Get_Start_Time()) {
                GameWindow *window = anim_win->Get_Game_Window();

                if (window != nullptr) {
                    ICoord2D cur_pos = anim_win->Get_Cur_Pos();
                    ICoord2D start_pos = anim_win->Get_Start_Pos();
                    Coord2D vel = anim_win->Get_Vel();
                    cur_pos.x += vel.x;

                    if (cur_pos.x <= start_pos.x) {
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        anim_win->Set_Cur_Pos(cur_pos);
                        ICoord2D end_pos = anim_win->Get_End_Pos();

                        if (cur_pos.x - end_pos.x > m_slowDownThreshold) {
                            vel.x = -m_maxVel.x;
                        } else {
                            vel.x = vel.x * m_speedUpRatio;
                        }

                        if (-m_maxVel.x < vel.x) {
                            vel.x = -m_maxVel.x;
                        }

                        anim_win->Set_Vel(vel);
                        return false;
                    } else {
                        cur_pos.x = start_pos.x;
                        anim_win->Set_Finished(true);
                        window->Win_Set_Position(cur_pos.x, cur_pos.y);
                        return true;
                    }
                } else {
                    captainslog_dbgassert(false, "anim_win contains a NULL Pointer for it's GameWindow... Whatup wit dat?");
                    return true;
                }
            } else {
                return false;
            }
        }
    } else {
        captainslog_dbgassert(false, "anim_win was passed into Reverse_Animate_Window as a NULL Pointer... bad bad bad!");
        return true;
    }
}
