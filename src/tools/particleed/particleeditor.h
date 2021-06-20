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
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool HasRequestedKillAllSystems();
bool HasRequestedReload();
void AppendParticleSystem(const char *sysname);
void AppendThingTemplate(const char *sysname);
void CreateParticleSystemDialog();
void DestroyParticleSystemDialog();
int32_t GetNewParticleCap();
void GetSelectedParticleAsciiStringParm(int32_t entry, char *name, void **system);
void GetSelectedParticleSystemName(char *name);
bool HasUpdatedSelectedParticleSystem();
int32_t NextParticleEditorBehavior();
void RemoveAllParticleSystems();
void RemoveAllThingTemplates();
bool ShouldBusyWait();
bool ShouldReloadTextures();
bool ShouldUpdateParticleCap();
bool ShouldWriteINI();
void UpdateCurrentNumParticles(int32_t currentnum);
void UpdateCurrentParticleCap(int32_t cap);
void UpdateCurrentParticleSystem(void *system);
void UpdateParticleAsciiStringParm(int32_t entry, char *name, void **system);
void UpdateSystemUseParameters(void *system);

#ifdef __cplusplus
} // extern "C"
#endif
