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
#pragma once
#include "always.h"
#include "coord.h"
#include "mempoolobj.h"
#include "processanimatewindow.h"
#include "subsysteminterface.h"
#include <list>

class GameWindow;

enum AnimTypes
{
    WIN_ANIMATION_NONE,
    WIN_ANIMATION_SLIDE_RIGHT,
    WIN_ANIMATION_SLIDE_RIGHT_FAST,
    WIN_ANIMATION_SLIDE_LEFT,
    WIN_ANIMATION_SLIDE_TOP,
    WIN_ANIMATION_SLIDE_BOTTOM,
    WIN_ANIMATION_SPIRAL,
    WIN_ANIMATION_SLIDE_BOTTOM_TIMED,
    WIN_ANIMATION_SLIDE_TOP_FAST,
    WIN_ANIMATION_COUNT,
};

class AnimateWindow : public MemoryPoolObject
{
    IMPLEMENT_POOL(AnimateWindow)

public:
    AnimateWindow();
    virtual ~AnimateWindow() override;
    void Set_Anim_Data(ICoord2D start_pos,
        ICoord2D end_pos,
        ICoord2D cur_pos,
        ICoord2D rest_pos,
        Coord2D vel,
        unsigned int start_time,
        unsigned int end_time);

    void Set_Game_Window(GameWindow *window) { m_win = window; }
    void Set_Anim_Type(AnimTypes type) { m_animType = type; }
    void Set_Delay(unsigned int delay) { m_delay = delay; }
    void Set_Needs_To_Finish(bool finish) { m_needsToFinish = finish; }
    unsigned int Get_Delay() const { return m_delay; }
    void Set_Finished(bool finished) { m_isFinished = finished; }
    const ICoord2D Get_Rest_Pos() const { return m_restPos; }
    GameWindow *Get_Game_Window() const { return m_win; }
    AnimTypes Get_Anim_Type() const { return m_animType; }
    const Coord2D Get_Vel() const { return m_vel; }
    void Set_Vel(Coord2D vel) { m_vel = vel; }
    void Set_Start_Time(unsigned int time) { m_startTime = time; }
    const ICoord2D Get_Cur_Pos() const { return m_curPos; }
    const ICoord2D Get_End_Pos() const { return m_endPos; }
    unsigned int Get_Start_Time() const { return m_startTime; }
    void Set_Cur_Pos(ICoord2D pos) { m_curPos = pos; }
    bool Is_Finished() const { return m_isFinished; }
    ICoord2D Get_Start_Pos() const { return m_startPos; }
    unsigned int Get_End_Time() const { return m_endTime; }
    void Set_Start_Pos(ICoord2D pos) { m_startPos = pos; }
    void Set_End_Pos(ICoord2D pos) { m_endPos = pos; }

private:
    unsigned int m_delay;
    ICoord2D m_startPos;
    ICoord2D m_endPos;
    ICoord2D m_curPos;
    ICoord2D m_restPos;
    GameWindow *m_win;
    Coord2D m_vel;
    unsigned int m_startTime;
    unsigned int m_endTime;
    AnimTypes m_animType;
    bool m_needsToFinish;
    bool m_isFinished;
};

class AnimateWindowManager : public SubsystemInterface
{
public:
    AnimateWindowManager();
    virtual ~AnimateWindowManager() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    void Register_Game_Window(
        GameWindow *win, AnimTypes anim_type, bool needs_to_finish, unsigned int ms, unsigned int delay_ms);
    ProcessAnimateWindow *Get_Process_Animate(AnimTypes anim_type);
    void Reverse_Animate_Window();
    void Reset_To_Rest_Position();

    bool Is_Finished() const { return !m_needsUpdate; }
    bool Is_Reversed() const { return m_reverse; }

#ifdef GAME_DLL
    AnimateWindowManager *Hook_Ctor() { return new (this) AnimateWindowManager; }
#endif

private:
    std::list<AnimateWindow *> m_winList;
    std::list<AnimateWindow *> m_winMustFinishList;
    bool m_needsUpdate;
    bool m_reverse;
    ProcessAnimateWindowSlideFromRight *m_slideFromRight;
    ProcessAnimateWindowSlideFromRightFast *m_slideFromRightFast;
    ProcessAnimateWindowSlideFromTop *m_slideFromTop;
    ProcessAnimateWindowSlideFromLeft *m_slideFromLeft;
    ProcessAnimateWindowSlideFromBottom *m_slideFromBottom;
    ProcessAnimateWindowSpiral *m_spiral;
    ProcessAnimateWindowSlideFromBottomTimed *m_slideFromBottomTimed;
    ProcessAnimateWindowSlideFromTopFast *m_slideFromTopFast;
};
