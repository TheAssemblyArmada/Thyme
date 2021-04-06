/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief C Interface for DebugWindow developer plugin.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void AdjustVariable(const char *var, const char *val);
void AdjustVariableAndPause(const char *var, const char *val);
void AppendMessage(const char *msg);
void AppendMessageAndPause(const char *msg);
bool CanAppContinue();
void CreateDebugDialog();
void DestroyDebugDialog();
void ForceAppContinue();
bool RunAppFast();
void SetFrameNumber(int frame);

#ifdef __cplusplus
} // extern "C"
#endif
