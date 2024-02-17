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
#include "vector3.h"

class Matrix3D;
class LightClass;

class LightEnvironmentClass
{
public:
    LightEnvironmentClass();
    ~LightEnvironmentClass() {}

    void Reset(const Vector3 &object_center, const Vector3 &scene_ambient);
    void Add_Light(const LightClass &light);
    void Pre_Render_Update(const Matrix3D &camera_tm);

    const Vector3 &Get_Equivalent_Ambient() const { return m_outputAmbient; }
    void Set_Equivalent_Ambient(Vector3 &ambient) { m_outputAmbient = ambient; }
    int Get_Light_Count() const { return m_lightCount; }
    const Vector3 &Get_Light_Direction(int i) const { return m_inputLights[i].m_direction; }
    const Vector3 &Get_Light_Diffuse(int i) const { return m_inputLights[i].m_diffuse; }

    void Add_Fill_Light();
    void Calculate_Fill_Light();

    bool Is_Point_Light(int i) { return m_inputLights[i].m_isPoint; }
    float Get_Point_Inner_Radius(int i) { return m_inputLights[i].m_pointInnerRadius; }
    float Get_Point_Outer_Radius(int i) { return m_inputLights[i].m_pointOuterRadius; }
    const Vector3 &Get_Point_Diffuse(int i) { return m_inputLights[i].m_pointDiffuse; }
    const Vector3 &Get_Point_Ambient(int i) { return m_inputLights[i].m_pointAmbient; }
    const Vector3 &Get_Point_Center(int i) { return m_inputLights[i].m_pointCenter; }

    static void Set_Lighting_LOD_Cutoff(float inten);
    static float Get_Lighting_LOD_Cutoff();

    enum
    {
        MAX_LIGHTS = 4
    };

protected:
    struct InputLightStruct
    {
        void Init(const LightClass &light, const Vector3 &object_center);
        void Init_From_Point_Or_Spot_Light(const LightClass &light, const Vector3 &object_center);
        void Init_From_Directional_Light(const LightClass &light, const Vector3 &object_center);
        float Contribution();

        Vector3 m_direction;
        Vector3 m_ambient;
        Vector3 m_diffuse;
        bool m_diffuseRejected;
        bool m_isPoint;
        // only applies to point lights
        Vector3 m_pointCenter;
        float m_pointInnerRadius;
        float m_pointOuterRadius;
        Vector3 m_pointAmbient;
        Vector3 m_pointDiffuse;
    };

    struct OutputLightStruct
    {
        void Init(const InputLightStruct &input, const Matrix3D &camera_tm);

        Vector3 m_direction; // direction to the light.
        Vector3 m_diffuse; // diffuse color * attenuation
    };

    int m_lightCount; // number of lights this environment has
    Vector3 m_objectCenter; // center of the object to be lit
    InputLightStruct m_inputLights[MAX_LIGHTS]; // input lights
    Vector3 m_outputAmbient; // scene ambient + lights ambients
    OutputLightStruct m_outputLights[MAX_LIGHTS]; // output lights
    InputLightStruct m_fillLight;
    float m_intensity; // not sure......
};
