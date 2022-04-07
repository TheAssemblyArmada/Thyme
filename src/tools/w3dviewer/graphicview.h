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
    enum RotationFlags
    {
        ROTATE_PLUSX = 1,
        ROTATE_PLUSY = 2,
        ROTATE_PLUSZ = 4,
        ROTATE_MINUSX = 8,
        ROTATE_MINUSY = 16,
        ROTATE_MINUSZ = 32,
    };

    enum AnimationState
    {
        ANIMATION_NONE = -1,
        ANIMATION_PLAY = 0,
        ANIMATION_STOP = 1,
        ANIMATION_PAUSE = 2,
    };

    enum CameraRotateConstraint
    {
        ROTATE_NONE = 0,
        ROTATE_X_ONLY = 1,
        ROTATE_Y_ONLY = 2,
        ROTATE_Z_ONLY = 4,
    };

    enum CameraDirection
    {
        DIRECTION_FRONT = -1,
        DIRECTION_BACK = 0,
        DIRECTION_TOP = 1,
        DIRECTION_BOTTOM = 2,
        DIRECTION_LEFT = 3,
        DIRECTION_RIGHT = 4,
    };

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
    void UpdateAnimation(AnimationState flag);
    void SetCameraDirection(CameraDirection direction);
    void SetRotationFlags(int flags);
    void SetCameraRotateConstraints(CameraRotateConstraint constraints);
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
    AnimationState m_animationState;
    int m_objectRotationFlags;
    int m_lightRotationFlags;
    CameraRotateConstraint m_cameraRotateConstraints;
};
