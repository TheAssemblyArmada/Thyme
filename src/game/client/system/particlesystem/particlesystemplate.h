/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Template object that particle systems are instantiated from.
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
#include "asciistring.h"
#include "mempoolobj.h"
#include "particlesysinfo.h"

class INI;
class ParticleSystem;

class ParticleSystemTemplate : public MemoryPoolObject, public ParticleSystemInfo
{
    friend class ParticleSystem;
    IMPLEMENT_NAMED_POOL(ParticleSystemTemplate, ParticleSystemTemplatePool);
    
public:
    ParticleSystemTemplate(const AsciiString &name) : m_name(name), m_slaveTemplate(nullptr) {}
    virtual ~ParticleSystemTemplate() {}

    static void Parse_Random_Keyframe(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_RGB_Color_Keyframe(INI *ini, void *formal, void *store, const void *user_data);

    AsciiString Get_Name() const { return m_name; }

private:
    ParticleSystem *Create_Slave_System(bool create_slaves) const;

private:
    AsciiString m_name;
    mutable ParticleSystemTemplate *m_slaveTemplate;
};
