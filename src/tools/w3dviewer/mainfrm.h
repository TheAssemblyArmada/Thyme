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
#pragma once
#include "w3dafx.h"
#include "toolbar.h"

class CMainFrame : public CFrameWnd
{
public:
    virtual ~CMainFrame() override;
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo) override;

protected:
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext) override;

    void RestoreWindowPos();
    void DoProperties();
    void UpdateMenus();
    void UpdateStatusBar(unsigned int value);
    void UpdatePolyCount(int polys);
    void UpdateParticleCount(int particles);
    void UpdateFrameCount(int frame, int framecount, float fps);
    void UpdateCameraDistance(float distance);
    void GetDevice(bool doDeviceDlg);
    void UpdateEmitterMenu();

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
    afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR *lpwndpos);
    afx_msg void OnDestroy();

    afx_msg void OnProperties();
    afx_msg void OnGenerateLOD();
    afx_msg void OnOpen();
    afx_msg void OnAnimSettings();
    afx_msg void OnStopAnim();
    afx_msg void OnPlayAnim();
    afx_msg void OnPauseAnim();
    afx_msg void OnBack();
    afx_msg void OnBottom();
    afx_msg void OnFront();
    afx_msg void OnLeft();
    afx_msg void OnResetCamera();
    afx_msg void OnRight();
    afx_msg void OnTop();
    afx_msg void OnRotateZ();
    afx_msg void OnRotateY();
    afx_msg void OnRotateX();
    afx_msg void OnAmbient();
    afx_msg void OnSceneLight();
    afx_msg void OnBackgroundColor();
    afx_msg void OnBackgroundBitmap();
    afx_msg void OnSaveSettings();
    afx_msg void OnLoadSettings();
    afx_msg void OnEditLod();
    afx_msg void OnExportLOD();
    afx_msg void OnBackgroundObject();
    afx_msg void OnAnimationToolbar();
    afx_msg void OnObjectToolbar();
    afx_msg void OnStepForward();
    afx_msg void OnStepBack();
    afx_msg void OnReset();
    afx_msg void OnCameraRotateX();
    afx_msg void OnCameraRotateY();
    afx_msg void OnCameraRotateZ();
    afx_msg void OnChangeDevice();
    afx_msg void OnFullScreen();
    afx_msg void OnCreateEmitter();
    afx_msg void OnEditEmitter();
    afx_msg void OnExportEmitter();
    afx_msg void OnAutoAssignBone();
    afx_msg void OnBoneManagement();
    afx_msg void OnExportAggregate();
    afx_msg void OnAnimateCamera();
    afx_msg void OnResetOnDisplay();
    afx_msg void OnRotateYBackwards();
    afx_msg void OnRotateZBackwards();
    afx_msg void OnLightingRotateY();
    afx_msg void OnLightingRotateYBackwards();
    afx_msg void OnLightingRotateZ();
    afx_msg void OnLightingRotateZBackwards();
    afx_msg void OnDecSceneLight();
    afx_msg void OnIncSceneLight();
    afx_msg void OnDecAmbient();
    afx_msg void OnIncAmbient();
    afx_msg void OnMakeAggregate();
    afx_msg void OnRenameAggregate();
    afx_msg void OnRecordSceneCamera();
    afx_msg void OnIncludeNull();
    afx_msg void OnPrevLod();
    afx_msg void OnNextLod();
    afx_msg void OnAutoSwitching();
    afx_msg void OnMovie();
    afx_msg void OnScreenshot();
    afx_msg void OnNext();
    afx_msg void OnPrev();
    afx_msg void OnAnimAdvanced();
    afx_msg void OnCameraSettings();
    afx_msg void OnCopyScreenSize();
    afx_msg void OnMissingTextures();
    afx_msg void OnCopyDeps();
    afx_msg void OnExposePrecalc();
    afx_msg void OnTexturePath();
    afx_msg void OnChangeResolution();
    afx_msg void OnCreateSphere();
    afx_msg void OnCreateRing();
    afx_msg void OnEditPrimitive();
    afx_msg void OnExportPrimitive();
    afx_msg void OnKillSceneLight();
    afx_msg void OnMultiPassLighting();
    afx_msg void OnMultiTextureLighting();
    afx_msg void OnVertexLighting();
    afx_msg void OnAddObject();
    afx_msg void OnImportFacial();
    afx_msg void OnRestrictAnims();
    afx_msg void OnBindSubobject();
    afx_msg void OnSetDistance();
    afx_msg void OnAlternateMaterials();
    afx_msg void OnCreateSoundObject();
    afx_msg void OnEditSoundObject();
    afx_msg void OnExportSoundObject();
    afx_msg void OnWireframeMode();
    afx_msg void OnFog();
    afx_msg void OnScaleEmitter();
    afx_msg void OnPolygonSorting();
    afx_msg void OnPlusXCamera();
    afx_msg void OnNPatchesGap();
    afx_msg void OnNPatches1();
    afx_msg void OnNPatches2();
    afx_msg void OnNPatches3();
    afx_msg void OnNPatches4();
    afx_msg void OnNPatches5();
    afx_msg void OnNPatches6();
    afx_msg void OnNPatches7();
    afx_msg void OnNPatches8();
    afx_msg void OnMungeSort();
    afx_msg void OnEnableGamma();
    afx_msg void OnSetGamma();

    afx_msg void OnUpdateProperties(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAnimationToolbar(CCmdUI *pCmdUI);
    afx_msg void OnUpdateObjectToolbar(CCmdUI *pCmdUI);
    afx_msg void OnUpdateCameraRotateX(CCmdUI *pCmdUI);
    afx_msg void OnUpdateCameraRotateY(CCmdUI *pCmdUI);
    afx_msg void OnUpdateCameraRotateZ(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRotateX(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRotateY(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRotateZ(CCmdUI *pCmdUI);
    afx_msg void OnUpdateFullScreen(CCmdUI *pCmdUI);
    afx_msg void OnUpdateEditEmitter(CCmdUI *pCmdUI);
    afx_msg void OnUpdateExportEmitter(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAnimateCamera(CCmdUI *pCmdUI);
    afx_msg void OnUpdateExportLod(CCmdUI *pCmdUI);
    afx_msg void OnUpdateExportAggregate(CCmdUI *pCmdUI);
    afx_msg void OnUpdateResetOnDisplay(CCmdUI *pCmdUI);
    afx_msg void OnUpdateIncludeNull(CCmdUI *pCmdUI);
    afx_msg void OnUpdatePrevLod(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNextLod(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAutoSwitching(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMovie(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAnimAdvanced(CCmdUI *pCmdUI);
    afx_msg void OnUpdateCopyDeps(CCmdUI *pCmdUI);
    afx_msg void OnUpdateExposePrecalc(CCmdUI *pCmdUI);
    afx_msg void OnUpdateEditPrimitive(CCmdUI *pCmdUI);
    afx_msg void OnUpdateExportPrimitive(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMultiPassLighting(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMultiTextureLighting(CCmdUI *pCmdUI);
    afx_msg void OnUpdateVertexLighting(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAddObject(CCmdUI *pCmdUI);
    afx_msg void OnUpdateImportFacial(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRestrictAnims(CCmdUI *pCmdUI);
    afx_msg void OnUpdateBindSubobject(CCmdUI *pCmdUI);
    afx_msg void OnUpdateEditSoundObject(CCmdUI *pCmdUI);
    afx_msg void OnUpdateExportSoundObject(CCmdUI *pCmdUI);
    afx_msg void OnUpdateWireframeMode(CCmdUI *pCmdUI);
    afx_msg void OnUpdateFog(CCmdUI *pCmdUI);
    afx_msg void OnUpdateScaleEmitter(CCmdUI *pCmdUI);
    afx_msg void OnUpdatePolygonSorting(CCmdUI *pCmdUI);
    afx_msg void OnUpdatePlusXCamera(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatchesGap(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches1(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches2(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches3(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches4(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches5(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches6(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches7(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNPatches8(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMungeSort(CCmdUI *pCmdUI);
    afx_msg void OnUpdateEnableGamma(CCmdUI *pCmdUI);

    CSplitterWnd m_splitter;
    CStatusBar m_statusBar;
    CToolBar m_toolBar;
    int m_currentType;
    CFancyToolbar m_objectToolbar;
    CFancyToolbar m_animationToolbar;
    BOOL m_animationToolbarVisible;
    RECT m_windowRect;
    CMenu *m_subMenu;
    BOOL m_initialized;
};
