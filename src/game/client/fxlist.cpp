/**
 * @file
 *
 * @author Duncans_Pumpkin
 *
 * @brief FXList objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "fxlist.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "object.h"
#include "player.h"

#ifndef GAME_DLL
FXListStore *g_theFXListStore = nullptr;
#endif

// zh: 0x004CC200 wb: 0x0075F326
FXList *FXListStore::Find_FXList(const char *name)
{
    if (strcasecmp(name, "None") == 0) {
        return nullptr;
    }
    const auto key = g_theNameKeyGenerator->Name_To_Key(name);
    auto res = m_fxmap.find(key);
    if (res == m_fxmap.end()) {
        return nullptr;
    }
    return &res->second;
}

// clang-format off
static const FieldParse s_theFXListFieldParse[] = {
    { "Sound",              &SoundFXNugget::Parse,             nullptr, 0 },
#ifdef GAME_DLL
    { "RayEffect",          reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CA610, 0x007600D0)) /*&RayEffectFXNugget::Parse */,         nullptr, 0 },
    { "Tracer",             reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CA100, 0x0075FA00)) /*&TracerFXNugget::Parse */,            nullptr, 0 },
    { "LightPulse",         reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CA920, 0x00760640)) /*&LightPulseFXNugget::Parse */,        nullptr, 0 },
    { "ViewShake",          reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CAB00, 0x00760B20)) /*&ViewShakeFXNugget::Parse */,         nullptr, 0 },
    { "TerrainScorch",      reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CAC80, 0x00760F20)) /*&TerrainScorchFXNugget::Parse */,     nullptr, 0 },
    { "ParticleSystem",     reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CAE10, 0x00761350)) /*&ParticleSystemFXNugget::Parse */,    nullptr, 0 },
    { "FXListAtBonePos",    reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004CB8E0, 0x00761D00)) /*&FXListAtBonePosFXNugget::Parse */,   nullptr, 0 },
#else // DUMMIES
    // { "RayEffect",          &RayEffectFXNugget::Parse,         nullptr, 0 },
    { "Tracer",             &TracerFXNugget::Parse,            nullptr, 0 },
    { "LightPulse",         &LightPulseFXNugget::Parse,        nullptr, 0 },
    { "ViewShake",          &ViewShakeFXNugget::Parse,         nullptr, 0 },
    { "TerrainScorch",      &TerrainScorchFXNugget::Parse,     nullptr, 0 },
    { "ParticleSystem",     &ParticleSystemFXNugget::Parse,    nullptr, 0 },
    { "FXListAtBonePos",    &FXListAtBonePosFXNugget::Parse,    nullptr, 0 },
#endif
    { nullptr,              nullptr,                                                                                                        nullptr, 0 },
};
// clang-format on

// zh: 0x004CC260 wb: 0x0075F3A5
void FXListStore::Parse_FXList_Definition(INI *ini)
{
    const auto key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    auto &fx_list = g_theFXListStore->m_fxmap[key];

    fx_list.Clear();
    ini->Init_From_INI(&fx_list, s_theFXListFieldParse);
}

// zh: 0x0041CCB0 wb: 0x007A3A51
void FXList::Parse(INI *ini, void *, void *store, const void *)
{
    const auto *fx_name = ini->Get_Next_Token();
    auto *fx_list = g_theFXListStore->Find_FXList(fx_name);

    captainslog_dbgassert(fx_list != nullptr || strcasecmp(fx_name, "None") == 0, "FXList %s not found!\n", fx_name);

    *static_cast<FXList **>(store) = fx_list;
}

void FXList::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
#ifdef GAME_DLL
    Call_Method<void, const FXList, const Coord3D *, const Matrix3D *, float, const Coord3D *, float>(
        0x004CBE70, this, primary, primary_mtx, primary_speed, secondary, radius);
#endif
}

// zh: 0x004C9E00 wb: 0x0075EEB0
void FXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    const Coord3D *primary_pos = nullptr;
    const Coord3D *secondary_pos = nullptr;
    const Matrix3D *primary_mtx = nullptr;
    if (primary != nullptr) {
        primary_pos = primary->Get_Position();
        primary_mtx = primary->Get_Transform_Matrix();
    }
    if (secondary != nullptr) {
        secondary_pos = secondary->Get_Position();
    }
    Do_FX_Pos(primary_pos, primary_mtx, 0.0f, secondary_pos, 0.0f);
}

// zh: 0x004C9F20 wb: 0x0075F800
void SoundFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    AudioEventRTS audio_event{ m_soundName };
    if (primary != nullptr) {
        audio_event.Set_Position(primary);
    }
    g_theAudio->Add_Audio_Event(&audio_event);
}

// zh: 0x004C9FA0 wb: 0x0075F880
void SoundFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    AudioEventRTS audio_event{ m_soundName };
    if (primary != nullptr) {
        audio_event.Set_Player_Index(primary->Get_Controlling_Player()->Get_Player_Index());
        audio_event.Set_Position(primary->Get_Position());
    }
    g_theAudio->Add_Audio_Event(&audio_event);
}

// zh: 0x004C9E50 wb: 0x0075F410
void SoundFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "Name", INI::Parse_AsciiString, nullptr, offsetof(SoundFXNugget, m_soundName) },
        { nullptr, nullptr, nullptr, 0 },
    };

    SoundFXNugget *nugget = new SoundFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}

void TracerFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    captainslog_dbgassert(false, "TracerFXNugget::Do_FX_Pos not implemented!");
}

void TracerFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    captainslog_dbgassert(false, "TracerFXNugget::Do_FX_Obj not implemented!");
}

void TracerFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "DecayAt", &INI::Parse_Int, nullptr, offsetof(TracerFXNugget, m_decayAt) },
        { "Length", &INI::Parse_Int, nullptr, offsetof(TracerFXNugget, m_length) },
        { "Width", &INI::Parse_Real, nullptr, offsetof(TracerFXNugget, m_width) },
        { "Color", &INI::Parse_RGB_Color, nullptr, offsetof(TracerFXNugget, m_color) },
        { "Speed", &INI::Parse_Int, nullptr, offsetof(TracerFXNugget, speed) },
        { "Probability", &INI::Parse_Real, nullptr, offsetof(TracerFXNugget, probability) },
        { nullptr, nullptr, nullptr, 0 },
    };

    TracerFXNugget *nugget = new TracerFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}

void LightPulseFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    captainslog_dbgassert(false, "LightPulseFXNugget::Do_FX_Pos not implemented!");
}

void LightPulseFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    captainslog_dbgassert(false, "LightPulseFXNugget::Do_FX_Obj not implemented!");
}

void LightPulseFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "Color", &INI::Parse_RGB_Color, nullptr, offsetof(LightPulseFXNugget, m_color) },
        { "Radius", &INI::Parse_Int, nullptr, offsetof(LightPulseFXNugget, m_radius) },
        { "RadiusAsPercentOfObjectSize",
            &INI::Parse_Percent_To_Real,
            nullptr,
            offsetof(LightPulseFXNugget, m_radiusAsPercentOfObjectSize) },
        { "IncreaseTime", &INI::Parse_Int, nullptr, offsetof(LightPulseFXNugget, m_increaseTime) },
        { "DecreaseTime", &INI::Parse_Int, nullptr, offsetof(LightPulseFXNugget, m_decreaseTime) },
        { nullptr, nullptr, nullptr, 0 },
    };

    LightPulseFXNugget *nugget = new LightPulseFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}

void ViewShakeFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    captainslog_dbgassert(false, "ViewShakeFXNugget::Do_FX_Pos not implemented!");
}

void ViewShakeFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    captainslog_dbgassert(false, "ViewShakeFXNugget::Do_FX_Obj not implemented!");
}

void ViewShakeFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "Type", &INI::Parse_Index_List, g_shakeIntensityNames, offsetof(ViewShakeFXNugget, m_type) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ViewShakeFXNugget *nugget = new ViewShakeFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}

void TerrainScorchFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    captainslog_dbgassert(false, "TerrainScorchFXNugget::Do_FX_Pos not implemented!");
}

void TerrainScorchFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    captainslog_dbgassert(false, "TerrainScorchFXNugget::Do_FX_Obj not implemented!");
}

void TerrainScorchFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "Type", &INI::Parse_AsciiString, nullptr, offsetof(TerrainScorchFXNugget, m_type) },
        { "Radius", &INI::Parse_Int, nullptr, offsetof(TerrainScorchFXNugget, m_radius) },
        { nullptr, nullptr, nullptr, 0 },
    };

    TerrainScorchFXNugget *nugget = new TerrainScorchFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}

void ParticleSystemFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    captainslog_dbgassert(false, "ParticleSystemFXNugget::Do_FX_Pos not implemented!");
}

void ParticleSystemFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    captainslog_dbgassert(false, "ParticleSystemFXNugget::Do_FX_Obj not implemented!");
}

void ParticleSystemFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "Name", &INI::Parse_AsciiString, nullptr, offsetof(ParticleSystemFXNugget, m_sysName) },
        { "Count", &INI::Parse_Int, nullptr, offsetof(ParticleSystemFXNugget, m_count) },
        { "Radius", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemFXNugget, m_radius) },
        { "InitialDelay", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemFXNugget, m_initialDelay) },
        { "AttachToObject", &INI::Parse_Bool, nullptr, offsetof(ParticleSystemFXNugget, m_attachToObject) },
        { "Offset", &INI::Parse_Coord3D, nullptr, offsetof(ParticleSystemFXNugget, m_offset) },
        { "Height", &GameClientRandomVariable::Parse, nullptr, offsetof(ParticleSystemFXNugget, m_height) },
        { "OrientToObject", &INI::Parse_Bool, nullptr, offsetof(ParticleSystemFXNugget, m_orientToObject) },
        { "RotateY", &INI::Parse_Angle_Real, nullptr, offsetof(ParticleSystemFXNugget, m_rotateY) },
        { "Ricochet", &INI::Parse_Bool, nullptr, offsetof(ParticleSystemFXNugget, m_ricochet) },
        { "CreateAtGroundHeight", &INI::Parse_Bool, nullptr, offsetof(ParticleSystemFXNugget, m_createAtGroundHeight) },
        { "UseCallersRadius", &INI::Parse_Bool, nullptr, offsetof(ParticleSystemFXNugget, m_useCallersRadius) },
        { nullptr, nullptr, nullptr, 0 },
    };

    ParticleSystemFXNugget *nugget = new ParticleSystemFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}

void FXListAtBonePosFXNugget::Do_FX_Pos(
    const Coord3D *primary, const Matrix3D *primary_mtx, float primary_speed, const Coord3D *secondary, float radius) const
{
    captainslog_dbgassert(false, "FXListAtBonePosFXNugget::Do_FX_Pos not implemented!");
}

void FXListAtBonePosFXNugget::Do_FX_Obj(const Object *primary, const Object *secondary) const
{
    captainslog_dbgassert(false, "FXListAtBonePosFXNugget::Do_FX_Obj not implemented!");
}

void FXListAtBonePosFXNugget::Parse(INI *ini, void *formal, void *, const void *)
{
    static const FieldParse _fieldParse[] = {
        { "FX", &FXList::Parse, nullptr, offsetof(FXListAtBonePosFXNugget, m_fx) },
        { "BoneName", &INI::Parse_AsciiString, nullptr, offsetof(FXListAtBonePosFXNugget, m_boneName) },
        { "OrientToBone", &INI::Parse_Bool, nullptr, offsetof(FXListAtBonePosFXNugget, m_orientToBone) },
        { nullptr, nullptr, nullptr, 0 },
    };

    FXListAtBonePosFXNugget *nugget = new FXListAtBonePosFXNugget{};
    ini->Init_From_INI(nugget, _fieldParse);
    reinterpret_cast<FXList *>(formal)->Add_FXNugget(nugget);
}