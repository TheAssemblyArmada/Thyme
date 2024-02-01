/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D In Game UI
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dingameui.h"
#include "gameclient.h"
#include "gamewindowmanager.h"
#include "globaldata.h"
#include "hanim.h"
#include "rendobj.h"
#include "terrainlogic.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dscene.h"
#include "w3dview.h"

W3DInGameUI::W3DInGameUI() : m_locater01(nullptr), m_locater02(nullptr)
{
    for (int i = 0; i < MAX_MOVE_HINTS; i++) {
        m_moveHintRenderObj[i] = nullptr;
        m_moveHintAnim[i] = nullptr;
    }
}

W3DInGameUI::~W3DInGameUI()
{
    for (int i = 0; i < MAX_MOVE_HINTS; i++) {
        Ref_Ptr_Release(m_moveHintRenderObj[i]);
        Ref_Ptr_Release(m_moveHintAnim[i]);
    }

    Ref_Ptr_Release(m_locater01);
    Ref_Ptr_Release(m_locater02);
}

void W3DInGameUI::Init()
{
    InGameUI::Init();
}

void W3DInGameUI::Reset()
{
    InGameUI::Reset();
}

void W3DInGameUI::Update()
{
    InGameUI::Update();
}

void W3DInGameUI::Draw()
{
    Pre_Draw();

    if (m_isDragSelecting) {
        Draw_Selection_Region();
    }

    if (g_theDisplay != nullptr) {
        for (View *view = g_theDisplay->Get_First_View(); view != nullptr; view = g_theDisplay->Get_Next_View(view)) {
            Draw_Move_Hints(view);
            Draw_Attack_Hints(view);
            Draw_Place_Angle(view);
        }
    }

    Post_Draw();
    g_theWindowManager->Win_Repaint();
}

View *W3DInGameUI::Create_View()
{
    return new W3DView();
}

void W3DInGameUI::Draw_Selection_Region()
{
    g_theDisplay->Draw_Open_Rect(m_dragSelectRegion.lo.x,
        m_dragSelectRegion.lo.y,
        m_dragSelectRegion.hi.x - m_dragSelectRegion.lo.x,
        m_dragSelectRegion.hi.y - m_dragSelectRegion.lo.y,
        2.0f,
        0x9933FF33);
}

void W3DInGameUI::Draw_Move_Hints(View *view)
{
    int count = 0;
    Matrix3D tm;

    for (;;) {
        if ((g_theGameClient->Get_Frame() - m_moveHint[count].frame) > 40) {
            if (m_moveHintRenderObj[count] != nullptr && !m_moveHintRenderObj[count]->Is_Hidden()) {
                m_moveHintRenderObj[count]->Set_Hidden(true);
                W3DDisplay::s_3DScene->Remove_Render_Object(m_moveHintRenderObj[count]);
            }

            if (++count >= MAX_MOVE_HINTS) {
                return;
            }

            continue;
        }

        if (m_moveHintRenderObj[count] == nullptr) {
            break;
        }

    label2:
        if (m_moveHintRenderObj[count]->Is_Hidden()) {
            m_moveHintRenderObj[count]->Set_Hidden(false);
            W3DDisplay::s_3DScene->Add_Render_Object(m_moveHintRenderObj[count]);

            if (m_moveHintAnim[count] != nullptr) {
                m_moveHintRenderObj[count]->Set_Animation(m_moveHintAnim[count], 0.0f, RenderObjClass::ANIM_MODE_ONCE);
            }
        }

        float waterz;

        if (g_theTerrainLogic->Align_On_Terrain(0.0f, m_moveHint[count].pos, true, tm) == LAYER_GROUND
            && g_theTerrainLogic->Is_Underwater(m_moveHint[count].pos.x, m_moveHint[count].pos.y, &waterz, nullptr)) {
            Coord3D pos;
            Coord3D n;
            pos.x = m_moveHint[count].pos.x;
            pos.y = m_moveHint[count].pos.y;
            pos.z = waterz;
            n.x = 0.0f;
            n.y = 0.0f;
            n.z = 1.0f;
            Make_Align_To_Normal_Matrix(0.0f, pos, n, tm);
        }

        m_moveHintRenderObj[count]->Set_Transform(tm);

        if (++count >= MAX_MOVE_HINTS) {
            return;
        }
    }

    RenderObjClass *robj = W3DDisplay::s_assetManager->Create_Render_Obj(g_theWriteableGlobalData->m_moveHintName.Str());

    Utf8String anim_name;
    anim_name.Format(
        "%s.%s", g_theWriteableGlobalData->m_moveHintName.Str(), g_theWriteableGlobalData->m_moveHintName.Str());
    HAnimClass *anim = W3DDisplay::s_assetManager->Get_HAnim(anim_name.Str());

    if (robj != nullptr) {
        m_moveHintRenderObj[count] = robj;
        Ref_Ptr_Release(m_moveHintAnim[count]);
        m_moveHintAnim[count] = anim;
        goto label2;
    }
}

void W3DInGameUI::Draw_Attack_Hints(View *view) {}

void W3DInGameUI::Draw_Place_Angle(View *view)
{
    if (m_locater01 == nullptr) {
        m_locater01 = W3DDisplay::s_assetManager->Create_Render_Obj("Locater01");

        if (m_locater01 == nullptr) {
            return;
        }
    }

    if (m_locater02 == nullptr) {
        m_locater02 = W3DDisplay::s_assetManager->Create_Render_Obj("Locater02");

        if (m_locater02 == nullptr) {
            return;
        }
    }

    bool locater1_in_scene = m_locater01->Peek_Scene() != nullptr;
    bool locater2_in_scene = m_locater02->Peek_Scene() != nullptr;

    if (!Is_Placement_Anchored()) {
        if (locater1_in_scene) {
            W3DDisplay::s_3DScene->Remove_Render_Object(m_locater01);
        }

        if (locater2_in_scene) {
            W3DDisplay::s_3DScene->Remove_Render_Object(m_locater02);
        }

        return;
    }

    ICoord2D start;
    ICoord2D end;
    Get_Placement_Points(&start, &end);

    if (GameMath::Sqrt((end.y - start.y) * (end.y - start.y) + (end.x - start.x) * (end.x - start.x)) < 5.0f) {
        if (locater2_in_scene) {
            W3DDisplay::s_3DScene->Remove_Render_Object(m_locater02);
        }

        if (!locater1_in_scene) {
            W3DDisplay::s_3DScene->Add_Render_Object(m_locater01);
        }

        if (m_placeIcon[0] != nullptr) {
            m_locater01->Set_Transform(*m_placeIcon[0]->Get_Transform_Matrix());
        }

        return;
    }

    if (locater1_in_scene) {
        W3DDisplay::s_3DScene->Remove_Render_Object(m_locater01);
    }

    if (!locater2_in_scene) {
        W3DDisplay::s_3DScene->Add_Render_Object(m_locater02);
    }

    if (locater1_in_scene) {
        if (m_placeIcon[0] != nullptr) {
            m_locater01->Set_Transform(*m_placeIcon[0]->Get_Transform_Matrix());
        }

        return;
    }

    if (m_placeIcon[0] != nullptr) {
        m_locater02->Set_Transform(*m_placeIcon[0]->Get_Transform_Matrix());
    }
}
