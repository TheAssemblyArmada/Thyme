/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Bib buffer
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
#include "gametype.h"
#include "vector3.h"
#include <new>

class TextureClass;
class DX8IndexBufferClass;
class DX8VertexBufferClass;

class W3DBib
{
    friend class W3DBibBuffer;

public:
    W3DBib();
    ~W3DBib() {}

private:
    Vector3 m_points[4];
    bool m_highlighted;
    int m_unknown;
    ObjectID m_objectID;
    DrawableID m_drawableID;
    bool m_isUnused;
};

class W3DBibBuffer
{
public:
    enum
    {
        MAX_BIBS = 1000,
    };

    W3DBibBuffer();
    ~W3DBibBuffer();

    void Load_Bibs_In_Vertex_And_Index_Buffers();
    void Free_Bib_Buffers();
    void Allocate_Bib_Buffers();

    void Clear_All_Bibs();

    void Remove_Highlighting();

    void Add_Bib_To_Object(Vector3 *points, ObjectID id, bool highlighted);
    void Add_Bib_To_Drawable(Vector3 *points, DrawableID id, bool highlighted);
    void Remove_Bib_From_Object(ObjectID id);
    void Remove_Bib_From_Drawable(DrawableID id);

    void Render_Bibs();

#ifdef GAME_DLL
    W3DBibBuffer *Hook_Ctor() { return new (this) W3DBibBuffer(); }
    void Hook_Dtor() { W3DBibBuffer::~W3DBibBuffer(); }
#endif

private:
    DX8VertexBufferClass *m_vertexBib;
    int m_maxBibVertex;
    DX8IndexBufferClass *m_indexBib;
    int m_maxBibIndex;
    TextureClass *m_bibTexture;
    TextureClass *m_redBibTexture;
    int m_numBibVertices;
    int m_numBibIndices;
    int m_firstIndex;
    int m_firstVertex;
    W3DBib m_bibs[MAX_BIBS];
    int m_numBibs;
    bool m_anythingChanged;
    bool m_updateVis;
    bool m_initialized;
};
