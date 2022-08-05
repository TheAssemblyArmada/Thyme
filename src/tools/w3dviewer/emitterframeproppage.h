/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter frame prop page
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
#include "part_emt.h"

class EmitterInstanceList;
class ColorBarClass;

class EmitterFramePropPageClass : public CPropertyPage
{
public:
    EmitterFramePropPageClass();
    virtual ~EmitterFramePropPageClass() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnApply() override;

    void Initialize();
    void UpdateLifetime(float lifetime);

    bool IsValid() const { return m_isValid; }
    void SetInstanceList(EmitterInstanceList *list) { m_instanceList = list; }

private:
    DECLARE_DYNCREATE(EmitterFramePropPageClass)
    DECLARE_MESSAGE_MAP()

    void UpdateFrame();

    CComboBox m_textureGrid;
    CSpinButtonCtrl m_frameSpin;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    ColorBarClass *m_frameBar;
    ParticlePropertyStruct<float> m_frameKeyFrames;
    float m_lifetime;
    float m_minFrame;
    float m_maxFrame;
};
