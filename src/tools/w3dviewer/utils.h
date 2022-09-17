/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View utillity functions
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "w3dafx.h"
class CGraphicView;
class CW3DViewDoc;

CString GetFilePath(const char *name);
CString GetFilenameFromPath(const char *path);
CGraphicView *GetCurrentGraphicView();
bool HasBaseModelName(const char *name);
bool HasMultipleLODs(const char *name);
CW3DViewDoc *GetCurrentDocument();
void SetDlgItemFloat(HWND hDlg, int nIDDlgItem, float f);
float GetDlgItemFloat(HWND hDlg, int nIDDlgItem);
void InitializeSpinButton(CSpinButtonCtrl *spin, float value, float min, float max);
void UpdateEditCtrl(HWND hwnd, int delta);
void DisableWindows(HWND window, bool disable);
void SetWindowFloat(HWND hWnd, float f);
float GetWindowFloat(HWND hWnd);
void PositionWindow(HWND hWnd);
void PaintGradient(HWND hwnd, bool red, bool green, bool blue);
