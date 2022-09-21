/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter general prop page
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
#include "shader.h"

class EmitterInstanceList;
class ColorBarClass;
class EmitterPropertySheetClass;

class EmitterGeneralPropPageClass : public CPropertyPage
{
public:
    EmitterGeneralPropPageClass();
    virtual ~EmitterGeneralPropPageClass() override {}
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnApply() override;

    void Initialize();

    bool IsValid() const { return m_isValid; }
    float GetLifetime() const { return m_lifetime; }
    void SetInstanceList(EmitterInstanceList *list) { m_instanceList = list; }
    void SetPropertySheet(EmitterPropertySheetClass *sheet) { m_sheet = sheet; }

private:
    DECLARE_DYNCREATE(EmitterGeneralPropPageClass)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnBrowse();
    afx_msg void OnTextureChanged();
    afx_msg void OnNameChanged();
    afx_msg void OnLifetimeChanged();
    afx_msg void OnShaderChanged();
    afx_msg void OnLifetime();
    afx_msg void OnFutureStartTimeChanged();

    void AddShader(ShaderClass *shader, const char *name);

    CSpinButtonCtrl m_startTimeSpin;
    CComboBox m_renderMode;
    CSpinButtonCtrl m_lifetimeSpin;
    EmitterInstanceList *m_instanceList;
    EmitterPropertySheetClass *m_sheet;
    CString m_name;
    CString m_textureName;
    ShaderClass m_shader;
    float m_lifetime;
    float m_futureStartTime;
    bool m_isValid;
};
