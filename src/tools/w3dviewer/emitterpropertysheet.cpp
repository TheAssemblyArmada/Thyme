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
#include "emitterpropertysheet.h"
#include "assetmgr.h"
#include "assettypes.h"
#include "datatreeview.h"
#include "emitterinstancelist.h"
#include "utils.h"
#include "w3dviewdoc.h"

IMPLEMENT_DYNAMIC(EmitterPropertySheetClass, CPropertySheet)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterPropertySheetClass, CPropertySheet)
END_MESSAGE_MAP()
// clang-format on

EmitterPropertySheetClass::EmitterPropertySheetClass(EmitterInstanceList *list, UINT uID, CWnd *pParentWnd) :
    CPropertySheet(uID, pParentWnd), m_instanceList(list)
{
    Initialize();
}

EmitterPropertySheetClass::~EmitterPropertySheetClass()
{
    if (m_instanceList != nullptr) {
        delete m_instanceList;
        m_instanceList = nullptr;
    }
}

LRESULT EmitterPropertySheetClass::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message != WM_COMMAND) {
        return CWnd::WindowProc(message, wParam, lParam);
    }

    switch (LOWORD(wParam)) {
        case IDOK:
            if (!::IsWindowEnabled(::GetDlgItem(m_hWnd, ID_APPLY_NOW))) {
                return CWnd::WindowProc(message, wParam, lParam);
            }

            break;
        case IDCANCEL:
            GetCurrentDocument()->Deselect();
            return CWnd::WindowProc(message, wParam, lParam);
            break;
        case ID_APPLY_NOW:
            break;
        default:
            return CWnd::WindowProc(message, wParam, lParam);
    }

    if (HIWORD(wParam)) {
        return CWnd::WindowProc(message, wParam, lParam);
    }

    HRESULT r = CWnd::WindowProc(message, wParam, lParam);

    if (m_generalPage.IsValid() && m_particlePage.IsValid() && m_physicsPage.IsValid() && m_colorPage.IsValid()
        && m_userPage.IsValid() && m_sizePage.IsValid() && m_linePage.IsValid() && m_rotationPage.IsValid()
        && m_framePage.IsValid() && m_lineGroupPage.IsValid()) {
        float lifetime = m_generalPage.GetLifetime();
        m_colorPage.UpdateLifetime(lifetime);
        m_sizePage.UpdateLifetime(lifetime);
        m_rotationPage.UpdateLifetime(lifetime);
        m_framePage.UpdateLifetime(lifetime);
        m_lineGroupPage.UpdateLifetime(lifetime);
        UpdateEmitter();
    }

    return r;
}

void EmitterPropertySheetClass::UpdateRenderingMode(int mode)
{
    DisableWindows(m_linePage.m_hWnd, mode == W3D_EMITTER_RENDER_MODE_LINE);
}

void EmitterPropertySheetClass::UpdateEmitter()
{
    CW3DViewDoc *document = GetCurrentDocument();
    ParticleEmitterPrototypeClass *emitter;

    if (document != nullptr && m_instanceList != nullptr) {
        ParticleEmitterDefClass *def = new ParticleEmitterDefClass(*m_instanceList);
        emitter = new ParticleEmitterPrototypeClass(def);

        if (m_name.GetLength()) {
            W3DAssetManager::Get_Instance()->Remove_Prototype(m_name);
        }

        W3DAssetManager::Get_Instance()->Add_Prototype(emitter);
        document->GetDataTreeView()->RenameItem(m_instanceList->Get_Name(), m_name, ASSET_TYPE_PARTICLEEMITTER);
        document->Deselect();
        m_name = m_instanceList->Get_Name();
        m_instanceList->Clear_Emitters();
        document->AddEmittersToList(m_instanceList, m_instanceList->Get_Name(), nullptr);
    }
}

void EmitterPropertySheetClass::Initialize()
{
    if (m_instanceList != nullptr) {
        m_name = m_instanceList->Get_Name();
    } else {
        CreateEmitter();
    }

    m_generalPage.SetInstanceList(m_instanceList);
    m_generalPage.Initialize();
    m_particlePage.SetInstanceList(m_instanceList);
    m_particlePage.Initialize();
    m_physicsPage.SetInstanceList(m_instanceList);
    m_physicsPage.Initialize();
    m_colorPage.SetInstanceList(m_instanceList);
    m_colorPage.Initialize();
    m_userPage.SetInstanceList(m_instanceList);
    m_userPage.Initialize();
    m_sizePage.SetInstanceList(m_instanceList);
    m_sizePage.Initialize();
    m_linePage.SetInstanceList(m_instanceList);
    m_linePage.Initialize();
    m_rotationPage.SetInstanceList(m_instanceList);
    m_rotationPage.Initialize();
    m_framePage.SetInstanceList(m_instanceList);
    m_framePage.Initialize();
    m_lineGroupPage.SetInstanceList(m_instanceList);
    m_lineGroupPage.Initialize();

    AddPage(&m_generalPage);
    AddPage(&m_particlePage);
    AddPage(&m_physicsPage);
    AddPage(&m_colorPage);
    AddPage(&m_sizePage);
    AddPage(&m_userPage);
    AddPage(&m_linePage);
    AddPage(&m_rotationPage);
    AddPage(&m_framePage);
    AddPage(&m_lineGroupPage);
    m_generalPage.SetPropertySheet(this);
}

void EmitterPropertySheetClass::CreateEmitter()
{
    ParticlePropertyStruct<Vector3> color;
    memset(&color, 0, sizeof(color));
    color.Start = Vector3(1.0f, 1.0f, 1.0f);
    ParticlePropertyStruct<float> opacity;
    memset(&opacity, 0, sizeof(opacity));
    opacity.Start = 1.0f;
    ParticlePropertyStruct<float> size;
    memset(&size, 0, sizeof(size));
    size.Start = 0.1f;
    ParticlePropertyStruct<float> rotation;
    memset(&rotation, 0, sizeof(rotation));
    ParticlePropertyStruct<float> frames;
    memset(&frames, 0, sizeof(frames));
    ParticlePropertyStruct<float> blurtimes;
    memset(&blurtimes, 0, sizeof(blurtimes));

    ParticleEmitterClass *emitter = new ParticleEmitterClass(10.0f,
        1,
        new Vector3SolidBoxRandomizer(Vector3(0.1f, 0.1f, 0.1f)),
        Vector3(0.0f, 0.0f, 1.0f),
        new Vector3SolidBoxRandomizer(Vector3(0.0f, 0.0f, 1.0f)),
        0.0f,
        0.0f,
        color,
        opacity,
        size,
        rotation,
        0.0f,
        frames,
        blurtimes,
        Vector3(0.0f, 0.0f, 0.0f),
        1.0f,
        0.0f,
        nullptr);

    m_instanceList = new EmitterInstanceList();
    m_instanceList->Add_Emitter(emitter);
    GetCurrentDocument()->SetParticleEmitter(emitter, true, true);
    Ref_Ptr_Release(emitter);
}
