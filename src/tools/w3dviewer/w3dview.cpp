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
#include "w3dview.h"
#include "assetmgr.h"
#include "colorbar.h"
#include "colorpicker.h"
#include "gamemath.h"
#include "mainfrm.h"
#include "resource.h"
#include "w3dviewdoc.h"
#include "w3dviewview.h"

CW3DViewApp theApp;

BOOL CW3DViewApp::InitInstance()
{
    ColorPickerClass::RegisterWndClass(AfxGetInstanceHandle());
    ColorBarClass::RegisterWndClass(AfxGetInstanceHandle());
    GameMath::Init();
    CWinApp::SetRegistryKey("Westwood Studios");
    CWinApp::LoadStdProfileSettings(9);
    CWinApp::AddDocTemplate(new CSingleDocTemplate(
        IDR_MAINFRAME, CW3DViewDoc::GetThisClass(), CMainFrame::GetThisClass(), CW3DViewView::GetThisClass()));
    CWinApp::EnableShellOpen();
    CWinApp::RegisterShellFileTypes(true);

    CCommandLineInfo info;
    CWinApp::ParseCommandLine(info);

    new W3DAssetManager();

    if (!CWinApp::ProcessShellCommand(info)) {
        return FALSE;
    }

    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();
    m_pMainWnd->DragAcceptFiles(true);
    m_isInitialized = true;

    return CWinApp::InitInstance();
}

int CW3DViewApp::ExitInstance()
{
    if (m_isInitialized) {
        W3DAssetManager::Get_Instance()->Free_Assets();
        GameMath::Shutdown();
        W3DAssetManager::Get_Instance()->Delete_This();
    }

    return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(CW3DViewApp, CWinApp)
ON_COMMAND(ID_APP_ABOUT, &CW3DViewApp::OnAppAbout)
ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CAboutDialog, CDialog)
END_MESSAGE_MAP()

CAboutDialog::CAboutDialog() : CDialog(IDD_ABOUT) {}

void CW3DViewApp::OnAppAbout()
{
    CAboutDialog dialog;
    dialog.DoModal();
}
