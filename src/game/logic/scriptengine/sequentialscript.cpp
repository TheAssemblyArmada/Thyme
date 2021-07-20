/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object for handling scripts that should execute sequentially.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "sequentialscript.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

SequentialScript::SequentialScript() :
    m_teamToExecOn(nullptr),
    m_objectID(OBJECT_UNK),
    m_scriptToExecuteSequentially(nullptr),
    m_currentInstruction(-1),
    m_timesToLoop(0),
    m_framesToWait(-1),
    m_unkbool1(false),
    m_nextScriptInSequence(nullptr)
{
}

void SequentialScript::Xfer_Snapshot(Xfer *xfer)
{
    // Requires parts of Team and TeamFactory
#ifdef GAME_DLL
    Call_Method<void, SequentialScript, Xfer *>(PICK_ADDRESS(0x00436980, 0x007128E7), this, xfer);
#endif
}
