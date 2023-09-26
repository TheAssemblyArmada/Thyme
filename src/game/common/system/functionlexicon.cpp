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
#include "functionlexicon.h"
#include <captainslog.h>

#ifndef GAME_DLL
FunctionLexicon *g_theFunctionLexicon = nullptr;
#else
#include "hooker.h"
#endif

FunctionLexicon::FunctionLexicon()
{
    memset(m_tables, 0, sizeof(m_tables));
}

/**
 * @brief Load a table of function pointers, generating keys for their names for easy lookup.
 */
void FunctionLexicon::Load_Table(TableEntry *table, TableIndex index)
{
    if (table == nullptr) {
        return;
    }

    // Generate keys for all the entry names.
    for (TableEntry *te = table; te->name != nullptr; ++te) {
        te->key = g_theNameKeyGenerator->Name_To_Key(te->name);
    }

    m_tables[index] = table;
}

/**
 * @brief Internal function that validates that no duplicate functions exist in the lexicon.
 */
bool FunctionLexicon::Validate()
{
    bool valid = true;

    // Iterate over all the tables looking for function duplicates, log if found and fail validate
    for (int i = 0; i < MAX_FUNCTION_TABLES; ++i) {
        for (TableEntry *i_ent = m_tables[i]; i_ent != nullptr && i_ent->key != NAMEKEY_INVALID; ++i_ent) {
            for (int j = 0; j < MAX_FUNCTION_TABLES; ++j) {
                for (TableEntry *j_ent = m_tables[j]; j_ent != nullptr && j_ent->key != NAMEKEY_INVALID; ++j_ent) {
                    if (i_ent != j_ent && i_ent->func == j_ent->func) {
                        captainslog_warn(
                            "Function lexicon entries match same address! '%s' and '%s'\n", i_ent->name, j_ent->name);
                        valid = false;
                    }
                }
            }
        }
    }

    return valid;
}

/**
 * @brief Internal function to look up a function in the lexicon from a key.
 */
void *FunctionLexicon::Key_To_Func(NameKeyType key, TableEntry *table)
{
    if (key == NAMEKEY_INVALID || table == nullptr) {
        return nullptr;
    }

    for (TableEntry *tbl = table; tbl != nullptr && tbl->key != NAMEKEY_INVALID; ++tbl) {
        if (tbl->key == key) {
            return tbl->func;
        }
    }

    return nullptr;
}

/**
 * @brief Initialises all the tables with appropriate key values.
 */
void FunctionLexicon::Init()
{
#ifdef GAME_DLL
    Call_Method<void, FunctionLexicon>(PICK_ADDRESS(0x004F3EC0, 0x00A31671), this);
#else
// TODO requires several function pointer tables, do them as functions that call them require it?
#endif
}

/**
 * @brief Finds a function from a key in the table specified by index.
 */
void *FunctionLexicon::Find_Function(NameKeyType key, TableIndex index)
{
    if (key == NAMEKEY_INVALID) {
        return nullptr;
    }

    // If the index is specific, access that table.
    if (index != TABLE_ANY) {
        return Key_To_Func(key, m_tables[index]);
    }

    // Otherwise iterate through all tables to find a match.
    for (int i = 0; i < MAX_FUNCTION_TABLES; ++i) {
        void *tmp = Key_To_Func(key, m_tables[i]);

        if (tmp != nullptr) {
            return tmp;
        }
    }

    return nullptr;
}

/**
 * @brief Specifically locates functions relating to window drawing.
 */
WindowDrawFunc FunctionLexicon::Game_Win_Draw_Func(NameKeyType key, TableIndex index)
{
    if (index != TABLE_ANY) {
        return (WindowDrawFunc)Find_Function(key, index);
    }

    void *tmp = Find_Function(key, TABLE_GAME_WIN_DEVICEDRAW);

    if (tmp == nullptr) {
        tmp = Find_Function(key, TABLE_GAME_WIN_DRAW);
    }

    return (WindowDrawFunc)tmp;
}

/**
 * @brief Specifically locates functions relating to layout initialisation.
 */
WindowLayoutCallbackFunc FunctionLexicon::Win_Layout_Init_Func(NameKeyType key, TableIndex index)
{
    if (index != TABLE_ANY) {
        return (WindowLayoutCallbackFunc)Find_Function(key, index);
    }

    void *tmp = Find_Function(key, TABLE_WIN_LAYOUT_INIT);

    if (tmp == nullptr) {
        tmp = Find_Function(key, TABLE_LAYOUT_INIT);
    }

    return (WindowLayoutCallbackFunc)tmp;
}
