/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Selection Xlat
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
#include <map>

class ThingTemplate;
class Drawable;

class SelectionTranslator : public GameMessageTranslator
{
public:
    SelectionTranslator();
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;
    virtual ~SelectionTranslator() override;

    void Set_Left_Mouse_Button(bool set) { m_leftMouseButtonIsDown = set; }
    void Set_Drag_Selecting(bool set) { m_dragSelecting = set; }

private:
    bool m_leftMouseButtonIsDown;
    bool m_dragSelecting;
    unsigned int m_lastGroupSelTime;
    int m_lastGroupSelGroup;
    ICoord2D m_selectAnchor;
    ICoord2D m_deselectAnchor;
    bool m_displayedMaxWarning;
    int m_wheelPos;
    std::map<const ThingTemplate *, int> m_selectCountMap;
    Coord3D m_viewPos;
#ifdef GAME_DEBUG_STRUCTS
    bool m_handOfGodMode;
#endif
};

bool Can_Select_Drawable(const Drawable *drawable, bool drag_selecting);

#ifdef GAME_DLL
extern SelectionTranslator *&g_theSelectionTranslator;
#else
extern SelectionTranslator *g_theSelectionTranslator;
#endif
