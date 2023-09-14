/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Obsolete W3D stuff
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "quat.h"

enum
{
    W3D_CHUNK_MESH_HEADER = 0x1,
    O_W3D_CHUNK_MATERIALS = 0x6,
    O_W3D_CHUNK_TRIANGLES = 0x7,
    O_W3D_CHUNK_QUADRANGLES = 0x8,
    O_W3D_CHUNK_SURRENDER_TRIANGLES = 0x9,
    O_W3D_CHUNK_POV_TRIANGLES = 0xA,
    O_W3D_CHUNK_POV_QUADRANGLES = 0xB,
    W3D_CHUNK_VERTEX_COLORS = 0xD,
    W3D_CHUNK_DAMAGE = 0xF,
    W3D_CHUNK_DAMAGE_HEADER = 0x10,
    W3D_CHUNK_DAMAGE_VERTICES = 0x11,
    W3D_CHUNK_DAMAGE_COLORS = 0x12,
    W3D_CHUNK_DAMAGE_MATERIALS = 0x13,
    O_W3D_CHUNK_MATERIALS2 = 0x14,
    W3D_CHUNK_MATERIALS3 = 0x15,
    W3D_CHUNK_MATERIAL3 = 0x16,
    W3D_CHUNK_MATERIAL3_NAME = 0x17,
    W3D_CHUNK_MATERIAL3_INFO = 0x18,
    W3D_CHUNK_MATERIAL3_DC_MAP = 0x19,
    W3D_CHUNK_MAP3_FILENAME = 0x1A,
    W3D_CHUNK_MAP3_INFO = 0x1B,
    W3D_CHUNK_MATERIAL3_DI_MAP = 0x1cC,
    W3D_CHUNK_MATERIAL3_SC_MAP = 0x1D,
    W3D_CHUNK_MATERIAL3_SI_MAP = 0x1E,
    W3D_CHUNK_PER_TRI_MATERIALS = 0x21,
    W3D_CHUNK_FXSHADER_IDS = 0x3F,
    W3D_CHUNK_FX_SHADERS = 0x50,
    W3D_CHUNK_FX_SHADER = 0x51,
    W3D_CHUNK_FX_SHADER_INFO = 0x52,
    W3D_CHUNK_FX_SHADER_CONSTANT = 0x53,
    W3D_CHUNK_TANGENTS = 0x60,
    W3D_CHUNK_BINORMALS = 0x61,
    W3D_CHUNK_PIVOT_UNKNOWN1 = 0x104,
    W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL = 0x284,
    W3D_CHUNK_SPOT_LIGHT_INFO_5_0 = 0x465,
    W3D_CHUNK_SHDMESH = 0xB00,
    W3D_CHUNK_SHDMESH_NAME = 0xB01,
    W3D_CHUNK_SHDMESH_HEADER = 0xB02,
    W3D_CHUNK_SHDMESH_USER_TEXT = 0xB03,
    W3D_CHUNK_SHDSUBMESH = 0xB20,
    W3D_CHUNK_SHDSUBMESH_HEADER = 0xB21,
    W3D_CHUNK_SHDSUBMESH_SHADER = 0xB40,
    W3D_CHUNK_SHDSUBMESH_SHADER_TYPE = 0xB41,
    W3D_CHUNK_SHDSUBMESH_SHADER_DATA = 0xB42,
    W3D_CHUNK_SHDSUBMESH_VERTICES = 0xB43,
    W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS = 0xB44,
    W3D_CHUNK_SHDSUBMESH_TRIANGLES = 0xB45,
    W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES = 0xB46,
    W3D_CHUNK_SHDSUBMESH_UV0 = 0xB47,
    W3D_CHUNK_SHDSUBMESH_UV1 = 0xB48,
    W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S = 0xB49,
    W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T = 0xB4A,
    W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT = 0xB4B,
    W3D_CHUNK_SHDSUBMESH_B4C = 0xB4C,
    W3D_CHUNK_SHDSUBMESH_VERTEX_INFLUENCES = 0xB4D,
    W3D_CHUNK_SECONDARY_VERTICES = 0xC00,
    W3D_CHUNK_SECONDARY_VERTEX_NORMALS = 0xC01,
    W3D_CHUNK_LIGHTMAP_UV = 0xC02,
};

struct W3dMaterial2Struct
{
    char MaterialName[16];
    char PrimaryName[16];
    char SecondaryName[16];
    int RenderFlags;
    char Red;
    char Green;
    char Blue;
    uint8_t Alpha;
    uint16_t PrimaryNumFrames;
    uint16_t SecondaryNumFrames;
    uint32_t reserved[3];
};

struct W3dHModelNodeStruct
{
    char RenderObjName[16];
    uint16_t PivotIdx;
};

struct W3dMaterialStruct
{
    char MaterialName[16];
    char PrimaryName[16];
    char SecondaryName[16];
    uint32_t RenderFlags;
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

struct W3dSurrenderTriangleStruct
{
    uint32_t VertexIndices[3];
    W3dTexCoordStruct TexCoord[3];
    uint32_t MaterialIdx;
    W3dVectorStruct Normal;
    uint32_t Attributes;
    W3dRGBStruct Gourad[3];
};

struct W3dAggregateHeaderStruct
{
    uint32_t Version;
    char Name[16];
};

struct W3dAggregateInfoStruct
{
    char BaseModelName[32];
    uint32_t SubobjectCount;
};

struct W3dAggregateSubobjectStruct
{
    char SubobjectName[32];
    char BoneName[32];
};

struct W3dAggregateMiscInfo
{
    uint32_t OriginalClassID;
    uint32_t Flags;
    uint32_t reserved[3];
};

struct W3dLightAttenuationStruct
{
    float Start;
    float End;
};

struct W3dCollectionHeaderStruct
{
    unsigned long Version;
    char Name[16];
    unsigned long RenderObjectCount;
    unsigned long pad[2];
};

struct W3dDamageStruct
{
    uint32_t NumDamageMaterials;
    uint32_t NumDamageVerts;
    uint32_t NumDamageColors;
    uint32_t DamageIndex;
    uint32_t reserved[4];
};

struct W3dDamageVertexStruct
{
    uint32_t VertexIndex;
    uint32_t NewVertex;
    int reserved[2];
};

struct W3dDamageColorStruct
{
    uint32_t VertexIndex;
    W3dRGBStruct NewColor;
};

struct W3dHModelAuxDataStruct
{
    uint32_t Attributes;
    uint32_t MeshCount;
    uint32_t CollisionCount;
    uint32_t SkinCount;
    uint32_t FutureCounts[8];
    float LODMin;
    float LODMax;
    uint32_t FutureUse[32];
};

struct W3dHModelHeaderStruct
{
    uint32_t Version;
    char Name[16];
    char HierarchyName[16];
    uint16_t NumConnections;
};

struct W3dLightStruct
{
    uint32_t Attributes;
    uint32_t Unused;
    W3dRGBStruct Ambient;
    W3dRGBStruct Diffuse;
    W3dRGBStruct Specular;
    float Intensity;
};

struct W3dLightTransformStruct
{
    float Transform[3][4];
};

struct W3dLODStruct
{
    char RenderObjName[32];
    float LODMin;
    float LODMax;
};

struct W3dLODModelHeaderStruct
{
    uint32_t Version;
    char Name[16];
    uint16_t NumLODs;
};

struct W3dMap3Struct
{
    uint16_t MappingType;
    uint16_t FrameCount;
    float FrameRate;
};

struct W3dMeshHeaderStruct
{
    uint32_t Version;
    char MeshName[16];
    uint32_t Attributes;
    uint32_t NumTris;
    uint32_t NumQuads;
    uint32_t NumSrTris;
    uint32_t field_24;
    uint32_t NumPovQuads;
    uint32_t NumVertices;
    uint32_t NumNormals;
    uint32_t NumSrNormals;
    uint32_t NumTexCoords;
    uint32_t NumMaterials;
    uint32_t NumVertColors;
    uint32_t NumVertInfluences;
    uint32_t NumDamageStages;
    uint32_t FutureCounts[5];
    float LODMin;
    float LODMax;
    W3dVectorStruct Min;
    W3dVectorStruct Max;
    W3dVectorStruct SphCenter;
    float SphRadius;
    W3dVectorStruct Translation;
    float Rotation[9];
    W3dVectorStruct MassCenter;
    float Inertia[9];
    float Volume;
    char HierarchyTreeName[16];
    char HierarchyModelName[16];
    uint32_t FutureUse[24];
};

struct W3dPlaceholderStruct
{
    uint32_t Version;
    float Transform[4][3];
    uint32_t reserved;
    char Name[1];
};

struct W3dPS2ShaderStruct
{
    uint8_t DepthCompare;
    uint8_t DepthMask;
    uint8_t PriGradient;
    uint8_t Texturing;
    uint8_t AlphaTest;
    uint8_t AParam;
    uint8_t BParam;
    uint8_t CParam;
    uint8_t DParam;
    uint8_t pad[3];
};

struct W3dSpotLightStruct
{
    W3dVectorStruct SpotDirection;
    float SpotAngle;
    float SpotExponent;
};

struct W3dSpotLightStruct_v5_0
{
    float SpotOuterAngle;
    float SpotInnerAngle;
};

struct W3dTextureReplacerHeaderStruct
{
    uint32_t ReplacedTexturesCount;
};

struct W3dTextureReplacerStruct
{
    char MeshPath[15][32];
    char BonePath[15][32];
    char OldTextureName[260];
    char NewTextureName[260];
    W3dTextureInfoStruct TextureParams;
};

struct W3dMorphAnimHeaderStruct
{
    uint32_t Version;
    char Name[16];
    char HierarchyName[16];
    uint32_t FrameCount;
    float FrameRate;
    uint32_t ChannelCount;
};

struct W3dMorphAnimKeyStruct
{
    uint32_t MorphFrame;
    uint32_t PoseFrame;
};

struct W3dSoundRObjHeaderStruct
{
    uint32_t Version;
    char Name[16];
    uint32_t Flags;
    uint32_t Padding[8];
};

struct W3dCompressedMotionChannelStruct
{
    uint8_t Zero;
    uint8_t Flavor;
    uint8_t VectorLen;
    uint8_t Flags;
    uint16_t NumTimeCodes;
    uint16_t Pivot;
};

struct W3dFXShaderStruct
{
    char shadername[32];
    uint8_t technique;
    uint8_t pad[3];
};

struct AlphaVectorStruct
{
    Quaternion Quat;
    float Magnitude;
    AlphaVectorStruct() : Quat(0.0f, 0.0f, 0.0f, 1.0f), Magnitude(1.0f) {}
    AlphaVectorStruct(Quaternion &quat, float magnitude) : Quat(quat), Magnitude(magnitude) {}
};

enum
{
    ANIM_FLAVOR_NEW_TIMECODED = 0,
    ANIM_FLAVOR_NEW_ADAPTIVE_DELTA_4,
    ANIM_FLAVOR_NEW_ADAPTIVE_DELTA_8,
    ANIM_FLAVOR_NEW_VALID
};

enum
{
    CONSTANT_TYPE_TEXTURE = 1,
    CONSTANT_TYPE_FLOAT1 = 2,
    CONSTANT_TYPE_FLOAT2 = 3,
    CONSTANT_TYPE_FLOAT3 = 4,
    CONSTANT_TYPE_FLOAT4 = 5,
    CONSTANT_TYPE_INT = 6,
    CONSTANT_TYPE_BOOL = 7
};

struct W3dRingStruct
{
    int unk0;
    int Flags;
    char Name[32];
    IOVector3Struct Center;
    IOVector3Struct Extent;
    float AnimationDuration;
    IOVector3Struct Color;
    float Alpha;
    IOVector2Struct InnerScale;
    IOVector2Struct OuterScale;
    IOVector2Struct InnerExtent;
    IOVector2Struct OuterExtent;
    char TextureName[32];
    W3dShaderStruct Shader;
    int TextureTiling;
};

enum
{
    RING_CAMERA_ALIGNED = 1,
    RING_LOOPING = 2
};

struct W3dSphereStruct
{
    int unk0;
    int Flags;
    char Name[32];
    IOVector3Struct Center;
    IOVector3Struct Extent;
    float AnimationDuration;
    IOVector3Struct Color;
    float Alpha;
    IOVector3Struct Scale;
    AlphaVectorStruct Vector;
    char TextureName[32];
    W3dShaderStruct Shader;
};

enum
{
    SPHERE_ALPHA_VECTOR = 1,
    SPHERE_CAMERA_ALIGNED = 2,
    SPHERE_INVERT_EFFECT = 4,
    SPHERE_LOOPING = 8
};

// clang-format off
#define W3DMATERIAL_USE_ALPHA                         0x00000001
#define W3DMATERIAL_USE_SORTING                       0x00000002
#define W3DMATERIAL_HINT_DIT_OVER_DCT                 0x00000010
#define W3DMATERIAL_HINT_SIT_OVER_SCT                 0x00000020
#define W3DMATERIAL_HINT_DIT_OVER_DIG                 0x00000040
#define W3DMATERIAL_HINT_SIT_OVER_SIG                 0x00000080
#define W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA    0x00000100
#define W3DMATERIAL_PSX_TRANS_100                     0x01000000
#define W3DMATERIAL_PSX_TRANS_50                      0x02000000
#define W3DMATERIAL_PSX_TRANS_25                      0x03000000
#define W3DMATERIAL_PSX_TRANS_MINUS_100               0x04000000
#define W3DMATERIAL_PSX_NO_RT_LIGHTING                0x08000000

#define W3DVERTMAT_PSX_MASK                           0xFF000000
#define W3DVERTMAT_PSX_TRANS_MASK                     0x07000000
#define W3DVERTMAT_PSX_TRANS_NONE                     0x00000000
#define W3DVERTMAT_PSX_TRANS_100                      0x01000000
#define W3DVERTMAT_PSX_TRANS_50                       0x02000000
#define W3DVERTMAT_PSX_TRANS_25                       0x03000000
#define W3DVERTMAT_PSX_TRANS_MINUS_100                0x04000000
#define W3DVERTMAT_PSX_NO_RT_LIGHTING                 0x08000000

#define W3D_LIGHT_ATTRIBUTE_TYPE_MASK                 0x000000FF
#define W3D_LIGHT_ATTRIBUTE_POINT                     0x00000001
#define W3D_LIGHT_ATTRIBUTE_DIRECTIONAL               0x00000002
#define W3D_LIGHT_ATTRIBUTE_SPOT                      0x00000003
#define W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS              0x00000100

#define W3D_MESH_FLAG_PRELIT                          0x40000000
#define W3D_MESH_FLAG_ALWAYSDYNLIGHT                  0x80000000

#define W3D_MESH_FLAG_COLLISION_TYPE_USER1            0x00000200
#define W3D_MESH_FLAG_COLLISION_TYPE_USER2            0x00000400

#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED 0x00070000

#define W3D_VERTEX_CHANNEL_TANGENT                    0x00000020
#define W3D_VERTEX_CHANNEL_BINORMAL                   0x00000040
#define W3D_VERTEX_CHANNEL_SMOOTHSKIN                 0x00000080
// clang-format on
