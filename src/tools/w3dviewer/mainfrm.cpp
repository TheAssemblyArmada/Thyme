/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View main frame window
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mainfrm.h"
#include "assetmgr.h"
#include "datatreeview.h"
#include "deviceselectiondialog.h"
#include "graphicview.h"
#include "light.h"
#include "renderdevicedesc.h"
#include "resource.h"
#include "utils.h"
#include "viewerscene.h"
#include "w3d.h"
#include "w3dview.h"
#include "w3dviewdoc.h"

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

// clang-format off
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     ON_WM_CREATE()
     ON_COMMAND(ID_OBJECT_PROPERTIES, OnProperties)
     ON_UPDATE_COMMAND_UI(ID_OBJECT_PROPERTIES, OnUpdateProperties)
     ON_COMMAND(ID_AGGREGATE_GENERATELOD, OnGenerateLOD)
     ON_WM_ACTIVATEAPP()
     ON_COMMAND(ID_FILE_OPEN, OnOpen)
     ON_COMMAND(ID_ANIMATION_SETTINGS, OnAnimSettings)
     ON_COMMAND(ID_ANIMATION_STOP, OnStopAnim)
     ON_COMMAND(ID_ANIMATION_PLAY, OnPlayAnim)
     ON_COMMAND(ID_ANIMATION_PAUSE, OnPauseAnim)
     ON_COMMAND(ID_CAMERA_BACK, OnBack)
     ON_COMMAND(ID_CAMERA_BOTTOM, OnBottom)
     ON_COMMAND(ID_CAMERA_FRONT, OnFront)
     ON_COMMAND(ID_CAMERA_LEFT, OnLeft)
     ON_COMMAND(ID_CAMERA_RESET, OnResetCamera)
     ON_COMMAND(ID_CAMERA_RIGHT, OnRight)
     ON_COMMAND(ID_CAMERA_TOP, OnTop)
     ON_COMMAND(ID_OBJECT_ROTATEZ, OnRotateZ)
     ON_COMMAND(ID_OBJECT_ROTATEY, OnRotateY)
     ON_COMMAND(ID_OBJECT_ROTATEX, OnRotateX)
     ON_COMMAND(ID_LIGHTING_AMBIENT, OnAmbient)
     ON_COMMAND(ID_LIGHTING_SCENELIGHT, OnSceneLight)
     ON_COMMAND(ID_BACKGROUND_COLOR, OnBackgroundColor)
     ON_COMMAND(ID_BACKGROUND_BITMAP, OnBackgroundBitmap)
     ON_COMMAND(ID_EXPORT_LOD, OnExportLOD)
     ON_COMMAND(ID_BACKGROUND_OBJECT, OnBackgroundObject)
     ON_UPDATE_COMMAND_UI(ID_TOOLBARS_ANIMATION, OnUpdateAnimationToolbar)
     ON_UPDATE_COMMAND_UI(ID_TOOLBARS_OBJECT, OnUpdateObjectToolbar)
     ON_COMMAND(ID_TOOLBARS_ANIMATION, OnAnimationToolbar)
     ON_COMMAND(ID_TOOLBARS_OBJECT, OnObjectToolbar)
     ON_COMMAND(ID_ANIMATION_STEPFORWARD, OnStepForward)
     ON_COMMAND(ID_ANIMATION_STEPBACK, OnStepBack)
     ON_COMMAND(ID_OBJECT_RESET, OnReset)
     ON_COMMAND(ID_CAMERA_ROTATEXONLY, OnCameraRotateX)
     ON_COMMAND(ID_CAMERA_ROTATEYONLY, OnCameraRotateY)
     ON_COMMAND(ID_CAMERA_ROTATEZONLY, OnCameraRotateZ)
     ON_UPDATE_COMMAND_UI(ID_CAMERA_ROTATEXONLY, OnUpdateCameraRotateX)
     ON_UPDATE_COMMAND_UI(ID_CAMERA_ROTATEYONLY, OnUpdateCameraRotateY)
     ON_UPDATE_COMMAND_UI(ID_CAMERA_ROTATEZONLY, OnUpdateCameraRotateZ)
     ON_UPDATE_COMMAND_UI(ID_OBJECT_ROTATEX, OnUpdateRotateX)
     ON_UPDATE_COMMAND_UI(ID_OBJECT_ROTATEY, OnUpdateRotateY)
     ON_UPDATE_COMMAND_UI(ID_OBJECT_ROTATEZ, OnUpdateRotateZ)
     ON_WM_WINDOWPOSCHANGING()
     ON_WM_WINDOWPOSCHANGED()
     ON_COMMAND(ID_EMITTERS_CREATEEMITTER, OnCreateEmitter)
     ON_COMMAND(ID_EMITTERS_EDITEMITTER, OnEditEmitter)
     ON_UPDATE_COMMAND_UI(ID_EMITTERS_EDITEMITTER, OnUpdateEditEmitter)
     ON_COMMAND(ID_EXPORT_EMITTER, OnExportEmitter)
     ON_UPDATE_COMMAND_UI(ID_EXPORT_EMITTER, OnUpdateExportEmitter)
     ON_COMMAND(ID_AGGREGATE_AUTOASSIGNBONEMODELS, OnAutoAssignBone)
     ON_COMMAND(ID_AGGREGATE_BONEMANAGEMENT, OnBoneManagement)
     ON_COMMAND(ID_EXPORT_AGGREGATE, OnExportAggregate)
     ON_COMMAND(ID_CAMERA_ANIMATECAMERA, OnAnimateCamera)
     ON_UPDATE_COMMAND_UI(ID_CAMERA_ANIMATECAMERA, OnUpdateAnimateCamera)
     ON_UPDATE_COMMAND_UI(ID_EXPORT_LOD, OnUpdateExportLod)
     ON_UPDATE_COMMAND_UI(ID_EXPORT_AGGREGATE, OnUpdateExportAggregate)
     ON_COMMAND(ID_CAMERA_RESETONDISPLAY, OnResetOnDisplay)
     ON_UPDATE_COMMAND_UI(ID_CAMERA_RESETONDISPLAY, OnUpdateResetOnDisplay)
     ON_COMMAND(ID_OBJECT_ROTATEYBACKWARDS, OnRotateYBackwards)
     ON_COMMAND(ID_OBJECT_ROTATEZBACKWARDS, OnRotateZBackwards)
     ON_COMMAND(ID_LIGHTING_ROTATEY, OnLightingRotateY)
     ON_COMMAND(ID_LIGHTING_ROTATEYBACKWARDS, OnLightingRotateYBackwards)
     ON_COMMAND(ID_LIGHTING_ROTATEZ, OnLightingRotateZ)
     ON_COMMAND(ID_LIGHTING_ROTATEZBACKWARDS, OnLightingRotateZBackwards)
     ON_WM_DESTROY()
     ON_COMMAND(ID_LIGHTING_DECSCENELIGHTINTENSITY, OnDecSceneLight)
     ON_COMMAND(ID_LIGHTING_INCSCENELIGHTINTENSITY, OnIncSceneLight)
     ON_COMMAND(ID_LIGHTING_DECAMBIENTINTENSITY, OnDecAmbient)
     ON_COMMAND(ID_LIGHTING_INCAMBIENTINTENSITY, OnIncAmbient)
     ON_COMMAND(ID_HEIRARCHY_MAKEAGGREGATE, OnMakeAggregate)
     ON_COMMAND(ID_AGGREGATE_RENAMEAGGREGATE, OnRenameAggregate)
     ON_COMMAND(ID_LOD_RECORDSCREENAREA, OnRecordSceneCamera)
     ON_COMMAND(ID_LOD_INCLUDENULLOBJECT, OnIncludeNull)
     ON_UPDATE_COMMAND_UI(ID_LOD_INCLUDENULLOBJECT, OnUpdateIncludeNull)
     ON_COMMAND(ID_LOD_PREVLEVEL, OnPrevLod)
     ON_UPDATE_COMMAND_UI(ID_LOD_PREVLEVEL, OnUpdatePrevLod)
     ON_COMMAND(ID_LOD_NEXTLEVEL, OnNextLod)
     ON_UPDATE_COMMAND_UI(ID_LOD_NEXTLEVEL, OnUpdateNextLod)
     ON_COMMAND(ID_LOD_AUTOSWITCHING, OnAutoSwitching)
     ON_UPDATE_COMMAND_UI(ID_LOD_AUTOSWITCHING, OnUpdateAutoSwitching)
     ON_UPDATE_COMMAND_UI(ID_MOVIE_MAKEMOVIE, OnUpdateMovie)
     ON_COMMAND(ID_MOVIE_MAKEMOVIE, OnMovie)
     ON_COMMAND(ID_MOVIE_CAPTURESCREENSHOT, OnScreenshot)
     ON_COMMAND(ID_VIEW_NEXT, OnNext)
     ON_COMMAND(ID_VIEW_PREV, OnPrev)
     ON_COMMAND(ID_ANIMATION_ADVANCED, OnAnimAdvanced)
     ON_UPDATE_COMMAND_UI(ID_ANIMATION_ADVANCED, OnUpdateAnimAdvanced)
     ON_COMMAND(ID_CAMERA_SETTINGS, OnCameraSettings)
     ON_COMMAND(ID_CAMERA_COPYSCREENSIZETOCLIPBOARD, OnCopyScreenSize)
     ON_COMMAND(ID_MISSINGTEXTURES, OnMissingTextures)
     ON_COMMAND(ID_COPYDEPENDENCIES, OnCopyDeps)
     ON_UPDATE_COMMAND_UI(ID_COPYDEPENDENCIES, OnUpdateCopyDeps)
     ON_COMMAND(ID_LIGHTING_EXPOSEPRECALCULATEDLIGHTING, OnExposePrecalc)
     ON_UPDATE_COMMAND_UI(ID_LIGHTING_EXPOSEPRECALCULATEDLIGHTING, OnUpdateExposePrecalc)
     ON_COMMAND(ID_FILE_TEXTUREPATH, OnTexturePath)
     ON_COMMAND(ID_PRIMITIVES_CREATESPHERE, OnCreateSphere)
     ON_COMMAND(ID_PRIMITIVES_CREATERING, OnCreateRing)
     ON_UPDATE_COMMAND_UI(ID_PRIMITIVES_EDITPRIMITIVE, OnUpdateEditPrimitive)
     ON_COMMAND(ID_PRIMITIVES_EDITPRIMITIVE, OnEditPrimitive)
     ON_COMMAND(ID_EXPORT_PRIMITIVE, OnExportPrimitive)
     ON_UPDATE_COMMAND_UI(ID_EXPORT_PRIMITIVE, OnUpdateExportPrimitive)
     ON_COMMAND(ID_LIGHTING_KILLSCENELIGHT, OnKillSceneLight)
     ON_COMMAND(ID_LIGHTING_MULTIPASSLIGHTING, OnMultiPassLighting)
     ON_UPDATE_COMMAND_UI(ID_LIGHTING_MULTIPASSLIGHTING, OnUpdateMultiPassLighting)
     ON_COMMAND(ID_LIGHTING_MULTITEXTURELIGHTING, OnMultiTextureLighting)
     ON_UPDATE_COMMAND_UI(ID_LIGHTING_MULTITEXTURELIGHTING, OnUpdateMultiTextureLighting)
     ON_COMMAND(ID_LIGHTING_VERTEXLIGHTING, OnVertexLighting)
     ON_UPDATE_COMMAND_UI(ID_LIGHTING_VERTEXLIGHTING, OnUpdateVertexLighting)
     ON_COMMAND(ID_ADDOBJECT, OnAddObject)
     ON_UPDATE_COMMAND_UI(ID_ADDOBJECT, OnUpdateAddObject)
     ON_COMMAND(ID_FILE_IMPORTFACIALANIMS, OnImportFacial)
     ON_UPDATE_COMMAND_UI(ID_FILE_IMPORTFACIALANIMS, OnUpdateImportFacial)
     ON_COMMAND(ID_OBJECT_RESTRICTANIMS, OnRestrictAnims)
     ON_UPDATE_COMMAND_UI(ID_OBJECT_RESTRICTANIMS, OnUpdateRestrictAnims)
     ON_COMMAND(ID_AGGREGATE_BINDSUBOBJECTLOD, OnBindSubobject)
     ON_UPDATE_COMMAND_UI(ID_AGGREGATE_BINDSUBOBJECTLOD, OnUpdateBindSubobject)
     ON_COMMAND(ID_CAMERA_SETDISTANCE, OnSetDistance)
     ON_COMMAND(ID_OBJECT_TOGGLEALTERNATEMATERIALS, OnAlternateMaterials)
     ON_COMMAND(ID_SOUND_CREATESOUNDOBJECT, OnCreateSoundObject)
     ON_COMMAND(ID_SOUND_EDITSOUNDOBJECT, OnEditSoundObject)
     ON_UPDATE_COMMAND_UI(ID_SOUND_EDITSOUNDOBJECT, OnUpdateEditSoundObject)
     ON_COMMAND(ID_EXPORT_SOUNDOBJECT, OnExportSoundObject)
     ON_UPDATE_COMMAND_UI(ID_EXPORT_SOUNDOBJECT, OnUpdateEditSoundObject)
     ON_COMMAND(ID_VIEW_WIREFRAMEMODE, OnWireframeMode)
     ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAMEMODE, OnUpdateWireframeMode)
     ON_UPDATE_COMMAND_UI(ID_BACKGROUND_FOG, OnUpdateFog)
     ON_COMMAND(ID_BACKGROUND_FOG, OnFog)
     ON_UPDATE_COMMAND_UI(ID_EMITTERS_SCALEEMITTER, OnUpdateScaleEmitter)
     ON_COMMAND(ID_EMITTERS_SCALEEMITTER, OnScaleEmitter)
     ON_UPDATE_COMMAND_UI(ID_VIEW_POLYGONSORTING, OnUpdatePolygonSorting)
     ON_COMMAND(ID_VIEW_POLYGONSORTING, OnPolygonSorting)
     ON_COMMAND(ID_CAMERA_XCAMERA, OnPlusXCamera)
     ON_UPDATE_COMMAND_UI(ID_CAMERA_XCAMERA, OnUpdatePlusXCamera)
     ON_COMMAND(ID_FILE_MUNGESORTONLOAD, OnMungeSort)
     ON_UPDATE_COMMAND_UI(ID_FILE_MUNGESORTONLOAD, OnUpdateMungeSort)
     ON_COMMAND(ID_FILE_ENABLEGAMMACORRECTION, OnEnableGamma)
     ON_UPDATE_COMMAND_UI(ID_FILE_ENABLEGAMMACORRECTION, OnUpdateEnableGamma)
     ON_COMMAND(ID_VIEW_SETGAMMA, OnSetGamma)
END_MESSAGE_MAP()
// clang-format on

int DeviceWidth = 640;
int DeviceHeight = 480;
int Device = -1;
int BPP = -1;

CMainFrame::CMainFrame() : m_currentType(-1), m_animationToolbarVisible(true), m_initialized(false) {}

CMainFrame::~CMainFrame() {}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT &cs)
{
    return CFrameWnd::PreCreateWindow(cs);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
{
    if (nCode != -1) {
        return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    if (pExtra == nullptr) {
        return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    CCmdUI *pCmdUI = (CCmdUI *)pExtra;

    if (pCmdUI->m_nID < 1000 || pCmdUI->m_nID > 1100) {
        return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    pCmdUI->Enable();
    return TRUE;
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (lParam == 0 && LOWORD(wParam) >= 1000 && LOWORD(wParam < 1100)) {
        MENUITEMINFO info;
        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        info.fMask = MIIM_DATA | MIIM_TYPE;
        info.dwTypeData = (char *)&info.hbmpItem;
        info.cch = 200;

        if (GetMenuItemInfo(m_subMenu, LOWORD(wParam), FALSE, &info)) {
            // do later
            // EditorParticleEmitterDefClas *def = new EditorParticleEmitterDefClass();
            // GetCurrentDocument()->AddEmittersToDef(def, (const char *)&info.hbmpItem, nullptr);
            // EmitterPropertySheetClass sheet(def, IDS_EMITTERPROPERTIES, this);
            // sheet.DoModal();
        }
    }

    return CFrameWnd::OnCommand(wParam, lParam);
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CLOSE) {
        CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();

        if (doc != nullptr) {
            doc->Free();
        }
    }

    return CFrameWnd::WindowProc(message, wParam, lParam);
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext)
{
    BOOL ret = m_splitter.CreateStatic(this, 1, 2);

    if (ret) {
        ret &= m_splitter.CreateView(0, 0, CDataTreeView::GetThisClass(), SIZE{ 340, 10 }, pContext);
        ret &= m_splitter.CreateView(0, 1, CGraphicView::GetThisClass(), SIZE{ 120, 10 }, pContext);

        if (ret) {
            CWnd *pane = m_splitter.GetPane(0, 1);

            if (pane != nullptr) {
                char path[MAX_PATH];
                GetModuleFileName(nullptr, path, MAX_PATH);
                char *str = strrchr(path, '\\');

                if (str != nullptr) {
                    *str = 0;
                    SetCurrentDirectory(path);
                }

                W3D::Init(pane->m_hWnd);
                W3D::Enable_Static_Sort_Lists(true);
                DeviceWidth = theApp.GetProfileInt("Config", "DeviceWidth", 640);
                DeviceHeight = theApp.GetProfileInt("Config", "DeviceHeight", 480);
                GetDevice(false);
                W3D::Enable_Munge_Sort_On_Load(AfxGetApp()->GetProfileInt("Config", "MungeSortOnLoad", 0) == 1);
                W3D::Enable_Sorting(AfxGetApp()->GetProfileInt("Config", "EnableSorting", 1) == 1);
            }
        }
    }

    return ret;
}

void CMainFrame::RestoreWindowPos()
{
    int left = theApp.GetProfileInt("Window", "Left", -1);
    int right = theApp.GetProfileInt("Window", "Right", -1);
    int top = theApp.GetProfileInt("Window", "Top", -1);
    int bottom = theApp.GetProfileInt("Window", "Bottom", -1);
    bool maximized = theApp.GetProfileInt("Window", "Maximized", -1) == 1;

    if (left != -1 && right != -1 && top != -1 && bottom != -1) {
        if (maximized) {
            ShowWindow(SW_SHOWMAXIMIZED);
        } else {
            SetWindowPos(nullptr, left, top, right - left, bottom - top, SWP_NOZORDER);
        }
    }
}

void CMainFrame::DoProperties()
{
    // do later
}

void CMainFrame::UpdateMenus(int type)
{
    if (m_currentType != type) {
        if (m_currentType >= 2 && (m_currentType <= 5 || m_currentType == 8)) {
            CMenu *menu = GetMenu();

            if (menu != nullptr) {
                menu->RemoveMenu(6, MF_BYPOSITION | MF_DELETE);
                DrawMenuBar();
            }

            if (m_currentType == 5 || m_currentType == 8) {
                m_animationToolbarVisible = m_animationToolbar.IsWindowVisible();
                ShowControlBar(&m_animationToolbar, FALSE, FALSE);
            }
        }

        switch (type) {
            case 2: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_HEIRARCHYMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = "&Hierarchy";
                    menu->InsertMenuItem(6, &info, TRUE);
                    DrawMenuBar();
                }
            } break;
            case 3: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_AGGREGATEMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = "&Aggregate";
                    menu->InsertMenuItem(6, &info, TRUE);
                    DrawMenuBar();
                }
            } break;
            case 4: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_LODMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = "&LOD";
                    menu->InsertMenuItem(6, &info, TRUE);
                    DrawMenuBar();
                }
            } break;
            case 5:
            case 8: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ANIMATIONMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = "&Animation";
                    menu->InsertMenuItem(6, &info, TRUE);
                    DrawMenuBar();
                }

                if (m_animationToolbarVisible) {
                    if (m_animationToolbar.IsWindowVisible()) {
                        ShowControlBar(&m_animationToolbar, FALSE, FALSE);
                        m_animationToolbarVisible = FALSE;
                    } else {
                        ShowControlBar(&m_animationToolbar, TRUE, FALSE);
                        m_animationToolbarVisible = TRUE;
                    }
                }
            } break;
        }

        m_currentType = type;
    }
}

void CMainFrame::UpdateStatusBar(unsigned int time)
{
    static int Time = 0;
    Time += time;
    static int Frames = 0;
    Frames++;
    static int Ticks = 0;

    if (GetTickCount() - Ticks >= 1000) {
        CString str;
        str.Format("Clocks: %.2f", (float)Time / (float)Frames);
        m_statusBar.SetPaneText(5, str);
        Frames = 0;
        Time = 0;
        Ticks = GetTickCount();
    }

    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        RECT r;
        view->GetWindowRect(&r);
        CString str;
        str.Format(" %d x %d ", r.right - r.left, r.bottom - r.top);
        m_statusBar.SetPaneText(6, str);
    }
}

void CMainFrame::UpdatePolyCount(int polys)
{
    CString str;
    str.Format("Polys %d", polys);
    m_statusBar.SetPaneText(1, str);
}

void CMainFrame::UpdateParticleCount(int particles)
{
    CString str;
    str.Format("Particles %d", particles);
    m_statusBar.SetPaneText(2, str);
}

void CMainFrame::UpdateFrameCount(int frame, int framecount, float fps)
{
    CString str;
    str.Format("Frame %d/%d at %.2f fps", frame, framecount, fps);
    m_statusBar.SetPaneText(4, str);
}

void CMainFrame::UpdateCameraDistance(float distance)
{
    CString str;
    str.Format("Camera %.3f", distance);
    m_statusBar.SetPaneText(3, str);
}

void CMainFrame::GetDevice(bool doDeviceDlg)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        CDeviceSelectionDialog dlg(!doDeviceDlg, this);

        if (dlg.DoModal() != IDOK) {
            return;
        }

        Device = dlg.m_device;
        BPP = dlg.m_bpp;

        if (!view->Create()) {
            return;
        }
    }
}

void CMainFrame::UpdateEmitterMenu()
{
    // do later
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    static UINT indicators[] = { 0, 61210, 61218, 61211, 61212, 61219, 61222 };

    theApp.m_pMainWnd = this;

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1 || !m_toolBar.Create(this) || !m_toolBar.LoadToolBar(IDR_MAINFRAME)
        || !m_statusBar.Create(this) || !m_statusBar.SetIndicators(indicators, _countof(indicators))) {
        return -1;
    }

    m_objectToolbar.Create("Object controls", this, ID_OBJECT_TOOLBAR);
    m_objectToolbar.AddItem(IDB_XDIR, IDB_XDIRSEL, ID_CAMERA_ROTATEYONLY, TRUE);
    m_objectToolbar.AddItem(IDB_YDIR, IDB_YDIRSEL, ID_CAMERA_ROTATEXONLY, TRUE);
    m_objectToolbar.AddItem(IDB_ZDIR, IDB_ZDIRSEL, ID_CAMERA_ROTATEZONLY, TRUE);
    m_objectToolbar.AddItem(IDB_ROTATEZ, IDB_ROTATEZSEL, ID_OBJECT_ROTATEZ, TRUE);

    m_animationToolbar.Create("Animation controls", this, ID_ANIMATION_TOOLBAR);
    m_animationToolbar.AddItem(IDB_PLAY, IDB_PLAYSEL, ID_ANIMATION_PLAY, TRUE);
    m_animationToolbar.AddItem(IDB_STOP, IDB_STOPSEL, ID_ANIMATION_STOP, FALSE);
    m_animationToolbar.AddItem(IDB_PAUSE, IDB_PAUSESEL, ID_ANIMATION_PAUSE, TRUE);
    m_animationToolbar.AddItem(IDB_REVERSE, IDB_REVERSESEL, ID_ANIMATION_STEPBACK, FALSE);
    m_animationToolbar.AddItem(IDB_FFWD, IDB_FFWDSEL, ID_ANIMATION_STEPFORWARD, FALSE);
    m_animationToolbar.ShowWindow(SW_HIDE);

    m_toolBar.SetBarStyle(m_toolBar.m_dwStyle | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_toolBar.ModifyStyle(0, CBRS_BORDER_BOTTOM);
    m_toolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_toolBar, nullptr);

    RECT r;
    GetWindowRect(&r);
    FloatControlBar(&m_objectToolbar, CPoint(r.left + 10, r.bottom - 100), CBRS_ALIGN_LEFT);
    FloatControlBar(&m_animationToolbar, CPoint(r.left + 210, r.bottom - 100), CBRS_ALIGN_LEFT);
    ShowControlBar(&m_animationToolbar, FALSE, FALSE);

    m_statusBar.SetPaneText(1, "");
    m_statusBar.SetPaneText(2, "");
    m_statusBar.SetPaneText(3, "");
    m_statusBar.SetPaneText(4, "");
    m_statusBar.SetPaneText(5, "");
    m_statusBar.SetPaneText(6, "");

    W3DAssetManager::Get_Instance()->Set_W3D_Load_On_Demand(true);
    W3DAssetManager::Get_Instance()->Set_Activate_Fog_On_Load(true);
    GetWindowRect(&m_windowRect);
    HMENU menu = ::GetMenu(m_hWnd);
    HMENU SubMenu = GetSubMenu(menu, 3);
    m_subMenu = SubMenu;
    MENUITEMINFO info;
    memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STRING;
    char buffer[200];
    info.dwTypeData = buffer;
    info.cch = 100;
    GetMenuItemInfo(SubMenu, 4, TRUE, &info);
    m_subMenu = GetSubMenu(SubMenu, 4);
    RestoreWindowPos();
    m_initialized = TRUE;
    return (Device != -1) - 1;
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->m_isActive = bActive;

        if (!bActive) {
            SetProp(m_hWnd, "Inactive", (HANDLE)1);
            CWnd::Default();
            return;
        }

        RemoveProp(m_hWnd, "Inactive");
        view->m_time = GetTickCount();
    }

    CWnd::Default();
    CWnd::Default();
}

void CMainFrame::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
    CWnd::Default();
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS FAR *lpwndpos)
{
    CWnd::Default();
}

void CMainFrame::OnDestroy()
{
    WINDOWPLACEMENT placement;
    memset(&placement, 0, sizeof(placement));
    placement.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&placement);
    RECT r;
    GetWindowRect(&r);
    theApp.WriteProfileInt("Window", "Left", r.left);
    theApp.WriteProfileInt("Window", "Right", r.right);
    theApp.WriteProfileInt("Window", "Top", r.top);
    theApp.WriteProfileInt("Window", "Bottom", r.bottom);
    theApp.WriteProfileInt("Window", "Maximized", placement.showCmd == SW_SHOWMAXIMIZED);
    theApp.WriteProfileInt("Config", "AnimateCamera", ((CW3DViewDoc *)GetActiveDocument())->m_animateCamera);
    theApp.WriteProfileInt("Config", "ResetCamera", ((CW3DViewDoc *)GetActiveDocument())->m_resetCamera);
    CFrameWnd::OnDestroy();
}

void CMainFrame::OnProperties()
{
    // do later
}

void CMainFrame::OnGenerateLOD()
{
    // do later
}

void CMainFrame::OnOpen()
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();

    if (doc != nullptr) {
        CFileDialog dlg(TRUE,
            ".w3d",
            nullptr,
            OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
            "Westwood 3D Files (*.w3d)|*.w3d||",
            this);
        char file[2600] = {};
        dlg.GetOFN().lpstrFile = file;
        dlg.GetOFN().nMaxFile = 2600;

        if (doc->m_filePath.GetLength() > 0) {
            dlg.GetOFN().lpstrInitialDir = doc->m_filePath;
        } else {
            dlg.GetOFN().lpstrInitialDir = 0;
        }

        if (dlg.DoModal() == IDOK) {
            SetCursor(LoadCursor(nullptr, IDC_WAIT));

            POSITION pos = dlg.GetStartPosition();

            while (pos != nullptr) {
                CString path = dlg.GetNextPathName(pos);
                doc->LoadFile(path);
                AfxGetApp()->AddToRecentFileList(path);
            }

            CDataTreeView *view = doc->GetDataTreeView();

            if (view != nullptr) {
                view->AddRenderObjects();
            }

            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
    }
}

void CMainFrame::OnAnimSettings()
{
    // do later
}

void CMainFrame::OnStopAnim()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->UpdateAnimation(1);
        m_animationToolbar.SetStatus(ID_ANIMATION_PLAY, FALSE, TRUE);
        m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, FALSE, TRUE);
    }
}

void CMainFrame::OnPlayAnim()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->UpdateAnimation(0);
        m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, FALSE, TRUE);
        m_animationToolbar.SetStatus(ID_ANIMATION_PLAY, TRUE, TRUE);
    }
}

void CMainFrame::OnPauseAnim()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        if (view->m_animationPlaying != 0) {
            if (view->m_animationPlaying == 2) {
                view->UpdateAnimation(0);
            }

            m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, FALSE, TRUE);
        } else {
            view->UpdateAnimation(2);
            m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, TRUE, TRUE);
        }
    }
}

void CMainFrame::OnBack()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->SetCameraDirection(0);
    }
}

void CMainFrame::OnBottom()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->SetCameraDirection(2);
    }
}

void CMainFrame::OnFront()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->SetCameraDirection(-1);
    }
}

void CMainFrame::OnLeft()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->SetCameraDirection(3);
    }
}

void CMainFrame::OnResetCamera()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();

        if (doc != nullptr) {
            if (doc->m_model != nullptr) {
                if (doc->m_model->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
                    view->ResetParticleEmitterCamera((ParticleEmitterClass *)doc->m_model);
                } else {
                    view->ResetCamera(doc->m_model);
                }
            }
        }
    }
}

void CMainFrame::OnRight()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->SetCameraDirection(4);
    }
}

void CMainFrame::OnTop()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->SetCameraDirection(1);
    }
}

void CMainFrame::OnRotateZ()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ 4) & ~32;
        view->SetRotationFlags(flags);

        if ((flags & 4) != 0) {
            m_objectToolbar.SetStatus(ID_OBJECT_ROTATEZ, TRUE, TRUE);
        } else {
            m_objectToolbar.SetStatus(ID_OBJECT_ROTATEZ, FALSE, TRUE);
        }
    }
}

void CMainFrame::OnRotateY()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ 2) & ~16;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnRotateX()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ 1) & ~8;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnAmbient()
{
    // do later
}

void CMainFrame::OnSceneLight()
{
    // do later
}

void CMainFrame::OnBackgroundColor()
{
    // do later
}

void CMainFrame::OnBackgroundBitmap()
{
    // do later
}

void CMainFrame::OnExportLOD()
{
    // do later
}

void CMainFrame::OnBackgroundObject()
{
    // do later
}

void CMainFrame::OnAnimationToolbar()
{
    if (m_animationToolbar.IsVisible()) {
        ShowControlBar(&m_animationToolbar, FALSE, FALSE);
        m_animationToolbarVisible = FALSE;
    } else {
        ShowControlBar(&m_animationToolbar, TRUE, FALSE);
        m_animationToolbarVisible = TRUE;
    }
}

void CMainFrame::OnObjectToolbar()
{
    if (m_objectToolbar.IsVisible()) {
        ShowControlBar(&m_objectToolbar, FALSE, FALSE);
    } else {
        ShowControlBar(&m_objectToolbar, TRUE, FALSE);
    }
}

void CMainFrame::OnStepForward()
{
    CW3DViewDoc *document = GetCurrentDocument();
    document->OnStep(1);
}

void CMainFrame::OnStepBack()
{
    CW3DViewDoc *document = GetCurrentDocument();
    document->OnStep(-1);
}

void CMainFrame::OnReset()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->ResetRenderObj();
    }
}

void CMainFrame::OnCameraRotateX()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        if (view->m_cameraRotateConstraints == 1) {
            view->SetCameraRotateConstraints(0);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, FALSE, TRUE);
        } else {
            view->SetCameraRotateConstraints(1);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, TRUE, TRUE);
        }

        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, FALSE, TRUE);
        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, FALSE, TRUE);
    }
}

void CMainFrame::OnCameraRotateY()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        if (view->m_cameraRotateConstraints == 2) {
            view->SetCameraRotateConstraints(0);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, FALSE, TRUE);
        } else {
            view->SetCameraRotateConstraints(2);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, TRUE, TRUE);
        }

        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, FALSE, TRUE);
        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, FALSE, TRUE);
    }
}

void CMainFrame::OnCameraRotateZ()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        if (view->m_cameraRotateConstraints == 4) {
            view->SetCameraRotateConstraints(0);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, FALSE, TRUE);
        } else {
            view->SetCameraRotateConstraints(4);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, TRUE, TRUE);
        }

        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, FALSE, TRUE);
        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, FALSE, TRUE);
    }
}

void CMainFrame::OnCreateEmitter()
{
    // do later
}

void CMainFrame::OnEditEmitter()
{
    // do later
}

void CMainFrame::OnExportEmitter()
{
    // do later
}

void CMainFrame::OnAutoAssignBone()
{
    // do later
}

void CMainFrame::OnBoneManagement()
{
    // do later
}

void CMainFrame::OnExportAggregate()
{
    // do later
}

void CMainFrame::OnAnimateCamera()
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();
    doc->AnimateCamera(!doc->m_animateCamera);
}

void CMainFrame::OnResetOnDisplay()
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();
    doc->m_resetCamera = !doc->m_resetCamera;
}

void CMainFrame::OnRotateYBackwards()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        int flags = view->m_objectRotationFlags & ~2 ^ 16;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnRotateZBackwards()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        int flags = view->m_objectRotationFlags & ~4 ^ 32;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnLightingRotateY()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->m_lightRotationFlags = (view->m_lightRotationFlags ^ 2) & ~16;
    }
}

void CMainFrame::OnLightingRotateYBackwards()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->m_lightRotationFlags = view->m_lightRotationFlags & ~2 ^ 16;
    }
}

void CMainFrame::OnLightingRotateZ()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->m_lightRotationFlags = (view->m_lightRotationFlags ^ 4) & ~32;
    }
}

void CMainFrame::OnLightingRotateZBackwards()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->m_lightRotationFlags = view->m_lightRotationFlags & ~4 ^ 32;
    }
}

void CMainFrame::OnDecSceneLight()
{
    LightClass *light = GetCurrentDocument()->m_light;

    if (light != nullptr) {
        Vector3 diffuse;
        light->Get_Diffuse(&diffuse);
        Vector3 specular;
        light->Get_Specular(&specular);

        diffuse.X -= 0.05f;
        diffuse.Y -= 0.05f;
        diffuse.Z -= 0.05f;
        diffuse.X = std::clamp(diffuse.X, 0.0f, 1.0f);
        diffuse.Y = std::clamp(diffuse.Y, 0.0f, 1.0f);
        diffuse.Z = std::clamp(diffuse.Z, 0.0f, 1.0f);

        specular.X -= 0.05f;
        specular.Y -= 0.05f;
        specular.Z -= 0.05f;
        specular.X = std::clamp(specular.X, 0.0f, 1.0f);
        specular.Y = std::clamp(specular.Y, 0.0f, 1.0f);
        specular.Z = std::clamp(specular.Z, 0.0f, 1.0f);

        light->Set_Diffuse(diffuse);
        light->Set_Specular(specular);
    }
}

void CMainFrame::OnIncSceneLight()
{
    LightClass *light = GetCurrentDocument()->m_light;

    if (light != nullptr) {
        Vector3 diffuse;
        light->Get_Diffuse(&diffuse);
        Vector3 specular;
        light->Get_Specular(&specular);

        diffuse.X += 0.05f;
        diffuse.Y += 0.05f;
        diffuse.Z += 0.05f;
        diffuse.X = std::clamp(diffuse.X, 0.0f, 1.0f);
        diffuse.Y = std::clamp(diffuse.Y, 0.0f, 1.0f);
        diffuse.Z = std::clamp(diffuse.Z, 0.0f, 1.0f);

        specular.X += 0.05f;
        specular.Y += 0.05f;
        specular.Z += 0.05f;
        specular.X = std::clamp(specular.X, 0.0f, 1.0f);
        specular.Y = std::clamp(specular.Y, 0.0f, 1.0f);
        specular.Z = std::clamp(specular.Z, 0.0f, 1.0f);

        light->Set_Diffuse(diffuse);
        light->Set_Specular(specular);
    }
}

void CMainFrame::OnDecAmbient()
{
    ViewerSceneClass *scene = GetCurrentDocument()->m_scene;

    if (scene != nullptr) {
        Vector3 ambient = scene->Get_Ambient_Light();

        ambient.X -= 0.05f;
        ambient.Y -= 0.05f;
        ambient.Z -= 0.05f;
        ambient.X = std::clamp(ambient.X, 0.0f, 1.0f);
        ambient.Y = std::clamp(ambient.Y, 0.0f, 1.0f);
        ambient.Z = std::clamp(ambient.Z, 0.0f, 1.0f);

        scene->Set_Ambient_Light(ambient);
    }
}

void CMainFrame::OnIncAmbient()
{
    ViewerSceneClass *scene = GetCurrentDocument()->m_scene;

    if (scene != nullptr) {
        Vector3 ambient = scene->Get_Ambient_Light();

        ambient.X += 0.05f;
        ambient.Y += 0.05f;
        ambient.Z += 0.05f;
        ambient.X = std::clamp(ambient.X, 0.0f, 1.0f);
        ambient.Y = std::clamp(ambient.Y, 0.0f, 1.0f);
        ambient.Z = std::clamp(ambient.Z, 0.0f, 1.0f);

        scene->Set_Ambient_Light(ambient);
    }
}

void CMainFrame::OnMakeAggregate()
{
    // do later
}

void CMainFrame::OnRenameAggregate()
{
    // do later
}

void CMainFrame::OnRecordSceneCamera()
{
    // do later
}

void CMainFrame::OnIncludeNull()
{
    // do later
}

void CMainFrame::OnPrevLod()
{
    // do later
}

void CMainFrame::OnNextLod()
{
    // do later
}

void CMainFrame::OnAutoSwitching()
{
    // do later
}

void CMainFrame::OnMovie()
{
    // do later
}

void CMainFrame::OnScreenshot()
{
    // do later
}

void CMainFrame::OnNext()
{
    CDataTreeView *view = (CDataTreeView *)m_splitter.GetPane(0, 0);

    if (view) {
        view->SelectNext();
    }
}

void CMainFrame::OnPrev()
{
    CDataTreeView *view = (CDataTreeView *)m_splitter.GetPane(0, 0);

    if (view) {
        view->SelectPrev();
    }
}

void CMainFrame::OnAnimAdvanced()
{
    // do later
}

void CMainFrame::OnCameraSettings()
{
    // do later
}

void CMainFrame::OnCopyScreenSize()
{
    // do later
}

void CMainFrame::OnMissingTextures()
{
    // do later
}

void CMainFrame::OnCopyDeps()
{
    // do later
}

void CMainFrame::OnExposePrecalc()
{
    // do later
}

void CMainFrame::OnTexturePath()
{
    // do later
}

void CMainFrame::OnCreateSphere()
{
    // do later
}

void CMainFrame::OnCreateRing()
{
    // do later
}

void CMainFrame::OnEditPrimitive()
{
    // do later
}

void CMainFrame::OnExportPrimitive()
{
    // do later
}

void CMainFrame::OnKillSceneLight()
{
    LightClass *light = GetCurrentDocument()->m_light;

    if (light != nullptr) {
        light->Set_Diffuse(Vector3(0.0f, 0.0f, 0.0f));
        light->Set_Specular(Vector3(0.0f, 0.0f, 0.0f));
    }
}

void CMainFrame::OnMultiPassLighting()
{
    // do later
}

void CMainFrame::OnMultiTextureLighting()
{
    // do later
}

void CMainFrame::OnVertexLighting()
{
    // do later
}

void CMainFrame::OnAddObject()
{
    // do later
}

void CMainFrame::OnImportFacial()
{
    // do later
}

void CMainFrame::OnRestrictAnims()
{
    CDataTreeView *view = (CDataTreeView *)m_splitter.GetPane(0, 0);

    if (view) {
        view->RestrictAnims(!view->m_restrictAnims);
    }
}

void CMainFrame::OnBindSubobject()
{
    // do later
}

void CMainFrame::OnSetDistance()
{
    // do later
}

void CMainFrame::OnAlternateMaterials()
{
    CW3DViewDoc *document = GetCurrentDocument();
    document->ToggleAlternateMaterials(nullptr);
}

void CMainFrame::OnCreateSoundObject()
{
    // do later
}

void CMainFrame::OnEditSoundObject()
{
    // do later
}

void CMainFrame::OnExportSoundObject()
{
    // do later
}

void CMainFrame::OnWireframeMode()
{
    CW3DViewDoc *document = GetCurrentDocument();
    document->m_scene->Set_Polygon_Mode((SceneClass::PolyRenderType)(2 - (document->m_scene->Get_Polygon_Mode() != 1)));
}

void CMainFrame::OnFog()
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();

    if (doc != nullptr) {
        doc->EnableFog(!doc->m_fogEnabled);
    }
}

void CMainFrame::OnScaleEmitter()
{
    // do later
}

void CMainFrame::OnPolygonSorting()
{
    bool enable = W3D::Is_Sorting_Enabled() == false;
    W3D::Invalidate_Mesh_Cache();
    W3D::Enable_Sorting(enable);

    AfxGetApp()->WriteProfileInt("Config", "EnableSorting", enable);
}

void CMainFrame::OnPlusXCamera()
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        view->m_plusXCamera = !view->m_plusXCamera;
    }
}

void CMainFrame::OnMungeSort()
{
    W3D::Enable_Munge_Sort_On_Load(W3D::Is_Munge_Sort_On_Load_Enabled() == false);
    AfxGetApp()->WriteProfileInt("Config", "MungeSortOnLoad", W3D::Is_Munge_Sort_On_Load_Enabled());
}

void CMainFrame::OnEnableGamma()
{
    // do later
}

void CMainFrame::OnSetGamma()
{
    // do later
}

void CMainFrame::OnUpdateProperties(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateAnimationToolbar(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(FALSE);
    pCmdUI->SetCheck(FALSE);
}

void CMainFrame::OnUpdateObjectToolbar(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(m_objectToolbar.IsVisible());
}

void CMainFrame::OnUpdateCameraRotateX(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_cameraRotateConstraints == 1);
    }
}

void CMainFrame::OnUpdateCameraRotateY(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_cameraRotateConstraints == 2);
    }
}

void CMainFrame::OnUpdateCameraRotateZ(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_cameraRotateConstraints == 4);
    }
}

void CMainFrame::OnUpdateRotateX(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_objectRotationFlags & 1);
    }
}

void CMainFrame::OnUpdateRotateY(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_objectRotationFlags & 2);
    }
}

void CMainFrame::OnUpdateRotateZ(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_objectRotationFlags & 4);
    }
}

void CMainFrame::OnUpdateEditEmitter(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateExportEmitter(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateAnimateCamera(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();
    pCmdUI->SetCheck(doc->m_animateCamera);
}

void CMainFrame::OnUpdateExportLod(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateExportAggregate(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateResetOnDisplay(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();
    pCmdUI->SetCheck(doc->m_resetCamera);
}

void CMainFrame::OnUpdateIncludeNull(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdatePrevLod(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateNextLod(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateAutoSwitching(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateMovie(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateAnimAdvanced(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateCopyDeps(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateExposePrecalc(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateEditPrimitive(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateExportPrimitive(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateMultiPassLighting(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateMultiTextureLighting(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateVertexLighting(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateAddObject(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateImportFacial(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateRestrictAnims(CCmdUI *pCmdUI)
{
    bool restrict = true;
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();
    CDataTreeView *view = doc->GetDataTreeView();

    if (view != nullptr) {
        restrict = view->m_restrictAnims;
    }

    pCmdUI->SetCheck(restrict);
}

void CMainFrame::OnUpdateBindSubobject(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateEditSoundObject(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateExportSoundObject(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdateWireframeMode(CCmdUI *pCmdUI)
{
    CW3DViewDoc *document = GetCurrentDocument();
    pCmdUI->SetCheck(document->m_scene->Get_Polygon_Mode() == SceneClass::LINE);
}

void CMainFrame::OnUpdateFog(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();

    if (doc != nullptr) {
        pCmdUI->SetCheck(doc->m_fogEnabled);
    }
}

void CMainFrame::OnUpdateScaleEmitter(CCmdUI *pCmdUI)
{
    // do later
}

void CMainFrame::OnUpdatePolygonSorting(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(W3D::Is_Sorting_Enabled() != 0);
}

void CMainFrame::OnUpdatePlusXCamera(CCmdUI *pCmdUI)
{
    CGraphicView *view = (CGraphicView *)m_splitter.GetPane(0, 1);

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_plusXCamera);
    }
}

void CMainFrame::OnUpdateMungeSort(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(W3D::Is_Munge_Sort_On_Load_Enabled() != 0);
}

void CMainFrame::OnUpdateEnableGamma(CCmdUI *pCmdUI)
{
    // do later
}
