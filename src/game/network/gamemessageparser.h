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
#pragma once
#include "always.h"
#include "gamemessage.h"
#include "mempoolobj.h"
#include <new>

class GameMessageParserArgumentType : public MemoryPoolObject
{
    IMPLEMENT_POOL(GameMessageParserArgumentType);

public:
    GameMessageParserArgumentType(ArgumentDataType type, int arg_count);
    virtual ~GameMessageParserArgumentType() {}

    GameMessageParserArgumentType *Get_Next() { return m_next; }
    void Set_Next(GameMessageParserArgumentType *next) { m_next = next; }
    int Get_Arg_Count() { return m_argCount; }
    ArgumentDataType Get_Type() { return m_type; }

private:
    GameMessageParserArgumentType *m_next;
    ArgumentDataType m_type;
    int m_argCount;
};

class GameMessageParser : public MemoryPoolObject
{
    IMPLEMENT_POOL(GameMessageParser);

protected:
    virtual ~GameMessageParser() override;

public:
    GameMessageParser();
    GameMessageParser(GameMessage *msg);

    GameMessageParserArgumentType *Get_First_Arg_Type() { return m_first; }
    void Add_Arg_Type(ArgumentDataType type, int arg_count);
    int Get_Num_Types() { return m_argTypeCount; }

#ifdef GAME_DLL
    GameMessageParser *Hook_Ctor(GameMessage *msg) { return new (this) GameMessageParser(msg); }
    void Hook_Dtor() { GameMessageParser::~GameMessageParser(); }
#endif

private:
    GameMessageParserArgumentType *m_first;
    GameMessageParserArgumentType *m_last;
    int m_argTypeCount;
};
