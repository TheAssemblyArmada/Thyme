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

class Matrix3D;
class Object;
class INI;

class FXNugget : public MemoryPoolObject
{
    IMPLEMENT_ABSTRACT_POOL(FXNugget);

public:
    virtual ~FXNugget(){};
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
    SoundFXNugget(){};
    virtual ~SoundFXNugget() override{};

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

// TODO: NOT VALIDATED
class ParticleSystemFXNugget : public FXNugget
{
    IMPLEMENT_POOL(ParticleSystemFXNugget);

public:
    ParticleSystemFXNugget() : m_count(1){};
    virtual ~ParticleSystemFXNugget() override{};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override{
        // TODO
    };
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override
    {
        // TODO
    }

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    Utf8String m_particleSystemName;
    Coord3D m_offset;
    GameLogicRandomVariable m_radius;
    GameLogicRandomVariable m_height;
    GameLogicRandomVariable m_initialDelay;
    float m_rotate[3];
    int m_count;
    bool m_orientToObject;
    bool m_ricochet;
    bool m_attachToObject;
    bool m_createAtGroundHeight;
    bool m_useCallersRadius;
};

// TODO: NOT VALIDATED
class LightPulseFXNugget : public FXNugget
{
    IMPLEMENT_POOL(LightPulseFXNugget);

public:
    LightPulseFXNugget() : m_radiusAsPercentOfObjectSize(0.0f){};
    virtual ~LightPulseFXNugget() override{};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override{
        // TODO
    };
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override
    {
        // TODO
    }

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    RGBColor m_color;
    float m_radius;
    // this is a new option for LightPulse. If present (and nonzero), and the
    // FXList is applied  to an Object (rather than a Position), it overrides
    // the value of "Radius" by applying the given percentage of the Object's bounding circle.
    float m_radiusAsPercentOfObjectSize;
    unsigned int m_increaseTime;
    unsigned int m_decreaseTime;
};

// TODO: NOT VALIDATED
class ViewShakeFXNugget : public FXNugget
{
    IMPLEMENT_POOL(ViewShakeFXNugget);

public:
    enum ViewShakeType
    {
        SUBTLE,
        NORMAL,
        STRONG,
        SEVERE,
        CINE_EXTREME,
        CINE_INSANE
    };

    ViewShakeFXNugget(){};
    virtual ~ViewShakeFXNugget() override{};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override{
        // TODO
    };
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override
    {
        // TODO
    }

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    ViewShakeType m_type;
};

// TODO: NOT VALIDATED
class TerrainScorchFXNugget : public FXNugget
{
    IMPLEMENT_POOL(TerrainScorchFXNugget);

public:
    enum TerrainScorchType
    {
        RANDOM,
        SCORCH_1,
        SCORCH_2,
        SCORCH_3,
        SCORCH_4,
        SCORCH_5,
        SCORCH_6,
        SCORCH_7,
        SCORCH_8,
        SCORCH_9,
        SCORCH_10,
        SCORCH_11,
        SCORCH_12,
        SCORCH_13,
        SCORCH_14,
        SCORCH_15,
        SCORCH_16,
        TERRAIN_MAPPED
    };

    TerrainScorchFXNugget() : m_type(RANDOM){};
    virtual ~TerrainScorchFXNugget() override{};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override{
        // TODO
    };
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override
    {
        // TODO
    }

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    TerrainScorchType m_type;
    float m_radius;
};

// TODO: NOT VALIDATED
class TracerFXNugget : public FXNugget
{
    IMPLEMENT_POOL(TracerFXNugget);

public:
    TracerFXNugget(){};
    virtual ~TracerFXNugget() override{};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override{
        // TODO
    };
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override
    {
        // TODO
    }

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    float m_decayAt;
    float m_length;
    float m_width;
    RGBColor m_color;
    float m_speed;
    float m_probability;
};

// TODO: NOT VALIDATED
class FXListAtBonePosFXNugget : public FXNugget
{
    IMPLEMENT_POOL(FXListAtBonePosFXNugget);

public:
    FXListAtBonePosFXNugget(){};
    virtual ~FXListAtBonePosFXNugget() override{};

    virtual void Do_FX_Pos(const Coord3D *primary,
        const Matrix3D *primary_mtx,
        float primary_speed,
        const Coord3D *secondary,
        float radius) const override{
        // TODO
    };
    virtual void Do_FX_Obj(const Object *primary, const Object *secondary) const override
    {
        // TODO
    }

    static void Parse(INI *ini, void *formal, void *, const void *);

private:
    Utf8String m_fx; // Reference
    Utf8String m_boneName;
    bool m_orientToBone;
};