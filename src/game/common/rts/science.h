////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SCIENCE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Science handling classes.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef SCIENCE_H
#define SCIENCE_H

#include "hooker.h"
#include "mempoolobj.h"
#include "overridable.h"
#include "subsysteminterface.h"
#include <vector>

class ScienceInfo : public Overridable
{
    IMPLEMENT_POOL(ScienceInfo);
public:

private:

};

class ScienceStore : public SubsystemInterface
{
public:
    virtual ~ScienceStore() {}

    virtual void Init() override { m_infoVec.clear(); }
    virtual void Reset() override;
    virtual void Update() override {}

private:
    std::vector<ScienceInfo*> m_infoVec;
};

#define g_theScienceStore (Make_Global<ScienceStore*>(0x00A2B978))
// extern ScienceStore *g_theScienceStore;

#endif // SCIENCE_H
