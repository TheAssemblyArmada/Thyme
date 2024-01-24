/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radius Decal
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "radiusdecal.h"
#include "gamelogic.h"
#include "ini.h"
#include "player.h"
#include "playerlist.h"
#include "w3dprojectedshadow.h"
#include "xfer.h"

void RadiusDecalTemplate::Create_Radius_Decal(
    const Coord3D *pos, float radius, const Player *owning_player, RadiusDecal *decal)
{
    decal->Clear();

    if (owning_player != nullptr) {
        if (!m_texture.Is_Empty() && radius > 0.0f) {
            decal->m_needsInit = false;

            if (!m_onlyVisibleToOwningPlayer
                || owning_player->Get_Player_Index() == g_thePlayerList->Get_Local_Player()->Get_Player_Index()) {
                Shadow::ShadowTypeInfo info;
                info.m_allowUpdates = false;
                info.m_allowWorldAlign = true;
                info.m_type = m_style;
                strcpy(info.m_shadowName, m_texture.Str());
                info.m_sizeX = radius + radius;
                info.m_sizeY = info.m_sizeX;
                decal->m_shadow = g_theProjectedShadowManager->Add_Decal(&info);

                if (decal->m_shadow != nullptr) {
                    decal->m_shadow->Set_Angle(0.0f);

                    if (m_color != 0) {
                        decal->m_shadow->Set_Color(m_color);
                    } else {
                        decal->m_shadow->Set_Color(owning_player->Get_Color());
                    }

                    decal->m_shadow->Set_Position(pos->x, pos->y, pos->z);
                    decal->m_template = this;
                } else {
                    captainslog_dbgassert(false, "Unable to add decal %s", info.m_shadowName);
                }
            }
        }
    } else {
        captainslog_dbgassert(false, "You MUST specify a non-NULL owning_player to Create_Radius_Decal.");
    }
}

void RadiusDecalTemplate::Xfer_Radius_Decal_Template(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferAsciiString(&m_texture);
    xfer->xferUser(&m_style, sizeof(m_style));
    xfer->xferReal(&m_opacityMin);
    xfer->xferReal(&m_opacityMax);
    xfer->xferUnsignedInt(&m_opacityThrobTime);
    xfer->xferColor(&m_color);
    xfer->xferBool(&m_onlyVisibleToOwningPlayer);
}

static const char *s_theShadowNames[] = { "SHADOW_DECAL",
    "SHADOW_VOLUME",
    "SHADOW_PROJECTION",
    "SHADOW_DYNAMIC_PROJECTION",
    "SHADOW_DIRECTIONAL_PROJECTION",
    "SHADOW_ALPHA_DECAL",
    "SHADOW_ADDITIVE_DECAL",
    nullptr };

void RadiusDecalTemplate::Parse_Radius_Decal_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    static const FieldParse dataFieldParse[] = {
        { "Texture", INI::Parse_AsciiString, nullptr, offsetof(RadiusDecalTemplate, m_texture) },
        { "Style", &INI::Parse_Bitstring32, s_theShadowNames, offsetof(RadiusDecalTemplate, m_style) },
        { "OpacityMin", &INI::Parse_Percent_To_Real, nullptr, offsetof(RadiusDecalTemplate, m_opacityMin) },
        { "OpacityMax", &INI::Parse_Percent_To_Real, nullptr, offsetof(RadiusDecalTemplate, m_opacityMax) },
        { "OpacityThrobTime",
            &INI::Parse_Duration_Unsigned_Int,
            nullptr,
            offsetof(RadiusDecalTemplate, m_opacityThrobTime) },
        { "Color", &INI::Parse_Color_Int, nullptr, offsetof(RadiusDecalTemplate, m_color) },
        { "OnlyVisibleToOwningPlayer",
            &INI::Parse_Bool,
            nullptr,
            offsetof(RadiusDecalTemplate, m_onlyVisibleToOwningPlayer) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ini->Init_From_INI(store, dataFieldParse);
}

void RadiusDecal::Xfer_Radius_Decal(Xfer *xfer)
{
    if (xfer->Get_Mode() == XFER_LOAD) {
        Clear();
    }
}

void RadiusDecal::Clear()
{
    m_template = nullptr;

    if (m_shadow != nullptr) {
        m_shadow->Release();
        m_shadow = nullptr;
    }

    m_needsInit = true;
}

void RadiusDecal::Update()
{
    if (m_shadow != nullptr && m_template != nullptr) {
        float opacity_evo = (g_theGameLogic->Get_Frame() % m_template->m_opacityThrobTime) * DEG_TO_RADF(360.f)
            / m_template->m_opacityThrobTime;
        float opacity_amount = (GameMath::Sin(opacity_evo) + 1.0f) * 0.5f;

        if (g_theGameLogic->Get_Draw_Icon_UI()) {
            m_shadow->Set_Opacity(GameMath::Fast_To_Int_Truncate(
                ((m_template->m_opacityMax - m_template->m_opacityMin) * opacity_amount + m_template->m_opacityMin)
                * 255.0f));
        } else {
            m_shadow->Set_Opacity(0);
        }
    }
}

void RadiusDecal::Set_Opacity(float opacity)
{
    if (m_shadow != nullptr) {
        m_shadow->Set_Opacity(GameMath::Fast_To_Int_Truncate(255.0f * opacity));
    }
}

void RadiusDecal::Set_Position(const Coord3D &pos)
{
    if (m_shadow != nullptr) {
        m_shadow->Set_Position(pos.x, pos.y, pos.z);
    }
}
