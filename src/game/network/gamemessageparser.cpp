/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamemessageparser.h"

GameMessageParserArgumentType::GameMessageParserArgumentType(ArgumentDataType type, int arg_count) :
    m_next(nullptr), m_type(type), m_argCount(arg_count)
{
}

GameMessageParser::GameMessageParser() : m_first(nullptr), m_last(nullptr), m_argTypeCount(0) {}

GameMessageParser::GameMessageParser(GameMessage *msg) : m_first(nullptr), m_last(nullptr), m_argTypeCount(0)
{
    ArgumentDataType type = ARGUMENTDATATYPE_UNKNOWN;
    int count = 0;

    for (int i = 0; i < msg->Get_Argument_Count(); ++i) {
        ArgumentDataType data_type = msg->Get_Argument_Type(i);
        if (data_type != type) {
            if (count > 0) {
                Add_Arg_Type(type, count);
                ++m_argTypeCount;
            }
            type = data_type;
            count = 0;
        }
        ++count;
    }

    if (count > 0) {
        Add_Arg_Type(type, count);
        ++m_argTypeCount;
    }
}

GameMessageParser::~GameMessageParser()
{
    while (m_first != nullptr) {
        GameMessageParserArgumentType *next = m_first->Get_Next();
        m_first->Delete_Instance();
        m_first = next;
    }
}

void GameMessageParser::Add_Arg_Type(ArgumentDataType type, int arg_count)
{
    if (m_first == nullptr) {
        m_first = NEW_POOL_OBJ(GameMessageParserArgumentType, type, arg_count);
        m_last = m_first;
    } else {
        m_last->Set_Next(NEW_POOL_OBJ(GameMessageParserArgumentType, type, arg_count));
        m_last = m_last->Get_Next();
    }
}
