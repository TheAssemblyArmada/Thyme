/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief W3D UI function pointer manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "functionlexicon.h"

class W3DFunctionLexicon : public FunctionLexicon
{
public:
    W3DFunctionLexicon() {}
    ~W3DFunctionLexicon() override {}

    void Init() override;
    void Reset() override { FunctionLexicon::Reset(); }
    void Update() override {}
};
