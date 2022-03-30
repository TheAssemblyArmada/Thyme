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
#include "deviceselectiondialog.h"
#include "renderdevicedesc.h"
#include "resource.h"
#include "w3d.h"
#include "w3dview.h"
#include "w3dviewdoc.h"

class CGraphicView;

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
     ON_COMMAND(ID_FILE_SAVESETTINGS, OnSaveSettings)
     ON_COMMAND(ID_FILE_LOADSETTINGS, OnLoadSettings)
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
     ON_COMMAND(ID_VIEW_CHANGEDEVICE, OnChangeDevice)
     ON_COMMAND(ID_VIEW_VIEWFULLSCREEN, OnFullScreen)
     ON_UPDATE_COMMAND_UI(ID_VIEW_VIEWFULLSCREEN, OnUpdateFullScreen)
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
     ON_COMMAND(ID_VIEW_CHANGERESOLUTION, OnChangeResolution)
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
     ON_COMMAND(ID_VIEW_NPATCHESGAPFILLING, OnNPatchesGap)
     ON_UPDATE_COMMAND_UI(ID_VIEW_NPATCHESGAPFILLING, OnUpdateNPatchesGap)
     ON_COMMAND(ID_N_1, OnNPatches1)
     ON_UPDATE_COMMAND_UI(ID_N_1, OnUpdateNPatches1)
     ON_COMMAND(ID_N_2, OnNPatches2)
     ON_UPDATE_COMMAND_UI(ID_N_2, OnUpdateNPatches2)
     ON_COMMAND(ID_N_3, OnNPatches3)
     ON_UPDATE_COMMAND_UI(ID_N_3, OnUpdateNPatches3)
     ON_COMMAND(ID_N_4, OnNPatches4)
     ON_UPDATE_COMMAND_UI(ID_N_4, OnUpdateNPatches4)
     ON_COMMAND(ID_N_5, OnNPatches5)
     ON_UPDATE_COMMAND_UI(ID_N_5, OnUpdateNPatches5)
     ON_COMMAND(ID_N_6, OnNPatches6)
     ON_UPDATE_COMMAND_UI(ID_N_6, OnUpdateNPatches6)
     ON_COMMAND(ID_N_7, OnNPatches7)
     ON_UPDATE_COMMAND_UI(ID_N_7, OnUpdateNPatches7)
     ON_COMMAND(ID_N_8, OnNPatches8)
     ON_UPDATE_COMMAND_UI(ID_N_8, OnUpdateNPatches8)
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

        if (m_subMenu->GetMenuItemInfo(LOWORD(wParam), &info)) {
            // TODO
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
    } else if (message == WM_COMMAND && LOWORD(wParam) >= ID_SETTINGS1 && LOWORD(wParam) <= ID_SETTINGS9) {
        char path[MAX_PATH];
        GetModuleFileName(nullptr, path, MAX_PATH);
        char *str = strrchr(path, '\\');

        if (str != nullptr) {
            *str = 0;
        }

        strcat(path, "\\settings");
        char fname[MAX_PATH];
        wsprintf(fname, "%s%d.dat", path, LOWORD(wParam) - ID_SETTINGS1 - 1);

        if (GetFileAttributes(fname) != INVALID_FILE_ATTRIBUTES) {
            CW3DViewDoc *doc = (CW3DViewDoc *)GetActiveDocument();

            if (doc != nullptr) {
                doc->SaveSettings(fname);
            }
        }
    }

    return CFrameWnd::WindowProc(message, wParam, lParam);
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext)
{
    BOOL ret = m_splitter.CreateStatic(this, 1, 2);

    if (ret) {
        // TODO xxxx
        // ret &= m_splitter.CreateView(0, 0, CDataTreeView::GetRuntimeClass(), SIZE{ 340, 10 }, pContext);
        // ret &= m_splitter.CreateView(0, 1, CGraphicView::GetRuntimeClass(), SIZE{ 120, 10 }, pContext);

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
    // TODO
}

void CMainFrame::UpdateMenus()
{
    // TODO
}

void CMainFrame::UpdateStatusBar(unsigned int value)
{
    // TODO
}

void CMainFrame::UpdatePolyCount(int polys)
{
    // TODO
}

void CMainFrame::UpdateParticleCount(int particles)
{
    // TODO
}

void CMainFrame::UpdateFrameCount(int frame, int framecount, float fps)
{
    // TODO
}

void CMainFrame::UpdateCameraDistance(float distance)
{
    // TODO
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

        // TODO xxxx
        // if (!view.Create()) {
        //     return;
        // }
    }
}

void CMainFrame::UpdateEmitterMenu()
{
    // TODO
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
    CMenu *menu = GetMenu();
    CMenu *SubMenu = menu->GetSubMenu(3);
    m_subMenu = SubMenu;
    m_subMenu = SubMenu->GetSubMenu(3);
    RestoreWindowPos();
    m_initialized = TRUE;
    return (Device != -1) - 1;
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
    // TODO xxxx
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
    // TODO
}

void CMainFrame::OnGenerateLOD()
{
    // TODO
}

void CMainFrame::OnOpen()
{
    // TODO
}

void CMainFrame::OnAnimSettings()
{
    // TODO
}

void CMainFrame::OnStopAnim()
{
    // TODO
}

void CMainFrame::OnPlayAnim()
{
    // TODO
}

void CMainFrame::OnPauseAnim()
{
    // TODO
}

void CMainFrame::OnBack()
{
    // TODO
}

void CMainFrame::OnBottom()
{
    // TODO
}

void CMainFrame::OnFront()
{
    // TODO
}

void CMainFrame::OnLeft()
{
    // TODO
}

void CMainFrame::OnResetCamera()
{
    // TODO
}

void CMainFrame::OnRight()
{
    // TODO
}

void CMainFrame::OnTop()
{
    // TODO
}

void CMainFrame::OnRotateZ()
{
    // TODO
}

void CMainFrame::OnRotateY()
{
    // TODO
}

void CMainFrame::OnRotateX()
{
    // TODO
}

void CMainFrame::OnAmbient()
{
    // TODO
}

void CMainFrame::OnSceneLight()
{
    // TODO
}

void CMainFrame::OnBackgroundColor()
{
    // TODO
}

void CMainFrame::OnBackgroundBitmap()
{
    // TODO
}

void CMainFrame::OnSaveSettings()
{
    // TODO
}

void CMainFrame::OnLoadSettings()
{
    // TODO
}

void CMainFrame::OnEditLod()
{
    // TODO
}

void CMainFrame::OnExportLOD()
{
    // TODO
}

void CMainFrame::OnBackgroundObject()
{
    // TODO
}

void CMainFrame::OnAnimationToolbar()
{
    // TODO
}

void CMainFrame::OnObjectToolbar()
{
    // TODO
}

void CMainFrame::OnStepForward()
{
    // TODO
}

void CMainFrame::OnStepBack()
{
    // TODO
}

void CMainFrame::OnReset()
{
    // TODO
}

void CMainFrame::OnCameraRotateX()
{
    // TODO
}

void CMainFrame::OnCameraRotateY()
{
    // TODO
}

void CMainFrame::OnCameraRotateZ()
{
    // TODO
}

void CMainFrame::OnChangeDevice()
{
    // TODO
}

void CMainFrame::OnFullScreen()
{
    // TODO
}

void CMainFrame::OnCreateEmitter()
{
    // TODO
}

void CMainFrame::OnEditEmitter()
{
    // TODO
}

void CMainFrame::OnExportEmitter()
{
    // TODO
}

void CMainFrame::OnAutoAssignBone()
{
    // TODO
}

void CMainFrame::OnBoneManagement()
{
    // TODO
}

void CMainFrame::OnExportAggregate()
{
    // TODO
}

void CMainFrame::OnAnimateCamera()
{
    // TODO
}

void CMainFrame::OnResetOnDisplay()
{
    // TODO
}

void CMainFrame::OnRotateYBackwards()
{
    // TODO
}

void CMainFrame::OnRotateZBackwards()
{
    // TODO
}

void CMainFrame::OnLightingRotateY()
{
    // TODO
}

void CMainFrame::OnLightingRotateYBackwards()
{
    // TODO
}

void CMainFrame::OnLightingRotateZ()
{
    // TODO
}

void CMainFrame::OnLightingRotateZBackwards()
{
    // TODO
}

void CMainFrame::OnDecSceneLight()
{
    // TODO
}

void CMainFrame::OnIncSceneLight()
{
    // TODO
}

void CMainFrame::OnDecAmbient()
{
    // TODO
}

void CMainFrame::OnIncAmbient()
{
    // TODO
}

void CMainFrame::OnMakeAggregate()
{
    // TODO
}

void CMainFrame::OnRenameAggregate()
{
    // TODO
}

void CMainFrame::OnRecordSceneCamera()
{
    // TODO
}

void CMainFrame::OnIncludeNull()
{
    // TODO
}

void CMainFrame::OnPrevLod()
{
    // TODO
}

void CMainFrame::OnNextLod()
{
    // TODO
}

void CMainFrame::OnAutoSwitching()
{
    // TODO
}

void CMainFrame::OnMovie()
{
    // TODO
}

void CMainFrame::OnScreenshot()
{
    // TODO
}

void CMainFrame::OnNext()
{
    // TODO
}

void CMainFrame::OnPrev()
{
    // TODO
}

void CMainFrame::OnAnimAdvanced()
{
    // TODO
}

void CMainFrame::OnCameraSettings()
{
    // TODO
}

void CMainFrame::OnCopyScreenSize()
{
    // TODO
}

void CMainFrame::OnMissingTextures()
{
    // TODO
}

void CMainFrame::OnCopyDeps()
{
    // TODO
}

void CMainFrame::OnExposePrecalc()
{
    // TODO
}

void CMainFrame::OnTexturePath()
{
    // TODO
}

void CMainFrame::OnChangeResolution()
{
    // TODO
}

void CMainFrame::OnCreateSphere()
{
    // TODO
}

void CMainFrame::OnCreateRing()
{
    // TODO
}

void CMainFrame::OnEditPrimitive()
{
    // TODO
}

void CMainFrame::OnExportPrimitive()
{
    // TODO
}

void CMainFrame::OnKillSceneLight()
{
    // TODO
}

void CMainFrame::OnMultiPassLighting()
{
    // TODO
}

void CMainFrame::OnMultiTextureLighting()
{
    // TODO
}

void CMainFrame::OnVertexLighting()
{
    // TODO
}

void CMainFrame::OnAddObject()
{
    // TODO
}

void CMainFrame::OnImportFacial()
{
    // TODO
}

void CMainFrame::OnRestrictAnims()
{
    // TODO
}

void CMainFrame::OnBindSubobject()
{
    // TODO
}

void CMainFrame::OnSetDistance()
{
    // TODO
}

void CMainFrame::OnAlternateMaterials()
{
    // TODO
}

void CMainFrame::OnCreateSoundObject()
{
    // TODO
}

void CMainFrame::OnEditSoundObject()
{
    // TODO
}

void CMainFrame::OnExportSoundObject()
{
    // TODO
}

void CMainFrame::OnWireframeMode()
{
    // TODO
}

void CMainFrame::OnFog()
{
    // TODO
}

void CMainFrame::OnScaleEmitter()
{
    // TODO
}

void CMainFrame::OnPolygonSorting()
{
    // TODO
}

void CMainFrame::OnPlusXCamera()
{
    // TODO
}

void CMainFrame::OnNPatchesGap()
{
    // TODO
}

void CMainFrame::OnNPatches1()
{
    // TODO
}

void CMainFrame::OnNPatches2()
{
    // TODO
}

void CMainFrame::OnNPatches3()
{
    // TODO
}

void CMainFrame::OnNPatches4()
{
    // TODO
}

void CMainFrame::OnNPatches5()
{
    // TODO
}

void CMainFrame::OnNPatches6()
{
    // TODO
}

void CMainFrame::OnNPatches7()
{
    // TODO
}

void CMainFrame::OnNPatches8()
{
    // TODO
}

void CMainFrame::OnMungeSort()
{
    // TODO
}

void CMainFrame::OnEnableGamma()
{
    // TODO
}

void CMainFrame::OnSetGamma()
{
    // TODO
}

void CMainFrame::OnUpdateProperties(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateAnimationToolbar(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateObjectToolbar(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateCameraRotateX(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateCameraRotateY(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateCameraRotateZ(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateRotateX(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateRotateY(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateRotateZ(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateFullScreen(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateEditEmitter(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateExportEmitter(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateAnimateCamera(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateExportLod(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateExportAggregate(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateResetOnDisplay(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateIncludeNull(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdatePrevLod(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNextLod(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateAutoSwitching(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateMovie(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateAnimAdvanced(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateCopyDeps(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateExposePrecalc(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateEditPrimitive(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateExportPrimitive(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateMultiPassLighting(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateMultiTextureLighting(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateVertexLighting(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateAddObject(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateImportFacial(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateRestrictAnims(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateBindSubobject(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateEditSoundObject(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateExportSoundObject(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateWireframeMode(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateFog(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateScaleEmitter(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdatePolygonSorting(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdatePlusXCamera(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatchesGap(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches1(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches2(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches3(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches4(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches5(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches6(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches7(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateNPatches8(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateMungeSort(CCmdUI *pCmdUI)
{
    // TODO
}

void CMainFrame::OnUpdateEnableGamma(CCmdUI *pCmdUI)
{
    // TODO
}
