/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D dump tool
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "bufffile.h"
#include "chunkio.h"
#include "resource.h"
#include "simplevec.h"
#include "vector.h"
#include "vector2.h"
#include "vector3.h"
#include "vector3i.h"
#include "w3d_file.h"
#include "w3d_obsolete.h"
#include <commctrl.h>
#include <commdlg.h>
#include <unordered_map>

HWND g_mainwnd = nullptr;
HWND g_treewnd = nullptr;
HWND g_listwnd = nullptr;
HMENU g_menu = nullptr;
HACCEL g_accel = nullptr;
int g_mainwidth = 0;
int g_mainheight = 0;

#define CLASS_NAME "WDUMP"
#define WND_TITLE "wdump"

struct ChunkInfo
{
    StringClass name;
    StringClass type;
    StringClass value;
};

struct ChunkData
{
    StringClass name;
    SimpleDynVecClass<ChunkInfo *> data;
    SimpleDynVecClass<ChunkData *> subchunks;
    DynamicVectorClass<StringClass> unknowndata;

    ~ChunkData()
    {
        for (int i = 0; i < data.Count(); i++) {
            delete data[i];
        }

        for (int i = 0; i < subchunks.Count(); i++) {
            delete subchunks[i];
        }
    }
};

struct ChunkDumper
{
    const char *name;
    void (*function)(ChunkLoadClass &cload, ChunkData *data);
};

std::unordered_map<int, ChunkDumper> g_chunkDumpMap;

void AddString(ChunkData *data, const char *name, const char *value, const char *type)
{
    ChunkInfo *c = new ChunkInfo;
    c->value = value;
    c->name = name;
    c->type = type;
    data->data.Add(c);
}

void AddVersion(ChunkData *data, uint32_t value)
{
    char c[64];
    sprintf(c, "%u.%hu", value >> 16, (uint16_t)value);
    AddString(data, "Version", c, "string");
}

void AddInt32(ChunkData *data, const char *name, uint32_t value)
{
    char c[256];
    sprintf(c, "%u", value);
    AddString(data, name, c, "int32");
}

void AddInt16(ChunkData *data, const char *name, uint16_t value)
{
    char c[256];
    sprintf(c, "%hu", value);
    AddString(data, name, c, "int16");
}

void AddInt8(ChunkData *data, const char *name, uint8_t value)
{
    char c[256];
    sprintf(c, "%hhu", value);
    AddString(data, name, c, "int8");
}

void AddInt8Array(ChunkData *data, const char *name, uint8_t *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("%hhu ", values[i]);
        str += str2;
    }
    char c[256];
    sprintf(c, "int8[%d]", count);
    AddString(data, name, str, c);
}

void AddFloat(ChunkData *data, const char *name, float value)
{
    char c[256];
    sprintf(c, "%f", value);
    AddString(data, name, c, "float");
}

void AddInt32Array(ChunkData *data, const char *name, uint32_t *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("%u ", values[i]);
        str += str2;
    }
    char c[256];
    sprintf(c, "int32[%d]", count);
    AddString(data, name, str, c);
}

void AddFloatArray(ChunkData *data, const char *name, float *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("%f ", values[i]);
        str += str2;
    }
    char c[256];
    sprintf(c, "float[%d]", count);
    AddString(data, name, str, c);
}

void AddVector(ChunkData *data, const char *name, W3dVectorStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f", value->x, value->y, value->z);
    AddString(data, name, c, "vector");
}

void AddQuaternion(ChunkData *data, const char *name, W3dQuaternionStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f %f", value->q[0], value->q[1], value->q[2], value->q[3]);
    AddString(data, name, c, "quaternion");
}

void AddRGB(ChunkData *data, const char *name, W3dRGBStruct *value)
{
    StringClass str;
    str.Format("(%hhu %hhu %hhu) ", value->r, value->g, value->b);
    AddString(data, name, str, "RGB");
}

void AddRGBArray(ChunkData *data, const char *name, W3dRGBStruct *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("(%hhu %hhu %hhu) ", values[i].r, values[i].g, values[i].b);
        str += str2;
    }
    char c[256];
    sprintf(c, "float[%d]", count);
    AddString(data, name, str, c);
}

void AddRGBA(ChunkData *data, const char *name, W3dRGBAStruct *value)
{
    StringClass str;
    str.Format("(%hhu %hhu %hhu %hhu) ", value->R, value->G, value->B, value->A);
    AddString(data, name, str, "RGBA");
}

void AddTexCoord(ChunkData *data, const char *name, W3dTexCoordStruct *value)
{
    char c[256];
    sprintf(c, "%f %f", value->U, value->V);
    AddString(data, name, c, "UV");
}

void AddTexCoordArray(ChunkData *data, const char *name, W3dTexCoordStruct *values, int count)
{
    StringClass str;
    for (int i = 0; i < count; i++) {
        char c[256];
        sprintf(c, "%s.TexCoord[%d]", name, i);
        AddTexCoord(data, c, &values[i]);
    }
}

const char *DepthCompareValues[] = { "Pass Never",
    "Pass Less",
    "Pass Equal",
    "Pass Less or Equal",
    "Pass Greater",
    "Pass Not Equal",
    "Pass Greater or Equal",
    "Pass Always" };

const char *DepthMaskValues[] = { "Write Disable", "Write Enable", "Write Disable", "Write Enable" };

const char *DestBlendValues[] = { "Zero",
    "One",
    "Src Color",
    "One Minus Src Color",
    "Src Alpha",
    "One Minus Src Alpha",
    "Src Color Prefog",
    "Disable",
    "Enable",
    "Scale Fragment",
    "Replace Fragment" };

const char *PriGradientValues[] = {
    "Disable", "Modulate", "Add", "Bump-Environment", "Bump-Environment Luminance", "Modulate 2x"
};

const char *SecGradientValues[] = { "Disable", "Enable" };

const char *SrcBlendValues[] = { "Zero", "One", "Src Alpha", "One Minus Src Alpha" };

const char *TexturingValues[] = { "Disable", "Enable" };

const char *DetailColorValues[] = { "Disable",
    "Detail",
    "Scale",
    "InvScale",
    "Add",
    "Sub",
    "SubR",
    "Blend",
    "DetailBlend",
    "Add Signed",
    "Add Signed 2x",
    "Scale 2x",
    "Mod Alpha Add Color" };

const char *DetailAlphaValues[] = { "Disable", "Detail", "Scale", "InvScale", "Disable", "Enable", "Smooth", "Flat" };

const char *AlphaTestValues[] = { "Alpha Test Disable", "Alpha Test Enable" };

void AddShader(ChunkData *data, const char *name, W3dShaderStruct *value)
{
    char c[256];
    sprintf(c, "%s.DepthCompare", name);

    if (value->DepthCompare < W3DSHADER_DEPTHCOMPARE_PASS_MAX) {
        AddString(data, c, DepthCompareValues[value->DepthCompare], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Depth Compare type %x", c, value->DepthCompare);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.DepthMask", name);

    if (value->DepthMask < W3DSHADER_DEPTHMASK_WRITE_MAX) {
        AddString(data, c, DepthMaskValues[value->DepthMask], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Depth Mask type %x", c, value->DepthMask);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.DestBlend", name);

    if (value->DestBlend < W3DSHADER_DESTBLENDFUNC_MAX) {
        AddString(data, c, DestBlendValues[value->DestBlend], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Dest Blend type %x", c, value->DestBlend);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.PriGradient", name);

    if (value->PriGradient < W3DSHADER_PRIGRADIENT_MAX) {
        AddString(data, c, PriGradientValues[value->PriGradient], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Primary Gradient type %x", c, value->PriGradient);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.SecGradient", name);

    if (value->SecGradient < W3DSHADER_SECGRADIENT_MAX) {
        AddString(data, c, SecGradientValues[value->SecGradient], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Secondary Gradient type %x", c, value->SecGradient);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.SrcBlend", name);

    if (value->SrcBlend < W3DSHADER_SRCBLENDFUNC_MAX) {
        AddString(data, c, SrcBlendValues[value->SrcBlend], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Src Blend type %x", c, value->SrcBlend);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.Texturing", name);

    if (value->Texturing < W3DSHADER_TEXTURING_MAX) {
        AddString(data, c, TexturingValues[value->Texturing], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Texturing type %x", c, value->Texturing);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.DetailColor", name);

    if (value->DetailColorFunc < W3DSHADER_DETAILCOLORFUNC_MAX) {
        AddString(data, c, DetailColorValues[value->DetailColorFunc], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Detail Color Func type %x", c, value->DetailColorFunc);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.DetailAlpha", name);

    if (value->DetailAlphaFunc < W3DSHADER_DETAILALPHAFUNC_MAX) {
        AddString(data, c, DetailAlphaValues[value->DetailAlphaFunc], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Detail Alpha Func type %x", c, value->DetailAlphaFunc);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }

    sprintf(c, "%s.AlphaTest", name);

    if (value->AlphaTest < W3DSHADER_ALPHATEST_MAX) {
        AddString(data, c, AlphaTestValues[value->AlphaTest], "string");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Alpha Test type %x", c, value->AlphaTest);
        data->unknowndata.Add(str);
        AddString(data, c, "Unknown", "string");
    }
}

const char *PS2DepthCompareValues[] = { "Pass Never", "Pass Less", "Pass Always", "Pass Less or Equal" };

const char *PS2DepthMaskValues[] = { "Write Disable", "Write Enable", "Write Disable", "Write Enable" };

const char *PS2ABDParamValues[] = { "Src Color", "Dest Color", "Zero" };

const char *PS2CParamValues[] = {
    "Src Alpha", "Dest Alpha", "One", "Disable", "Enable", "Scale Fragment", "Replace Fragment"
};

const char *PS2PriGradientValues[] = { "Disable", "Modulate", "Highlight", "Highlight2", "Disable", "Enable" };

const char *PS2TexturingValues[] = { "Disable",
    "Enable",
    "Disable",
    "Detail",
    "Scale",
    "InvScale",
    "Add",
    "Sub",
    "SubR",
    "Blend",
    "DetailBlend",
    "Disable",
    "Detail",
    "Scale",
    "InvScale",
    "Disable",
    "Enable",
    "Smooth",
    "Flat" };

void AddPS2Shader(ChunkData *data, const char *name, W3dPS2ShaderStruct *value)
{
    char c[256];
    sprintf(c, "%s.DepthCompare", name);
    AddString(data, c, PS2DepthCompareValues[value->DepthCompare], "string");
    sprintf(c, "%s.DepthMask", name);
    AddString(data, c, PS2DepthMaskValues[value->DepthMask], "string");
    sprintf(c, "%s.PriGradient", name);
    AddString(data, c, PS2PriGradientValues[value->PriGradient], "string");
    sprintf(c, "%s.Texturing", name);
    AddString(data, c, PS2TexturingValues[value->Texturing], "string");
    sprintf(c, "%s.AParam", name);
    AddString(data, c, PS2ABDParamValues[value->AParam], "string");
    sprintf(c, "%s.BParam", name);
    AddString(data, c, PS2ABDParamValues[value->BParam], "string");
    sprintf(c, "%s.CParam", name);
    AddString(data, c, PS2CParamValues[value->CParam], "string");
    sprintf(c, "%s.DParam", name);
    AddString(data, c, PS2ABDParamValues[value->DParam], "string");
}

void AddIJK(ChunkData *data, const char *name, Vector3i *value)
{
    char c[256];
    sprintf(c, "%d %d %d", value->I, value->J, value->K);
    AddString(data, name, c, "IJK");
}

void AddIJK16(ChunkData *data, const char *name, Vector3i16 *value)
{
    char c[256];
    sprintf(c, "%d %d %d", value->I, value->J, value->K);
    AddString(data, name, c, "IJK");
}

char *ReadChunkData(ChunkLoadClass &cload)
{
    if (!cload.Cur_Chunk_Length()) {
        char *c = new char[1];
        c[0] = 0;
        return c;
    }
    char *c = new char[cload.Cur_Chunk_Length()];
    cload.Read(c, cload.Cur_Chunk_Length());
    return c;
}

char table[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void ParseSubchunks(ChunkLoadClass &cload, ChunkData *data)
{
    while (cload.Open_Chunk()) {
        auto iter = g_chunkDumpMap.find(cload.Cur_Chunk_ID());

        if (iter == g_chunkDumpMap.end()) {
            StringClass str;
            str.Format("Unknown Chunk %x", cload.Cur_Chunk_ID());
            data->unknowndata.Add(str);
            ChunkData *d = new ChunkData;
            data->subchunks.Add(d);
            StringClass str2;
            str2.Format("%x", cload.Cur_Chunk_ID());
            d->name = str2;
            char *chunkdata = ReadChunkData(cload);
            StringClass str3;

            for (unsigned int i = 0; i < cload.Cur_Chunk_Length(); i++) {
                unsigned char n = chunkdata[i];
                str3 += table[n >> 4];
                str3 += table[n & 0xf];
                str3 += ' ';
            }

            AddString(d, "Chunk Data", str3, "Unknown");
            delete[] chunkdata;
        } else {
            ChunkData *d = new ChunkData;
            data->subchunks.Add(d);
            d->name = iter->second.name;
            iter->second.function(cload, d);
        }

        cload.Close_Chunk();
    }
}

void Dump_O_W3D_CHUNK_MATERIALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMaterialStruct *materials = (W3dMaterialStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dMaterialStruct); i++) {
        char c[256];
        sprintf(c, "Material[%d].MaterialName", i);
        AddString(data, c, materials[i].MaterialName, "string");
        sprintf(c, "Material[%d].PrimaryName", i);
        AddString(data, c, materials[i].PrimaryName, "string");
        sprintf(c, "Material[%d].SecondaryName", i);
        AddString(data, c, materials[i].SecondaryName, "string");
        sprintf(c, "Material[%d].RenderFlags", i);
        AddInt32(data, c, materials[i].RenderFlags);
        sprintf(c, "Material[%d].Red", i);
        AddInt8(data, c, materials[i].Red);
        sprintf(c, "Material[%d].Green", i);
        AddInt8(data, c, materials[i].Green);
        sprintf(c, "Material[%d].Blue", i);
        AddInt8(data, c, materials[i].Blue);
    }

    delete[] chunkdata;
}

void Dump_O_W3D_CHUNK_MATERIALS2(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMaterial2Struct *materials = (W3dMaterial2Struct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dMaterial2Struct); i++) {
        char c[256];
        sprintf(c, "Material[%d].MaterialName", i);
        AddString(data, c, materials[i].MaterialName, "string");
        sprintf(c, "Material[%d].PrimaryName", i);
        AddString(data, c, materials[i].PrimaryName, "string");
        sprintf(c, "Material[%d].SecondaryName", i);
        AddString(data, c, materials[i].SecondaryName, "string");
        sprintf(c, "Material[%d].RenderFlags", i);
        AddInt32(data, c, materials[i].RenderFlags);
        sprintf(c, "Material[%d].Red", i);
        AddInt8(data, c, materials[i].Red);
        sprintf(c, "Material[%d].Green", i);
        AddInt8(data, c, materials[i].Green);
        sprintf(c, "Material[%d].Blue", i);
        AddInt8(data, c, materials[i].Blue);
        sprintf(c, "Material[%d].Alpha", i);
        AddInt8(data, c, materials[i].Alpha);
        sprintf(c, "Material[%d].PrimaryNumFrames", i);
        AddInt16(data, c, materials[i].PrimaryNumFrames);
        sprintf(c, "Material[%d].SecondaryNumFrames", i);
        AddInt16(data, c, materials[i].SecondaryNumFrames);
    }

    delete[] chunkdata;
}

void Dump_O_W3D_CHUNK_POV_QUADRANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    AddString(data, "Contact Greg if you need to look at this!", "unsupported", "string");
}

void Dump_O_W3D_CHUNK_POV_TRIANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    AddString(data, "Contact Greg if you need to look at this!", "unsupported", "string");
}

void Dump_O_W3D_CHUNK_QUADRANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Outdated structure", "", "string");
    delete[] chunkdata;
}

void Dump_O_W3D_CHUNK_SURRENDER_TRIANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dSurrenderTriangleStruct *triangles = (W3dSurrenderTriangleStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dSurrenderTriangleStruct); i++) {
        char c[256];
        sprintf(c, "Triangle[%d].Attributes", i);
        AddInt32(data, c, triangles[i].Attributes);
        sprintf(c, "Triangle[%d].Gouraud", i);
        AddRGBArray(data, c, triangles[i].Gourad, 3);
        sprintf(c, "Triangle[%d].VertexIndices", i);
        AddInt32Array(data, c, triangles[i].VertexIndices, 3);
        sprintf(c, "Triangle[%d].MaterialIdx", i);
        AddInt32(data, c, triangles[i].MaterialIdx);
        sprintf(c, "Triangle[%d].Normal", i);
        AddVector(data, c, &triangles[i].Normal);
        sprintf(c, "Triangle[%d].TexCoord", i);
        AddTexCoordArray(data, c, triangles[i].TexCoord, 3);
    }

    delete[] chunkdata;
}

void Dump_O_W3D_CHUNK_TRIANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Obsolete structure", "", "string");
    delete[] chunkdata;
}

void Dump_OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterColorKeyframeStruct *frame = (W3dEmitterColorKeyframeStruct *)chunkdata;
    AddFloat(data, "Time", frame->Time);
    AddRGBA(data, "Color", &frame->Color);
    delete[] chunkdata;
}

void Dump_OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterOpacityKeyframeStruct *frame = (W3dEmitterOpacityKeyframeStruct *)chunkdata;
    AddFloat(data, "Time", frame->Time);
    AddFloat(data, "Opacity", frame->Opacity);
    delete[] chunkdata;
}

void Dump_OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterSizeKeyframeStruct *frame = (W3dEmitterSizeKeyframeStruct *)chunkdata;
    AddFloat(data, "Time", frame->Time);
    AddFloat(data, "Size", frame->Size);
    delete[] chunkdata;
}

void Dump_OBSOLETE_W3D_CHUNK_SHADOW_NODE(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHModelNodeStruct *node = (W3dHModelNodeStruct *)chunkdata;
    AddString(data, "ShadowMeshName", node->RenderObjName, "string");
    AddInt16(data, "PivotIdx", node->PivotIdx);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_AABTREE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_AABTREE_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMeshAABTreeHeader *header = (W3dMeshAABTreeHeader *)chunkdata;
    AddInt32(data, "NodeCount", header->NodeCount);
    AddInt32(data, "PolyCount", header->PolyCount);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_AABTREE_NODES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMeshAABTreeNode *nodes = (W3dMeshAABTreeNode *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dMeshAABTreeNode); i++) {
        char c[256];
        sprintf(c, "Node[%d].Min", i);
        AddVector(data, c, &nodes[i].Min);
        sprintf(c, "Node[%d].Max", i);
        AddVector(data, c, &nodes[i].Max);

        if ((nodes[i].FrontOrPoly0 & 0x80000000) == 0) {
            sprintf(c, "Node[%d].Front", i);
            AddInt32(data, c, nodes[i].FrontOrPoly0);
            sprintf(c, "Node[%d].Back", i);
        } else {
            sprintf(c, "Node[%d].Poly0", i);
            AddInt32(data, c, nodes[i].FrontOrPoly0 & 0x7FFFFFFF);
            sprintf(c, "Node[%d].PolyCount", i);
        }

        AddInt32(data, c, nodes[i].BackOrPolyCount);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_AABTREE_POLYINDICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *indices = (uint32_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Polygon Index[%d]", i);
        AddInt32(data, c, indices[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_AGGREGATE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_AGGREGATE_CLASS_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAggregateMiscInfo *info = (W3dAggregateMiscInfo *)chunkdata;
    AddInt32(data, "OriginalClassID", info->OriginalClassID);
    AddInt32(data, "Flags", info->Flags);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_AGGREGATE_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAggregateHeaderStruct *header = (W3dAggregateHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_AGGREGATE_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAggregateInfoStruct *info = (W3dAggregateInfoStruct *)chunkdata;
    AddString(data, "BaseModelName", info->BaseModelName, "string");
    AddInt32(data, "SubobjectCount", info->SubobjectCount);
    W3dAggregateSubobjectStruct *subobj = (W3dAggregateSubobjectStruct *)(chunkdata + sizeof(W3dAggregateInfoStruct));

    for (unsigned int i = 0; i < info->SubobjectCount; i++) {
        char c[256];
        sprintf(c, "SubObject[%u].SubobjectName", i);
        AddString(data, c, subobj[i].SubobjectName, "string");
        sprintf(c, "SubObject[%u].BoneName", i);
        AddString(data, c, subobj[i].BoneName, "string");
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_ANIMATION(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

const char *ChannelTypes[] = { "X Translation",
    "Y Translation",
    "Z Translation",
    "X Rotation",
    "Y Rotation",
    "Z Rotation",
    "Quaternion",
    "Timecoded X Translation",
    "Timecoded Y Translation",
    "Timecoded Z Translation",
    "Timecoded Quaternion",
    "Adaptive Delta X Translation",
    "Adaptive Delta Y Translation",
    "Adaptive Delta Z Translation",
    "Adaptive Delta Quaternion",
    "Vis" };

void Dump_W3D_CHUNK_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAnimChannelStruct *channel = (W3dAnimChannelStruct *)chunkdata;
    AddInt16(data, "FirstFrame", channel->FirstFrame);
    AddInt16(data, "LastFrame", channel->LastFrame);

    if (channel->Flags <= ANIM_CHANNEL_VIS) {
        AddString(data, "ChannelType", ChannelTypes[channel->Flags], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        data->unknowndata.Add(str);
        AddString(data, "Channel Type", "Unknown", "string");
    }

    AddInt16(data, "Pivot", channel->Pivot);
    AddInt16(data, "VectorLen", channel->VectorLen);

    for (int i = 0; i < channel->LastFrame - channel->FirstFrame; i++) {
        for (int j = 0; j < channel->VectorLen; j++) {
            StringClass str;
            str.Format("Data[%d][%d]", i, j);
            AddFloat(data, str, channel->Data[j + i * channel->VectorLen]);
        }
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_ANIMATION_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAnimHeaderStruct *header = (W3dAnimHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddString(data, "HierarchyName", header->HierarchyName, "string");
    AddInt32(data, "NumFrames", header->NumFrames);
    AddInt32(data, "FrameRate", header->FrameRate);
    delete[] chunkdata;
}

const char *BitChannelTypes[] = { "Visibility", "Timecoded Visibility" };

bool UnpackBitChannel(uint8_t *data, uint32_t bit)
{
    return (data[bit / 8] & (1 << bit % 8)) != 0;
}

void Dump_W3D_CHUNK_BIT_CHANNEL(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dBitChannelStruct *channel = (W3dBitChannelStruct *)chunkdata;
    AddInt16(data, "FirstFrame", channel->FirstFrame);
    AddInt16(data, "LastFrame", channel->LastFrame);

    if (channel->Flags <= BIT_CHANNEL_TIMECODED_VIS) {
        AddString(data, "ChannelType", BitChannelTypes[channel->Flags], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        data->unknowndata.Add(str);
        AddString(data, "Channel Type", "Unknown", "string");
    }

    AddInt16(data, "Pivot", channel->Pivot);
    AddInt8(data, "Default Value", channel->DefaultVal);

    for (int i = 0; i < channel->LastFrame - channel->FirstFrame; i++) {
        StringClass str;
        str.Format("Data[%d]", i + channel->FirstFrame);
        bool b = UnpackBitChannel(channel->Data, i);
        AddInt8(data, str, b);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_BOX(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dBoxStruct *box = (W3dBoxStruct *)chunkdata;
    AddVersion(data, box->Version);
    AddInt32(data, "Attributes", box->Attributes);

    if (box->Attributes & W3D_BOX_ATTRIBUTE_ORIENTED) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBUTE_ORIENTED", "flag");
    }

    if (box->Attributes & W3D_BOX_ATTRIBUTE_ALIGNED) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBUTE_ALIGNED", "flag");
    }

    if (box->Attributes & 4) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attribute 0x00000004");
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    if (box->Attributes & 8) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attribute 0x00000008");
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL", "flag");
    }

    if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE", "flag");
    }

    if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS", "flag");
    }

    if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA", "flag");
    }

    if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE) {
        AddString(data, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE", "flag");
    }

    if (box->Attributes & 0xFFFFFE00) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attributes %x", box->Attributes & 0xFFFFFE00);
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    AddString(data, "Name", box->Name, "string");
    AddRGB(data, "Color", &box->Color);
    AddVector(data, "Center", &box->Center);
    AddVector(data, "Extent", &box->Extent);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_COLLECTION(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_COLLECTION_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCollectionHeaderStruct *header = (W3dCollectionHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddInt32(data, "RenderObjectCount", header->RenderObjectCount);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_COLLECTION_OBJ_NAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Render Object Name", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_COLLISION_NODE(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHModelNodeStruct *node = (W3dHModelNodeStruct *)chunkdata;
    AddString(data, "CollisionMeshName", node->RenderObjName, "string");
    AddInt16(data, "PivotIdx", node->PivotIdx);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DAMAGE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_DAMAGE_COLORS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dDamageColorStruct *colors = (W3dDamageColorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dDamageColorStruct); i++) {
        char c[256];
        sprintf(c, "DamageColorStruct[%d].VertexIndex", i);
        AddInt32(data, c, colors[i].VertexIndex);
        sprintf(c, "DamageColorStruct[%d].NewColor", i);
        AddRGB(data, c, &colors[i].NewColor);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DAMAGE_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dDamageStruct *damage = (W3dDamageStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dDamageStruct); i++) {
        char c[256];
        sprintf(c, "DamageStruct[%d].NumDamageMaterials", i);
        AddInt32(data, c, damage[i].NumDamageMaterials);
        sprintf(c, "DamageStruct[%d].NumDamageVerts", i);
        AddInt32(data, c, damage[i].NumDamageVerts);
        sprintf(c, "DamageStruct[%d].NumDamageColors", i);
        AddInt32(data, c, damage[i].NumDamageColors);
        sprintf(c, "DamageStruct[%d].DamageIndex", i);
        AddInt32(data, c, damage[i].DamageIndex);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DAMAGE_VERTICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dDamageVertexStruct *vertices = (W3dDamageVertexStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dDamageVertexStruct); i++) {
        char c[256];
        sprintf(c, "DamageVertexStruct[%d].VertexIndex", i);
        AddInt32(data, c, vertices[i].VertexIndex);
        sprintf(c, "DamageVertexStruct[%d].NewVertex", i);
        AddInt32(data, c, vertices[i].VertexIndex);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DAZZLE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_DAZZLE_NAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Dazzle Name", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DAZZLE_TYPENAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Dazzle Type Name", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DCG(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dRGBAStruct *colors = (W3dRGBAStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dRGBAStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d].DCG", i);
        AddRGBA(data, c, &colors[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_DIG(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dRGBStruct *colors = (W3dRGBStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dRGBStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d].DIG", i);
        AddRGB(data, c, &colors[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterBlurTimeHeaderStruct *header = (W3dEmitterBlurTimeHeaderStruct *)chunkdata;
    AddInt32(data, "KeyframeCount", header->KeyframeCount);
    AddFloat(data, "Random", header->Random);
    W3dEmitterBlurTimeKeyframeStruct *blurtime =
        (W3dEmitterBlurTimeKeyframeStruct *)(chunkdata + sizeof(W3dEmitterBlurTimeHeaderStruct));

    for (unsigned int i = 0; i < header->KeyframeCount + 1; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, blurtime[i].Time);
        sprintf(c, "BlurTime[%u]", i);
        AddFloat(data, c, blurtime[i].BlurTime);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterFrameHeaderStruct *header = (W3dEmitterFrameHeaderStruct *)chunkdata;
    AddInt32(data, "KeyframeCount", header->KeyframeCount);
    AddFloat(data, "Random", header->Random);
    W3dEmitterFrameKeyframeStruct *frame =
        (W3dEmitterFrameKeyframeStruct *)(chunkdata + sizeof(W3dEmitterFrameHeaderStruct));

    for (unsigned int i = 0; i < header->KeyframeCount + 1; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, frame[i].Time);
        sprintf(c, "Frame[%u]", i);
        AddFloat(data, c, frame[i].Frame);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterHeaderStruct *header = (W3dEmitterHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterInfoStruct *info = (W3dEmitterInfoStruct *)chunkdata;
    AddString(data, "Texture Name", info->TextureFilename, "string");
    AddFloat(data, "StartSize", info->StartSize);
    AddFloat(data, "EndSize", info->EndSize);
    AddFloat(data, "Lifetime", info->Lifetime);
    AddFloat(data, "EmissionRate", info->EmissionRate);
    AddFloat(data, "MaxEmissions", info->MaxEmissions);
    AddFloat(data, "VelocityRandom", info->VelocityRandom);
    AddFloat(data, "PositionRandom", info->PositionRandom);
    AddFloat(data, "FadeTime", info->FadeTime);
    AddFloat(data, "Gravity", info->Gravity);
    AddFloat(data, "Elasticity", info->Elasticity);
    AddVector(data, "Velocity", &info->Velocity);
    AddVector(data, "Acceleration", &info->Acceleration);
    AddRGBA(data, "StartColor", &info->StartColor);
    AddRGBA(data, "EndColor", &info->EndColor);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_INFOV2(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterInfoStructV2 *info = (W3dEmitterInfoStructV2 *)chunkdata;
    AddInt32(data, "BurstSize", info->BurstSize);
    AddInt32(data, "CreationVolume.ClassID", info->CreationVolume.ClassID);
    AddFloat(data, "CreationVolume.Value1", info->CreationVolume.Value1);
    AddFloat(data, "CreationVolume.Value2", info->CreationVolume.Value2);
    AddFloat(data, "CreationVolume.Value3", info->CreationVolume.Value3);
    AddInt32(data, "VelRandom.ClassID", info->VelRandom.ClassID);
    AddFloat(data, "VelRandom.Value1", info->VelRandom.Value1);
    AddFloat(data, "VelRandom.Value2", info->VelRandom.Value2);
    AddFloat(data, "VelRandom.Value3", info->VelRandom.Value3);
    AddFloat(data, "OutwardVel", info->OutwardVel);
    AddFloat(data, "VelInherit", info->VelInherit);
    AddShader(data, "Shader", &info->Shader);
    AddInt32(data, "RenderMode", info->RenderMode);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_PROPS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterPropertyStruct *props = (W3dEmitterPropertyStruct *)chunkdata;
    AddInt32(data, "ColorKeyframes", props->ColorKeyframes);
    AddInt32(data, "OpacityKeyframes", props->OpacityKeyframes);
    AddInt32(data, "SizeKeyframes", props->SizeKeyframes);
    AddRGBA(data, "ColorRandom", &props->ColorRandom);
    AddFloat(data, "OpacityRandom", props->OpacityRandom);
    AddFloat(data, "SizeRandom", props->SizeRandom);
    W3dEmitterColorKeyframeStruct *color = (W3dEmitterColorKeyframeStruct *)(chunkdata + sizeof(W3dEmitterPropertyStruct));

    for (unsigned int i = 0; i < props->ColorKeyframes; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, color[i].Time);
        sprintf(c, "Color[%u]", i);
        AddRGBA(data, c, &color[i].Color);
    }

    W3dEmitterOpacityKeyframeStruct *opacity = (W3dEmitterOpacityKeyframeStruct *)(chunkdata
        + sizeof(W3dEmitterPropertyStruct) + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct)));

    for (unsigned int i = 0; i < props->OpacityKeyframes; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, opacity[i].Time);
        sprintf(c, "Opacity[%u]", i);
        AddFloat(data, c, opacity[i].Opacity);
    }

    W3dEmitterSizeKeyframeStruct *size = (W3dEmitterSizeKeyframeStruct *)(chunkdata + sizeof(W3dEmitterPropertyStruct)
        + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct))
        + (props->OpacityKeyframes * sizeof(W3dEmitterOpacityKeyframeStruct)));

    for (unsigned int i = 0; i < props->OpacityKeyframes; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, size[i].Time);
        sprintf(c, "Size[%u]", i);
        AddFloat(data, c, size[i].Size);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterRotationHeaderStruct *header = (W3dEmitterRotationHeaderStruct *)chunkdata;
    AddInt32(data, "KeyframeCount", header->KeyframeCount);
    AddFloat(data, "Random", header->Random);
    AddFloat(data, "OrientationRandom", header->OrientationRandom);
    W3dEmitterRotationKeyframeStruct *frame =
        (W3dEmitterRotationKeyframeStruct *)(chunkdata + sizeof(W3dEmitterRotationHeaderStruct));

    for (unsigned int i = 0; i < header->KeyframeCount + 1; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, frame[i].Time);
        sprintf(c, "Rotation[%u]", i);
        AddFloat(data, c, frame[i].Rotation);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_USER_DATA(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "User Data", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_FAR_ATTENUATION(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLightAttenuationStruct *atten = (W3dLightAttenuationStruct *)chunkdata;
    AddFloat(data, "Far Atten Start", atten->Start);
    AddFloat(data, "Far Atten End", atten->End);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_HIERARCHY(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_HIERARCHY_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHierarchyStruct *header = (W3dHierarchyStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddInt32(data, "NumPivots", header->NumPivots);
    AddVector(data, "Center", &header->Center);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_HLOD(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_HLOD_AGGREGATE_ARRAY(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_HLOD_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHLodHeaderStruct *header = (W3dHLodHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddInt32(data, "LodCount", header->LodCount);
    AddString(data, "Name", header->Name, "string");
    AddString(data, "HTree Name", header->HierarchyName, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_HLOD_LOD_ARRAY(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHLodArrayHeaderStruct *header = (W3dHLodArrayHeaderStruct *)chunkdata;
    AddInt32(data, "ModelCount", header->ModelCount);
    AddFloat(data, "MaxScreenSize", header->MaxScreenSize);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_HLOD_PROXY_ARRAY(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_HLOD_SUB_OBJECT(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHLodSubObjectStruct *obj = (W3dHLodSubObjectStruct *)chunkdata;
    AddString(data, "Name", obj->Name, "string");
    AddInt32(data, "BoneIndex", obj->BoneIndex);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_HMODEL(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHModelAuxDataStruct *auxdata = (W3dHModelAuxDataStruct *)chunkdata;
    AddInt32(data, "Attributes", auxdata->Attributes);
    AddInt32(data, "MeshCount", auxdata->MeshCount);
    AddInt32(data, "CollisionCount", auxdata->CollisionCount);
    AddInt32(data, "SkinCount", auxdata->SkinCount);
    AddInt32Array(data, "FutureCounts", auxdata->FutureCounts, 8);
    AddFloat(data, "LODMin", auxdata->LODMin);
    AddFloat(data, "LODMax", auxdata->LODMax);
    AddInt32Array(data, "FutureUse", auxdata->FutureUse, 32);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_HMODEL_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHModelHeaderStruct *header = (W3dHModelHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddString(data, "HierarchyName", header->HierarchyName, "string");
    AddInt16(data, "NumConnections", header->NumConnections);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_LIGHT(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_LIGHT_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLightStruct *light = (W3dLightStruct *)chunkdata;
    int type = light->Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK;

    if (type == W3D_LIGHT_ATTRIBUTE_POINT) {
        AddString(data, "Attributes", "W3D_LIGHT_ATTRIBUTE_POINT", "string");
    } else if (type == W3D_LIGHT_ATTRIBUTE_SPOT) {
        AddString(data, "Attributes", "W3D_LIGHT_ATTRIBUTE_SPOT", "string");
    } else if (type == W3D_LIGHT_ATTRIBUTE_DIRECTIONAL) {
        AddString(data, "Attributes", "W3D_LIGHT_ATTRIBUTE_DIRECTIONAL", "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_LIGHT_INFO Unknown Light Type %x", type);
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    if (light->Attributes & W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS) {
        AddString(data, "Attributes", "W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS", "flag");
    }

    if (light->Attributes & 0xFFFFFE00) {
        StringClass str;
        str.Format("W3D_CHUNK_LIGHT_INFO Unknown Light Flags %x", light->Attributes & 0xFFFFFE00);
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    AddRGBArray(data, "Ambient", &light->Ambient, 1);
    AddRGBArray(data, "Diffuse", &light->Diffuse, 1);
    AddRGBArray(data, "Specular", &light->Specular, 1);
    AddFloat(data, "Intensity", light->Intensity);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_LIGHT_TRANSFORM(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLightTransformStruct *transform = (W3dLightTransformStruct *)chunkdata;
    AddFloatArray(data, "Transform", transform->Transform[0], 4);
    AddFloatArray(data, "Transform", transform->Transform[1], 4);
    AddFloatArray(data, "Transform", transform->Transform[2], 4);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_LIGHTSCAPE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_LIGHTSCAPE_LIGHT(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_LOD(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLODStruct *lod = (W3dLODStruct *)chunkdata;
    AddString(data, "Render Object Name", lod->RenderObjName, "string");
    AddFloat(data, "LOD Min Distance", lod->LODMin);
    AddFloat(data, "LOD Max Distance", lod->LODMax);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_LODMODEL(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_LODMODEL_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLODModelHeaderStruct *header = (W3dLODModelHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddInt16(data, "NumLODs", header->NumLODs);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MAP3_FILENAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Texture Filename:", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MAP3_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMap3Struct *map = (W3dMap3Struct *)chunkdata;
    AddInt16(data, "Mapping Type", map->MappingType);
    AddInt16(data, "Frame Count", map->FrameCount);
    AddFloat(data, "Frame Rate", map->FrameRate);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MATERIAL_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMaterialInfoStruct *info = (W3dMaterialInfoStruct *)chunkdata;
    AddInt32(data, "PassCount", info->PassCount);
    AddInt32(data, "VertexMaterialCount", info->VertexMaterialCount);
    AddInt32(data, "ShaderCount", info->ShaderCount);
    AddInt32(data, "TextureCount", info->TextureCount);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MATERIAL_PASS(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MATERIAL3(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MATERIAL3_DC_MAP(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MATERIAL3_DI_MAP(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MATERIAL3_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMaterial3Struct *material = (W3dMaterial3Struct *)chunkdata;
    AddInt32(data, "Attributes", material->attributes);

    if (material->attributes & W3DMATERIAL_USE_ALPHA) {
        AddString(data, "Attributes", "W3DMATERIAL_USE_ALPHA", "string");
    }

    if (material->attributes & W3DMATERIAL_USE_SORTING) {
        AddString(data, "Attributes", "W3DMATERIAL_USE_SORTING", "string");
    }

    if (material->attributes & W3DMATERIAL_HINT_DIT_OVER_DCT) {
        AddString(data, "Attributes", "W3DMATERIAL_HINT_DIT_OVER_DCT", "string");
    }

    if (material->attributes & W3DMATERIAL_HINT_SIT_OVER_SCT) {
        AddString(data, "Attributes", "W3DMATERIAL_HINT_SIT_OVER_SCT", "string");
    }

    if (material->attributes & W3DMATERIAL_HINT_DIT_OVER_DIG) {
        AddString(data, "Attributes", "W3DMATERIAL_HINT_DIT_OVER_DIG", "string");
    }

    if (material->attributes & W3DMATERIAL_HINT_SIT_OVER_SIG) {
        AddString(data, "Attributes", "W3DMATERIAL_HINT_SIT_OVER_SIG", "string");
    }

    if (material->attributes & W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA) {
        AddString(data, "Attributes", "W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA", "string");
    }

    if (material->attributes & W3DMATERIAL_PSX_TRANS_100) {
        AddString(data, "Attributes", "W3DMATERIAL_PSX_TRANS_100", "string");
    }

    if (material->attributes & W3DMATERIAL_PSX_TRANS_50) {
        AddString(data, "Attributes", "W3DMATERIAL_PSX_TRANS_50", "string");
    }

    if (material->attributes & W3DMATERIAL_PSX_TRANS_25) {
        AddString(data, "Attributes", "W3DMATERIAL_PSX_TRANS_25", "string");
    }

    if (material->attributes & W3DMATERIAL_PSX_TRANS_MINUS_100) {
        AddString(data, "Attributes", "W3DMATERIAL_PSX_TRANS_MINUS_100", "string");
    }

    if (material->attributes & W3DMATERIAL_PSX_NO_RT_LIGHTING) {
        AddString(data, "Attributes", "W3DMATERIAL_PSX_NO_RT_LIGHTING", "string");
    }

    AddRGB(data, "Diffuse Color", &material->diffuse_color);
    AddRGB(data, "Specular Color", &material->specular_color);
    AddRGB(data, "Emissive Coefficients", &material->emissive_coeffs);
    AddRGB(data, "Ambient Coefficients", &material->ambient_coeffs);
    AddRGB(data, "Diffuse Coefficients", &material->diffuse_coeffs);
    AddRGB(data, "Specular Coefficients", &material->specular_coeffs);
    AddFloat(data, "Shininess", material->shininess);
    AddFloat(data, "Opacity", material->opacity);
    AddFloat(data, "Translucency", material->translucency);
    AddFloat(data, "Fog Coefficient", material->fog_coeff);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MATERIAL3_NAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Material Name:", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MATERIAL3_SC_MAP(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MATERIAL3_SI_MAP(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MATERIALS3(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MESH(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MESH_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMeshHeaderStruct *header = (W3dMeshHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "MeshName", header->MeshName, "string");
    AddInt32(data, "Attributes", header->Attributes);

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_BOX) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_BOX", "string");
    }

    if (header->Attributes & W3D_MESH_FLAG_SKIN) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_SKIN", "string");
    }

    if (header->Attributes & W3D_MESH_FLAG_SHADOW) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_SHADOW", "string");
    }

    if (header->Attributes & W3D_MESH_FLAG_ALIGNED) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_ALIGNED", "string");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL", "string");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE", "string");
    }

    AddInt32(data, "NumTris", header->NumTris);
    AddInt32(data, "NumQuads", header->NumQuads);
    AddInt32(data, "NumSrTris", header->NumSrTris);
    AddInt32(data, "NumPovQuads", header->NumPovQuads);
    AddInt32(data, "NumVertices", header->NumVertices);
    AddInt32(data, "NumNormals", header->NumNormals);
    AddInt32(data, "NumSrNormals", header->NumSrNormals);
    AddInt32(data, "NumTexCoords", header->NumTexCoords);
    AddInt32(data, "NumMaterials", header->NumMaterials);
    AddInt32(data, "NumVertColors", header->NumVertColors);
    AddInt32(data, "NumVertInfluences", header->NumVertInfluences);
    AddInt32(data, "NumDamageStages", header->NumDamageStages);
    AddInt32Array(data, "FutureCounts", header->FutureCounts, 5);
    AddFloat(data, "LODMin", header->LODMin);
    AddFloat(data, "LODMax", header->LODMax);
    AddVector(data, "Min", &header->Min);
    AddVector(data, "Max", &header->Max);
    AddVector(data, "SphCenter", &header->SphCenter);
    AddFloat(data, "SphRadius", header->SphRadius);
    AddVector(data, "Translation", &header->Translation);
    AddFloatArray(data, "Rotation", header->Rotation, 9);
    AddVector(data, "MassCenter", &header->MassCenter);
    AddFloatArray(data, "Inertia", header->Inertia, 9);
    AddFloat(data, "Volume", header->Volume);
    AddString(data, "HierarchyTreeName", header->HierarchyTreeName, "string");
    AddString(data, "HierarchyModelName", header->HierarchyModelName, "string");
    AddInt32Array(data, "FutureUse", header->FutureUse, 24);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MESH_HEADER3(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMeshHeader3Struct *header = (W3dMeshHeader3Struct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "MeshName", header->MeshName, "string");
    AddString(data, "ContainerName", header->ContainerName, "string");
    AddInt32(data, "Attributes", header->Attributes);
    int type = header->Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;

    switch (type) {
        case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL", "flag");
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED", "flag");
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN", "flag");
            break;
        case OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW:
            AddString(data, "Attributes", "OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW", "flag");
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX", "flag");
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX", "flag");
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED", "flag");
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED:
            AddString(data, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED", "flag");
            break;
        default: {
            StringClass str;
            str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Mesh Type %x", type);
            data->unknowndata.Add(str);
            AddString(data, "Attributes", "Unknown", "string");
            break;
        }
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_VIS) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_VIS", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_CAMERA) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_CAMERA", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_USER1) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_USER1", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_USER2) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_USER2", "flag");
    }

    if (header->Attributes & 0x00000800) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Attribute 0x00000800");
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    if (header->Attributes & W3D_MESH_FLAG_COLLISION_BOX) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_COLLISION_BOX", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_SKIN) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_SKIN", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_SHADOW) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_SHADOW", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_ALIGNED) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_ALIGNED", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_HIDDEN) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_HIDDEN", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_TWO_SIDED) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_TWO_SIDED", "flag");
    }

    if (header->Attributes & OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED) {
        AddString(data, "Attributes", "OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_CAST_SHADOW) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_CAST_SHADOW", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_SHATTERABLE) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_SHATTERABLE", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_NPATCHABLE) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_NPATCHABLE", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_PRELIT", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_ALWAYSDYNLIGHT) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_ALWAYSDYNLIGHT", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_UNLIT) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_PRELIT_UNLIT", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_VERTEX) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_PRELIT_VERTEX", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS", "flag");
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) {
        AddString(data, "Attributes", "W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE", "flag");
    }

    AddInt32(data, "NumTris", header->NumTris);
    AddInt32(data, "NumVertices", header->NumVertices);
    AddInt32(data, "NumMaterials", header->NumMaterials);
    AddInt32(data, "NumDamageStages", header->NumDamageStages);

    if (header->SortLevel) {
        AddInt8(data, "SortLevel", (uint8_t)header->SortLevel);
    } else {
        AddString(data, "SortLevel", "NONE", "string");
    }

    char c[64];

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_MASK) {
        if (header->PrelitVersion) {
            sprintf(c, "%u.%hu", header->PrelitVersion >> 16, (uint16_t)header->PrelitVersion);
        } else {
            sprintf(c, "UNKNOWN");
        }
    } else {
        sprintf(c, "N/A");
    }

    AddString(data, "PrelitVersion", c, "string");
    AddInt32Array(data, "FutureCounts", header->FutureCounts, W3D_VERTEX_CHANNEL_LOCATION);
    AddInt32(data, "VertexChannels", header->VertexChannels);

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_LOCATION) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_LOCATION", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_NORMAL) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_NORMAL", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_TEXCOORD) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_TEXCOORD", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_COLOR) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_COLOR", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_BONEID) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_BONEID", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_TANGENT) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_TANGENT", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_BINORMAL) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_BINORMAL", "flag");
    }

    if (header->VertexChannels & W3D_VERTEX_CHANNEL_SMOOTHSKIN) {
        AddString(data, "VertexChannels", "W3D_VERTEX_CHANNEL_SMOOTHSKIN", "flag");
    }

    if (header->VertexChannels & 0xFFFFFF00) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Vertex Channels %x", header->VertexChannels);
        data->unknowndata.Add(str);
        AddString(data, "VertexChannels", "Unknown", "string");
    }

    AddInt32(data, "FaceChannels", header->FaceChannels);

    if (header->FaceChannels & W3D_FACE_CHANNEL_FACE) {
        AddString(data, "FaceChannels", "W3D_FACE_CHANNEL_FACE", "flag");
    }

    if (header->FaceChannels & 0xFFFFFFFE) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Face Channels %x", header->FaceChannels);
        data->unknowndata.Add(str);
        AddString(data, "FaceChannels", "Unknown", "string");
    }

    AddVector(data, "Min", &header->Min);
    AddVector(data, "Max", &header->Max);
    AddVector(data, "SphCenter", &header->SphCenter);
    AddFloat(data, "SphRadius", header->SphRadius);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MESH_USER_TEXT(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "UserText", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_NEAR_ATTENUATION(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLightAttenuationStruct *atten = (W3dLightAttenuationStruct *)chunkdata;
    AddFloat(data, "Near Atten Start", atten->Start);
    AddFloat(data, "Near Atten End", atten->End);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_NODE(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHModelNodeStruct *node = (W3dHModelNodeStruct *)chunkdata;
    AddString(data, "RenderObjName", node->RenderObjName, "string");
    AddInt16(data, "PivotIdx", node->PivotIdx);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_NULL_OBJECT(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dNullObjectStruct *obj = (W3dNullObjectStruct *)chunkdata;
    AddVersion(data, obj->version);
    AddInt32(data, "Attributes", obj->attributes);
    AddString(data, "Name", obj->name, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_PER_FACE_TEXCOORD_IDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    Vector3i *ids = (Vector3i *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(Vector3i); i++) {
        char c[256];
        sprintf(c, "Face[%d] UV Indices", i);
        AddIJK(data, c, &ids[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_PER_TRI_MATERIALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint16_t *materials = (uint16_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint16_t); i++) {
        char c[256];
        sprintf(c, "Triangle[%d].MaterialIdx", i);
        AddInt16(data, c, materials[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_PIVOT_FIXUPS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dPivotFixupStruct *pivots = (W3dPivotFixupStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dPivotFixupStruct); i++) {
        for (unsigned int j = 0; j < 4; j++) {
            char c[256];
            sprintf(c, "Transform %d, Row[%d]", i, j);
            AddFloatArray(data, c, pivots[i].TM[j], 3);
        }
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_PIVOTS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dPivotStruct *pivots = (W3dPivotStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dPivotStruct); i++) {
        char c[256];
        sprintf(c, "Pivot[%d].Name", i);
        AddString(data, c, pivots[i].Name, "string");
        sprintf(c, "Pivot[%d].ParentIdx", i);
        AddInt32(data, c, pivots[i].ParentIdx);
        sprintf(c, "Pivot[%d].Translation", i);
        AddVector(data, c, &pivots[i].Translation);
        sprintf(c, "Pivot[%d].EulerAngles", i);
        AddVector(data, c, &pivots[i].EulerAngles);
        sprintf(c, "Pivot[%d].Rotation", i);
        AddQuaternion(data, c, &pivots[i].Rotation);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_PLACEHOLDER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dPlaceholderStruct *placeholder = (W3dPlaceholderStruct *)chunkdata;
    AddVersion(data, placeholder->Version);
    AddFloatArray(data, "Transform", placeholder->Transform[0], 3);
    AddFloatArray(data, "Transform", placeholder->Transform[1], 3);
    AddFloatArray(data, "Transform", placeholder->Transform[2], 3);
    AddFloatArray(data, "Transform", placeholder->Transform[3], 3);
    AddString(data, "Name", placeholder->Name, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_POINTS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *points = (W3dVectorStruct *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Point[%d]", i);
        AddVector(data, c, &points[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_PRELIT_UNLIT(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_PRELIT_VERTEX(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_PS2_SHADERS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dPS2ShaderStruct *shaders = (W3dPS2ShaderStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dPS2ShaderStruct); i++) {
        char c[256];
        sprintf(c, "shader[%d]", i);
        AddPS2Shader(data, c, &shaders[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SCG(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dRGBStruct *colors = (W3dRGBStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dRGBStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d].SCG", i);
        AddRGB(data, c, &colors[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHADER_IDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *ids = (uint32_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Face[%d] Shader Index", i);
        AddInt32(data, c, ids[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHADERS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dShaderStruct *shaders = (W3dShaderStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dShaderStruct); i++) {
        char c[256];
        sprintf(c, "shader[%d]", i);
        AddShader(data, c, &shaders[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SKIN_NODE(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dHModelNodeStruct *node = (W3dHModelNodeStruct *)chunkdata;
    AddString(data, "SkinMeshName", node->RenderObjName, "string");
    AddInt16(data, "PivotIdx", node->PivotIdx);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SPOT_LIGHT_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dSpotLightStruct *light = (W3dSpotLightStruct *)chunkdata;
    AddVector(data, "SpotDirection", &light->SpotDirection);
    AddFloat(data, "SpotAngle", light->SpotAngle);
    AddFloat(data, "SpotExponent", light->SpotExponent);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SPOT_LIGHT_INFO_5_0(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dSpotLightStruct_v5_0 *light = (W3dSpotLightStruct_v5_0 *)chunkdata;
    AddFloat(data, "SpotOuterAngle", light->SpotOuterAngle);
    AddFloat(data, "SpotInnerAngle", light->SpotInnerAngle);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_STAGE_TEXCOORDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTexCoordStruct *coords = (W3dTexCoordStruct *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dTexCoordStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d].UV", i);
        AddTexCoord(data, c, &coords[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SURRENDER_NORMALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *normals = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "SRNormal[%d]", i);
        AddVector(data, c, &normals[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TEXCOORDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTexCoordStruct *texcoords = (W3dTexCoordStruct *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dTexCoordStruct); i++) {
        char c[256];
        sprintf(c, "TexCoord[%d]", i);
        AddTexCoord(data, c, &texcoords[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TEXTURE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_TEXTURE_IDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *ids = (uint32_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Face[%d] Texture Index", i);
        AddInt32(data, c, ids[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TEXTURE_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTextureInfoStruct *info = (W3dTextureInfoStruct *)chunkdata;
    AddInt16(data, "Texture.Attributes", info->Attributes);

    if (info->Attributes & W3DTEXTURE_PUBLISH) {
        AddString(data, "Attributes", "W3DTEXTURE_PUBLISH", "flag");
    }

    if (info->Attributes & W3DTEXTURE_RESIZE_OBSOLETE) {
        AddString(data, "Attributes", "W3DTEXTURE_RESIZE_OBSOLETE", "flag");
    }

    if (info->Attributes & W3DTEXTURE_NO_LOD) {
        AddString(data, "Attributes", "W3DTEXTURE_NO_LOD", "flag");
    }

    if (info->Attributes & W3DTEXTURE_CLAMP_U) {
        AddString(data, "Attributes", "W3DTEXTURE_CLAMP_U", "flag");
    }

    if (info->Attributes & W3DTEXTURE_CLAMP_V) {
        AddString(data, "Attributes", "W3DTEXTURE_CLAMP_V", "flag");
    }

    if (info->Attributes & W3DTEXTURE_ALPHA_BITMAP) {
        AddString(data, "Attributes", "W3DTEXTURE_ALPHA_BITMAP", "flag");
    }

    int mip = info->Attributes & W3DTEXTURE_MIP_LEVELS_MASK;

    if (mip == W3DTEXTURE_MIP_LEVELS_ALL) {
        AddString(data, "Attributes", "W3DTEXTURE_MIP_LEVELS_ALL", "flag");
    } else if (mip == W3DTEXTURE_MIP_LEVELS_2) {
        AddString(data, "Attributes", "W3DTEXTURE_MIP_LEVELS_2", "flag");
    } else if (mip == W3DTEXTURE_MIP_LEVELS_3) {
        AddString(data, "Attributes", "W3DTEXTURE_MIP_LEVELS_3", "flag");
    } else if (mip == W3DTEXTURE_MIP_LEVELS_4) {
        AddString(data, "Attributes", "W3DTEXTURE_MIP_LEVELS_4", "flag");
    }

    int hint = info->Attributes & 0xF00;

    if (hint == W3DTEXTURE_HINT_BASE) {
        AddString(data, "Attributes", "W3DTEXTURE_HINT_BASE", "flag");
    } else if (hint == W3DTEXTURE_HINT_EMISSIVE) {
        AddString(data, "Attributes", "W3DTEXTURE_HINT_EMISSIVE", "flag");
    } else if (hint == W3DTEXTURE_HINT_ENVIRONMENT) {
        AddString(data, "Attributes", "W3DTEXTURE_HINT_ENVIRONMENT", "flag");
    } else if (hint == W3DTEXTURE_HINT_SHINY_MASK) {
        AddString(data, "Attributes", "W3DTEXTURE_HINT_SHINY_MASK", "flag");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Hints %x", hint);
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    if (info->Attributes & W3DTEXTURE_TYPE_MASK) {
        AddString(data, "Attributes", "W3DTEXTURE_TYPE_BUMPMAP", "flag");
    } else {
        AddString(data, "Attributes", "W3DTEXTURE_TYPE_COLORMAP", "flag");
    }

    if (info->Attributes & 0xE000) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Flags %x", info->Attributes & 0xE000);
        data->unknowndata.Add(str);
        AddString(data, "Attributes", "Unknown", "string");
    }

    AddInt16(data, "Texture.AnimType", info->AnimType);

    if (info->AnimType == W3DTEXTURE_ANIM_LOOP) {
        AddString(data, "AnimType", "W3DTEXTURE_ANIM_LOOP", "flag");
    } else if (info->AnimType == W3DTEXTURE_ANIM_PINGPONG) {
        AddString(data, "AnimType", "W3DTEXTURE_ANIM_PINGPONG", "flag");
    } else if (info->AnimType == W3DTEXTURE_ANIM_ONCE) {
        AddString(data, "AnimType", "W3DTEXTURE_ANIM_ONCE", "flag");
    } else if (info->AnimType == W3DTEXTURE_ANIM_MANUAL) {
        AddString(data, "AnimType", "W3DTEXTURE_ANIM_MANUAL", "flag");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Anim Type %x", info->AnimType);
        data->unknowndata.Add(str);
        AddString(data, "AnimType", "Unknown", "string");
    }

    AddInt32(data, "Texture.FrameCount", info->FrameCount);
    AddFloat(data, "Texture.FrameRate", info->FrameRate);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TEXTURE_NAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Texture Name:", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TEXTURE_REPLACER_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTextureReplacerHeaderStruct *header = (W3dTextureReplacerHeaderStruct *)chunkdata;
    AddInt32(data, "ReplacedTexturesCount", header->ReplacedTexturesCount);
    W3dTextureReplacerStruct *replacer = (W3dTextureReplacerStruct *)(chunkdata + sizeof(W3dTextureReplacerHeaderStruct));

    for (unsigned int i = 0; i < header->ReplacedTexturesCount; i++) {
        char c[256];

        for (int j = 0; j < 15; j++) {
            sprintf(c, "Replacer[%u].MeshPath[%d]", i, j);
            AddString(data, c, replacer[i].MeshPath[j], "string");
        }

        for (int j = 0; j < 15; j++) {
            sprintf(c, "Replacer[%u].BonePath[%d]", i, j);
            AddString(data, c, replacer[i].BonePath[j], "string");
        }

        AddString(data, "OldTextureName", replacer[i].OldTextureName, "string");
        AddString(data, "NewTextureName", replacer[i].NewTextureName, "string");
        AddInt16(data, "TextureParams.Attributes", replacer[i].TextureParams.Attributes);
        AddInt16(data, "TextureParams.AnimType", replacer[i].TextureParams.AnimType);
        AddInt32(data, "TextureParams.FrameCount", replacer[i].TextureParams.FrameCount);
        AddFloat(data, "TextureParams.FrameRate", replacer[i].TextureParams.FrameRate);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TEXTURE_STAGE(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_TEXTURES(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_TRANSFORM_NODE(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dPlaceholderStruct *placeholder = (W3dPlaceholderStruct *)chunkdata;
    AddVersion(data, placeholder->Version);
    AddFloatArray(data, "Transform", placeholder->Transform[0], 3);
    AddFloatArray(data, "Transform", placeholder->Transform[1], 3);
    AddFloatArray(data, "Transform", placeholder->Transform[2], 3);
    AddFloatArray(data, "Transform", placeholder->Transform[3], 3);
    AddString(data, "Name", placeholder->Name, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TRIANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTriStruct *triangles = (W3dTriStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dTriStruct); i++) {
        char c[256];
        sprintf(c, "Triangle[%d].VertexIndices", i);
        AddInt32Array(data, c, triangles[i].Vindex, 3);
        sprintf(c, "Triangle[%d].Attributes", i);
        AddInt32(data, c, triangles[i].Attributes);
        sprintf(c, "Triangle[%d].Normal", i);
        AddVector(data, c, &triangles[i].Normal);
        sprintf(c, "Triangle[%d].Dist", i);
        AddFloat(data, c, triangles[i].Dist);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_COLORS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dRGBStruct *colors = (W3dRGBStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dRGBStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d].RGB", i);
        AddRGB(data, c, &colors[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_INFLUENCES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVertInfStruct *vertinf = (W3dVertInfStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVertInfStruct); i++) {
        char c[256];
        sprintf(c, "VertexInfluence[%d].BoneIdx[0]", i);
        AddInt16(data, c, vertinf[i].BoneIdx[0]);
        sprintf(c, "VertexInfluence[%d].Weight[0]", i);
        AddInt16(data, c, vertinf[i].Weight[0]);
        sprintf(c, "VertexInfluence[%d].BoneIdx[1]", i);
        AddInt16(data, c, vertinf[i].BoneIdx[1]);
        sprintf(c, "VertexInfluence[%d].Weight[1]", i);
        AddInt16(data, c, vertinf[i].Weight[1]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_MAPPER_ARGS0(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Stage0 Mapper Args:", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_MAPPER_ARGS1(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Stage1 Mapper Args:", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_MATERIAL(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_VERTEX_MATERIAL_IDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *ids = (uint32_t *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Vertex[%d] Vertex Material Index", i);
        AddInt32(data, c, ids[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVertexMaterialStruct *material = (W3dVertexMaterialStruct *)chunkdata;

    if (material->Attributes & W3DVERTMAT_USE_DEPTH_CUE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_USE_DEPTH_CUE", "flag");
    }

    if (material->Attributes & W3DVERTMAT_ARGB_EMISSIVE_ONLY) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_ARGB_EMISSIVE_ONLY", "flag");
    }

    if (material->Attributes & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE", "flag");
    }

    if (material->Attributes & W3DVERTMAT_DEPTH_CUE_TO_ALPHA) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_DEPTH_CUE_TO_ALPHA", "flag");
    }

    if (material->Attributes & 0x00000010) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000010");
        data->unknowndata.Add(str);
        AddString(data, "Material.Attributes", "Unknown", "string");
    }

    if (material->Attributes & 0x00000020) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000020");
        data->unknowndata.Add(str);
        AddString(data, "Material.Attributes", "Unknown", "string");
    }

    if (material->Attributes & 0x00000040) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000040");
        data->unknowndata.Add(str);
        AddString(data, "Material.Attributes", "Unknown", "string");
    }

    if (material->Attributes & 0x00000080) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000080");
        data->unknowndata.Add(str);
        AddString(data, "Material.Attributes", "Unknown", "string");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_UV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_UV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SCREEN) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SCREEN", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SCALE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SCALE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_ROTATE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ROTATE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_RANDOM) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_RANDOM", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_EDGE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_EDGE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_BUMPENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_BUMPENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) > W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Stage 0 Mapper %x",
            material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK);
        data->unknowndata.Add(str);
        AddString(data, "Material.Attributes", "Unknown", "string");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_UV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_UV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SCREEN) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SCREEN", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SCALE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SCALE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_ROTATE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ROTATE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_RANDOM) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_RANDOM", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_EDGE) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_EDGE", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_BUMPENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_BUMPENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT", "flag");
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) > W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Stage 1 Mapper %x",
            material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK);
        data->unknowndata.Add(str);
        AddString(data, "Material.Attributes", "Unknown", "string");
    }

    if (material->Attributes & W3DVERTMAT_PSX_MASK) {
        if (material->Attributes & W3DVERTMAT_PSX_NO_RT_LIGHTING) {
            AddString(data, "Material.Attributes", "W3DVERTMAT_PSX_NO_RT_LIGHTING", "flag");
        } else {
            if ((material->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_NONE)
                AddString(data, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_NONE", "flag");
            if ((material->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_100)
                AddString(data, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_100", "flag");
            if ((material->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_50)
                AddString(data, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_50", "flag");
            if ((material->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_25)
                AddString(data, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_25", "flag");
            if ((material->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_MINUS_100)
                AddString(data, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_MINUS_100", "flag");
        }

        if (material->Attributes & 0xF0000000) {
            StringClass str;
            str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown PSX material flag %x", material->Attributes & 0xF0000000);
            data->unknowndata.Add(str);
            AddString(data, "Material.Attributes", "Unknown", "string");
        }
    }

    AddInt32(data, "Material.Attributes", material->Attributes);
    AddRGB(data, "Material.Ambient", &material->Ambient);
    AddRGB(data, "Material.Diffuse", &material->Diffuse);
    AddRGB(data, "Material.Specular", &material->Specular);
    AddRGB(data, "Material.Emissive", &material->Emissive);
    AddFloat(data, "Material.Shininess", material->Shininess);
    AddFloat(data, "Material.Opacity", material->Opacity);
    AddFloat(data, "Material.Translucency", material->Translucency);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_MATERIAL_NAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Material Name:", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_MATERIALS(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_VERTEX_NORMALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *normals = (W3dVectorStruct *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Normal[%d]", i);
        AddVector(data, c, &normals[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTEX_SHADE_INDICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *indices = (uint32_t *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Index[%d]", i);
        AddInt32(data, c, indices[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_VERTICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *vertices = (W3dVectorStruct *)chunkdata;
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d]", i);
        AddVector(data, c, &vertices[i]);
    }
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_LINE_PROPERTIES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterLinePropertiesStruct *props = (W3dEmitterLinePropertiesStruct *)chunkdata;
    AddInt32(data, "Flags", props->Flags);

    if ((props->Flags & W3D_ELINE_MERGE_INTERSECTIONS) == W3D_ELINE_MERGE_INTERSECTIONS) {
        AddString(data, "Flags", "W3D_ELINE_MERGE_INTERSECTIONS", "flag");
    }

    if ((props->Flags & W3D_ELINE_FREEZE_RANDOM) == W3D_ELINE_FREEZE_RANDOM) {
        AddString(data, "Flags", "W3D_ELINE_FREEZE_RANDOM", "flag");
    }

    if ((props->Flags & W3D_ELINE_DISABLE_SORTING) == W3D_ELINE_DISABLE_SORTING) {
        AddString(data, "Flags", "W3D_ELINE_DISABLE_SORTING", "flag");
    }

    if ((props->Flags & W3D_ELINE_END_CAPS) == W3D_ELINE_END_CAPS) {
        AddString(data, "Flags", "W3D_ELINE_END_CAPS", "flag");
    }

    if (props->Flags & 0x00FFFFF0) {
        StringClass str;
        str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Line Properties flags %x", props->Flags & 0x00FFFFF0);
        data->unknowndata.Add(str);
        AddString(data, "Flags", "Unknown", "string");
    }

    int mapmode = props->Flags >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET;

    switch (mapmode) {
        case W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP:
            AddString(data, "Flags", "W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP", "flag");
            break;
        case W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP:
            AddString(data, "Flags", "W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP", "flag");
            break;
        case W3D_ELINE_TILED_TEXTURE_MAP:
            AddString(data, "Flags", "W3D_ELINE_TILED_TEXTURE_MAP", "flag");
            break;
        default: {
            StringClass str;
            str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Mapping Mode %x", mapmode);
            data->unknowndata.Add(str);
            AddString(data, "Flags", "Unknown", "string");
            break;
        }
    }

    AddInt32(data, "SubdivisionLevel", props->SubdivisionLevel);
    AddFloat(data, "NoiseAmplitude", props->NoiseAmplitude);
    AddFloat(data, "MergeAbortFactor", props->MergeAbortFactor);
    AddFloat(data, "TextureTileFactor", props->TextureTileFactor);
    AddFloat(data, "UPerSec", props->UPerSec);
    AddFloat(data, "VPerSec", props->VPerSec);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SECONDARY_VERTICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *vertices = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d]", i);
        AddVector(data, c, &vertices[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SECONDARY_VERTEX_NORMALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *normals = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Normal[%d]", i);
        AddVector(data, c, &normals[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_TANGENTS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *tangents = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Tangent[%d]", i);
        AddVector(data, c, &tangents[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_BINORMALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *binormals = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Binormal[%d]", i);
        AddVector(data, c, &binormals[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

int flavor = ANIM_FLAVOR_TIMECODED;
const char *FlavorTypes[] = { "Timecoded", "Adaptive Delta" };

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCompressedAnimHeaderStruct *header = (W3dCompressedAnimHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddString(data, "HierarchyName", header->HierarchyName, "string");
    AddInt32(data, "NumFrames", header->NumFrames);
    AddInt16(data, "FrameRate", header->FrameRate);

    if (header->Flavor < ANIM_FLAVOR_VALID) {
        AddString(data, "Flavor", FlavorTypes[header->Flavor], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_HEADER Unknown Flavor Type %x", header->Flavor);
        data->unknowndata.Add(str);
        AddString(data, "Flavor", "Unknown", "string");
    }

    flavor = header->Flavor;
    delete[] chunkdata;
}
void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);

    if (flavor == ANIM_FLAVOR_TIMECODED) {
        W3dTimeCodedAnimChannelStruct *channel = (W3dTimeCodedAnimChannelStruct *)chunkdata;
        AddInt32(data, "NumTimeCodes", channel->NumTimeCodes);
        AddInt16(data, "Pivot", channel->Pivot);
        AddInt8(data, "VectorLen", channel->VectorLen);

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
            AddString(data, "ChannelType", ChannelTypes[channel->Flags], "string");
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            data->unknowndata.Add(str);
            AddString(data, "ChnanelType", "Unknown", "string");
        }

        int len = cload.Cur_Chunk_Length() - sizeof(W3dTimeCodedAnimChannelStruct);
        int datalen = ((len >> 2) + 1);

        for (int i = 0; i < datalen; i++) {
            StringClass str;
            str.Format("Data[%d]", i);
            AddInt32(data, str, channel->Data[i]);
        }

        delete[] chunkdata;
    } else {
        W3dAdaptiveDeltaAnimChannelStruct *channel = (W3dAdaptiveDeltaAnimChannelStruct *)chunkdata;
        AddInt32(data, "NumFrames", channel->NumFrames);
        AddInt16(data, "Pivot", channel->Pivot);
        AddInt8(data, "VectorLen", channel->VectorLen);

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
            AddString(data, "ChannelType", ChannelTypes[channel->Flags], "string");
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            data->unknowndata.Add(str);
            AddString(data, "ChannelType", "Unknown", "string");
        }

        AddFloat(data, "Scale", channel->Scale);
        int len = cload.Cur_Chunk_Length() - sizeof(W3dAdaptiveDeltaAnimChannelStruct);
        int datalen = ((len >> 2) + 1);

        for (int i = 0; i < datalen; i++) {
            StringClass str;
            str.Format("Data[%d]", i);
            AddInt32(data, str, channel->Data[i]);
        }

        delete[] chunkdata;
    }
}

void Dump_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTimeCodedBitChannelStruct *channel = (W3dTimeCodedBitChannelStruct *)chunkdata;
    AddInt32(data, "NumTimeCodes", channel->NumTimeCodes);
    AddInt16(data, "Pivot", channel->Pivot);

    if (channel->Flags <= BIT_CHANNEL_TIMECODED_VIS) {
        AddString(data, "ChannelType", BitChannelTypes[channel->Flags], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        data->unknowndata.Add(str);
        AddString(data, "ChannelType", "Unknown", "string");
    }

    AddInt8(data, "Default Value", channel->DefaultVal);
    int datalen = channel->NumTimeCodes;

    for (int i = 0; i < datalen; i++) {
        StringClass str;
        str.Format("Data[%d]", i);
        AddInt32(data, str, channel->Data[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MORPH_ANIMATION(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MORPHANIM_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMorphAnimHeaderStruct *header = (W3dMorphAnimHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddString(data, "HierarchyName", header->HierarchyName, "string");
    AddInt32(data, "FrameCount", header->FrameCount);
    AddFloat(data, "FrameRate", header->FrameRate);
    AddInt32(data, "ChannelCount", header->ChannelCount);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MORPHANIM_CHANNEL(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_MORPHANIM_POSENAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Pose Name", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MORPHANIM_KEYDATA(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMorphAnimKeyStruct *key = (W3dMorphAnimKeyStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dMorphAnimKeyStruct); i++) {
        char c[256];
        sprintf(c, "MorphKeys[%d].MorphFrame", i);
        AddInt32(data, c, key[i].MorphFrame);
        sprintf(c, "MorphKeys[%d].PoseFrame", i);
        AddInt32(data, c, key[i].PoseFrame);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *pivot = (uint32_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "PivotChannel[%d]", i);
        AddInt32(data, c, pivot[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SOUNDROBJ(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_SOUNDROBJ_HEADER(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dSoundRObjHeaderStruct *header = (W3dSoundRObjHeaderStruct *)chunkdata;
    AddVersion(data, header->Version);
    AddString(data, "Name", header->Name, "string");
    AddInt32(data, "Flags", header->Flags);
    delete[] chunkdata;
}

#define READ_FLOAT(id, name) \
    case id: { \
        float f; \
        cload.Read(&f, sizeof(f)); \
        AddFloat(data, #name, f); \
        break; \
    }

#define READ_INT(id, name) \
    case id: { \
        int i; \
        cload.Read(&i, sizeof(i)); \
        AddInt32(data, #name, i); \
        break; \
    }

#define READ_BOOL(id, name) \
    case id: { \
        bool b; \
        cload.Read(&b, sizeof(b)); \
        AddInt8(data, #name, b); \
        break; \
    }

#define READ_VECTOR(id, name) \
    case id: { \
        Vector3 v; \
        cload.Read(&v, sizeof(v)); \
        W3dVectorStruct v2; \
        v2.x = v.X; \
        v2.y = v.Y; \
        v2.z = v.Z; \
        AddVector(data, #name, &v2); \
        break; \
    }

#define READ_STRING(id, name) \
    case id: { \
        StringClass str; \
        cload.Read(str.Get_Buffer(cload.Cur_Micro_Chunk_Length()), cload.Cur_Micro_Chunk_Length()); \
        AddString(data, #name, str, "String"); \
        break; \
    }

void Dump_W3D_CHUNK_SOUNDROBJ_DEFINITION(ChunkLoadClass &cload, ChunkData *data)
{
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 0x100:
                while (cload.Open_Micro_Chunk()) {
                    switch (cload.Cur_Micro_Chunk_ID()) {
                        READ_FLOAT(3, m_Priority);
                        READ_FLOAT(4, m_Volume);
                        READ_FLOAT(5, m_Pan);
                        READ_INT(6, m_LoopCount);
                        READ_FLOAT(7, m_DropoffRadius);
                        READ_FLOAT(8, m_MaxVolRadius);
                        READ_INT(9, m_Type);
                        READ_BOOL(10, m_Is3DSound);
                        READ_STRING(11, m_Filename);
                        READ_STRING(12, m_DisplayText);
                        READ_FLOAT(18, m_StartOffset);
                        READ_FLOAT(19, m_PitchFactor);
                        READ_FLOAT(20, m_PitchFactorRandomizer);
                        READ_FLOAT(21, m_VolumeRandomizer);
                        READ_INT(22, m_VirtualChannel);
                        READ_INT(13, m_LogicalType);
                        READ_FLOAT(14, m_LogicalNotifDelay);
                        READ_BOOL(15, m_CreateLogicalSound);
                        READ_FLOAT(16, m_LogicalDropoffRadius);
                        READ_VECTOR(17, m_SphereColor);
                    }

                    cload.Close_Micro_Chunk();
                }

                break;
            case 0x200:
                while (cload.Open_Chunk()) {
                    if (cload.Cur_Chunk_ID() == 0x100) {
                        while (cload.Open_Micro_Chunk()) {
                            switch (cload.Cur_Micro_Chunk_ID()) {
                                READ_INT(1, m_ID);
                                READ_STRING(3, m_Name);
                            }

                            cload.Close_Micro_Chunk();
                        }
                    }

                    cload.Close_Chunk();
                }

                break;
        }

        cload.Close_Chunk();
    }
}

void DoVector3Channel(ChunkLoadClass &cload, ChunkData *data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    Vector3 value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value.X", name, i);
                    AddFloat(data, str, value.X);
                    str.Format("%s[%d].Value.Y", name, i);
                    AddFloat(data, str, value.Y);
                    str.Format("%s[%d].Value.Z", name, i);
                    AddFloat(data, str, value.Z);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void DoVector2Channel(ChunkLoadClass &cload, ChunkData *data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    Vector2 value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value.X", name, i);
                    AddFloat(data, str, value.X);
                    str.Format("%s[%d].Value.Y", name, i);
                    AddFloat(data, str, value.Y);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void DofloatChannel(ChunkLoadClass &cload, ChunkData *data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    float value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value", name, i);
                    AddFloat(data, str, value);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void DoAlphaVectorStructChannel(ChunkLoadClass &cload, ChunkData *data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    AlphaVectorStruct value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value.Quat.X", name, i);
                    AddFloat(data, str, value.Quat.X);
                    str.Format("%s[%d].Value.Quat.Y", name, i);
                    AddFloat(data, str, value.Quat.Y);
                    str.Format("%s[%d].Value.Quat.Z", name, i);
                    AddFloat(data, str, value.Quat.Z);
                    str.Format("%s[%d].Value.Quat.W", name, i);
                    AddFloat(data, str, value.Quat.W);
                    str.Format("%s[%d].Value.Magnitude", name, i);
                    AddFloat(data, str, value.Magnitude);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void Dump_W3D_CHUNK_RING(ChunkLoadClass &cload, ChunkData *data)
{
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 1: {
                W3dRingStruct RingStruct;
                cload.Read(&RingStruct, sizeof(RingStruct));
                AddInt32(data, "unk0", RingStruct.unk0);
                AddInt32(data, "Flags", RingStruct.Flags);

                if (RingStruct.Flags & RING_CAMERA_ALIGNED) {
                    AddString(data, "Flags", "RING_CAMERA_ALIGNED", "flag");
                }

                if (RingStruct.Flags & RING_LOOPING) {
                    AddString(data, "Flags", "RING_LOOPING", "flag");
                }

                if (RingStruct.Flags & 0xFFFFFFFC) {
                    StringClass str;
                    str.Format("W3D_CHUNK_RING Unknown Ring Flags %x", RingStruct.Flags & 0xFFFFFFFC);
                    data->unknowndata.Add(str);
                    AddString(data, "Flags", "Unknown", "string");
                }

                AddString(data, "Name", RingStruct.Name, "string");
                AddVector(data, "Center", &RingStruct.Center);
                AddVector(data, "Extent", &RingStruct.Extent);
                AddFloat(data, "AnimationDuration", RingStruct.AnimationDuration);
                AddVector(data, "Color", &RingStruct.Color);
                AddFloat(data, "Alpha", RingStruct.Alpha);
                AddFloat(data, "InnerScale.X", RingStruct.InnerScale.x);
                AddFloat(data, "InnerScale.Y", RingStruct.InnerScale.y);
                AddFloat(data, "OuterScale.X", RingStruct.OuterScale.x);
                AddFloat(data, "OuterScale.Y", RingStruct.OuterScale.y);
                AddFloat(data, "InnerExtent.X", RingStruct.InnerExtent.x);
                AddFloat(data, "InnerExtent.Y", RingStruct.InnerExtent.y);
                AddFloat(data, "OuterExtent.X", RingStruct.OuterExtent.x);
                AddFloat(data, "OuterExtent.Y", RingStruct.OuterExtent.y);
                AddString(data, "TextureName", RingStruct.TextureName, "string");
                AddShader(data, "Shader", &RingStruct.Shader);
                AddInt32(data, "TextureTiling", RingStruct.TextureTiling);
                break;
            }
            case 2:
                DoVector3Channel(cload, data, "ColorChannel");
                break;
            case 3:
                DofloatChannel(cload, data, "AlphaChannel");
                break;
            case 4:
                DoVector2Channel(cload, data, "InnerScaleChannel");
                break;
            case 5:
                DoVector2Channel(cload, data, "OuterScaleChannel");
                break;
        }
        cload.Close_Chunk();
    }
}
void Dump_W3D_CHUNK_SPHERE(ChunkLoadClass &cload, ChunkData *data)
{
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 1: {
                W3dSphereStruct SphereStruct;
                cload.Read(&SphereStruct, sizeof(SphereStruct));
                AddInt32(data, "unk0", SphereStruct.unk0);
                AddInt32(data, "Flags", SphereStruct.Flags);
                if (SphereStruct.Flags & SPHERE_ALPHA_VECTOR) {
                    AddString(data, "Flags", "SPHERE_ALPHA_VECTOR", "flag");
                }
                if (SphereStruct.Flags & SPHERE_CAMERA_ALIGNED) {
                    AddString(data, "Flags", "SPHERE_CAMERA_ALIGNED", "flag");
                }
                if (SphereStruct.Flags & SPHERE_INVERT_EFFECT) {
                    AddString(data, "Flags", "SPHERE_INVERT_EFFECT", "flag");
                }
                if (SphereStruct.Flags & SPHERE_LOOPING) {
                    AddString(data, "Flags", "SPHERE_LOOPING", "flag");
                }
                if (SphereStruct.Flags & 0xFFFFFFF0) {
                    StringClass str;
                    str.Format("W3D_CHUNK_SPHERE Unknown Sphere Flags %x", SphereStruct.Flags & 0xFFFFFFFC);
                    data->unknowndata.Add(str);
                    AddString(data, "Flags", "Unknown", "string");
                }
                AddString(data, "Name", SphereStruct.Name, "string");
                AddVector(data, "Center", &SphereStruct.Center);
                AddVector(data, "Extent", &SphereStruct.Extent);
                AddFloat(data, "AnimationDuration", SphereStruct.AnimationDuration);
                AddVector(data, "Color", &SphereStruct.Color);
                AddFloat(data, "Alpha", SphereStruct.Alpha);
                AddVector(data, "Scale", &SphereStruct.Scale);
                AddFloat(data, "Vector.Quat.X", SphereStruct.Vector.Quat.X);
                AddFloat(data, "Vector.Quat.Y", SphereStruct.Vector.Quat.Y);
                AddFloat(data, "Vector.Quat.Z", SphereStruct.Vector.Quat.Z);
                AddFloat(data, "Vector.Quat.W", SphereStruct.Vector.Quat.W);
                AddFloat(data, "Vector.Magnutide", SphereStruct.Vector.Magnitude);
                AddString(data, "TextureName", SphereStruct.TextureName, "string");
                AddShader(data, "Shader", &SphereStruct.Shader);
            } break;
            case 2:
                DoVector3Channel(cload, data, "ColorChannel");
                break;
            case 3:
                DofloatChannel(cload, data, "AlphaChannel");
                break;
            case 4:
                DoVector3Channel(cload, data, "ScaleChannel");
                break;
            case 5:
                DoAlphaVectorStructChannel(cload, data, "VectorChannel");
                break;
        }
        cload.Close_Chunk();
    }
}

void Dump_W3D_CHUNK_SHDMESH(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_SHDMESH_NAME(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "Name", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_SHDSUBMESH_SHADER(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_SHDSUBMESH_SHADER_TYPE(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *type = (uint32_t *)chunkdata;
    AddInt32(data, "Shader Type", *type);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_VERTICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *vertices = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Vertex[%d]", i);
        AddVector(data, c, &vertices[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *normals = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Normal[%d]", i);
        AddVector(data, c, &normals[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_TRIANGLES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    Vector3i16 *ids = (Vector3i16 *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(Vector3i16); i++) {
        char c[256];
        sprintf(c, "Triangle[%d]", i);
        AddIJK16(data, c, &ids[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *indices = (uint32_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Index[%d]", i);
        AddInt32(data, c, indices[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_UV0(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTexCoordStruct *coords = (W3dTexCoordStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dTexCoordStruct); i++) {
        char c[256];
        sprintf(c, "UV0[%d]", i);
        AddTexCoord(data, c, &coords[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_UV1(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTexCoordStruct *coords = (W3dTexCoordStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dTexCoordStruct); i++) {
        char c[256];
        sprintf(c, "UV1[%d]", i);
        AddTexCoord(data, c, &coords[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *tangents = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Tangent Basis S[%d]", i);
        AddVector(data, c, &tangents[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *tangents = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Tangent Basis T[%d]", i);
        AddVector(data, c, &tangents[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVectorStruct *tangents = (W3dVectorStruct *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dVectorStruct); i++) {
        char c[256];
        sprintf(c, "Tangent Basis SXT[%d]", i);
        AddVector(data, c, &tangents[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_EMITTER_EXTRA_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterExtraInfoStruct *info = (W3dEmitterExtraInfoStruct *)chunkdata;
    AddFloat(data, "FutureStartTime", info->FutureStartTime);
    AddInt8(data, "unk1", info->unk1);
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_SHDMESH_USER_TEXT(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "UserText", chunkdata, "string");
    delete[] chunkdata;
}

void Dump_W3D_CHUNK_FXSHADER_IDS(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t *ids = (uint32_t *)chunkdata;

    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(uint32_t); i++) {
        char c[256];
        sprintf(c, "Face[%d] FX Shader Index", i);
        AddInt32(data, c, ids[i]);
    }

    delete[] chunkdata;
}

void Dump_W3D_CHUNK_FX_SHADERS(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_FX_SHADER(ChunkLoadClass &cload, ChunkData *data)
{
    ParseSubchunks(cload, data);
}

void Dump_W3D_CHUNK_FX_SHADER_INFO(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint8_t *version = (uint8_t *)chunkdata;
    AddInt8(data, "Version", *version);
    W3dFXShaderStruct *shader = (W3dFXShaderStruct *)(chunkdata + 1);
    AddString(data, "ShaderName", shader->shadername, "string");
    AddInt8(data, "Technique", shader->technique);
    delete[] chunkdata;
}

const char *Types[] = { "Texture", "Float", "Vector2", "Vector3", "Vector4", "Int", "Bool" };

void Dump_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t type = *(uint32_t *)chunkdata;
    uint32_t length = *(uint32_t *)(chunkdata + 4);
    char *constantname = (char *)(chunkdata + 4 + 4);
    AddString(data, "Type", Types[type - 1], "String");
    AddString(data, "Constant Name", constantname, "String");

    if (type == CONSTANT_TYPE_TEXTURE) {
        char *texture = (char *)(chunkdata + 4 + 4 + length + 4);
        AddString(data, "Texture", texture, "String");
    } else if (type >= CONSTANT_TYPE_FLOAT1 && type <= CONSTANT_TYPE_FLOAT4) {
        int count = type - 1;
        float *floats = (float *)(chunkdata + 4 + 4 + length);
        AddFloatArray(data, "Floats", floats, count);
    } else if (type == CONSTANT_TYPE_INT) {
        uint32_t u = *(uint32_t *)(chunkdata + 4 + 4 + length);
        AddInt32(data, "Int", u);
    } else if (type == CONSTANT_TYPE_BOOL) {
        uint8_t u = *(uint8_t *)(chunkdata + 4 + 4 + length);
        AddInt32(data, "Bool", u);
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_FX_SHADER_CONSTANT Unknown Constant Type %x", type);
        data->unknowndata.Add(str);
        AddString(data, "Unknown", "Unknown", "string");
    }

    delete[] chunkdata;
}
const char *NewFlavorTypes[] = { "Timecoded", "Adaptive Delta 4", "Adaptive Delta 8" };

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkLoadClass &cload, ChunkData *data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCompressedMotionChannelStruct *channel = (W3dCompressedMotionChannelStruct *)chunkdata;

    if (channel->Flavor < ANIM_FLAVOR_NEW_VALID) {
        AddString(data, "Flavor", NewFlavorTypes[channel->Flavor], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Flavor Type %x", channel->Flavor);
        data->unknowndata.Add(str);
        AddString(data, "Flavor", "Unknown", "string");
    }

    AddInt8(data, "VectorLen", channel->VectorLen);

    if (channel->Flags <= ANIM_CHANNEL_VIS) {
        AddString(data, "ChannelType", ChannelTypes[channel->Flags], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        data->unknowndata.Add(str);
        AddString(data, "ChnanelType", "Unknown", "string");
    }

    AddInt32(data, "NumTimeCodes", channel->NumTimeCodes);
    AddInt16(data, "Pivot", channel->Pivot);

    if (channel->Flavor == ANIM_FLAVOR_NEW_TIMECODED) {
        uint16_t *keyframes = (uint16_t *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct));

        for (int i = 0; i < channel->NumTimeCodes; i++) {
            StringClass str;
            str.Format("KeyFrames[%d]", i);
            AddInt32(data, str, keyframes[i]);
        }

        int datalen = channel->VectorLen * channel->NumTimeCodes;
        int pos = sizeof(W3dCompressedMotionChannelStruct);
        pos += channel->NumTimeCodes * 2;

        if (channel->NumTimeCodes & 1) {
            pos += 2;
        }

        uint32_t *values = (uint32_t *)(chunkdata + pos);

        for (int i = 0; i < datalen; i++) {
            StringClass str;
            str.Format("Data[%d]", i);
            AddInt32(data, str, values[i]);
        }
    } else {
        float *scale = (float *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct));
        AddFloat(data, "Scale", *scale);
        float *initial = (float *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct) + 4);

        for (int i = 0; i < channel->VectorLen; i++) {
            StringClass str;
            str.Format("Initial[%d]", i);
            AddFloat(data, str, initial[i]);
        }

        int count = (cload.Cur_Chunk_Length() - sizeof(W3dCompressedMotionChannelStruct) - 4 - 4 * channel->VectorLen) / 4;
        uint32_t *values = (uint32_t *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct) + 4 + 4 * channel->VectorLen);

        for (int i = 0; i < count; i++) {
            StringClass str;
            str.Format("Data[%d]", i);
            AddInt32(data, str, values[i]);
        }
    }

    delete[] chunkdata;
}

#define CHUNK(id) \
    { \
        ChunkDumper c; \
        c.name = #id; \
        c.function = Dump_##id; \
        g_chunkDumpMap[id] = c; \
    }

void initmap()
{
    CHUNK(O_W3D_CHUNK_MATERIALS);
    CHUNK(O_W3D_CHUNK_MATERIALS2);
    CHUNK(O_W3D_CHUNK_POV_QUADRANGLES);
    CHUNK(O_W3D_CHUNK_POV_TRIANGLES);
    CHUNK(O_W3D_CHUNK_QUADRANGLES);
    CHUNK(O_W3D_CHUNK_SURRENDER_TRIANGLES);
    CHUNK(O_W3D_CHUNK_TRIANGLES);
    CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME);
    CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME);
    CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME);
    CHUNK(OBSOLETE_W3D_CHUNK_SHADOW_NODE);
    CHUNK(W3D_CHUNK_AABTREE);
    CHUNK(W3D_CHUNK_AABTREE_HEADER);
    CHUNK(W3D_CHUNK_AABTREE_NODES);
    CHUNK(W3D_CHUNK_AABTREE_POLYINDICES);
    CHUNK(W3D_CHUNK_AGGREGATE);
    CHUNK(W3D_CHUNK_AGGREGATE_CLASS_INFO);
    CHUNK(W3D_CHUNK_AGGREGATE_HEADER);
    CHUNK(W3D_CHUNK_AGGREGATE_INFO);
    CHUNK(W3D_CHUNK_ANIMATION);
    CHUNK(W3D_CHUNK_ANIMATION_CHANNEL);
    CHUNK(W3D_CHUNK_ANIMATION_HEADER);
    CHUNK(W3D_CHUNK_BIT_CHANNEL);
    CHUNK(W3D_CHUNK_BOX);
    CHUNK(W3D_CHUNK_COLLECTION);
    CHUNK(W3D_CHUNK_COLLECTION_HEADER);
    CHUNK(W3D_CHUNK_COLLECTION_OBJ_NAME);
    CHUNK(W3D_CHUNK_COLLISION_NODE);
    CHUNK(W3D_CHUNK_DAMAGE);
    CHUNK(W3D_CHUNK_DAMAGE_COLORS);
    CHUNK(W3D_CHUNK_DAMAGE_HEADER);
    CHUNK(W3D_CHUNK_DAMAGE_VERTICES);
    CHUNK(W3D_CHUNK_DAZZLE);
    CHUNK(W3D_CHUNK_DAZZLE_NAME);
    CHUNK(W3D_CHUNK_DAZZLE_TYPENAME);
    CHUNK(W3D_CHUNK_DCG);
    CHUNK(W3D_CHUNK_DIG);
    CHUNK(W3D_CHUNK_EMITTER);
    CHUNK(W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES);
    CHUNK(W3D_CHUNK_EMITTER_FRAME_KEYFRAMES);
    CHUNK(W3D_CHUNK_EMITTER_HEADER);
    CHUNK(W3D_CHUNK_EMITTER_INFO);
    CHUNK(W3D_CHUNK_EMITTER_INFOV2);
    CHUNK(W3D_CHUNK_EMITTER_PROPS);
    CHUNK(W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES);
    CHUNK(W3D_CHUNK_EMITTER_USER_DATA);
    CHUNK(W3D_CHUNK_FAR_ATTENUATION);
    CHUNK(W3D_CHUNK_HIERARCHY);
    CHUNK(W3D_CHUNK_HIERARCHY_HEADER);
    CHUNK(W3D_CHUNK_HLOD);
    CHUNK(W3D_CHUNK_HLOD_AGGREGATE_ARRAY);
    CHUNK(W3D_CHUNK_HLOD_HEADER);
    CHUNK(W3D_CHUNK_HLOD_LOD_ARRAY);
    CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER);
    CHUNK(W3D_CHUNK_HLOD_PROXY_ARRAY);
    CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT);
    CHUNK(W3D_CHUNK_HMODEL);
    CHUNK(OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA);
    CHUNK(W3D_CHUNK_HMODEL_HEADER);
    CHUNK(W3D_CHUNK_LIGHT);
    CHUNK(W3D_CHUNK_LIGHT_INFO);
    CHUNK(W3D_CHUNK_LIGHT_TRANSFORM);
    CHUNK(W3D_CHUNK_LIGHTSCAPE);
    CHUNK(W3D_CHUNK_LIGHTSCAPE_LIGHT);
    CHUNK(W3D_CHUNK_LOD);
    CHUNK(W3D_CHUNK_LODMODEL);
    CHUNK(W3D_CHUNK_LODMODEL_HEADER);
    CHUNK(W3D_CHUNK_MAP3_FILENAME);
    CHUNK(W3D_CHUNK_MAP3_INFO);
    CHUNK(W3D_CHUNK_MATERIAL_INFO);
    CHUNK(W3D_CHUNK_MATERIAL_PASS);
    CHUNK(W3D_CHUNK_MATERIAL3);
    CHUNK(W3D_CHUNK_MATERIAL3_DC_MAP);
    CHUNK(W3D_CHUNK_MATERIAL3_DI_MAP);
    CHUNK(W3D_CHUNK_MATERIAL3_INFO);
    CHUNK(W3D_CHUNK_MATERIAL3_NAME);
    CHUNK(W3D_CHUNK_MATERIAL3_SC_MAP);
    CHUNK(W3D_CHUNK_MATERIAL3_SI_MAP);
    CHUNK(W3D_CHUNK_MATERIALS3);
    CHUNK(W3D_CHUNK_MESH);
    CHUNK(W3D_CHUNK_MESH_HEADER);
    CHUNK(W3D_CHUNK_MESH_HEADER3);
    CHUNK(W3D_CHUNK_MESH_USER_TEXT);
    CHUNK(W3D_CHUNK_NEAR_ATTENUATION);
    CHUNK(W3D_CHUNK_NODE);
    CHUNK(W3D_CHUNK_NULL_OBJECT);
    CHUNK(W3D_CHUNK_PER_FACE_TEXCOORD_IDS);
    CHUNK(W3D_CHUNK_PER_TRI_MATERIALS);
    CHUNK(W3D_CHUNK_PIVOT_FIXUPS);
    CHUNK(W3D_CHUNK_PIVOTS);
    CHUNK(W3D_CHUNK_PLACEHOLDER);
    CHUNK(W3D_CHUNK_POINTS);
    CHUNK(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS);
    CHUNK(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE);
    CHUNK(W3D_CHUNK_PRELIT_UNLIT);
    CHUNK(W3D_CHUNK_PRELIT_VERTEX);
    CHUNK(W3D_CHUNK_PS2_SHADERS);
    CHUNK(W3D_CHUNK_SCG);
    CHUNK(W3D_CHUNK_SHADER_IDS);
    CHUNK(W3D_CHUNK_SHADERS);
    CHUNK(W3D_CHUNK_SKIN_NODE);
    CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO);
    CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO_5_0);
    CHUNK(W3D_CHUNK_STAGE_TEXCOORDS);
    CHUNK(W3D_CHUNK_SURRENDER_NORMALS);
    CHUNK(W3D_CHUNK_TEXCOORDS);
    CHUNK(W3D_CHUNK_TEXTURE);
    CHUNK(W3D_CHUNK_TEXTURE_IDS);
    CHUNK(W3D_CHUNK_TEXTURE_INFO);
    CHUNK(W3D_CHUNK_TEXTURE_NAME);
    CHUNK(W3D_CHUNK_TEXTURE_REPLACER_INFO);
    CHUNK(W3D_CHUNK_TEXTURE_STAGE);
    CHUNK(W3D_CHUNK_TEXTURES);
    CHUNK(W3D_CHUNK_TRANSFORM_NODE);
    CHUNK(W3D_CHUNK_TRIANGLES);
    CHUNK(W3D_CHUNK_VERTEX_COLORS);
    CHUNK(W3D_CHUNK_VERTEX_INFLUENCES);
    CHUNK(W3D_CHUNK_VERTEX_MAPPER_ARGS0);
    CHUNK(W3D_CHUNK_VERTEX_MAPPER_ARGS1);
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL);
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL_IDS);
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL_INFO);
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL_NAME);
    CHUNK(W3D_CHUNK_VERTEX_MATERIALS);
    CHUNK(W3D_CHUNK_VERTEX_NORMALS);
    CHUNK(W3D_CHUNK_VERTEX_SHADE_INDICES);
    CHUNK(W3D_CHUNK_VERTICES);
    CHUNK(W3D_CHUNK_EMITTER_LINE_PROPERTIES);
    CHUNK(W3D_CHUNK_SECONDARY_VERTICES)
    CHUNK(W3D_CHUNK_SECONDARY_VERTEX_NORMALS)
    CHUNK(W3D_CHUNK_TANGENTS)
    CHUNK(W3D_CHUNK_BINORMALS)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION_HEADER)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL)
    CHUNK(W3D_CHUNK_COMPRESSED_BIT_CHANNEL)
    CHUNK(W3D_CHUNK_MORPH_ANIMATION)
    CHUNK(W3D_CHUNK_MORPHANIM_HEADER)
    CHUNK(W3D_CHUNK_MORPHANIM_CHANNEL)
    CHUNK(W3D_CHUNK_MORPHANIM_POSENAME)
    CHUNK(W3D_CHUNK_MORPHANIM_KEYDATA)
    CHUNK(W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA)
    CHUNK(W3D_CHUNK_SOUNDROBJ)
    CHUNK(W3D_CHUNK_SOUNDROBJ_HEADER)
    CHUNK(W3D_CHUNK_SOUNDROBJ_DEFINITION)
    CHUNK(W3D_CHUNK_RING)
    CHUNK(W3D_CHUNK_SPHERE)
    CHUNK(W3D_CHUNK_SHDMESH)
    CHUNK(W3D_CHUNK_SHDMESH_NAME)
    CHUNK(W3D_CHUNK_SHDSUBMESH)
    CHUNK(W3D_CHUNK_SHDSUBMESH_SHADER)
    CHUNK(W3D_CHUNK_SHDSUBMESH_SHADER_TYPE)
    CHUNK(W3D_CHUNK_SHDSUBMESH_VERTICES)
    CHUNK(W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TRIANGLES)
    CHUNK(W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES)
    CHUNK(W3D_CHUNK_SHDSUBMESH_UV0)
    CHUNK(W3D_CHUNK_SHDSUBMESH_UV1)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT)
    CHUNK(W3D_CHUNK_EMITTER_EXTRA_INFO)
    CHUNK(W3D_CHUNK_SHDMESH_USER_TEXT)
    CHUNK(W3D_CHUNK_FXSHADER_IDS)
    CHUNK(W3D_CHUNK_FX_SHADERS)
    CHUNK(W3D_CHUNK_FX_SHADER)
    CHUNK(W3D_CHUNK_FX_SHADER_INFO)
    CHUNK(W3D_CHUNK_FX_SHADER_CONSTANT)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL)
}

void DumpData(FILE *out, FILE *unknown, ChunkData *data, StringClass tabs)
{
    StringClass str = tabs;
    str += '\t';

    for (int i = 0; i < data->data.Count(); i++) {
        fprintf(out, "%s%s %s\n", tabs.Peek_Buffer(), data->data[i]->name.Peek_Buffer(), data->data[i]->value.Peek_Buffer());
    }

    for (int i = 0; i < data->subchunks.Count(); i++) {
        fprintf(out, "%s%s\n", tabs.Peek_Buffer(), data->subchunks[i]->name.Peek_Buffer());
        DumpData(out, unknown, data->subchunks[i], str);
    }

    for (int i = 0; i < data->unknowndata.Count(); i++) {
        fprintf(unknown, "%s\n", data->unknowndata[i].Peek_Buffer());
    }
}

HTREEITEM TreeViewInsertItem(HWND tree, const char *text, HTREEITEM parent, HTREEITEM insertafter)
{
    TVINSERTSTRUCT str;
    str.hParent = parent;
    str.hInsertAfter = insertafter;
    str.item.mask = TVIF_TEXT;
    str.item.pszText = (LPSTR)text;
    return TreeView_InsertItem(tree, &str);
}

void TreeViewSetItem(HWND tree, HTREEITEM item, LPARAM param)
{
    TVITEM tv;
    tv.mask = TVIF_PARAM;
    tv.hItem = item;
    tv.lParam = param;
    TreeView_SetItem(tree, &tv);
}

LPARAM TreeViewGetItem(HWND tree, HTREEITEM item)
{
    TVITEM tv;
    tv.mask = TVIF_PARAM;
    tv.hItem = item;
    TreeView_GetItem(tree, &tv);
    return tv.lParam;
}

void ListViewInsertColumn(HWND list, int col, char *name, int width)
{
    LVCOLUMN column;
    column.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    column.pszText = name;
    column.fmt = LVCFMT_LEFT;
    column.cx = width;
    ListView_InsertColumn(list, col, &column);
}

int ListViewInsertItem(HWND list, int i, char *name)
{
    LVITEM item;
    memset(&item, 0, sizeof(item));
    item.mask = LVIF_TEXT;
    item.iItem = i;
    item.pszText = name;
    return ListView_InsertItem(list, &item);
}

void ListViewSetItemText(HWND list, int item, int subitem, char *str)
{
    ListView_SetItemText(list, item, subitem, str);
}

bool GetOpenFile(char *buf, const char *filter, const char *dir, HWND parent, const char *title)
{
    char lBuf[MAX_PATH] = "";
    StringClass ws = buf;
    strcpy(lBuf, ws);
    OPENFILENAME of;

    memset(&of, 0, sizeof(OPENFILENAME));

    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = parent;
    of.hInstance = nullptr;
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = nullptr;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = lBuf;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = nullptr;
    of.nMaxFileTitle = 0;
    of.lpstrInitialDir = dir;
    of.lpstrTitle = title;
    of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = nullptr;
    of.lCustData = 0;
    of.lpfnHook = nullptr;
    of.lpTemplateName = nullptr;

    if (!GetOpenFileName(&of)) {
        return false;
    }

    _snprintf(buf, MAX_PATH, "%s", of.lpstrFile);
    return true;
}

void AddItems(ChunkData *data, HTREEITEM item)
{
    StringClass str = data->name;
    HTREEITEM newitem = TreeViewInsertItem(g_treewnd, str, item, TVI_LAST);
    TreeViewSetItem(g_treewnd, newitem, (LPARAM)data);

    for (int i = 0; i < data->subchunks.Count(); i++) {
        AddItems(data->subchunks[i], newitem);
    }
}

ChunkData *g_main = nullptr;

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
            break;
        case WM_SIZE:
            if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED) {
                WORD cx = LOWORD(lParam);
                WORD cy = HIWORD(lParam);
                g_mainwidth = cx;
                g_mainheight = cy;
                SetWindowPos(g_listwnd, nullptr, 300, 0, g_mainwidth - 300, g_mainheight, SWP_NOZORDER);
                SetWindowPos(g_treewnd, nullptr, 0, 0, 300, g_mainheight, SWP_NOMOVE | SWP_NOZORDER);
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_OPEN: {
                    char fname[MAX_PATH];
                    fname[0] = 0;

                    if (GetOpenFile(fname,
                            "W3D Files (*.w3d)\0*.w3d\0WLT Files (*.wlt)\0*.wlt\0WHT Files (*.wht)\0*.wht\0WHA Files "
                            "(*.wha)\0*.wha\0WTM Files (*.wtm)\0*.wtm\0All Files (*.*)\0*.*\0\0",
                            nullptr,
                            g_mainwnd,
                            "Open File")) {
                        TreeView_SetItemState(g_treewnd, TreeView_GetSelection(g_treewnd), 0, TVIS_SELECTED);
                        TreeView_DeleteAllItems(g_treewnd);
                        BufferedFileClass file(fname);
                        file.Open(FM_READ);
                        ChunkLoadClass cload(&file);

                        if (g_main) {
                            delete g_main;
                        }

                        g_main = new ChunkData;
                        ParseSubchunks(cload, g_main);

                        for (int i = 0; i < g_main->subchunks.Count(); i++) {
                            AddItems(g_main->subchunks[i], TVI_ROOT);
                        }
                    }
                } break;
                case ID_EXIT:
                    SendMessage(g_mainwnd, WM_CLOSE, 0, 0);
                    break;
                default:
                    return FALSE;
            }
            return TRUE;
        case WM_NOTIFY: {
            LPNMHDR hdr = (LPNMHDR)lParam;
            if (hdr->hwndFrom == g_treewnd && hdr->code == TVN_SELCHANGED) {
                LPNMTREEVIEW nm = (LPNMTREEVIEW)lParam;
                ChunkData *cd = (ChunkData *)TreeViewGetItem(g_treewnd, nm->itemNew.hItem);
                ListView_DeleteAllItems(g_listwnd);

                for (int i = 0; i < cd->subchunks.Count(); i++) {
                    StringClass str = cd->subchunks[i]->name;
                    int item = ListViewInsertItem(g_listwnd, 0xFFFF, str.Peek_Buffer());
                    ListViewSetItemText(g_listwnd, item, 1, "chunk");
                }

                for (int i = 0; i < cd->data.Count(); i++) {
                    StringClass str = cd->data[i]->name;
                    int item = ListViewInsertItem(g_listwnd, 0xFFFF, str.Peek_Buffer());
                    str = cd->data[i]->type;
                    ListViewSetItemText(g_listwnd, item, 1, str.Peek_Buffer());
                    str = cd->data[i]->value;
                    ListViewSetItemText(g_listwnd, item, 2, str.Peek_Buffer());
                }

                return TRUE;
            }
            if (hdr->hwndFrom == g_treewnd && hdr->code == TVN_DELETEITEM) {
                LPNMTREEVIEW tvn = (LPNMTREEVIEW)lParam;
                TVITEM tv;
                tv.mask = TVIF_PARAM;
                tv.hItem = tvn->itemOld.hItem;
                tv.lParam = (LPARAM)0;
                TreeView_SetItem(g_treewnd, &tv);
                return TRUE;
            }
        } break;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#pragma warning(suppress : 28251) // warning C28251: Inconsistent annotation for function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    initmap();

    if (__argc > 1) {
        BufferedFileClass file(__argv[1]);
        file.Open(FM_READ);
        ChunkLoadClass cload(&file);
        g_main = new ChunkData;
        ParseSubchunks(cload, g_main);
        StringClass of = __argv[1];
        of += ".txt";
        StringClass uf = __argv[1];
        uf += ".unk";
        FILE *out = fopen(of, "wt");
        FILE *unknown = fopen(uf, "wt");
        DumpData(out, unknown, g_main, "");
        fclose(unknown);
        fclose(out);
        HANDLE h = CreateFileA(uf, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        DWORD size = GetFileSize(h, nullptr);
        CloseHandle(h);

        if (!size) {
            DeleteFileA(uf);
        }

        delete g_main;
        return 0;
    }

    INITCOMMONCONTROLSEX ex;
    ex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&ex);
    WNDCLASSEX wcls = {};
    wcls.lpszClassName = CLASS_NAME;
    wcls.cbSize = sizeof(wcls);
    wcls.style = 0;
    wcls.lpfnWndProc = MainWindowProc;
    wcls.hIcon = LoadIcon(hInstance, (LPCSTR)IDI_MAIN);
    wcls.hIconSm = LoadIcon(hInstance, (LPCSTR)IDI_MAIN);
    wcls.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcls.cbClsExtra = 0;
    wcls.cbWndExtra = 0;
    wcls.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcls.hInstance = hInstance;
    RegisterClassEx(&wcls);

    DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS;
    RECT window_rect = { 0, 0, 1024, 768 };
    AdjustWindowRect(&window_rect, window_style, false);

    g_menu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
    g_mainwidth = window_rect.right - window_rect.left;
    g_mainheight = window_rect.bottom - window_rect.top;

    g_mainwnd = CreateWindowEx(0,
        CLASS_NAME,
        WND_TITLE,
        window_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        g_mainwidth,
        g_mainheight,
        nullptr,
        g_menu,
        hInstance,
        nullptr);

    g_treewnd = CreateWindow(WC_TREEVIEW,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_DISABLEDRAGDROP
            | TVS_SHOWSELALWAYS,
        0,
        0,
        300,
        g_mainheight,
        g_mainwnd,
        (HMENU)101,
        hInstance,
        nullptr);

    g_listwnd = CreateWindow(WC_LISTVIEW,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
        300,
        0,
        g_mainwidth - 300,
        g_mainheight,
        g_mainwnd,
        (HMENU)101,
        hInstance,
        nullptr);

    ListViewInsertColumn(g_listwnd, 0, "Name", 230);
    ListViewInsertColumn(g_listwnd, 1, "Type", 70);
    ListViewInsertColumn(g_listwnd, 2, "Value", 0xFFFF);
    g_accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL));
    MSG msg = {};

    while (WM_QUIT != msg.message) {
        if (GetMessage(&msg, nullptr, 0, 0)) {
            if (!TranslateAccelerator(g_mainwnd, g_accel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    if (g_main) {
        delete g_main;
    }

    return 0;
}
