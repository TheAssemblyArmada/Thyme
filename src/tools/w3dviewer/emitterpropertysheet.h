/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter property sheet
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
#include "emittercolorproppage.h"
#include "emitterframeproppage.h"
#include "emittergeneralproppage.h"
#include "emitterlinegroupproppage.h"
#include "emitterlineproppage.h"
#include "emitterparticleproppage.h"
#include "emitterphysicsproppage.h"
#include "emitterrotationproppage.h"
#include "emittersizeproppage.h"
#include "emitteruserproppage.h"

class EmitterPropertySheetClass : public CPropertySheet
{
public:
    EmitterPropertySheetClass(EmitterInstanceList *list, UINT uID, CWnd *pParentWnd);
    virtual ~EmitterPropertySheetClass() override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
    void UpdateRenderingMode(int mode);

private:
    DECLARE_DYNAMIC(EmitterPropertySheetClass)
    DECLARE_MESSAGE_MAP()

    void UpdateEmitter();
    void Initialize();
    void CreateEmitter();

    EmitterGeneralPropPageClass m_generalPage;
    EmitterParticlePropPageClass m_particlePage;
    EmitterPhysicsPropPageClass m_physicsPage;
    EmitterColorPropPageClass m_colorPage;
    EmitterUserPropPageClass m_userPage;
    EmitterSizePropPageClass m_sizePage;
    EmitterLinePropPageClass m_linePage;
    EmitterRotationPropPageClass m_rotationPage;
    EmitterFramePropPageClass m_framePage;
    EmitterLineGroupPropPageClass m_lineGroupPage;
    EmitterInstanceList *m_instanceList;
    CString m_name;
};
