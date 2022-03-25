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
#include "gamemath.h"
#include "resource.h"

CW3DViewApp theApp;

BOOL CW3DViewApp::InitInstance()
{
    // TODO
    // ColorPickerClass::RegisterWndClass(AfxGetInstanceHandle());
    // ColorBarClass::RegisterWndClass(AfxGetInstanceHandle());
    GameMath::Init();
    CWinApp::SetRegistryKey("Westwood Studios");
    CWinApp::LoadStdProfileSettings(9);
    // TODO
    //  CWinApp::AddDocTemplate(new CSingleDocTemplate(
    //      IDR_MAINFRAME, CW3DViewDoc::GetRuntimeClass(), CMainFrame::GetRuntimeClass(), CW3DViewView::GetRuntimeClass()));
    CWinApp::EnableShellOpen();
    CWinApp::RegisterShellFileTypes(true);

    CCommandLineInfo info;
    CWinApp::ParseCommandLine(info);

    new W3DAssetManager();

    if (!CWinApp::ProcessShellCommand(info)) {
        return FALSE;
    }

    m_pMainWnd->ShowWindow(SW_SHOW);
    UpdateWindow(*m_pMainWnd);
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

void CW3DViewApp::OnAppAbout()
{
    // TODO
    // CAboutDialog dialog;
    // dialog.DoModal();
}
