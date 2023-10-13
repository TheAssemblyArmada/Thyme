/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar Scheme
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "controlbarscheme.h"

ControlBarSchemeImage::~ControlBarSchemeImage()
{
    m_image = nullptr;
}

ControlBarSchemeAnimation::~ControlBarSchemeAnimation()
{
    m_animImage = nullptr;
}

ControlBarScheme::~ControlBarScheme()
{
    Reset();
}

void ControlBarScheme::Reset()
{
    for (int i = 0; i < LAYER_COUNT; i++) {
        for (auto it = m_layer[i].begin(); it != m_layer[i].end(); it++) {
            if (*it != nullptr) {
                delete *it;
            }
        }

        m_layer[i].clear();
    }

    for (auto it = m_animations.begin(); it != m_animations.end(); it++) {
        if (*it != nullptr) {
            delete *it;
        }
    }

    m_animations.clear();
    m_name.Clear();
    m_ScreenCreationRes.x = 0.0f;
    m_ScreenCreationRes.y = 0.0f;
    m_side.Clear();
    m_buttonQueueImage = nullptr;
    m_rightHUDImage = nullptr;
    m_optionsButtonEnableImage = nullptr;
    m_optionsButtonHilitedImage = nullptr;
    m_optionsButtonPushedImage = nullptr;
    m_optionsButtonDisabledImage = nullptr;
    m_idleWorkerButtonEnableImage = nullptr;
    m_idleWorkerButtonHilitedImage = nullptr;
    m_idleWorkerButtonPushedImage = nullptr;
    m_idleWorkerButtonDisabledImage = nullptr;
    m_buddyButtonEnableImage = nullptr;
    m_buddyButtonHilitedImage = nullptr;
    m_buddyButtonPushedImage = nullptr;
    m_buddyButtonDisabledImage = nullptr;
    m_beaconButtonEnableImage = nullptr;
    m_beaconButtonHilitedImage = nullptr;
    m_beaconButtonPushedImage = nullptr;
    m_beaconButtonDisabledImage = nullptr;
    m_genBarButtonInImage = nullptr;
    m_genBarButtonOnImage = nullptr;
    m_toggleButtonUpInImage = nullptr;
    m_toggleButtonUpOnImage = nullptr;
    m_toggleButtonUpPushedImage = nullptr;
    m_toggleButtonDownInImage = nullptr;
    m_toggleButtonDownOnImage = nullptr;
    m_toggleButtonDownPushedImage = nullptr;
    m_commandMarkerImage = nullptr;
    m_expBarForegroundImage = nullptr;
    m_powerPurchaseImage = nullptr;
}

void ControlBarScheme::Update()
{
    for (auto it = m_animations.begin(); it != m_animations.end(); it++) {
        ControlBarSchemeAnimation *anim = *it;

        if (anim == nullptr) {
            captainslog_dbgassert(false, "There's no Animation in the ControlBarSchemeAnimationList:m_animations");
            return;
        }

        Update_Anim(anim);
    }
}

void Anim_Slide_Right(ControlBarSchemeAnimation *anim)
{
    if (anim->m_animImage != nullptr && anim->m_animDuration != 0) {
        unsigned int frame = anim->Get_Current_Frame();
        ICoord2D pos = anim->Get_Start_Pos();

        if (frame == anim->m_animDuration) {
            anim->m_animImage->m_position = pos;
            anim->Set_Current_Frame(0);
        } else {
            if (frame == 0) {
                pos = anim->m_animImage->m_position;
                anim->Set_Start_Pos(pos);
            }

            frame++;
            anim->Set_Current_Frame(frame);
            anim->m_animImage->m_position.x = frame * (anim->m_finalPos.x - pos.x) / anim->m_animDuration + pos.x;
        }
    }
}

void ControlBarScheme::Update_Anim(ControlBarSchemeAnimation *anim)
{
    if (anim->m_animType != ControlBarSchemeAnimation::CB_ANIM_SLIDE_RIGHT) {
        captainslog_dbgassert(false, "We tried to animate but not animate function was found %d", anim->m_animType);
    } else {
        Anim_Slide_Right(anim);
    }
}

ControlBarSchemeManager::ControlBarSchemeManager() : m_currentScheme(nullptr)
{
    m_multiplyer.x = 1.0f;
    m_multiplyer.y = 1.0f;
}

void ControlBarSchemeManager::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Default\\ControlBarScheme.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data\\INI\\ControlBarScheme.ini", INI_LOAD_OVERWRITE, nullptr);
    captainslog_dbgassert(m_schemeList.size() != 0,
        "There's no ControlBarScheme in the ControlBarSchemeList:m_schemeList that was just read from the INI file");
}

ControlBarSchemeManager::~ControlBarSchemeManager()
{
    for (auto it = m_schemeList.begin(); it != m_schemeList.end(); it++) {
        if (*it != nullptr) {
            delete *it;
        }
    }

    m_schemeList.clear();
    m_currentScheme = nullptr;
}

void ControlBarSchemeManager::Update()
{
    if (m_currentScheme != nullptr) {
        m_currentScheme->Update();
    }
}
