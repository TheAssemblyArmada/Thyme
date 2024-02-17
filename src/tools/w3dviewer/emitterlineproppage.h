/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter line prop page
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

class EmitterInstanceList;

class EmitterLinePropPageClass : public CPropertyPage
{
public:
    EmitterLinePropPageClass();
    virtual ~EmitterLinePropPageClass() override {}
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnApply() override;

    void Initialize();

    bool IsValid() const { return m_isValid; }
    void SetInstanceList(EmitterInstanceList *list) { m_instanceList = list; }

private:
    DECLARE_DYNCREATE(EmitterLinePropPageClass)
    DECLARE_MESSAGE_MAP()

    CComboBox m_mapping;
    CSpinButtonCtrl m_mergeAbortSpin;
    CSpinButtonCtrl m_vPerSecSpin;
    CSpinButtonCtrl m_uvTileSpin;
    CSpinButtonCtrl m_uPerSecSpin;
    CSpinButtonCtrl m_amplitudeSpin;
    CSpinButtonCtrl m_subdivisionSpin;
    EmitterInstanceList *m_instanceList;
    bool m_isValid;
    int m_textureMappingMode;
    bool m_mergeIntersections;
    bool m_endCaps;
    bool m_disableSorting;
    unsigned int m_subdivisionLevel;
    float m_noiseAmplitude;
    float m_mergeAbortFactor;
    float m_textureTileFactor;
    float m_uPerSec;
    float m_vPerSec;
};
