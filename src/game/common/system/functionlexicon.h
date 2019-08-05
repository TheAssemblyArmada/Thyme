/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for UI function pointer manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "namekeygenerator.h"
#include "subsysteminterface.h"

class GameWindow;
class WinInstanceData;
class WinLayout;

typedef void (*drawfunc_t)(GameWindow *, WinInstanceData *);
typedef void (*layoutfunc_t)(WinLayout *, void *);

class FunctionLexicon : public SubsystemInterface
{
    ALLOW_HOOKING
public:
    enum TableIndex
    {
        TABLE_ANY = -1,
        TABLE_GAME_WIN_SYSTEM,
        TABLE_GAME_WIN_INPUT,
        TABLE_GAME_WIN_TOOLTIP,
        TABLE_GAME_WIN_DEVICEDRAW,
        TABLE_GAME_WIN_DRAW,
        TABLE_LAYOUT_INIT,
        TABLE_WIN_LAYOUT_INIT,
        TABLE_WIN_LAYOUT_UPDATE,
        TABLE_WIN_LAYOUT_SHUTDOWN,
        MAX_FUNCTION_TABLES,
    };

    struct TableEntry
    {
        NameKeyType key;
        const char *name;
        void *func;
    };

public:
    FunctionLexicon();
    virtual ~FunctionLexicon() {}

    virtual void Init() override;
    virtual void Reset() override { Init(); }
    virtual void Update() override {}

    void *Find_Function(NameKeyType key, TableIndex index);
    drawfunc_t Game_Win_Draw_Func(NameKeyType key, TableIndex index);
    layoutfunc_t Win_Layout_Init_Func(NameKeyType key, TableIndex index);

protected:
    void Load_Table(TableEntry *table, TableIndex index);
    bool Validate();
    void *Key_To_Func(NameKeyType key, TableEntry *table);

private:
    TableEntry *m_tables[MAX_FUNCTION_TABLES];
};

#ifdef GAME_DLL
#include "hooker.h"
extern FunctionLexicon *&g_theFunctionLexicon;
#else
extern FunctionLexicon *g_theFunctionLexicon;
#endif
