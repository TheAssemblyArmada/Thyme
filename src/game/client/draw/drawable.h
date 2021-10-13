/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "module.h"
#include "thing.h"

class DisplayString;
class TintEnvelope;
class DynamicAudioEventInfo;
class DynamicAudioEventRTS;
class DrawableLocoInfo;
class DrawableIconInfo;

class Drawable : public Thing, public SnapShot
{
public:
    enum FadingMode
    {
        FADING_MODE_0,
        FADING_MODE_1,
        FADING_MODE_2,
    };

    const Object *Get_Object() const { return m_object; }
    bool Is_Fully_Obscured_By_Shroud() const { return m_fullyObscuredByShroud; }

    const Matrix3D *Get_Transform_Matrix() const;

private:
    TintEnvelope *m_unk1;
    TintEnvelope *m_unk2;

    TerrainDecalType m_terrainDecal;

    float m_opacity;
    float m_unk3;
    float m_unk4;
    float m_unk5;
    float m_unk6;
    float m_terrainDecalOpacity;

    Object *m_object;

    DrawableID m_id;
    Drawable *m_nextDrawable;
    Drawable *m_prevDrawable;

    DynamicAudioEventInfo *m_customSoundAmbientInfo;

    DrawableStatus m_status;

    unsigned int m_unk7;
    unsigned int m_unk8;

    FadingMode m_fadingMode;
    unsigned int m_unk9;
    unsigned int m_timeToFade;

    unsigned int m_unk10;

    DrawableLocoInfo *m_drawableLocoInfo;

    DynamicAudioEventRTS *m_ambientSound;

    Module **m_modules[NUM_DRAWABLE_MODULE_TYPES];

    StealthLookType m_stealthLook;

    int m_unk11;
    int m_unk12;

    Matrix3D m_instance;
    float m_instanceScale;

    int m_unk13; // TODO proper type, Xfer is done with xferObjectID but ctor inits this as [4]....
    int m_unk14;
    int m_unk15;
    int m_unk16;

    BitFlags<MODELCONDITION_COUNT> m_conditionState;

    float m_lastConstructDisplayed;

    DisplayString *m_constructDisplayString;
    DisplayString *m_captionText;
    DisplayString *m_groupString;

    unsigned int m_expirationDate;

    DrawableIconInfo *m_drawableIconInfo;

    float m_unk17;

    bool m_selected;
    bool m_hidden;
    bool m_unk18;
    bool m_instanceIsIdentity;
    bool m_fullyObscuredByShroud;
    bool m_unk19;
    bool m_unk20;
    bool m_receivesDynamicLights;
    bool m_isModelDirty;
};
