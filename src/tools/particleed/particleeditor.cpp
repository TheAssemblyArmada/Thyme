/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief API for the Zero Hour particle editor dll.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particleeditor.h"
#include "partedapp.h"
#include "partedframe.h"
#include <cstdio>
#include <wx/combobox.h>
#include <wx/dynlib.h>
#include <wx/msgdlg.h>
#include <captainslog.h>

bool HasRequestedKillAllSystems()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Has_Requested_Kill_All();
    }

    return false;
}

bool HasRequestedReload()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Has_Requested_Reload();
    }

    return false;
}

void AppendParticleSystem(const char *systemname)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("Appending '%s' to particle system list.", systemname);
        wxframe->Append_System(systemname);
    }
}

void AppendThingTemplate(const char *thing)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Thing(thing);
    }
}

int32_t GetNewParticleCap()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Get_New_Particle_Cap();
    }

    return -1;
}

void GetSelectedParticleAsciiStringParm(int32_t entry, char *name, void **temp)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Get_Selected_Particle_AsciiString_Parm(entry, name, temp);
    }
}

void GetSelectedParticleSystemName(char *name)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Get_Selected_Particle_System_Name(name);
    }
}

bool HasUpdatedSelectedParticleSystem()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Has_Updated_Selected_Particle_System();
    }

    return false;
}

int32_t NextParticleEditorBehavior()
{
    if (HasUpdatedSelectedParticleSystem()) {
        captainslog_info("Next behaviour is HasUpdatedSelectedParticleSystem");
        return 1;
    }
    if (ShouldWriteINI()) {
        captainslog_info("Next behaviour is ShouldWriteINI");
        return 3;
    }
    if (HasRequestedReload()) {
        captainslog_info("Next behaviour is HasRequestedReload");
        return 4;
    }
    if (ShouldUpdateParticleCap()) {
        captainslog_info("Next behaviour is ShouldUpdateParticleCap");
        return 5;
    }
    if (ShouldReloadTextures()) {
        captainslog_info("Next behaviour is ShouldReloadTextures");
        return 6;
    }
    if (HasRequestedKillAllSystems()) {
        captainslog_info("Next behaviour is HasRequestedKillAllSystems");
        return 7;
    }
    if (ShouldBusyWait()) {
        captainslog_info("Next behaviour is ShouldBusyWait");
        return 254;
    }
    captainslog_info("Next behaviour is <none>");
    return 0;
}

void RemoveAllParticleSystems()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Has_Updated_Selected_Particle_System();
    }
}

void RemoveAllThingTemplates()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Remove_All_Thing_Templates();
    }
}

bool ShouldBusyWait()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Should_Busy_Wait();
    }

    return false;
}

bool ShouldReloadTextures()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Should_Reload_Textures();
    }

    return false;
}

bool ShouldUpdateParticleCap()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Should_Update_Particle_Cap();
    }

    return false;
}

bool ShouldWriteINI()
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        return wxframe->Should_Write_INI();
    }

    return false;
}

void UpdateCurrentNumParticles(int32_t currentnum)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();
    char ParticleCurrNum[128];
    snprintf(ParticleCurrNum, sizeof(ParticleCurrNum), "%d", currentnum);
    wxTextCtrl *pText = (wxTextCtrl *)(g_mainWindow->FindWindowByName("m_curCount"));

    if (pText != nullptr) {
        pText->ChangeValue(ParticleCurrNum);
    }
}

void UpdateCurrentParticleCap(int32_t cap)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("Updating paricle cap to %" PRId32, cap);
        wxframe->Update_Current_Particle_Cap(cap);
    }
}

void UpdateCurrentParticleSystem(void *system)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Update_Current_Particle_System(system);
    }
}

void UpdateParticleAsciiStringParm(int32_t entry, char *name, void **system)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Update_Particle_AsciiString_Parm(entry, name, system);
    }
}

void UpdateSystemUseParameters(void *system)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        captainslog_info("%s", __CURRENT_FUNCTION__);
        wxframe->Update_System_Use_Parameters(system);
    }
}
