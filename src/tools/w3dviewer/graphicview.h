/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View graphic view
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
#include "sphere.h"
#include "vector3.h"
#include <mmsystem.h>

class CameraClass;
class ParticleEmitterClass;
class RenderObjClass;

#define GVM_RENDER 1125

class CGraphicView : public CView
{
public:
    virtual ~CGraphicView() override {}
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
    virtual void OnInitialUpdate() override;
    virtual void OnDraw(CDC *pDC) override {}

    BOOL Create();
    void EnableRendering(bool enable);
    void Render(BOOL update, unsigned int time);
    void ResetParticleEmitterCamera(ParticleEmitterClass *emitter);
    void ResetCameraValues(SphereClass &sphere);
    void ResetCamera(RenderObjClass *robj);
    void UpdateAnimation(int flag);
    void SetCameraDirection(int direction);
    void SetRotationFlags(int flags);
    void SetCameraRotateConstraints(int constraints);
    void ResetRenderObj();
    void UpdateObjectRotation();
    void UpdateLightTransform();
    void UpdateCamera();
    void UpdateCameraDistance(float distance);

protected:
    CGraphicView();
    DECLARE_DYNCREATE(CGraphicView)
    DECLARE_MESSAGE_MAP()

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR *lpMMI);

public:
    BOOL m_isInitialized;
    BOOL m_isActive;
    MMRESULT m_timer;
    CameraClass *m_camera;
    RenderObjClass *m_light;
    bool m_lightInScene;
    Vector3 m_objCenter;
    SphereClass m_objSphere;
    BOOL m_leftButtonDown;
    BOOL m_rightButtonDown;
    CPoint m_mousePos;
    BOOL m_isWindowed;
    int m_renderingDisabled;
    float m_radius;
    unsigned int m_lastCountUpdate;
    BOOL m_plusXCamera;
    unsigned int m_time;
    float m_animationSpeed;
    int m_animationPlaying;
    int m_objectRotationFlags;
    int m_lightRotationFlags;
    int m_cameraRotateConstraints;
};
