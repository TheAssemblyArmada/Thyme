/**
 * @file
 *
 * @author feliwir
 *
 * @brief Set of tests to test w3d model loading
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <captainslog.h>
#include <gtest/gtest.h>

#include "always.h"
#include "asciistring.h"
#include "assetmgr.h"
#include "bufffile.h"
#include "chunkio.h"
#include "meshmdl.h"
#include "w3d_file.h"

struct Chunk
{
    int type;
    int size;
    std::vector<Chunk> subchunks;
};

// clang-format off
Chunk cubemodel = { -1, -1, 
    { { W3D_CHUNK_MESH, 1701, 
        { { W3D_CHUNK_MESH_HEADER3, 116 },
          { W3D_CHUNK_VERTICES, 432 },
          { W3D_CHUNK_VERTEX_NORMALS, 432 },
          { W3D_CHUNK_TRIANGLES, 384 },
          { W3D_CHUNK_VERTEX_SHADE_INDICES, 144 },
          { W3D_CHUNK_MATERIAL_INFO, 16 },
          { W3D_CHUNK_VERTEX_MATERIALS, 65, 
            { { W3D_CHUNK_VERTEX_MATERIAL, 57, 
                { { W3D_CHUNK_VERTEX_MATERIAL_NAME, 9 }, 
                  { W3D_CHUNK_VERTEX_MATERIAL_INFO, 32 },
                } },
            } },
          { W3D_CHUNK_SHADERS, 16 },
          { W3D_CHUNK_MATERIAL_PASS, 24,
            { { W3D_CHUNK_VERTEX_MATERIAL_IDS, 4}, 
              { W3D_CHUNK_SHADER_IDS, 4},
            } }, 
        } },
    } 
};
// clang-format on

void validate_chunk(ChunkLoadClass &cload, const Chunk &chunk, int depth = 1)
{
    unsigned int idx = 0;
    while (cload.Open_Chunk()) {
        if (chunk.subchunks.size() <= idx) {
            ASSERT_TRUE(chunk.subchunks.size() > idx) << "Missing chunk: " << cload.Cur_Chunk_ID();
            cload.Close_Chunk();
        }

        const auto &ref = chunk.subchunks[idx];
        EXPECT_EQ(cload.Cur_Chunk_ID(), ref.type);
        EXPECT_EQ(cload.Cur_Chunk_Depth(), depth);
        EXPECT_EQ(cload.Cur_Chunk_Length(), ref.size);

        // Read the data and check we get all bytes
        std::vector<uint8_t> data;
        data.resize(cload.Cur_Chunk_Length());
        EXPECT_EQ(cload.Read(data.data(), data.size()), ref.size);

        if (cload.Contains_Chunks()) {
            validate_chunk(cload, ref, depth + 1);
        }

        cload.Close_Chunk();
        idx++;
    }
}

TEST(w3d_model, validate_chunk_loader)
{
    auto filepath = Utf8String(TESTDATA_PATH) + "/models/cube.w3d";

    BufferedFileClass file(filepath.Str());
    ASSERT_TRUE(file.Open(FM_READ));
    ChunkLoadClass cload(&file);

    // Traverse through the model to validate chunks
    validate_chunk(cload, cubemodel);
}

TEST(w3d_model, load_model)
{
    // We need to have an instance of this
    W3DAssetManager assetmngr;
    auto filepath = Utf8String(TESTDATA_PATH) + "/models/cube.w3d";

    BufferedFileClass file(filepath.Str());
    ASSERT_TRUE(file.Open(FM_READ));
    ChunkLoadClass cload(&file);
    EXPECT_TRUE(cload.Open_Chunk());
    EXPECT_EQ(cload.Cur_Chunk_ID(), W3D_CHUNK_MESH);

    MeshModelClass mesh;
    EXPECT_EQ(mesh.Load_W3D(cload), W3D_ERROR_OK);
    EXPECT_EQ(mesh.Get_Polygon_Count(), 12);
    EXPECT_EQ(mesh.Get_Vertex_Count(), 36);
    EXPECT_STREQ(mesh.Get_Name(), "cube");

    cload.Close_Chunk();
}
