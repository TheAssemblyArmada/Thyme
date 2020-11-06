/**
 * @file
 *
 * @author tomsons26
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
#include "rendobj.h"
#include "w3derr.h"

class ChunkLoadClass;
class ChunkSaveClass;

class LightClass : public RenderObjClass
{
public:
    enum LightType
    {
        POINT = 0,
        DIRECTIONAL,
        SPOT
    };

    enum FlagsType
    {
        NEAR_ATTENUATION = 0,
        FAR_ATTENUATION,
    };

    LightClass(LightType type = POINT);
    LightClass(const LightClass &src);
    virtual ~LightClass() {}

    LightClass &operator=(const LightClass &);

    virtual RenderObjClass *Clone() const override;

    virtual int Class_ID() const override { return CLASSID_LIGHT; }

    // Rendering
    virtual void Render(RenderInfoClass &rinfo) override {}

    // Scene
    virtual void Notify_Added(SceneClass *scene) override;
    virtual void Notify_Removed(SceneClass *scene) override;

    // Bounding
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

    // Lights are vertex processors
    virtual bool Is_Vertex_Processor() { return true; }

    LightType Get_Type() { return m_type; }

    void Set_Intensity(float inten) { m_intensity = inten; }
    float Get_Intensity() const { return m_intensity; }

    void Set_Ambient(const Vector3 &color) { m_ambient = color; }
    void Get_Ambient(Vector3 *set_c) const
    {
        if (set_c) {
            *set_c = m_ambient;
        }
    }

    void Set_Diffuse(const Vector3 &color) { m_diffuse = color; }
    void Get_Diffuse(Vector3 *set_c) const
    {
        if (set_c) {
            *set_c = m_diffuse;
        }
    }

    void Set_Specular(const Vector3 &color) { m_specular = color; }
    void Get_Specular(Vector3 *set_c) const
    {
        if (set_c) {
            *set_c = m_specular;
        }
    }

    void Set_Far_Attenuation_Range(double far_start, double far_end)
    {
        m_farAttenStart = far_start;
        m_farAttenEnd = far_end;
    }
    void Get_Far_Attenuation_Range(double &far_start, double &far_end) const
    {
        far_start = m_farAttenStart;
        far_end = m_farAttenEnd;
    }
    void Set_Near_Attenuation_Range(double near_start, double near_end)
    {
        m_nearAttenStart = near_start;
        m_nearAttenEnd = near_end;
    }
    void Get_Near_Attenuation_Range(double &near_start, double &near_end) const
    {
        near_start = m_nearAttenStart;
        near_end = m_nearAttenEnd;
    }
    float Get_Attenuation_Range() const { return m_farAttenEnd; }

    void Set_Flag(FlagsType flag, bool state)
    {
        if (state) {
            m_flags |= flag;
        } else {
            m_flags &= ~flag;
        }
    }
    int Get_Flag(FlagsType flag) const { return ((m_flags & flag) != 0); }
    void Enable_Shadows(bool state) { m_castShadows = state; }
    bool Are_Shadows_Enabled() const { return m_castShadows; }
    LightType Get_Type() const { return m_type; }

    void Set_Spot_Angle(float a)
    {
        m_spotAngle = a;
        m_spotAngleCos = GameMath::Fast_Cos(a);
    }
    float Get_Spot_Angle() const { return m_spotAngle; }
    float Get_Spot_Angle_Cos() const { return m_spotAngleCos; }
    void Set_Spot_Direction(const Vector3 &dir) { m_spotDirection = dir; }
    void Get_Spot_Direction(Vector3 &dir) const { dir = m_spotDirection; }
    void Set_Spot_Exponent(float k) { m_spotExponent = k; }
    float Get_Spot_Exponent() const { return m_spotExponent; }

    bool Is_Within_Attenuation_Radius(const Vector3 &pos);
    void Compute_Lighting(const Vector3 &pos, const Vector3 &norm, Vector3 *set_ambient, Vector3 *set_diffuse);
    W3DErrorType Load_W3D(ChunkLoadClass &cload);
    W3DErrorType Save_W3D(ChunkSaveClass &csave);

#ifdef GAME_DLL
    virtual const PersistFactoryClass &Get_Factory() const override;
#endif
    virtual bool Save(ChunkSaveClass &csave) override;
    virtual bool Load(ChunkLoadClass &cload) override;

protected:
    LightType m_type;
    unsigned int m_flags;
    bool m_castShadows;

    float m_intensity;
    Vector3 m_ambient;
    Vector3 m_diffuse;
    Vector3 m_specular;

    float m_nearAttenStart;
    float m_nearAttenEnd;
    float m_farAttenStart;
    float m_farAttenEnd;

    float m_spotAngle;
    float m_spotAngleCos;
    float m_spotExponent;
    Vector3 m_spotDirection;
};