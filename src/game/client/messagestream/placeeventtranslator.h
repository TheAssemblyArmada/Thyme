/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Place Event Translator
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

class PlaceEventTranslator : public GameMessageTranslator
{
public:
    PlaceEventTranslator() : m_frame(-1) {}
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;
    virtual ~PlaceEventTranslator() override {}

private:
    int m_frame;
};
