/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Prop Drawing Code
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
#include "partitionmanager.h"
#include "sphere.h"
#include "xfer.h"

class CameraClass;
class LightClass;
class RenderInfoClass;
class RenderObjClass;
class W3DShroudMaterialPassClass;

struct TPropType
{
    RenderObjClass *render_obj;
    Utf8String name;
    SphereClass bounding_sphere;
};

struct TProp
{
    RenderObjClass *render_obj;
    int id;
    Coord3D position;
    int prop_type;
    ObjectShroudStatus shroud_status;
    bool is_visible;
    SphereClass bounding_sphere;
};

class W3DPropBuffer : public SnapShot
{
public:
    W3DPropBuffer();
    ~W3DPropBuffer();

    virtual void CRC_Snapshot(Xfer *xfer) override {}

    virtual void Xfer_Snapshot(Xfer *xfer) override
    {
        unsigned char version = 1;
        xfer->xferVersion(&version, 1);
    }

    virtual void Load_Post_Process() override {}

    void Cull(CameraClass *camera);
    void Clear_All_Props();
    int Add_Prop_Type(const Utf8String &name);
    void Add_Prop(int id, Coord3D position, float orientation, float scale, const Utf8String &name);
    bool Update_Prop_Position(int id, const Coord3D &position, float orientation, float scale);
    void Remove_Prop(int id);
    void Remove_Props_For_Construction(const Coord3D *position, const GeometryInfo &geometry, float angle);
    void Notify_Shroud_Changed();
    void Draw_Props(RenderInfoClass &rinfo);

    void Update_Center() { m_redoCull = true; }

#ifdef GAME_DLL
    W3DPropBuffer *Hook_Ctor() { return new (this) W3DPropBuffer; }
    void Hook_Dtor() { W3DPropBuffer::~W3DPropBuffer(); }
#endif

private:
    enum
    {
        MAX_PROP_TYPES = 64,
        MAX_PROPS = 4000,
    };

    TProp m_props[MAX_PROPS];
    int m_numProps;
    bool m_propsUpdated;
    bool m_isInited;
    bool m_redoCull;
    TPropType m_propTypes[MAX_PROP_TYPES];
    int m_numPropTypes;
    W3DShroudMaterialPassClass *m_shroudMaterial;
    LightClass *m_light;
};
