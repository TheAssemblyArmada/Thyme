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

#ifndef GAME_DLL
FXListStore *g_theFXListStore = nullptr;
#endif

// zh: 0x004CC200 wb: 0x0075F326
FXList *FXListStore::Find_FXList(const char *name)
{
    Utf8String str(name);

    if (str.Is_None()) {
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
static FieldParse s_theFXListFieldParse[] = {
#ifdef GAME_DLL
    { "Sound",              reinterpret_cast<inifieldparse_t>(PICK_ADDRESS(0x004C9E50, 0x0075F410)) /*&SoundFXNugget::Parse */,             nullptr, 0 },
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
    Utf8String fx_name(ini->Get_Next_Token());
    auto *fx_list = g_theFXListStore->Find_FXList(fx_name.Str());

    captainslog_dbgassert(fx_list != nullptr || fx_name.Is_None(), "FXList %s not found!\n", fx_name.Str());

    *static_cast<FXList **>(store) = fx_list;
}
