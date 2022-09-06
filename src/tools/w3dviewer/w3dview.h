/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View
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

class CW3DViewApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnAppAbout();

private:
    bool m_isInitialized;
};

class CAboutDialog : public CDialog
{
public:
    CAboutDialog();
    DECLARE_MESSAGE_MAP()
};

extern CW3DViewApp theApp;
