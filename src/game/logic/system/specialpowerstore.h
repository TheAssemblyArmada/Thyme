/**
 * @file
 *
 * @author xezon
 *
 * @brief Special Power Store
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "subsysteminterface.h"
#include <vector>

class SpecialPowerStore;
class SpecialPowerTemplate;
class Object;
class INI;

#ifdef GAME_DLL
extern SpecialPowerStore *&TheSpecialPowerStore;
#else
extern SpecialPowerStore *TheSpecialPowerStore;
#endif

class SpecialPowerStore : public SubsystemInterface
{
    using SpecialPowerTemplates = std::vector<SpecialPowerTemplate *>;

public:
    SpecialPowerStore();

    virtual ~SpecialPowerStore() override;

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    static void Parse_Special_Power_Definition(INI *ini);

    unsigned int Get_Num_Special_Powers() const;
    const SpecialPowerTemplate *Find_SpecialPowerTemplate_By_Name(Utf8String name) const;
    const SpecialPowerTemplate *Find_SpecialPowerTemplate_By_ID(unsigned int id) const;
    const SpecialPowerTemplate *Get_SpecialPowerTemplate_By_Index(unsigned int index) const;

    static bool Can_Use_Special_Power(Object *object, const SpecialPowerTemplate *ptemplate);

private:
    SpecialPowerTemplate *Find_SpecialPowerTemplate_Private(Utf8String name);

private:
    SpecialPowerTemplates m_specialPowerTemplates;
    unsigned int m_nextSpecialPowerID;
};
