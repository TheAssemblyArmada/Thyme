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
#pragma once

#include "always.h"
#include "color.h"
#include "coord.h"
#include "namekeygenerator.h"
#include "randomvalue.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include <list>

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class Coord3D;
class Matrix3D;
class Object;
class INI;

class FXNugget : public MemoryPoolObject
{
    IMPLEMENT_ABSTRACT_POOL(FXNugget);

public:
    virtual ~FXNugget() {};
    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const = 0;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const;
};

class FXList
{
public:
    virtual ~FXList() { Clear(); }

    FXList() {}

    void Clear() { m_nuggets.clear(); }
    void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const;
    void Do_FX_Obj(const Object *primary, const Object *secondary) const;

    void Add_FXNugget(FXNugget *nugget) { m_nuggets.push_back(nugget); }

    static void Parse(INI *ini, void *, void *store, const void *);

private:
    std::list<FXNugget const *> m_nuggets;
};

#ifdef THYME_USE_STLPORT
using fxlistmap_t = std::hash_map<NameKeyType, FXList, rts::hash<NameKeyType>, std::equal_to<NameKeyType>>;
#else
using fxlistmap_t = std::unordered_map<NameKeyType, FXList, rts::hash<NameKeyType>, std::equal_to<NameKeyType>>;
#endif

class FXListStore : public SubsystemInterface
{
public:
    FXListStore() {}
    virtual ~FXListStore() override {}

    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    FXList *Find_FXList(const char *name);

    static void Parse_FXList_Definition(INI *ini);

private:
    fxlistmap_t m_fxmap;
};

#ifdef GAME_DLL
extern FXListStore *&g_theFXListStore;
#else
extern FXListStore *g_theFXListStore;
#endif

class SoundFXNugget : public FXNugget
{
    IMPLEMENT_POOL(SoundFXNugget);

public:
    SoundFXNugget() {};
    virtual ~SoundFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    Utf8String m_soundName;
};

class TracerFXNugget : public FXNugget
{
    IMPLEMENT_POOL(TracerFXNugget);

public:
    TracerFXNugget() {};
    virtual ~TracerFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    int m_decayAt;
    int m_length;
    float m_width;
    RGBColor m_color;
    int speed;
    float probability;
};

class LightPulseFXNugget : public FXNugget
{
    IMPLEMENT_POOL(LightPulseFXNugget);

public:
    LightPulseFXNugget() : m_radiusAsPercentOfObjectSize(0.0f) {};
    virtual ~LightPulseFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    RGBColor m_color;
    int m_radius;
    float m_radiusAsPercentOfObjectSize;
    int m_increaseTime;
    int m_decreaseTime;
};

class ViewShakeFXNugget : public FXNugget
{
    IMPLEMENT_POOL(ViewShakeFXNugget);

public:
    ViewShakeFXNugget() {};
    virtual ~ViewShakeFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    ShakeIntensities m_type;
};

class TerrainScorchFXNugget : public FXNugget
{
    IMPLEMENT_POOL(TerrainScorchFXNugget);

public:
    TerrainScorchFXNugget() {};
    virtual ~TerrainScorchFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    Utf8String m_type;
    int m_radius;
};

class ParticleSystemFXNugget : public FXNugget
{
    IMPLEMENT_POOL(ParticleSystemFXNugget);

public:
    ParticleSystemFXNugget() {};
    virtual ~ParticleSystemFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    Utf8String m_sysName;
    int m_count;
    GameClientRandomVariable m_radius;
    Coord3D m_offset;
    GameClientRandomVariable m_height;
    bool m_attachToObject;
    GameClientRandomVariable m_initialDelay;
    bool m_orientToObject;
    float m_rotateY;
    bool m_ricochet;
    bool m_createAtGroundHeight;
    bool m_useCallersRadius;
};

struct FXListAtBonePosFXNugget : public FXNugget
{
    IMPLEMENT_POOL(FXListAtBonePosFXNugget);

public:
    FXListAtBonePosFXNugget() {};
    virtual ~FXListAtBonePosFXNugget() override {};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override;
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override;

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    FXList *m_fx;
    Utf8String m_boneName;
    bool m_orientToBone;
};