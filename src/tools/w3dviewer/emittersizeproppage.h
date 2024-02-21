/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter size prop page
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

class EmitterSizePropPageClass : public CPropertyPage
{
public:
    EmitterSizePropPageClass();
    virtual ~EmitterSizePropPageClass() override;
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
    DECLARE_DYNCREATE(EmitterSizePropPageClass)
    DECLARE_MESSAGE_MAP()

    void UpdateSize();

    CSpinButtonCtrl m_randomizerSpin;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    ColorBarClass *m_sizeBar;
    ParticlePropertyStruct<float> m_oldSizeKeyFrames;
    ParticlePropertyStruct<float> m_sizeKeyFrames;
    float m_lifetime;
    float m_scale;
};
