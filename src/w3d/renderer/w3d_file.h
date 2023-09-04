/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D data structures
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
#include "chunkio.h"

// clang-format off
enum {

    W3D_CHUNK_MESH                                  = 0x00000000,    // Mesh definition 
        W3D_CHUNK_VERTICES                          = 0x00000002,    // array of vertices (array of W3dVectorStruct's)
        W3D_CHUNK_VERTEX_NORMALS                    = 0x00000003,    // array of normals (array of W3dVectorStruct's)
        W3D_CHUNK_SURRENDER_NORMALS                 = 0x00000004,    // obsolete
        W3D_CHUNK_TEXCOORDS                         = 0x00000005,    // obsolete
        W3D_CHUNK_MESH_USER_TEXT                    = 0x0000000C,    // Text from the MAX comment field (Null terminated string)
        W3D_CHUNK_VERTEX_INFLUENCES                 = 0x0000000E,    // Mesh Deformation vertex connections (array of W3dVertInfStruct's)
        W3D_CHUNK_MESH_HEADER3                      = 0x0000001F,    //    mesh header contains general info about the mesh. (W3dMeshHeader3Struct)
        W3D_CHUNK_TRIANGLES                         = 0x00000020,    // New improved triangles chunk (array of W3dTriangleStruct's)
        W3D_CHUNK_VERTEX_SHADE_INDICES              = 0x00000022,    // shade indexes for each vertex (array of uint32's)
        
        W3D_CHUNK_PRELIT_UNLIT                      = 0x00000023,    // optional unlit material chunk wrapper
        W3D_CHUNK_PRELIT_VERTEX                     = 0x00000024,    // optional vertex-lit material chunk wrapper
        W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS        = 0x00000025,    // optional lightmapped multi-pass material chunk wrapper
        W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE     = 0x00000026,    // optional lightmapped multi-texture material chunk wrapper

            W3D_CHUNK_MATERIAL_INFO                 = 0x00000028,    // materials information, pass count, etc (contains W3dMaterialInfoStruct)

            W3D_CHUNK_SHADERS                       = 0x00000029,    // shaders (array of W3dShaderStruct's)
            
            W3D_CHUNK_VERTEX_MATERIALS              = 0x0000002A,    // wraps the vertex materials
                W3D_CHUNK_VERTEX_MATERIAL           = 0x0000002B,
                    W3D_CHUNK_VERTEX_MATERIAL_NAME  = 0x0000002C,    // vertex material name (NULL-terminated string)
                    W3D_CHUNK_VERTEX_MATERIAL_INFO  = 0x0000002D,    // W3dVertexMaterialStruct
                    W3D_CHUNK_VERTEX_MAPPER_ARGS0   = 0x0000002E,    // Null-terminated string
                    W3D_CHUNK_VERTEX_MAPPER_ARGS1   = 0x0000002F,    // Null-terminated string

            W3D_CHUNK_TEXTURES                      = 0x00000030,    // wraps all of the texture info
                W3D_CHUNK_TEXTURE                   = 0x00000031,    // wraps a texture definition
                    W3D_CHUNK_TEXTURE_NAME          = 0x00000032,    // texture filename (NULL-terminated string)
                    W3D_CHUNK_TEXTURE_INFO          = 0x00000033,    // optional W3dTextureInfoStruct
            
            W3D_CHUNK_MATERIAL_PASS                 = 0x00000038,    // wraps the information for a single material pass
                W3D_CHUNK_VERTEX_MATERIAL_IDS       = 0x00000039,    // single or per-vertex array of uint32 vertex material indices (check chunk size)
                W3D_CHUNK_SHADER_IDS                = 0x0000003A,    // single or per-tri array of uint32 shader indices (check chunk size)
                W3D_CHUNK_DCG                       = 0x0000003B,    // per-vertex diffuse color values (array of W3dRGBAStruct's)
                W3D_CHUNK_DIG                       = 0x0000003C,    // per-vertex diffuse illumination values (array of W3dRGBStruct's)
                W3D_CHUNK_SCG                       = 0x0000003E,    // per-vertex specular color values (array of W3dRGBStruct's)

                W3D_CHUNK_TEXTURE_STAGE             = 0x00000048,    // wrapper around a texture stage.
                    W3D_CHUNK_TEXTURE_IDS           = 0x00000049,    // single or per-tri array of uint32 texture indices (check chunk size)
                    W3D_CHUNK_STAGE_TEXCOORDS       = 0x0000004A,    // per-vertex texture coordinates (array of W3dTexCoordStruct's)
                    W3D_CHUNK_PER_FACE_TEXCOORD_IDS = 0x0000004B,    // indices to W3D_CHUNK_STAGE_TEXCOORDS, (array of Vector3i)


        W3D_CHUNK_DEFORM                            = 0x00000058,    // mesh deform or 'damage' information.
            W3D_CHUNK_DEFORM_SET                    = 0x00000059,    // set of deform information
                W3D_CHUNK_DEFORM_KEYFRAME           = 0x0000005A,    // a keyframe of deform information in the set
                    W3D_CHUNK_DEFORM_DATA           = 0x0000005B,    // deform information about a single vertex

        W3D_CHUNK_PS2_SHADERS                       = 0x00000080,    // Shader info specific to the Playstation 2.
        
        W3D_CHUNK_AABTREE                           = 0x00000090,    // Axis-Aligned Box Tree for hierarchical polygon culling
            W3D_CHUNK_AABTREE_HEADER,                               // catalog of the contents of the AABTree
            W3D_CHUNK_AABTREE_POLYINDICES,                          // array of uint32 polygon indices with count=mesh.PolyCount
            W3D_CHUNK_AABTREE_NODES,                                // array of W3dMeshAABTreeNode's with count=aabheader.NodeCount

    W3D_CHUNK_HIERARCHY                             = 0x00000100,    // hierarchy tree definition
        W3D_CHUNK_HIERARCHY_HEADER,
        W3D_CHUNK_PIVOTS,
        W3D_CHUNK_PIVOT_FIXUPS,                                     // only needed by the exporter...
    
    W3D_CHUNK_ANIMATION                             = 0x00000200,    // hierarchy animation data
        W3D_CHUNK_ANIMATION_HEADER,
        W3D_CHUNK_ANIMATION_CHANNEL,                                // channel of vectors
        W3D_CHUNK_BIT_CHANNEL,                                      // channel of boolean values (e.g. visibility)

    W3D_CHUNK_COMPRESSED_ANIMATION                  = 0x00000280,    // compressed hierarchy animation data
        W3D_CHUNK_COMPRESSED_ANIMATION_HEADER,                      // describes playback rate, number of frames, and type of compression
        W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL,                     // compressed channel, format dependent on type of compression
        W3D_CHUNK_COMPRESSED_BIT_CHANNEL,                           // compressed bit stream channel, format dependent on type of compression
        
    W3D_CHUNK_MORPH_ANIMATION                       = 0x000002C0,    // hierarchy morphing animation data (morphs between poses, for facial animation)
        W3D_CHUNK_MORPHANIM_HEADER,                                 // W3dMorphAnimHeaderStruct describes playback rate, number of frames, and type of compression
        W3D_CHUNK_MORPHANIM_CHANNEL,                                // wrapper for a channel
            W3D_CHUNK_MORPHANIM_POSENAME,                           // name of the other anim which contains the poses for this morph channel
            W3D_CHUNK_MORPHANIM_KEYDATA,                            // morph key data for this channel
        W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA,                       // uin32 per pivot in the htree, indicating which channel controls the pivot

    W3D_CHUNK_HMODEL                                = 0x00000300,    // blueprint for a hierarchy model
        W3D_CHUNK_HMODEL_HEADER,                                    // Header for the hierarchy model
        W3D_CHUNK_NODE,                                             // render objects connected to the hierarchy
        W3D_CHUNK_COLLISION_NODE,                                   // collision meshes connected to the hierarchy
        W3D_CHUNK_SKIN_NODE,                                        // skins connected to the hierarchy
        OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA,                         // extension of the hierarchy model header
        OBSOLETE_W3D_CHUNK_SHADOW_NODE,                             // shadow object connected to the hierarchy

    W3D_CHUNK_LODMODEL                              = 0x00000400,    // blueprint for an LOD model.  This is simply a
        W3D_CHUNK_LODMODEL_HEADER,                                  // collection of 'n' render objects, ordered in terms
        W3D_CHUNK_LOD,                                              // of their expected rendering costs. (highest is first)

    W3D_CHUNK_COLLECTION                            = 0x00000420,    // collection of render object names
        W3D_CHUNK_COLLECTION_HEADER,                                // general info regarding the collection
        W3D_CHUNK_COLLECTION_OBJ_NAME,                              // contains a string which is the name of a render object
        W3D_CHUNK_PLACEHOLDER,                                      // contains information about a 'dummy' object that will be instanced later
        W3D_CHUNK_TRANSFORM_NODE,                                   // contains the filename of another w3d file that should be transformed by this node

    W3D_CHUNK_POINTS                                = 0x00000440,    // array of W3dVectorStruct's.  May appear in meshes, hmodels, lodmodels, or collections.

    W3D_CHUNK_LIGHT                                 = 0x00000460,    // description of a light
        W3D_CHUNK_LIGHT_INFO,                                       // generic light parameters
        W3D_CHUNK_SPOT_LIGHT_INFO,                                  // extra spot light parameters
        W3D_CHUNK_NEAR_ATTENUATION,                                 // optional near attenuation parameters
        W3D_CHUNK_FAR_ATTENUATION,                                  // optional far attenuation parameters

    W3D_CHUNK_EMITTER                               = 0x00000500,    // description of a particle emitter
        W3D_CHUNK_EMITTER_HEADER,                                   // general information such as name and version
        W3D_CHUNK_EMITTER_USER_DATA,                                // user-defined data that specific loaders can switch on
        W3D_CHUNK_EMITTER_INFO,                                     // generic particle emitter definition
        W3D_CHUNK_EMITTER_INFOV2,                                   // generic particle emitter definition (version 2.0)
        W3D_CHUNK_EMITTER_PROPS,                                    // Key-frameable properties
        OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME,                  // structure defining a single color keyframe
        OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME,                // structure defining a single opacity keyframe
        OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME,                   // structure defining a single size keyframe
        W3D_CHUNK_EMITTER_LINE_PROPERTIES,                          // line properties, used by line rendering mode
        W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES,                       // rotation keys for the particles
        W3D_CHUNK_EMITTER_FRAME_KEYFRAMES,                          // frame keys (u-v based frame animation)
        W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES,                      // length of tail for line groups
        W3D_CHUNK_EMITTER_EXTRA_INFO,                               // extra info

    W3D_CHUNK_AGGREGATE                             = 0x00000600,    // description of an aggregate object
        W3D_CHUNK_AGGREGATE_HEADER,                                 // general information such as name and version
            W3D_CHUNK_AGGREGATE_INFO,                               // references to 'contained' models
        W3D_CHUNK_TEXTURE_REPLACER_INFO,                            // information about which meshes need textures replaced
        W3D_CHUNK_AGGREGATE_CLASS_INFO,                             // information about the original class that created this aggregate

    W3D_CHUNK_HLOD                                  = 0x00000700,    // description of an HLod object (see HLodClass)
        W3D_CHUNK_HLOD_HEADER,                                      // general information such as name and version
        W3D_CHUNK_HLOD_LOD_ARRAY,                                   // wrapper around the array of objects for each level of detail
            W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER,                 // info on the objects in this level of detail array
            W3D_CHUNK_HLOD_SUB_OBJECT,                              // an object in this level of detail array
        W3D_CHUNK_HLOD_AGGREGATE_ARRAY,                             // array of aggregates, contains W3D_CHUNK_SUB_OBJECT_ARRAY_HEADER and W3D_CHUNK_SUB_OBJECT_ARRAY
        W3D_CHUNK_HLOD_PROXY_ARRAY,                                 // array of proxies, used for application-defined purposes, provides a name and a bone.

    W3D_CHUNK_BOX                                   = 0x00000740,    // defines an collision box render object (W3dBoxStruct)
    W3D_CHUNK_SPHERE,
    W3D_CHUNK_RING,

    W3D_CHUNK_NULL_OBJECT                           = 0x00000750,    // defines a NULL object (W3dNullObjectStruct)

    W3D_CHUNK_LIGHTSCAPE                            = 0x00000800,    // wrapper for lights created with Lightscape.    
        W3D_CHUNK_LIGHTSCAPE_LIGHT,                                 // definition of a light created with Lightscape.
            W3D_CHUNK_LIGHT_TRANSFORM,                              // position and orientation (defined as right-handed 4x3 matrix transform W3dLightTransformStruct).

    W3D_CHUNK_DAZZLE                                = 0x00000900,    // wrapper for a glare object.  Creates halos and flare lines seen around a bright light source
        W3D_CHUNK_DAZZLE_NAME,                                      // null-terminated string, name of the dazzle (typical w3d object naming: "container.object")
        W3D_CHUNK_DAZZLE_TYPENAME,                                  // null-terminated string, type of dazzle (from dazzle.ini)

    W3D_CHUNK_SOUNDROBJ                             = 0x00000A00,   // description of a sound render object
        W3D_CHUNK_SOUNDROBJ_HEADER,                                 // general information such as name and version
        W3D_CHUNK_SOUNDROBJ_DEFINITION,                             // chunk containing the definition of the sound that is to play    

};
// clang-format on

typedef IOVector3Struct W3dVectorStruct;
typedef IOQuaternionStruct W3dQuaternionStruct;

struct W3dRGBStruct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t pad;
};

struct W3dMaterial3Struct
{
    uint32_t attributes;
    W3dRGBStruct diffuse_color;
    W3dRGBStruct specular_color;
    W3dRGBStruct emissive_coeffs;
    W3dRGBStruct ambient_coeffs;
    W3dRGBStruct diffuse_coeffs;
    W3dRGBStruct specular_coeffs;
    float shininess;
    float opacity;
    float translucency;
    float fog_coeff;
};

// clang-format off
#define W3DVERTMAT_USE_DEPTH_CUE                        0x00000001
#define W3DVERTMAT_ARGB_EMISSIVE_ONLY                   0x00000002
#define W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE             0x00000004
#define W3DVERTMAT_DEPTH_CUE_TO_ALPHA                   0x00000008

#define W3DVERTMAT_STAGE0_MAPPING_MASK                  0x00FF0000
#define W3DVERTMAT_STAGE0_MAPPING_UV                    0x00000000
#define W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT           0x00010000
#define W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT     0x00020000
#define W3DVERTMAT_STAGE0_MAPPING_SCREEN                0x00030000
#define W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET         0x00040000
#define W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE            0x00050000
#define W3DVERTMAT_STAGE0_MAPPING_SCALE                 0x00060000
#define W3DVERTMAT_STAGE0_MAPPING_GRID                  0x00070000
#define W3DVERTMAT_STAGE0_MAPPING_ROTATE                0x00080000
#define W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET    0x00090000
#define W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET    0x000A0000
#define W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET  0x000B0000
#define W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV        0x000C0000
#define W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT        0x000D0000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV      0x000E0000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT      0x000F0000
#define W3DVERTMAT_STAGE0_MAPPING_RANDOM                0x00100000
#define W3DVERTMAT_STAGE0_MAPPING_EDGE                  0x00110000
#define W3DVERTMAT_STAGE0_MAPPING_BUMPENV               0x00120000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV   0x00130000
#define W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT   0x00140000

#define W3DVERTMAT_STAGE1_MAPPING_MASK                  0x0000FF00
#define W3DVERTMAT_STAGE1_MAPPING_UV                    0x00000000
#define W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT           0x00000100
#define W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT     0x00000200
#define W3DVERTMAT_STAGE1_MAPPING_SCREEN                0x00000300
#define W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET         0x00000400
#define W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE            0x00000500
#define W3DVERTMAT_STAGE1_MAPPING_SCALE                 0x00000600
#define W3DVERTMAT_STAGE1_MAPPING_GRID                  0x00000700
#define W3DVERTMAT_STAGE1_MAPPING_ROTATE                0x00000800
#define W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET    0x00000900
#define W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET    0x00000A00
#define W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET  0x00000B00
#define W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV        0x00000C00
#define W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT        0x00000D00
#define W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV      0x00000E00
#define W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT      0x00000F00
#define W3DVERTMAT_STAGE1_MAPPING_RANDOM                0x00001000
#define W3DVERTMAT_STAGE1_MAPPING_EDGE                  0x00001100
#define W3DVERTMAT_STAGE1_MAPPING_BUMPENV               0x00001200
#define W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV   0x00001300
#define W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT   0x00001400
// clang-format on

struct W3dVertexMaterialStruct
{
    uint32_t Attributes;
    W3dRGBStruct Ambient;
    W3dRGBStruct Diffuse;
    W3dRGBStruct Specular;
    W3dRGBStruct Emissive;
    float Shininess;
    float Opacity;
    float Translucency;
};

struct W3dNullObjectStruct
{
    uint32_t version;
    uint32_t attributes;
    uint32_t pad[2];
    char name[32];
};

struct W3dHierarchyStruct
{
    uint32_t Version;
    char Name[16];
    uint32_t NumPivots;
    W3dVectorStruct Center;
};

struct W3dPivotStruct
{
    char Name[16];
    uint32_t ParentIdx;
    W3dVectorStruct Translation;
    W3dVectorStruct EulerAngles;
    W3dQuaternionStruct Rotation;
};

struct W3dPivotFixupStruct
{
    float TM[4][3];
};

struct W3dAnimHeaderStruct
{
    uint32_t Version;
    char Name[16];
    char HierarchyName[16];
    uint32_t NumFrames;
    uint32_t FrameRate;
};

struct W3dCompressedAnimHeaderStruct
{
    uint32_t Version;
    char Name[16];
    char HierarchyName[16];
    uint32_t NumFrames;
    uint16_t FrameRate;
    uint16_t Flavor;
};

enum
{
    ANIM_CHANNEL_X = 0,
    ANIM_CHANNEL_Y,
    ANIM_CHANNEL_Z,
    ANIM_CHANNEL_XR,
    ANIM_CHANNEL_YR,
    ANIM_CHANNEL_ZR,
    ANIM_CHANNEL_Q,

    ANIM_CHANNEL_TIMECODED_X,
    ANIM_CHANNEL_TIMECODED_Y,
    ANIM_CHANNEL_TIMECODED_Z,
    ANIM_CHANNEL_TIMECODED_Q,

    ANIM_CHANNEL_ADAPTIVEDELTA_X,
    ANIM_CHANNEL_ADAPTIVEDELTA_Y,
    ANIM_CHANNEL_ADAPTIVEDELTA_Z,
    ANIM_CHANNEL_ADAPTIVEDELTA_Q,

    ANIM_CHANNEL_VIS,
};

enum
{
    ANIM_FLAVOR_TIMECODED = 0,
    ANIM_FLAVOR_ADAPTIVE_DELTA,
    ANIM_FLAVOR_VALID
};

struct W3dAnimChannelStruct
{
    uint16_t FirstFrame;
    uint16_t LastFrame;
    uint16_t VectorLen;
    uint16_t Flags;
    uint16_t Pivot;
    uint16_t pad;
    float Data[1];
};

enum
{
    BIT_CHANNEL_VIS = 0,
    BIT_CHANNEL_TIMECODED_VIS,
};

struct W3dBitChannelStruct
{
    uint16_t FirstFrame;
    uint16_t LastFrame;
    uint16_t Flags;
    uint16_t Pivot;
    uint8_t DefaultVal;
    uint8_t Data[1];
};

struct W3dTimeCodedAnimChannelStruct
{
    uint32_t NumTimeCodes;
    uint16_t Pivot;
    uint8_t VectorLen;
    uint8_t Flags;
    uint32_t Data[1];
};

struct W3dTimeCodedBitChannelStruct
{
    uint32_t NumTimeCodes;
    uint16_t Pivot;
    uint8_t Flags;
    uint8_t DefaultVal;
    uint32_t Data[1];
};

struct W3dAdaptiveDeltaAnimChannelStruct
{
    uint32_t NumFrames;
    uint16_t Pivot;
    uint8_t VectorLen;
    uint8_t Flags;
    float Scale;
    uint32_t Data[1];
};

#define NO_MAX_SCREEN_SIZE GAMEMATH_FLOAT_MAX
struct W3dHLodHeaderStruct
{
    uint32_t Version;
    uint32_t LodCount;
    char Name[16];
    char HierarchyName[16];
};
struct W3dHLodArrayHeaderStruct
{
    uint32_t ModelCount;
    float MaxScreenSize;
};

struct W3dHLodSubObjectStruct
{
    uint32_t BoneIndex;
    char Name[32];
};

// clang-format off
#define W3D_BOX_ATTRIBUTE_ORIENTED                      0x00000001
#define W3D_BOX_ATTRIBUTE_ALIGNED                       0x00000002
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_MASK           0x00000FF0
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_SHIFT          4
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL       0x00000010
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE     0x00000020
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS            0x00000040
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA         0x00000080
#define W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE        0x00000100
// clang-format on

struct W3dBoxStruct
{
    uint32_t Version;
    uint32_t Attributes;
    char Name[32];
    W3dRGBStruct Color;
    W3dVectorStruct Center;
    W3dVectorStruct Extent;
};

struct W3dMeshAABTreeHeader
{
    uint32_t NodeCount;
    uint32_t PolyCount;
    uint32_t Padding[6];
};

struct W3dMeshAABTreeNode
{
    W3dVectorStruct Min;
    W3dVectorStruct Max;
    uint32_t FrontOrPoly0;
    uint32_t BackOrPolyCount;
};

#define SORT_LEVEL_NONE 0
#define MAX_SORT_LEVEL 32
#define SORT_LEVEL_BIN1 20
#define SORT_LEVEL_BIN2 15
#define SORT_LEVEL_BIN3 10

struct W3dMeshHeader3Struct
{
    uint32_t Version;
    uint32_t Attributes;

    char MeshName[16];
    char ContainerName[16];

    uint32_t NumTris;
    uint32_t NumVertices;
    uint32_t NumMaterials;
    uint32_t NumDamageStages;
    int32_t SortLevel;
    uint32_t PrelitVersion;
    uint32_t FutureCounts[1];

    uint32_t VertexChannels;
    uint32_t FaceChannels;

    W3dVectorStruct Min;
    W3dVectorStruct Max;
    W3dVectorStruct SphCenter;
    float SphRadius;
};

// clang-format off
#define W3D_MESH_FLAG_NONE                                      0x00000000
#define W3D_MESH_FLAG_COLLISION_BOX                             0x00000001
#define W3D_MESH_FLAG_SKIN                                      0x00000002
#define W3D_MESH_FLAG_SHADOW                                    0x00000004
#define W3D_MESH_FLAG_ALIGNED                                   0x00000008

#define W3D_MESH_FLAG_COLLISION_TYPE_MASK                       0x00000FF0
#define W3D_MESH_FLAG_COLLISION_TYPE_SHIFT                      4
#define W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL                   0x00000010
#define W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE                 0x00000020
#define W3D_MESH_FLAG_COLLISION_TYPE_VIS                        0x00000040
#define W3D_MESH_FLAG_COLLISION_TYPE_CAMERA                     0x00000080
#define W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE                    0x00000100

#define W3D_MESH_FLAG_HIDDEN                                    0x00001000
#define W3D_MESH_FLAG_TWO_SIDED                                 0x00002000
#define OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED                      0x00004000
#define W3D_MESH_FLAG_CAST_SHADOW                               0x00008000

#define W3D_MESH_FLAG_GEOMETRY_TYPE_MASK                        0x00FF0000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL                      0x00000000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED              0x00010000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN                        0x00020000
#define OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW             0x00030000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX                       0x00040000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX                       0x00050000
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED             0x00060000

#define W3D_MESH_FLAG_PRELIT_MASK                               0x0F000000
#define W3D_MESH_FLAG_PRELIT_UNLIT                              0x01000000
#define W3D_MESH_FLAG_PRELIT_VERTEX                             0x02000000
#define W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS                0x04000000
#define W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE             0x08000000

#define W3D_MESH_FLAG_SHATTERABLE                               0x10000000
#define W3D_MESH_FLAG_NPATCHABLE                                0x20000000

#define W3D_VERTEX_CHANNEL_LOCATION                             0x00000001
#define W3D_VERTEX_CHANNEL_NORMAL                               0x00000002
#define W3D_VERTEX_CHANNEL_TEXCOORD                             0x00000004
#define W3D_VERTEX_CHANNEL_COLOR                                0x00000008
#define W3D_VERTEX_CHANNEL_BONEID                               0x00000010

#define W3D_FACE_CHANNEL_FACE                                   0x00000001
// clang-format on

struct W3dTriStruct
{
    uint32_t Vindex[3];
    uint32_t Attributes;
    W3dVectorStruct Normal;
    float Dist;
};

struct W3dVertInfStruct
{
    uint16_t BoneIdx[2];
    uint16_t Weight[2];
};

struct W3dTexCoordStruct
{
    float U;
    float V;
};

struct W3dMaterialInfoStruct
{
    uint32_t PassCount;
    uint32_t VertexMaterialCount;
    uint32_t ShaderCount;
    uint32_t TextureCount;
};

struct W3dShaderStruct
{
    uint8_t DepthCompare;
    uint8_t DepthMask;
    uint8_t ColorMask;
    uint8_t DestBlend;
    uint8_t FogFunc;
    uint8_t PriGradient;
    uint8_t SecGradient;
    uint8_t SrcBlend;
    uint8_t Texturing;
    uint8_t DetailColorFunc;
    uint8_t DetailAlphaFunc;
    uint8_t ShaderPreset;
    uint8_t AlphaTest;
    uint8_t PostDetailColorFunc;
    uint8_t PostDetailAlphaFunc;
    uint8_t pad[1];
};

enum
{
    W3DSHADER_DEPTHCOMPARE_PASS_NEVER = 0,
    W3DSHADER_DEPTHCOMPARE_PASS_LESS,
    W3DSHADER_DEPTHCOMPARE_PASS_EQUAL,
    W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL,
    W3DSHADER_DEPTHCOMPARE_PASS_GREATER,
    W3DSHADER_DEPTHCOMPARE_PASS_NOTEQUAL,
    W3DSHADER_DEPTHCOMPARE_PASS_GEQUAL,
    W3DSHADER_DEPTHCOMPARE_PASS_ALWAYS,
    W3DSHADER_DEPTHCOMPARE_PASS_MAX,

    W3DSHADER_DEPTHMASK_WRITE_DISABLE = 0,
    W3DSHADER_DEPTHMASK_WRITE_ENABLE,
    W3DSHADER_DEPTHMASK_WRITE_MAX,

    W3DSHADER_ALPHATEST_DISABLE = 0,
    W3DSHADER_ALPHATEST_ENABLE,
    W3DSHADER_ALPHATEST_MAX,

    W3DSHADER_DESTBLENDFUNC_ZERO = 0,
    W3DSHADER_DESTBLENDFUNC_ONE,
    W3DSHADER_DESTBLENDFUNC_SRC_COLOR,
    W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR,
    W3DSHADER_DESTBLENDFUNC_SRC_ALPHA,
    W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA,
    W3DSHADER_DESTBLENDFUNC_SRC_COLOR_PREFOG,
    W3DSHADER_DESTBLENDFUNC_MAX,

    W3DSHADER_PRIGRADIENT_DISABLE = 0,
    W3DSHADER_PRIGRADIENT_MODULATE,
    W3DSHADER_PRIGRADIENT_ADD,
    W3DSHADER_PRIGRADIENT_BUMPENVMAP,
    W3DSHADER_PRIGRADIENT_BUMPENVMAPLUMINANCE,
    W3DSHADER_PRIGRADIENT_MODULATE2X,
    W3DSHADER_PRIGRADIENT_MAX,

    W3DSHADER_SECGRADIENT_DISABLE = 0,
    W3DSHADER_SECGRADIENT_ENABLE,
    W3DSHADER_SECGRADIENT_MAX,

    W3DSHADER_SRCBLENDFUNC_ZERO = 0,
    W3DSHADER_SRCBLENDFUNC_ONE,
    W3DSHADER_SRCBLENDFUNC_SRC_ALPHA,
    W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA,
    W3DSHADER_SRCBLENDFUNC_MAX,

    W3DSHADER_TEXTURING_DISABLE = 0,
    W3DSHADER_TEXTURING_ENABLE,
    W3DSHADER_TEXTURING_MAX,

    W3DSHADER_DETAILCOLORFUNC_DISABLE = 0,
    W3DSHADER_DETAILCOLORFUNC_DETAIL,
    W3DSHADER_DETAILCOLORFUNC_SCALE,
    W3DSHADER_DETAILCOLORFUNC_INVSCALE,
    W3DSHADER_DETAILCOLORFUNC_ADD,
    W3DSHADER_DETAILCOLORFUNC_SUB,
    W3DSHADER_DETAILCOLORFUNC_SUBR,
    W3DSHADER_DETAILCOLORFUNC_BLEND,
    W3DSHADER_DETAILCOLORFUNC_DETAILBLEND,
    W3DSHADER_DETAILCOLORFUNC_ADDSIGNED,
    W3DSHADER_DETAILCOLORFUNC_ADDSIGNED2X,
    W3DSHADER_DETAILCOLORFUNC_SCALE2X,
    W3DSHADER_DETAILCOLORFUNC_MODALPHAADDCLR,
    W3DSHADER_DETAILCOLORFUNC_MAX,

    W3DSHADER_DETAILALPHAFUNC_DISABLE = 0,
    W3DSHADER_DETAILALPHAFUNC_DETAIL,
    W3DSHADER_DETAILALPHAFUNC_SCALE,
    W3DSHADER_DETAILALPHAFUNC_INVSCALE,
    W3DSHADER_DETAILALPHAFUNC_MAX,

    W3DSHADER_DEPTHCOMPARE_DEFAULT = W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL,
    W3DSHADER_DEPTHMASK_DEFAULT = W3DSHADER_DEPTHMASK_WRITE_ENABLE,
    W3DSHADER_ALPHATEST_DEFAULT = W3DSHADER_ALPHATEST_DISABLE,
    W3DSHADER_DESTBLENDFUNC_DEFAULT = W3DSHADER_DESTBLENDFUNC_ZERO,
    W3DSHADER_PRIGRADIENT_DEFAULT = W3DSHADER_PRIGRADIENT_MODULATE,
    W3DSHADER_SECGRADIENT_DEFAULT = W3DSHADER_SECGRADIENT_DISABLE,
    W3DSHADER_SRCBLENDFUNC_DEFAULT = W3DSHADER_SRCBLENDFUNC_ONE,
    W3DSHADER_TEXTURING_DEFAULT = W3DSHADER_TEXTURING_DISABLE,
    W3DSHADER_DETAILCOLORFUNC_DEFAULT = W3DSHADER_DETAILCOLORFUNC_DISABLE,
    W3DSHADER_DETAILALPHAFUNC_DEFAULT = W3DSHADER_DETAILALPHAFUNC_DISABLE,
};

inline void W3d_Shader_Reset(W3dShaderStruct *s)
{
    s->DepthCompare = W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL;
    s->DepthMask = W3DSHADER_DEPTHMASK_WRITE_ENABLE;
    s->ColorMask = 0;
    s->DestBlend = W3DSHADER_DESTBLENDFUNC_ZERO;
    s->FogFunc = 0;
    s->PriGradient = W3DSHADER_PRIGRADIENT_MODULATE;
    s->SecGradient = W3DSHADER_SECGRADIENT_DISABLE;
    s->SrcBlend = W3DSHADER_SRCBLENDFUNC_ONE;
    s->Texturing = W3DSHADER_TEXTURING_DISABLE;
    s->DetailColorFunc = W3DSHADER_DETAILCOLORFUNC_DISABLE;
    s->DetailAlphaFunc = W3DSHADER_DETAILALPHAFUNC_DISABLE;
    s->ShaderPreset = 0;
    s->AlphaTest = W3DSHADER_ALPHATEST_DISABLE;
    s->PostDetailColorFunc = W3DSHADER_DETAILCOLORFUNC_DISABLE;
    s->PostDetailAlphaFunc = W3DSHADER_DETAILALPHAFUNC_DISABLE;
    s->pad[0] = 0;
}

inline int W3d_Shader_Get_Depth_Compare(const W3dShaderStruct *s)
{
    return s->DepthCompare;
}

inline int W3d_Shader_Get_Depth_Mask(const W3dShaderStruct *s)
{
    return s->DepthMask;
}

inline int W3d_Shader_Get_Dest_Blend_Func(const W3dShaderStruct *s)
{
    return s->DestBlend;
}

inline int W3d_Shader_Get_Pri_Gradient(const W3dShaderStruct *s)
{
    return s->PriGradient;
}

inline int W3d_Shader_Get_Sec_Gradient(const W3dShaderStruct *s)
{
    return s->SecGradient;
}

inline int W3d_Shader_Get_Src_Blend_Func(const W3dShaderStruct *s)
{
    return s->SrcBlend;
}

inline int W3d_Shader_Get_Texturing(const W3dShaderStruct *s)
{
    return s->Texturing;
}

inline int W3d_Shader_Get_Detail_Color_Func(const W3dShaderStruct *s)
{
    return s->DetailColorFunc;
}

inline int W3d_Shader_Get_Detail_Alpha_Func(const W3dShaderStruct *s)
{
    return s->DetailAlphaFunc;
}

inline int W3d_Shader_Get_Alpha_Test(const W3dShaderStruct *s)
{
    return s->AlphaTest;
}

inline int W3d_Shader_Get_Post_Detail_Color_Func(const W3dShaderStruct *s)
{
    return s->PostDetailColorFunc;
}

inline int W3d_Shader_Get_Post_Detail_Alpha_Func(const W3dShaderStruct *s)
{
    return s->PostDetailAlphaFunc;
}

inline void W3d_Shader_Set_Depth_Compare(W3dShaderStruct *s, int val)
{
    s->DepthCompare = val;
}
inline void W3d_Shader_Set_Depth_Mask(W3dShaderStruct *s, int val)
{
    s->DepthMask = val;
}
inline void W3d_Shader_Set_Dest_Blend_Func(W3dShaderStruct *s, int val)
{
    s->DestBlend = val;
}
inline void W3d_Shader_Set_Pri_Gradient(W3dShaderStruct *s, int val)
{
    s->PriGradient = val;
}
inline void W3d_Shader_Set_Sec_Gradient(W3dShaderStruct *s, int val)
{
    s->SecGradient = val;
}
inline void W3d_Shader_Set_Src_Blend_Func(W3dShaderStruct *s, int val)
{
    s->SrcBlend = val;
}
inline void W3d_Shader_Set_Texturing(W3dShaderStruct *s, int val)
{
    s->Texturing = val;
}
inline void W3d_Shader_Set_Detail_Color_Func(W3dShaderStruct *s, int val)
{
    s->DetailColorFunc = val;
}
inline void W3d_Shader_Set_Detail_Alpha_Func(W3dShaderStruct *s, int val)
{
    s->DetailAlphaFunc = val;
}
inline void W3d_Shader_Set_Alpha_Test(W3dShaderStruct *s, int val)
{
    s->AlphaTest = val;
}
inline void W3d_Shader_Set_Post_Detail_Color_Func(W3dShaderStruct *s, int val)
{
    s->PostDetailColorFunc = val;
}
inline void W3d_Shader_Set_Post_Detail_Alpha_Func(W3dShaderStruct *s, int val)
{
    s->PostDetailAlphaFunc = val;
}

struct W3dTextureInfoStruct
{
    uint16_t Attributes;
    uint16_t AnimType;
    uint32_t FrameCount;
    float FrameRate;
};

// clang-format off
#define W3DTEXTURE_PUBLISH                    0x0001
#define W3DTEXTURE_RESIZE_OBSOLETE            0x0002
#define W3DTEXTURE_NO_LOD                     0x0004
#define W3DTEXTURE_CLAMP_U                    0x0008
#define W3DTEXTURE_CLAMP_V                    0x0010
#define W3DTEXTURE_ALPHA_BITMAP               0x0020

#define W3DTEXTURE_MIP_LEVELS_MASK            0x00c0
#define W3DTEXTURE_MIP_LEVELS_ALL             0x0000
#define W3DTEXTURE_MIP_LEVELS_2               0x0040
#define W3DTEXTURE_MIP_LEVELS_3               0x0080
#define W3DTEXTURE_MIP_LEVELS_4               0x00c0

#define W3DTEXTURE_HINT_SHIFT                 8
#define W3DTEXTURE_HINT_MASK                  0x0000ff00

#define W3DTEXTURE_HINT_BASE                  0x0000
#define W3DTEXTURE_HINT_EMISSIVE              0x0100
#define W3DTEXTURE_HINT_ENVIRONMENT           0x0200
#define W3DTEXTURE_HINT_SHINY_MASK            0x0300

#define W3DTEXTURE_TYPE_MASK                  0x1000
#define W3DTEXTURE_TYPE_COLORMAP              0x0000
#define W3DTEXTURE_TYPE_BUMPMAP               0x1000

#define W3DTEXTURE_ANIM_LOOP                  0x0000
#define W3DTEXTURE_ANIM_PINGPONG              0x0001
#define W3DTEXTURE_ANIM_ONCE                  0x0002
#define W3DTEXTURE_ANIM_MANUAL                0x0003
// clang-format on

#define W3D_CURRENT_EMITTER_VERSION 0x00020000

enum
{
    EMITTER_TYPEID_DEFAULT = 0,
    EMITTER_TYPEID_COUNT
};

extern const char *s_emitterTypeNames[EMITTER_TYPEID_COUNT];

struct W3dRGBAStruct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
};

struct W3dEmitterHeaderStruct
{
    uint32_t Version;
    char Name[16];
};

struct W3dEmitterUserInfoStruct
{
    uint32_t Type;
    uint32_t SizeofStringParam;
    char StringParam[1];
};

struct W3dEmitterInfoStruct
{
    char TextureFilename[260];
    float StartSize;
    float EndSize;
    float Lifetime;
    float EmissionRate;
    float MaxEmissions;
    float VelocityRandom;
    float PositionRandom;
    float FadeTime;
    float Gravity;
    float Elasticity;
    W3dVectorStruct Velocity;
    W3dVectorStruct Acceleration;
    W3dRGBAStruct StartColor;
    W3dRGBAStruct EndColor;
};

struct W3dVolumeRandomizerStruct
{
    uint32_t ClassID;
    float Value1;
    float Value2;
    float Value3;
    uint32_t reserved[4];
};

struct W3dEmitterExtraInfoStruct
{
    float FutureStartTime;
    uint8_t unk1;
    uint32_t reserved[8];
};

// clang-format off
#define W3D_EMITTER_RENDER_MODE_TRI_PARTICLES  0
#define W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES 1
#define W3D_EMITTER_RENDER_MODE_LINE           2
#define W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA  3
#define W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM  4

#define W3D_EMITTER_FRAME_MODE_1x1   0
#define W3D_EMITTER_FRAME_MODE_2x2   1
#define W3D_EMITTER_FRAME_MODE_4x4   2
#define W3D_EMITTER_FRAME_MODE_8x8   3
#define W3D_EMITTER_FRAME_MODE_16x16 4
// clang-format on

struct W3dEmitterInfoStructV2
{
    uint32_t BurstSize;
    W3dVolumeRandomizerStruct CreationVolume;
    W3dVolumeRandomizerStruct VelRandom;
    float OutwardVel;
    float VelInherit;
    W3dShaderStruct Shader;
    uint32_t RenderMode;
    uint32_t FrameMode;
    uint32_t reserved[6];
};

struct W3dEmitterPropertyStruct
{
    uint32_t ColorKeyframes;
    uint32_t OpacityKeyframes;
    uint32_t SizeKeyframes;
    W3dRGBAStruct ColorRandom;
    float OpacityRandom;
    float SizeRandom;
    uint32_t reserved[4];
};

struct W3dEmitterColorKeyframeStruct
{
    float Time;
    W3dRGBAStruct Color;
};

struct W3dEmitterOpacityKeyframeStruct
{
    float Time;
    float Opacity;
};

struct W3dEmitterSizeKeyframeStruct
{
    float Time;
    float Size;
};

struct W3dEmitterRotationHeaderStruct
{
    uint32_t KeyframeCount;
    float Random; // random initial rotational velocity (rotations/sec)
    float OrientationRandom; // random initial orientation (rotations, 1.0=360deg)
    uint32_t Reserved[1];
};

struct W3dEmitterRotationKeyframeStruct
{
    float Time;
    float Rotation; // rotational velocity in rotations/sec
};

struct W3dEmitterFrameHeaderStruct
{
    uint32_t KeyframeCount;
    float Random;
    uint32_t Reserved[2];
};

struct W3dEmitterFrameKeyframeStruct
{
    float Time;
    float Frame;
};

struct W3dEmitterBlurTimeHeaderStruct
{
    uint32_t KeyframeCount;
    float Random;
    uint32_t Reserved[1];
};

struct W3dEmitterBlurTimeKeyframeStruct
{
    float Time;
    float BlurTime;
};

// clang-format off
#define W3D_ELINE_MERGE_INTERSECTIONS        0x00000001
#define W3D_ELINE_FREEZE_RANDOM              0x00000002
#define W3D_ELINE_DISABLE_SORTING            0x00000004
#define W3D_ELINE_END_CAPS                   0x00000008
#define W3D_ELINE_TEXTURE_MAP_MODE_MASK      0xFF000000
#define W3D_ELINE_TEXTURE_MAP_MODE_OFFSET    24
#define W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP  0x00000000
#define W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP 0x00000001
#define W3D_ELINE_TILED_TEXTURE_MAP          0x00000002
#define W3D_ELINE_DEFAULT_BITS \
    (W3D_ELINE_MERGE_INTERSECTIONS | (W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP << W3D_ELINE_TEXTURE_MAP_MODE_OFFSET))
// clang-format on

struct W3dEmitterLinePropertiesStruct
{
    uint32_t Flags;
    uint32_t SubdivisionLevel;
    float NoiseAmplitude;
    float MergeAbortFactor;
    float TextureTileFactor;
    float UPerSec;
    float VPerSec;
    uint32_t Reserved[9];
};

typedef enum
{
    SURFACE_TYPE_LIGHT_METAL = 0,
    SURFACE_TYPE_HEAVY_METAL,
    SURFACE_TYPE_WATER,
    SURFACE_TYPE_SAND,
    SURFACE_TYPE_DIRT,
    SURFACE_TYPE_MUD,
    SURFACE_TYPE_GRASS,
    SURFACE_TYPE_WOOD,
    SURFACE_TYPE_CONCRETE,
    SURFACE_TYPE_FLESH,
    SURFACE_TYPE_ROCK,
    SURFACE_TYPE_SNOW,
    SURFACE_TYPE_ICE,
    SURFACE_TYPE_DEFAULT,
    SURFACE_TYPE_GLASS,
    SURFACE_TYPE_CLOTH,
    SURFACE_TYPE_TIBERIUM_FIELD,
    SURFACE_TYPE_FOLIAGE_PERMEABLE,
    SURFACE_TYPE_GLASS_PERMEABLE,
    SURFACE_TYPE_ICE_PERMEABLE,
    SURFACE_TYPE_CLOTH_PERMEABLE,
    SURFACE_TYPE_ELECTRICAL,
    SURFACE_TYPE_FLAMMABLE,
    SURFACE_TYPE_STEAM,
    SURFACE_TYPE_ELECTRICAL_PERMEABLE,
    SURFACE_TYPE_FLAMMABLE_PERMEABLE,
    SURFACE_TYPE_STEAM_PERMEABLE,
    SURFACE_TYPE_WATER_PERMEABLE,
    SURFACE_TYPE_TIBERIUM_WATER,
    SURFACE_TYPE_TIBERIUM_WATER_PERMEABLE,
    SURFACE_TYPE_UNDERWATER_DIRT,
    SURFACE_TYPE_UNDERWATER_TIBERIUM_DIRT,
    SURFACE_TYPE_MAX
} W3D_SURFACE_TYPES;
