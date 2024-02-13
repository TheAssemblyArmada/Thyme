/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Object Defection Helper
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "objectdefectionhelper.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "color.h"
#include "drawable.h"
#include "gamelogic.h"

ObjectDefectionHelper::ObjectDefectionHelper(Thing *thing, const ModuleData *module_data) :
    ObjectHelper(thing, module_data), m_startFrame(0), m_endFrame(0), m_effectTimer(0.0f), m_doEffect(false)
{
}

ObjectDefectionHelper::~ObjectDefectionHelper() {}

NameKeyType ObjectDefectionHelper::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("ObjectDefectionHelper");
    return _key;
}

void ObjectDefectionHelper::CRC_Snapshot(Xfer *xfer)
{
    ObjectHelper::CRC_Snapshot(xfer);
}

void ObjectDefectionHelper::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    ObjectHelper::Xfer_Snapshot(xfer);
    xfer->xferUnsignedInt(&m_startFrame);
    xfer->xferUnsignedInt(&m_endFrame);
    xfer->xferReal(&m_effectTimer);
    xfer->xferBool(&m_doEffect);
}

void ObjectDefectionHelper::Load_Post_Process()
{
    ObjectHelper::Load_Post_Process();
}

BitFlags<DISABLED_TYPE_COUNT> ObjectDefectionHelper::Get_Disabled_Types_To_Process() const
{
    return DISABLEDMASK_ALL;
}

UpdateSleepTime ObjectDefectionHelper::Update()
{
    Object *obj = Get_Object();
    Drawable *drawable = obj->Get_Drawable();

    if (!obj->Is_Undetected_Defector()) {
        return UPDATE_SLEEP_TIME_MAX;
    }

    unsigned int frame = g_theGameLogic->Get_Frame();

    if (frame < m_endFrame) {
        if (obj->Is_Effectively_Dead() || obj->Get_Status_Bits().Test(OBJECT_STATUS_IS_FIRING_WEAPON)) {
            obj->Friend_Set_Undetected_Defector(false);
            return UPDATE_SLEEP_TIME_MAX;
        } else {
            if (drawable != nullptr) {
                if (m_doEffect) {
                    bool b1 = static_cast<int>(m_effectTimer) & 1;
                    m_effectTimer += (1.0f - (m_endFrame - frame) / 300.0f) * 0.5f;
                    bool b2 = static_cast<int>(m_effectTimer) & 1;

                    if (b1 && !b2) {
                        drawable->Flash_As_Selected(nullptr);
                        AudioEventRTS sound(g_theAudio->Get_Misc_Audio()->m_defectorTimerTick);
                        sound.Set_Object_ID(obj->Get_ID());
                        g_theAudio->Add_Audio_Event(&sound);
                    }
                }
            }

            return UPDATE_SLEEP_TIME_MIN;
        }
    } else {
        obj->Friend_Set_Undetected_Defector(false);

        if (drawable != nullptr && m_doEffect) {
            RGBColor color;
            color.red = 1.0f;
            color.green = 1.0f;
            color.blue = 1.0f;
            drawable->Flash_As_Selected(&color);
            AudioEventRTS sound(g_theAudio->Get_Misc_Audio()->m_defectorTimerDing);
            sound.Set_Object_ID(obj->Get_ID());
            g_theAudio->Add_Audio_Event(&sound);
        }

        return UPDATE_SLEEP_TIME_MAX;
    }
}

void ObjectDefectionHelper::Start_Defection_Timer(unsigned int timer, bool do_effect)
{
    Object *obj = Get_Object();

    if (obj->Is_Undetected_Defector()) {
        unsigned int frame = g_theGameLogic->Get_Frame();
        m_startFrame = frame;
        m_endFrame = frame + timer;
        m_effectTimer = 0.0f;
        m_doEffect = do_effect;
        Set_Wake_Frame(obj, UPDATE_SLEEP_TIME_MIN);
    } else {
        Set_Wake_Frame(obj, UPDATE_SLEEP_TIME_MAX);
    }
}
