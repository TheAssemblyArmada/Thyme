/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Look At Translator
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
#include "messagestream.h"
#include "view.h"

class LookAtTranslator : public GameMessageTranslator
{
public:
    LookAtTranslator();
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;
    virtual ~LookAtTranslator() override;
    virtual const ICoord2D *Get_RMB_Scroll_Anchor();
    bool Has_Mouse_Moved_Recently();

private:
    ICoord2D m_anchor;
    ICoord2D m_originalAnchor;
    ICoord2D m_currentPos;
    bool m_isScrolling;
    bool m_isRotating;
    bool m_isPitching;
    bool m_isChangingFOV;
    unsigned int m_timestamp;
    unsigned int m_lastPlaneID;
    ViewLocation m_viewLocation[8];
    int m_scrollType;
    unsigned int m_lastMouseMoveFrame;
};

#ifdef GAME_DLL
extern LookAtTranslator *&g_theLookAtTranslator;
#else
extern LookAtTranslator *g_theLookAtTranslator;
#endif
