#include "light.h"
#include "scene.h"
#include <algorithm>

LightClass::LightClass(LightType type) :
    m_type(type),
    m_flags(0),
    m_castShadows(false),
    m_intensity(1.0f),
    m_ambient(1.0f, 1.0f, 1.0f),
    m_diffuse(1.0f, 1.0f, 1.0f),
    m_specular(1.0f, 1.0f, 1.0f),
    m_nearAttenStart(0.0f),
    m_nearAttenEnd(0.0f),
    m_farAttenStart(50.0f),
    m_farAttenEnd(100.0f),
    m_spotAngle(DEG_TO_RADF(45.0f)),
    m_spotAngleCos(0.707f),
    m_spotExponent(1.0f),
    m_spotDirection(0, 0, 1)
{
    if (type == DIRECTIONAL) {
        // The light has no position so it can't be culled.
        Set_Force_Visible(true);
    }
}

LightClass::LightClass(const LightClass &src) :
    m_type(src.m_type),
    m_flags(src.m_flags),
    m_castShadows(src.m_castShadows),
    m_intensity(src.m_intensity),
    m_ambient(src.m_ambient),
    m_diffuse(src.m_diffuse),
    m_specular(src.m_specular),
    m_nearAttenStart(src.m_nearAttenStart),
    m_nearAttenEnd(src.m_nearAttenEnd),
    m_farAttenStart(src.m_farAttenStart),
    m_farAttenEnd(src.m_farAttenEnd),
    m_spotAngle(src.m_spotAngle),
    m_spotAngleCos(src.m_spotAngleCos),
    m_spotExponent(src.m_spotExponent),
    m_spotDirection(src.m_spotDirection)
{
}

LightClass &LightClass::operator=(const LightClass &that)
{
    if (this != &that) {
        RenderObjClass::operator=(that);

        m_type = that.m_type;
        m_flags = that.m_flags;
        m_castShadows = that.m_castShadows;
        m_intensity = that.m_intensity;
        m_ambient = that.m_ambient;
        m_diffuse = that.m_diffuse;
        m_specular = that.m_specular;
        m_nearAttenStart = that.m_nearAttenStart;
        m_nearAttenEnd = that.m_nearAttenEnd;
        m_farAttenStart = that.m_farAttenStart;
        m_farAttenEnd = that.m_farAttenEnd;
        m_spotAngle = that.m_spotAngle;
        m_spotAngleCos = that.m_spotAngleCos;
        m_spotExponent = that.m_spotExponent;
        m_spotDirection = that.m_spotDirection;
    }
    return *this;
}

RenderObjClass *LightClass::Clone() const
{
    return new LightClass(*this);
}

void LightClass::Notify_Added(SceneClass *scene)
{
    RenderObjClass::Notify_Added(scene);
    scene->Register(this, SceneClass::LIGHT);
}

void LightClass::Notify_Removed(SceneClass *scene)
{
    scene->Unregister(this, SceneClass::LIGHT);
    RenderObjClass::Notify_Removed(scene);
}

void LightClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init({ 0, 0, 0 }, Get_Attenuation_Range());
}

void LightClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    float r = Get_Attenuation_Range();
    box.Init({ 0, 0, 0 }, { r, r, r });
}

bool LightClass::Is_Within_Attenuation_Radius(const Vector3 &pos)
{
    if (!Get_Flag(FAR_ATTENUATION)) {
        return true;
    } else {
        Vector3 dir = Get_Position() - pos;
        float dist2 = dir.Length2();
        return (dist2 < m_farAttenEnd * m_farAttenEnd);
    }
}

W3DErrorType LightClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_dbgassert(false, "Unimplemented code called!");
    return W3D_ERROR_OK;
}

W3DErrorType LightClass::Save_W3D(ChunkSaveClass &csave)
{
    captainslog_dbgassert(false, "Unimplemented code called!");
    return W3D_ERROR_OK;
}

#ifdef GAME_DLL
const PersistFactoryClass &LightClass::Get_Factory() const
{
    // return _LightFactory;
    captainslog_dbgassert(false, "Unimplemented code called!");

    // temp
    static class PersistFactoryClass *p;
    return *p;
}
#endif

bool LightClass::Save(ChunkSaveClass &csave)
{
    captainslog_dbgassert(false, "Unimplemented code called!");
    return true;
}

bool LightClass::Load(ChunkLoadClass &cload)
{
    captainslog_dbgassert(false, "Unimplemented code called!");
    return true;
}
