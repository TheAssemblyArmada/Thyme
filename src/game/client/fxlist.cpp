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
