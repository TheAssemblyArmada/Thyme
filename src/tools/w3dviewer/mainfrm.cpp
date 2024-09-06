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
#include "addtolineupdialog.h"
#include "ambientlightdialog.h"
#include "animationproppage.h"
#include "animationspeed.h"
#include "assetmgr.h"
#include "assetpropertysheet.h"
#include "backgroundbmpdialog.h"
#include "backgroundcolordialog.h"
#include "backgroundobjectdialog.h"
#include "cameradistancedialog.h"
#include "camerasettingsdialog.h"
#include "colorpickerdialogclass.h"
#include "datatreeview.h"
#include "deviceselectiondialog.h"
#include "emitterinstancelist.h"
#include "emitterpropertysheet.h"
#include "graphicview.h"
#include "hierarchyproppage.h"
#include "hlod.h"
#include "light.h"
#include "meshproppage.h"
#include "part_emt.h"
#include "rddesc.h"
#include "resource.h"
#include "scaledialog.h"
#include "scenelightdialog.h"
#include "texturepathdialog.h"
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
    ON_COMMAND(ID_HIERARCHY_GENERATELOD, OnGenerateLOD)
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
    ON_COMMAND(ID_CAMERA_ANIMATECAMERA, OnAnimateCamera)
    ON_UPDATE_COMMAND_UI(ID_CAMERA_ANIMATECAMERA, OnUpdateAnimateCamera)
    ON_UPDATE_COMMAND_UI(ID_EXPORT_LOD, OnUpdateExportLod)
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
    ON_COMMAND(ID_LOD_RECORDSCREENAREA, OnRecordSceneCamera)
    ON_COMMAND(ID_LOD_INCLUDENULLOBJECT, OnIncludeNull)
    ON_UPDATE_COMMAND_UI(ID_LOD_INCLUDENULLOBJECT, OnUpdateIncludeNull)
    ON_COMMAND(ID_LOD_PREVLEVEL, OnPrevLod)
    ON_UPDATE_COMMAND_UI(ID_LOD_PREVLEVEL, OnUpdatePrevLod)
    ON_COMMAND(ID_LOD_NEXTLEVEL, OnNextLod)
    ON_UPDATE_COMMAND_UI(ID_LOD_NEXTLEVEL, OnUpdateNextLod)
    ON_COMMAND(ID_LOD_AUTOSWITCHING, OnAutoSwitching)
    ON_UPDATE_COMMAND_UI(ID_LOD_AUTOSWITCHING, OnUpdateAutoSwitching)
    ON_COMMAND(ID_VIEW_NEXT, OnNext)
    ON_COMMAND(ID_VIEW_PREV, OnPrev)
    ON_COMMAND(ID_CAMERA_SETTINGS, OnCameraSettings)
    ON_COMMAND(ID_CAMERA_COPYSCREENSIZETOCLIPBOARD, OnCopyScreenSize)
    ON_COMMAND(ID_FILE_TEXTUREPATH, OnTexturePath)
    ON_COMMAND(ID_LIGHTING_KILLSCENELIGHT, OnKillSceneLight)
    ON_COMMAND(ID_LIGHTING_MULTIPASSLIGHTING, OnMultiPassLighting)
    ON_UPDATE_COMMAND_UI(ID_LIGHTING_MULTIPASSLIGHTING, OnUpdateMultiPassLighting)
    ON_COMMAND(ID_LIGHTING_MULTITEXTURELIGHTING, OnMultiTextureLighting)
    ON_UPDATE_COMMAND_UI(ID_LIGHTING_MULTITEXTURELIGHTING, OnUpdateMultiTextureLighting)
    ON_COMMAND(ID_LIGHTING_VERTEXLIGHTING, OnVertexLighting)
    ON_UPDATE_COMMAND_UI(ID_LIGHTING_VERTEXLIGHTING, OnUpdateVertexLighting)
    ON_COMMAND(ID_ADDOBJECT, OnAddObject)
    ON_UPDATE_COMMAND_UI(ID_ADDOBJECT, OnUpdateAddObject)
    ON_COMMAND(ID_OBJECT_RESTRICTANIMS, OnRestrictAnims)
    ON_UPDATE_COMMAND_UI(ID_OBJECT_RESTRICTANIMS, OnUpdateRestrictAnims)
    ON_COMMAND(ID_CAMERA_SETDISTANCE, OnSetDistance)
    ON_COMMAND(ID_OBJECT_TOGGLEALTERNATEMATERIALS, OnAlternateMaterials)
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
END_MESSAGE_MAP()
// clang-format on

int DeviceWidth = 640;
int DeviceHeight = 480;
int Device = -1;
int BPP = -1;

CMainFrame::CMainFrame() :
    m_currentType(ASSET_TYPE_NONE), m_animationToolbarVisible(true), m_windowRect{}, m_subMenu(nullptr), m_initialized(false)
{
}

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

    CCmdUI *pCmdUI = static_cast<CCmdUI *>(pExtra);

    if (pCmdUI->m_nID < 1000 || pCmdUI->m_nID > 1100) {
        return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    pCmdUI->Enable();
    return TRUE;
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (lParam == 0 && LOWORD(wParam) >= 1000 && LOWORD(wParam < 1100)) {
        char name[200];
        MENUITEMINFO info;
        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        info.fMask = MIIM_DATA | MIIM_TYPE;
        info.dwTypeData = name;
        info.cch = sizeof(name);

        if (GetMenuItemInfo(m_subMenu, LOWORD(wParam), FALSE, &info)) {
            EmitterInstanceList *list = new EmitterInstanceList();
            GetCurrentDocument()->AddEmittersToList(list, name, nullptr);
            EmitterPropertySheetClass sheet(list, IDS_EMITTERPROPERTIES, this);
            sheet.DoModal();
        }
    }

    return CFrameWnd::OnCommand(wParam, lParam);
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CLOSE) {
        CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

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
                    *str = '\0';
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
    CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));

    if (view != nullptr) {
        switch (view->GetSelectedItemType()) {
            case ASSET_TYPE_MESH: {
                CString name(view->GetSelectedItemName());
                CMeshPropPage page(name);
                CAssetPropertySheet sheet(IDS_MESHPROPERTIES, &page, this);
                sheet.DoModal();
                break;
            }
            case ASSET_TYPE_HIERARCHY: {
                CString name(view->GetSelectedItemName());
                CHierarchyPropPage page(name);
                CAssetPropertySheet sheet(IDS_HIERARCHYPROPERTIES, &page, this);
                sheet.DoModal();
                break;
            }
            case ASSET_TYPE_ANIMATION: {
                CAnimationPropPage page;
                CAssetPropertySheet sheet(IDS_ANIMATIONPROPERTIES, &page, this);
                sheet.DoModal();
                break;
            }
            case ASSET_TYPE_PARTICLEEMITTER: {
                CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

                if (doc != nullptr) {
                    EmitterInstanceList *list = new EmitterInstanceList();
                    list->Add_Emitter(static_cast<ParticleEmitterClass *>(doc->m_model));
                    EmitterPropertySheetClass sheet(list, IDS_EMITTERPROPERTIES, this);
                    sheet.DoModal();
                }

                break;
            }
        }
    }
}

void CMainFrame::UpdateMenus(AssetType type)
{
    if (m_currentType != type) {
        if (m_currentType >= ASSET_TYPE_HIERARCHY
            && (m_currentType <= ASSET_TYPE_ANIMATION || m_currentType == ASSET_TYPE_MORPH)) {
            CMenu *menu = GetMenu();

            if (menu != nullptr) {
                menu->RemoveMenu(4, MF_BYPOSITION | MF_DELETE);
                DrawMenuBar();
            }

            if (m_currentType == ASSET_TYPE_ANIMATION || m_currentType == ASSET_TYPE_MORPH) {
                m_animationToolbarVisible = m_animationToolbar.IsWindowVisible();
                ShowControlBar(&m_animationToolbar, FALSE, FALSE);
            }
        }

        switch (type) {
            case ASSET_TYPE_HIERARCHY: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_HIERARCHYMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = const_cast<LPSTR>("&Hierarchy");
                    menu->InsertMenuItem(4, &info, TRUE);
                    DrawMenuBar();
                }
            } break;
            case ASSET_TYPE_HLOD: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_LODMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = const_cast<LPSTR>("&LOD");
                    menu->InsertMenuItem(4, &info, TRUE);
                    DrawMenuBar();
                }
            } break;
            case ASSET_TYPE_ANIMATION:
            case ASSET_TYPE_MORPH: {
                CMenu *menu = GetMenu();

                if (menu != nullptr) {
                    MENUITEMINFO info;
                    memset(&info, 0, sizeof(info));
                    info.cbSize = sizeof(info);
                    info.hSubMenu = GetSubMenu(LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ANIMATIONMENU)), 0);
                    info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
                    info.fType = MF_STRING;
                    info.dwTypeData = const_cast<LPSTR>("&Animation");
                    menu->InsertMenuItem(4, &info, TRUE);
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
    static unsigned int Time = 0;
    Time += time;
    static int Frames = 0;
    Frames++;
    static unsigned int Ticks = 0;

    if (GetTickCount() - Ticks >= 1000) {
        CString str;
        str.Format("Clocks: %.2f", (float)Time / (float)Frames);
        m_statusBar.SetPaneText(5, str);
        Frames = 0;
        Time = 0;
        Ticks = GetTickCount();
    }

    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

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
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

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

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    static UINT indicators[] = { 0, IDS_POLYS, IDS_PARTICLES, IDS_CAMERADIST, IDS_FRAMES, IDS_FRAMETIME, IDS_WWW };

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
    m_toolBar.ModifyStyle(0, TBSTYLE_FLAT);
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
    m_subMenu = GetSubMenu(SubMenu, 3);
    RestoreWindowPos();
    m_initialized = TRUE;
    return (Device != -1) - 1;
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->m_isActive = bActive;

        if (!bActive) {
            SetProp(view->m_hWnd, "Inactive", (HANDLE)1);
            CFrameWnd::OnActivateApp(bActive, dwThreadID);
            return;
        }

        RemoveProp(view->m_hWnd, "Inactive");
        view->m_time = GetTickCount();
    }

    CFrameWnd::OnActivateApp(bActive, dwThreadID);
}

void CMainFrame::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
    CFrameWnd::OnWindowPosChanging(lpWndPos);
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS FAR *lpwndpos)
{
    CFrameWnd::OnWindowPosChanged(lpwndpos);
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
    theApp.WriteProfileInt("Config", "AnimateCamera", static_cast<CW3DViewDoc *>(GetActiveDocument())->m_animateCamera);
    theApp.WriteProfileInt("Config", "ResetCamera", static_cast<CW3DViewDoc *>(GetActiveDocument())->m_resetCamera);
    CFrameWnd::OnDestroy();
}

void CMainFrame::OnProperties()
{
    DoProperties();
}

void CMainFrame::OnGenerateLOD()
{
    CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));

    if (view == nullptr || view->GetSelectedItemName() == nullptr) {
        return;
    }

    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        const char *name = view->GetSelectedItemName();

        int len = strlen(name);
        char c = name[len - 2];
        char c2 = name[len - 1];
        bool format = false;
        bool generate = false;

        if ((c == 'L' || c == 'l') && c2 >= '0' && c2 <= '9') {
            format = false;
            generate = true;
        }

        else if ((c2 >= 'a' && c2 <= 'z') || (c2 >= 'A' && c2 <= 'Z')) {
            format = true;
            generate = true;
        }

        if (generate) {
            CString namecopy = name;
            CString nolod;

            if (format) {
                nolod = namecopy.Left(namecopy.GetLength() - 1);
            } else {
                nolod = namecopy.Left(namecopy.GetLength() - 2);
            }

            PrototypeClass *proto = doc->GenerateLOD(nolod, format);

            if (proto != nullptr) {
                W3DAssetManager::Get_Instance()->Add_Prototype(proto);
                view->AddItem(proto->Get_Name(), ASSET_TYPE_HLOD, true);
            }
        }
    }
}

void CMainFrame::OnOpen()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        CFileDialog dlg(TRUE,
            ".w3d",
            nullptr,
            OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
            "Westwood 3D Files (*.w3d)|*.w3d||",
            this);
        char file[2600] = {};
        dlg.GetOFN().lpstrFile = file;
        dlg.GetOFN().nMaxFile = sizeof(file);

        if (doc->m_filePath.GetLength() > 0) {
            dlg.GetOFN().lpstrInitialDir = doc->m_filePath;
        } else {
            dlg.GetOFN().lpstrInitialDir = nullptr;
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
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        float speed = view->m_animationSpeed;
        CAnimationSpeed dlg(this);

        if (dlg.DoModal() != IDOK) {
            view->m_animationSpeed = speed;
        }
    }
}

void CMainFrame::OnStopAnim()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->UpdateAnimation(CGraphicView::ANIMATION_STOP);
        m_animationToolbar.SetStatus(ID_ANIMATION_PLAY, FALSE, TRUE);
        m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, FALSE, TRUE);
    }
}

void CMainFrame::OnPlayAnim()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->UpdateAnimation(CGraphicView::ANIMATION_PLAY);
        m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, FALSE, TRUE);
        m_animationToolbar.SetStatus(ID_ANIMATION_PLAY, TRUE, TRUE);
    }
}

void CMainFrame::OnPauseAnim()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        if (view->m_animationState != CGraphicView::ANIMATION_PLAY) {
            if (view->m_animationState == CGraphicView::ANIMATION_PAUSE) {
                view->UpdateAnimation(CGraphicView::ANIMATION_PLAY);
            }

            m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, FALSE, TRUE);
        } else {
            view->UpdateAnimation(CGraphicView::ANIMATION_PAUSE);
            m_animationToolbar.SetStatus(ID_ANIMATION_PAUSE, TRUE, TRUE);
        }
    }
}

void CMainFrame::OnBack()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->SetCameraDirection(CGraphicView::DIRECTION_BACK);
    }
}

void CMainFrame::OnBottom()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->SetCameraDirection(CGraphicView::DIRECTION_BOTTOM);
    }
}

void CMainFrame::OnFront()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->SetCameraDirection(CGraphicView::DIRECTION_FRONT);
    }
}

void CMainFrame::OnLeft()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->SetCameraDirection(CGraphicView::DIRECTION_LEFT);
    }
}

void CMainFrame::OnResetCamera()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

        if (doc != nullptr) {
            if (doc->m_model != nullptr) {
                if (doc->m_model->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
                    view->ResetParticleEmitterCamera(static_cast<ParticleEmitterClass *>(doc->m_model));
                } else {
                    view->ResetCamera(doc->m_model);
                }
            }
        }
    }
}

void CMainFrame::OnRight()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->SetCameraDirection(CGraphicView::DIRECTION_RIGHT);
    }
}

void CMainFrame::OnTop()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->SetCameraDirection(CGraphicView::DIRECTION_TOP);
    }
}

void CMainFrame::OnRotateZ()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ CGraphicView::ROTATE_PLUSZ) & ~CGraphicView::ROTATE_MINUSZ;
        view->SetRotationFlags(flags);

        if ((flags & CGraphicView::ROTATE_PLUSZ) != 0) {
            m_objectToolbar.SetStatus(ID_OBJECT_ROTATEZ, TRUE, TRUE);
        } else {
            m_objectToolbar.SetStatus(ID_OBJECT_ROTATEZ, FALSE, TRUE);
        }
    }
}

void CMainFrame::OnRotateY()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ CGraphicView::ROTATE_PLUSY) & ~CGraphicView::ROTATE_MINUSY;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnRotateX()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ CGraphicView::ROTATE_PLUSX) & ~CGraphicView::ROTATE_MINUSX;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnAmbient()
{
    CAmbientLightDialog dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnSceneLight()
{
    CSceneLightDialog dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnBackgroundColor()
{
    CBackgroundColorDialog dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnBackgroundBitmap()
{
    CBackgroundBMPDialog dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnExportLOD()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        doc->ExportLOD();
    }
}

void CMainFrame::OnBackgroundObject()
{
    CBackgroundObjectDialog dlg(this);
    dlg.DoModal();
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
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->ResetRenderObj();
    }
}

void CMainFrame::OnCameraRotateX()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        if (view->m_cameraRotateConstraints == CGraphicView::ROTATE_X_ONLY) {
            view->SetCameraRotateConstraints(CGraphicView::ROTATE_NONE);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, FALSE, TRUE);
        } else {
            view->SetCameraRotateConstraints(CGraphicView::ROTATE_X_ONLY);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, TRUE, TRUE);
        }

        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, FALSE, TRUE);
        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, FALSE, TRUE);
    }
}

void CMainFrame::OnCameraRotateY()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        if (view->m_cameraRotateConstraints == CGraphicView::ROTATE_Y_ONLY) {
            view->SetCameraRotateConstraints(CGraphicView::ROTATE_NONE);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, FALSE, TRUE);
        } else {
            view->SetCameraRotateConstraints(CGraphicView::ROTATE_Y_ONLY);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, TRUE, TRUE);
        }

        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, FALSE, TRUE);
        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, FALSE, TRUE);
    }
}

void CMainFrame::OnCameraRotateZ()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        if (view->m_cameraRotateConstraints == CGraphicView::ROTATE_Z_ONLY) {
            view->SetCameraRotateConstraints(CGraphicView::ROTATE_NONE);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, FALSE, TRUE);
        } else {
            view->SetCameraRotateConstraints(CGraphicView::ROTATE_Z_ONLY);
            m_objectToolbar.SetStatus(ID_CAMERA_ROTATEZONLY, TRUE, TRUE);
        }

        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEXONLY, FALSE, TRUE);
        m_objectToolbar.SetStatus(ID_CAMERA_ROTATEYONLY, FALSE, TRUE);
    }
}

void CMainFrame::OnCreateEmitter()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        doc->SetRenderObject(nullptr, true, true, false);
    }

    EmitterPropertySheetClass sheet(nullptr, IDS_EMITTERPROPERTIES, this);
    sheet.DoModal();
}

void CMainFrame::OnEditEmitter()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        EmitterInstanceList *list = new EmitterInstanceList();
        list->Add_Emitter(static_cast<ParticleEmitterClass *>(doc->m_model));
        EmitterPropertySheetClass sheet(list, IDS_EMITTERPROPERTIES, this);
        sheet.DoModal();
    }
}

void CMainFrame::OnExportEmitter()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    doc->ExportEmitter();
}

void CMainFrame::OnAnimateCamera()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    doc->AnimateCamera(!doc->m_animateCamera);
}

void CMainFrame::OnResetOnDisplay()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    doc->m_resetCamera = !doc->m_resetCamera;
}

void CMainFrame::OnRotateYBackwards()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ CGraphicView::ROTATE_MINUSY) & ~CGraphicView::ROTATE_PLUSY;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnRotateZBackwards()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        int flags = (view->m_objectRotationFlags ^ CGraphicView::ROTATE_MINUSZ) & ~CGraphicView::ROTATE_PLUSZ;
        view->SetRotationFlags(flags);
    }
}

void CMainFrame::OnLightingRotateY()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->m_lightRotationFlags =
            (view->m_lightRotationFlags ^ CGraphicView::ROTATE_PLUSY) & ~CGraphicView::ROTATE_MINUSY;
    }
}

void CMainFrame::OnLightingRotateYBackwards()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->m_lightRotationFlags =
            (view->m_lightRotationFlags ^ CGraphicView::ROTATE_MINUSY) & ~CGraphicView::ROTATE_PLUSY;
    }
}

void CMainFrame::OnLightingRotateZ()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->m_lightRotationFlags =
            (view->m_lightRotationFlags ^ CGraphicView::ROTATE_PLUSZ) & ~CGraphicView::ROTATE_MINUSZ;
    }
}

void CMainFrame::OnLightingRotateZBackwards()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->m_lightRotationFlags =
            (view->m_lightRotationFlags ^ CGraphicView::ROTATE_MINUSZ) & ~CGraphicView::ROTATE_PLUSZ;
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

void CMainFrame::OnRecordSceneCamera()
{
    RenderObjClass *robj = GetCurrentDocument()->m_model;

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            HLodClass *lod = static_cast<HLodClass *>(robj);
            CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));
            float size = lod->Get_Screen_Size(*view->m_camera);
            lod->Set_Max_Screen_Size(lod->Get_LOD_Level(), size);
            GetCurrentDocument()->CreateHLodPrototype(*lod);
        }
    }
}

void CMainFrame::OnIncludeNull()
{
    RenderObjClass *robj = GetCurrentDocument()->m_model;

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            HLodClass *lod = static_cast<HLodClass *>(robj);
            lod->Include_NULL_Lod(!lod->Is_NULL_Lod_Included());
            GetCurrentDocument()->CreateHLodPrototype(*lod);
        }
    }
}

void CMainFrame::OnPrevLod()
{
    GetCurrentDocument()->SetLODLevel(-1, nullptr);
}

void CMainFrame::OnNextLod()
{
    GetCurrentDocument()->SetLODLevel(1, nullptr);
}

void CMainFrame::OnAutoSwitching()
{
    ViewerSceneClass *scene = GetCurrentDocument()->m_scene;
    scene->Set_Auto_Switch_LOD(!scene->Get_Auto_Switch_LOD());
}

void CMainFrame::OnNext()
{
    CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));

    if (view != nullptr) {
        view->SelectNext();
    }
}

void CMainFrame::OnPrev()
{
    CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));

    if (view != nullptr) {
        view->SelectPrev();
    }
}

void CMainFrame::OnCameraSettings()
{
    CameraSettingsDialogClass dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnCopyScreenSize()
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));
    CameraClass *camera = view->m_camera;
    float size = GetCurrentDocument()->m_model->Get_Screen_Size(*camera);
    CString str;
    str.Format("MaxScreenSize=%f", size);
    int len = str.GetLength() + 1;
    HGLOBAL global = GlobalAlloc(GHND, len);
    LPVOID mem = GlobalLock(global);
    if (mem != NULL) {
        memcpy(mem, static_cast<const char *>(str), len);
        GlobalUnlock(global);
    }
    OpenClipboard();
    EmptyClipboard();
    SetClipboardData(CF_TEXT, global);
    CloseClipboard();
}

void CMainFrame::OnTexturePath()
{
    TexturePathDialogClass dlg(this);
    dlg.DoModal();
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
    if (W3D::Get_Prelit_Mode() != W3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS) {
        W3D::Set_Prelit_Mode(W3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
        CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));
        view->RefreshRenderObjects();
        GetCurrentDocument()->Deselect();
    }
}

void CMainFrame::OnMultiTextureLighting()
{
    if (W3D::Get_Prelit_Mode() != W3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE) {
        W3D::Set_Prelit_Mode(W3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
        CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));
        view->RefreshRenderObjects();
        GetCurrentDocument()->Deselect();
    }
}

void CMainFrame::OnVertexLighting()
{
    if (W3D::Get_Prelit_Mode() != W3D::PRELIT_MODE_VERTEX) {
        W3D::Set_Prelit_Mode(W3D::PRELIT_MODE_VERTEX);
        CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));
        view->RefreshRenderObjects();
        GetCurrentDocument()->Deselect();
    }
}

void CMainFrame::OnAddObject()
{
    ViewerSceneClass *scene = nullptr;

    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        scene = doc->m_scene;
    }

    CAddToLineupDialog dlg(scene, this);

    if (dlg.DoModal() == IDOK) {
        RenderObjClass *robj = W3DAssetManager::Get_Instance()->Create_Render_Obj(dlg.Get_Name());

        if (robj != nullptr && scene != nullptr) {
            SetLODLevel(robj);
            scene->Add_LOD_Object(robj);
        } else {
            CString str;
            str.Format("Unable to create render object '%s'!", static_cast<LPCSTR>(dlg.Get_Name()));
            AfxMessageBox(str, MB_ICONASTERISK, 0);
        }
    }
}

void CMainFrame::OnRestrictAnims()
{
    CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));

    if (view != nullptr) {
        view->RestrictAnims(!view->m_restrictAnims);
    }
}

void CMainFrame::OnSetDistance()
{
    CameraDistanceDialogClass dlg(this);
    dlg.DoModal();
}

void CMainFrame::OnAlternateMaterials()
{
    CW3DViewDoc *document = GetCurrentDocument();
    document->ToggleAlternateMaterials(nullptr);
}

void CMainFrame::OnWireframeMode()
{
    CW3DViewDoc *document = GetCurrentDocument();
    SceneClass::PolyRenderType curType = document->m_scene->Get_Polygon_Mode();
    SceneClass::PolyRenderType newType = curType == SceneClass::LINE ? SceneClass::FILL : SceneClass::LINE;
    document->m_scene->Set_Polygon_Mode(newType);
}

void CMainFrame::OnFog()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        doc->EnableFog(!doc->m_fogEnabled);
    }
}

void CMainFrame::OnScaleEmitter()
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        CScaleDialog dlg(1.0f, this, "Enter the scaling factor you want to apply to the current particle emitter");

        if (dlg.DoModal() != IDCANCEL) {
            ParticleEmitterClass *emitter = static_cast<ParticleEmitterClass *>(doc->m_model);
            emitter->Scale(dlg.Get_Scale());

            const char *name = emitter->Get_Name();
            if (name != nullptr && *name != 0) {
                W3DAssetManager::Get_Instance()->Remove_Prototype(name);
            }

            ParticleEmitterDefClass *def = emitter->Build_Definition();
            ParticleEmitterPrototypeClass *proto = new ParticleEmitterPrototypeClass(def);
            W3DAssetManager::Get_Instance()->Add_Prototype(proto);
        }
    }
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
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        view->m_plusXCamera = !view->m_plusXCamera;
    }
}

void CMainFrame::OnMungeSort()
{
    W3D::Enable_Munge_Sort_On_Load(W3D::Is_Munge_Sort_On_Load_Enabled() == false);
    AfxGetApp()->WriteProfileInt("Config", "MungeSortOnLoad", W3D::Is_Munge_Sort_On_Load_Enabled());
}

void CMainFrame::OnUpdateProperties(CCmdUI *pCmdUI)
{
    CDataTreeView *view = static_cast<CDataTreeView *>(m_splitter.GetPane(0, 0));

    if (view != nullptr) {
        pCmdUI->Enable(view->GetSelectedItemName() != nullptr);
    }
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
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_cameraRotateConstraints == CGraphicView::ROTATE_X_ONLY);
    }
}

void CMainFrame::OnUpdateCameraRotateY(CCmdUI *pCmdUI)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_cameraRotateConstraints == CGraphicView::ROTATE_Y_ONLY);
    }
}

void CMainFrame::OnUpdateCameraRotateZ(CCmdUI *pCmdUI)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_cameraRotateConstraints == CGraphicView::ROTATE_Z_ONLY);
    }
}

void CMainFrame::OnUpdateRotateX(CCmdUI *pCmdUI)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_objectRotationFlags & CGraphicView::ROTATE_PLUSX);
    }
}

void CMainFrame::OnUpdateRotateY(CCmdUI *pCmdUI)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_objectRotationFlags & CGraphicView::ROTATE_PLUSY);
    }
}

void CMainFrame::OnUpdateRotateZ(CCmdUI *pCmdUI)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_objectRotationFlags & CGraphicView::ROTATE_PLUSZ);
    }
}

void CMainFrame::OnUpdateEditEmitter(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_currentType == ASSET_TYPE_PARTICLEEMITTER);
}

void CMainFrame::OnUpdateExportEmitter(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_currentType == ASSET_TYPE_PARTICLEEMITTER);
}

void CMainFrame::OnUpdateAnimateCamera(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    pCmdUI->SetCheck(doc->m_animateCamera);
}

void CMainFrame::OnUpdateExportLod(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_currentType == ASSET_TYPE_HLOD);
}

void CMainFrame::OnUpdateResetOnDisplay(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    pCmdUI->SetCheck(doc->m_resetCamera);
}

void CMainFrame::OnUpdateIncludeNull(CCmdUI *pCmdUI)
{
    RenderObjClass *robj = GetCurrentDocument()->m_model;

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            HLodClass *lod = static_cast<HLodClass *>(robj);
            pCmdUI->SetCheck(lod->Is_NULL_Lod_Included());
        }
    }
}

void CMainFrame::OnUpdatePrevLod(CCmdUI *pCmdUI)
{
    RenderObjClass *robj = GetCurrentDocument()->m_model;

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            pCmdUI->Enable(robj->Get_LOD_Level() > 0);
        }
    }
}

void CMainFrame::OnUpdateNextLod(CCmdUI *pCmdUI)
{
    RenderObjClass *robj = GetCurrentDocument()->m_model;

    if (robj != nullptr) {
        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            pCmdUI->Enable(robj->Get_LOD_Level() + 1 < robj->Get_LOD_Count());
        }
    }
}

void CMainFrame::OnUpdateAutoSwitching(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(GetCurrentDocument()->m_scene->Get_Auto_Switch_LOD());
}

void CMainFrame::OnUpdateMultiPassLighting(CCmdUI *pCmdUI)
{
    pCmdUI->SetRadio(W3D::Get_Prelit_Mode() == W3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
}

void CMainFrame::OnUpdateMultiTextureLighting(CCmdUI *pCmdUI)
{
    pCmdUI->SetRadio(W3D::Get_Prelit_Mode() == W3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
}

void CMainFrame::OnUpdateVertexLighting(CCmdUI *pCmdUI)
{
    pCmdUI->SetRadio(W3D::Get_Prelit_Mode() == W3D::PRELIT_MODE_VERTEX);
}

void CMainFrame::OnUpdateAddObject(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    bool enable = false;

    if (doc != nullptr) {
        if (doc->m_scene != nullptr) {
            if (doc->m_model != nullptr && doc->m_scene->Is_LOD(doc->m_model->Class_ID())) {
                enable = true;
            }
        }
    }

    pCmdUI->Enable(enable);
}

void CMainFrame::OnUpdateRestrictAnims(CCmdUI *pCmdUI)
{
    bool b = true;
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());
    CDataTreeView *view = doc->GetDataTreeView();

    if (view != nullptr) {
        b = view->m_restrictAnims;
    }

    pCmdUI->SetCheck(b);
}

void CMainFrame::OnUpdateWireframeMode(CCmdUI *pCmdUI)
{
    CW3DViewDoc *document = GetCurrentDocument();
    pCmdUI->SetCheck(document->m_scene->Get_Polygon_Mode() == SceneClass::LINE);
}

void CMainFrame::OnUpdateFog(CCmdUI *pCmdUI)
{
    CW3DViewDoc *doc = static_cast<CW3DViewDoc *>(GetActiveDocument());

    if (doc != nullptr) {
        pCmdUI->SetCheck(doc->m_fogEnabled);
    }
}

void CMainFrame::OnUpdateScaleEmitter(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_currentType == ASSET_TYPE_PARTICLEEMITTER);
}

void CMainFrame::OnUpdatePolygonSorting(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(W3D::Is_Sorting_Enabled() != false);
}

void CMainFrame::OnUpdatePlusXCamera(CCmdUI *pCmdUI)
{
    CGraphicView *view = static_cast<CGraphicView *>(m_splitter.GetPane(0, 1));

    if (view != nullptr) {
        pCmdUI->SetCheck(view->m_plusXCamera);
    }
}

void CMainFrame::OnUpdateMungeSort(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(W3D::Is_Munge_Sort_On_Load_Enabled() != false);
}
