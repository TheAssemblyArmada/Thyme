/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Hint Spy
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

class HintSpyTranslator : public GameMessageTranslator
{
public:
    HintSpyTranslator() {}
    virtual GameMessageDisposition Translate_Game_Message(const GameMessage *msg) override;
    virtual ~HintSpyTranslator() override {}
};
