/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Base client object providing overall control of client IO and rendering.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameclient.h"
#include "drawable.h"
#include "gamemessage.h"
#include "rayeffect.h"

#ifdef GAME_DLL
#include "hooker.h"
#else
GameClient *g_theGameClient;
#endif

void GameClient::Remove_From_Ray_Effects(Drawable *drawable)
{
    g_theRayEffects->Delete_Ray_Effect(drawable);
}

void GameClient::Get_Ray_Effect_Data(Drawable *drawable, RayEffectData *data)
{
    g_theRayEffects->Get_Ray_Effect_Data(drawable, data);
}

GameMessageDisposition GameClientMessageDispatcher::Translate_Game_Message(const GameMessage *msg)
{
    GameMessage::MessageType type = msg->Get_Type();
    if (type >= GameMessage::MSG_BEGIN_NETWORK_MESSAGES && type <= GameMessage::MSG_END_NETWORK_MESSAGES) {
        return KEEP_MESSAGE;
    }

    switch (type) {
        case GameMessage::MSG_NEW_GAME: // Fallthrough
        case GameMessage::MSG_CLEAR_GAME_DATA: // Fallthrough
        case GameMessage::MSG_FRAME_TICK: // Fallthrough
            return KEEP_MESSAGE;
        default:
            break;
    }

    return DESTROY_MESSAGE;
}

GameMessageDisposition CommandTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, CommandTranslator, const GameMessage *>(
        PICK_ADDRESS(0x005EC8D0, 0x007ECAD8), this, msg);
#else
    return GameMessageDisposition();
#endif
}

void GameClient::Add_Text_Bearing_Drawable(Drawable *drawable)
{
    if (drawable) {
        m_drawableTB.push_back(drawable);
    }
}

void GameClient::Remove_Drawable_From_Lookup_Table(Drawable *drawable)
{
    if (drawable != nullptr) {
        m_drawableLUT[drawable->Get_ID()] = nullptr;
    }
}

void GameClient::Add_Drawable_To_Lookup_Table(Drawable *drawable)
{
    if (drawable != nullptr) {
        DrawableID id = drawable->Get_ID();

        while (id >= m_drawableLUT.size()) {
            m_drawableLUT.resize(2 * m_drawableLUT.size(), nullptr);
        }

        m_drawableLUT[id] = drawable;
    }
}

void GameClient::Flush_Text_Bearing_Drawables()
{
    for (auto it = m_drawableTB.begin(); it != m_drawableTB.end(); it++) {
        (*it)->Draw_UI_Text();
    }

    m_drawableTB.clear();
}
