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

class AnimateWindow;

class ProcessAnimateWindow
{
public:
    ProcessAnimateWindow() {}
    virtual ~ProcessAnimateWindow() {}
    virtual void Init_Animate_Window(AnimateWindow *anim_win) = 0;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) = 0;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) = 0;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) = 0;
    virtual void Set_Max_Duration(unsigned int max_duration) {}
};

class ProcessAnimateWindowSlideFromRight : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromRight();
    virtual ~ProcessAnimateWindowSlideFromRight() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    Coord2D m_maxVel;
    int m_slowDownThreshold;
    float m_slowDownRatio;
    float m_speedUpRatio;
};

class ProcessAnimateWindowSlideFromRightFast : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromRightFast();
    virtual ~ProcessAnimateWindowSlideFromRightFast() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    Coord2D m_maxVel;
    int m_slowDownThreshold;
    float m_slowDownRatio;
    float m_speedUpRatio;
};

class ProcessAnimateWindowSlideFromLeft : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromLeft();
    virtual ~ProcessAnimateWindowSlideFromLeft() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    Coord2D m_maxVel;
    int m_slowDownThreshold;
    float m_slowDownRatio;
    float m_speedUpRatio;
};

class ProcessAnimateWindowSlideFromTop : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromTop();
    virtual ~ProcessAnimateWindowSlideFromTop() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    Coord2D m_maxVel;
    int m_slowDownThreshold;
    float m_slowDownRatio;
    float m_speedUpRatio;
};

class ProcessAnimateWindowSlideFromTopFast : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromTopFast();
    virtual ~ProcessAnimateWindowSlideFromTopFast();
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    Coord2D m_maxVel;
    int m_slowDownThreshold;
    float m_slowDownRatio;
    float m_speedUpRatio;
};

class ProcessAnimateWindowSlideFromBottom : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromBottom();
    virtual ~ProcessAnimateWindowSlideFromBottom() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    Coord2D m_maxVel;
    int m_slowDownThreshold;
    float m_slowDownRatio;
    float m_speedUpRatio;
};

class ProcessAnimateWindowSpiral : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSpiral();
    virtual ~ProcessAnimateWindowSpiral() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;

private:
    float m_deltaTheta;
    int m_maxR;
};

class ProcessAnimateWindowSlideFromBottomTimed : public ProcessAnimateWindow
{
public:
    ProcessAnimateWindowSlideFromBottomTimed();
    virtual ~ProcessAnimateWindowSlideFromBottomTimed() override;
    virtual void Init_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Init_Reverse_Animate_Window(AnimateWindow *anim_win, unsigned int max_delay) override;
    virtual bool Update_Animate_Window(AnimateWindow *anim_win) override;
    virtual bool Reverse_Animate_Window(AnimateWindow *anim_win) override;
    virtual void Set_Max_Duration(unsigned int max_duration) override;

private:
    unsigned int m_maxDuration;
};
