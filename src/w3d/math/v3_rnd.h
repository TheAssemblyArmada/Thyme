/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
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
#include "random.h"
#include "vector3.h"

class Vector3Randomizer
{
public:
    enum
    {
        CLASSID_UNKNOWN = 0xFFFFFFFF,
        CLASSID_SOLIDBOX = 0,
        CLASSID_SOLIDSPHERE,
        CLASSID_HOLLOWSPHERE,
        CLASSID_SOLIDCYLINDER,
        CLASSID_MAXKNOWN,
        CLASSID_LAST = 0x0000FFFF
    };
    virtual ~Vector3Randomizer() {}
    virtual unsigned int Class_ID() const = 0;
    virtual void Get_Vector(Vector3 &vector) = 0;
    virtual float Get_Maximum_Extent() = 0;
    virtual void Scale(float scale) = 0;
    virtual Vector3Randomizer *Clone() const = 0;

protected:
    float Get_Random_Float_Minus1_To_1() { return randomizer * s_ooIntMax; }
    float Get_Random_Float_0_To_1() { return ((unsigned int)randomizer) * s_ooUIntMax; }
    static const float s_ooIntMax;
    static const float s_ooUIntMax;
    static Random3Class randomizer;
};

class Vector3SolidBoxRandomizer : public Vector3Randomizer
{
public:
    Vector3SolidBoxRandomizer(const Vector3 &extents);
    Vector3SolidBoxRandomizer(const Vector3SolidBoxRandomizer &that);
    ~Vector3SolidBoxRandomizer() override {}
    unsigned int Class_ID() const override { return CLASSID_SOLIDBOX; }
    void Get_Vector(Vector3 &vector) override;
    float Get_Maximum_Extent() override;
    void Scale(float scale) override;
    Vector3Randomizer *Clone() const override;
    virtual const Vector3 &Get_Extents() const { return m_extents; }

private:
    Vector3 m_extents;
};

class Vector3SolidSphereRandomizer : public Vector3Randomizer
{
public:
    Vector3SolidSphereRandomizer(float radius);
    Vector3SolidSphereRandomizer(const Vector3SolidSphereRandomizer &that);
    ~Vector3SolidSphereRandomizer() override {}
    unsigned int Class_ID() const override { return CLASSID_SOLIDSPHERE; }
    void Get_Vector(Vector3 &vector) override;
    float Get_Maximum_Extent() override;
    void Scale(float scale) override;
    Vector3Randomizer *Clone() const override;
    virtual float Get_Radius() const { return m_radius; }

private:
    float m_radius;
};

class Vector3HollowSphereRandomizer : public Vector3Randomizer
{
public:
    Vector3HollowSphereRandomizer(float radius);
    Vector3HollowSphereRandomizer(const Vector3HollowSphereRandomizer &that);
    ~Vector3HollowSphereRandomizer() override {}
    unsigned int Class_ID() const override { return CLASSID_HOLLOWSPHERE; }
    void Get_Vector(Vector3 &vector) override;
    float Get_Maximum_Extent() override;
    void Scale(float scale) override;
    Vector3Randomizer *Clone() const override;
    virtual float Get_Radius() const { return m_radius; }

private:
    float m_radius;
};

class Vector3SolidCylinderRandomizer : public Vector3Randomizer
{
public:
    Vector3SolidCylinderRandomizer(float extent, float radius);
    Vector3SolidCylinderRandomizer(const Vector3SolidCylinderRandomizer &that);
    ~Vector3SolidCylinderRandomizer() override {}
    unsigned int Class_ID() const override { return CLASSID_SOLIDCYLINDER; }
    void Get_Vector(Vector3 &vector) override;
    float Get_Maximum_Extent() override;
    void Scale(float scale) override;
    Vector3Randomizer *Clone() const override;
    virtual float Get_Radius() const { return m_radius; }
    virtual float Get_Height() const { return m_extent; }

private:
    float m_extent;
    float m_radius;
};
