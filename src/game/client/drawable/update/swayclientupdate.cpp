/**
 * @file
 *
 * @author xezon
 *
 * @brief Sway client update class. Used for foliage movements.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "swayclientupdate.h"
#include "drawable.h"
#include "matrix3d.h"
#include "object.h"
#include "randomvalue.h"
#include "scriptengine.h"

SwayClientUpdate::SwayClientUpdate(Thing *thing, const ModuleData *module_data) :
    ClientUpdateModule(thing, module_data),
    m_curValue(0.0f),
    m_curAngle(0.0f),
    m_curDelta(0.0f),
    m_curAngleLimit(0.0f),
    m_leanAngle(0.0f),
    m_curVersion(-1),
    m_swaying(true),
    m_unused(false)
{
}

SwayClientUpdate *SwayClientUpdate::Friend_New_Module_Instance(Thing *thing, const ModuleData *module_data)
{
    return new SwayClientUpdate(thing, module_data);
}

void SwayClientUpdate::CRC_Snapshot(Xfer *xfer)
{
    ClientUpdateModule::CRC_Snapshot(xfer);
}

void SwayClientUpdate::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    DrawableModule::Xfer_Snapshot(xfer);
    xfer->xferReal(&m_curValue);
    xfer->xferReal(&m_curAngle);
    xfer->xferReal(&m_curDelta);
    xfer->xferReal(&m_curAngleLimit);
    xfer->xferReal(&m_leanAngle);
    xfer->xferShort(&m_curVersion);
    xfer->xferBool(&m_swaying);
}

void SwayClientUpdate::Load_Post_Process()
{
    ClientUpdateModule::Load_Post_Process();
    Update_Sway();
}

NameKeyType SwayClientUpdate::Get_Module_Name_Key() const
{
    static const NameKeyType _key = Name_To_Key("SwayClientUpdate");
    return _key;
}

void SwayClientUpdate::Client_Update()
{
    if (m_swaying) {
        Drawable *drawable = Get_Drawable();
        const BreezeInfo *breeze = g_theScriptEngine->Get_Breeze_Info();

        if (m_curVersion != breeze->version) {
            Update_Sway();
        } else {
            if (drawable == nullptr || !drawable->Is_Visible()) {
                return;
            }
        }

        m_curValue = m_curValue + m_curDelta;
        if (m_curValue > GAMEMATH_PI2) {
            m_curValue -= GAMEMATH_PI2;
        }
        const float angle = GameMath::Cos(m_curValue);
        const float new_angle = angle * m_curAngleLimit + m_leanAngle;
        const float delta_angle = new_angle - m_curAngle;
        // #TODO Negate -delta_angle to make trees sway in same direction as trees not using this module.
        const float rotx = -delta_angle * breeze->sway_direction.x;
        const float roty = delta_angle * breeze->sway_direction.y;

        Matrix3D new_matrix = drawable->Get_Instance_Matrix();
        new_matrix.In_Place_Pre_Rotate_X(rotx);
        new_matrix.In_Place_Pre_Rotate_Y(roty);

        drawable->Set_Instance_Matrix(&new_matrix);
        m_curAngle = new_angle;
        const Object *object = drawable->Get_Object();

        if (object != nullptr) {
            const BitFlags<OBJECT_STATUS_COUNT> &status_bits = object->Get_Status_Bits();
            if (status_bits.Test(OBJECT_STATUS_BURNED)) {
                Stop_Sway();
            }
        }
    }
}

void SwayClientUpdate::Update_Sway()
{
    const BreezeInfo *breeze = g_theScriptEngine->Get_Breeze_Info();

    if (breeze->randomness == 0.0f) {
        m_curValue = 0.0f;
    }

    const float half_randomness = breeze->randomness * 0.5f;
    const float hi = 1.0f + half_randomness;
    const float lo = 1.0f - half_randomness;
    const float period = GAMEMATH_PI2 / (float)breeze->period;

    m_curAngleLimit = Get_Client_Random_Value_Real(lo, hi) * breeze->intensity;
    m_curDelta = Get_Client_Random_Value_Real(lo, hi) * period;
    m_leanAngle = Get_Client_Random_Value_Real(lo, hi) * breeze->lean;
    m_curVersion = breeze->version;
}

void SwayClientUpdate::Stop_Sway()
{
    m_swaying = false;
}
