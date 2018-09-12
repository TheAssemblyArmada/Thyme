/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Tracks prerequisites needed for a thing to be built.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef PRODUCTIONPREREQUISITE_H
#define PRODUCTIONPREREQUISITE_H

#include "always.h"
#include "asciistring.h"
#include "science.h"
#include <vector>

class ThingTemplate;

class ProductionPrerequisite
{
    struct PrereqUnitRec
    {
        ThingTemplate *unit;
        int flags;
        Utf8String name;
    };
public:
    ProductionPrerequisite() {}
    ~ProductionPrerequisite() {}

    // TODO member functions.
private:
    std::vector<PrereqUnitRec> m_prereqUnit;
    std::vector<ScienceType> m_prereqScience;
};

#endif // PRODUCTIONPREREQUISITE_H
