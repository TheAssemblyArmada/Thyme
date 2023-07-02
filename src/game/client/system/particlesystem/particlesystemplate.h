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
struct FieldParse;

class ParticleSystemTemplate : public MemoryPoolObject, public ParticleSystemInfo
{
    friend class ParticleSystem;
    IMPLEMENT_NAMED_POOL(ParticleSystemTemplate, ParticleSystemTemplatePool);

protected:
    virtual ~ParticleSystemTemplate() override {}

public:
    ParticleSystemTemplate(const Utf8String &name) : m_name(name), m_slaveTemplate(nullptr) {}

    static void Parse_Random_Keyframe(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Random_RGB_Color(INI *ini, void *formal, void *store, const void *user_data) {}
    static void Parse_RGB_Color_Keyframe(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse(INI *ini, void *, void *store, const void *);

    Utf8String Get_Name() const { return m_name; }

    static const FieldParse *Get_Particle_System_Template_Field_Parse_Table() { return s_fieldParseTable; }

private:
    ParticleSystem *Create_Slave_System(bool create_slaves) const;

private:
    Utf8String m_name;
    mutable ParticleSystemTemplate *m_slaveTemplate;

    static const FieldParse s_fieldParseTable[];
};
