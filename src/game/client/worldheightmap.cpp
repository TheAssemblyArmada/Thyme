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
#include "worldheightmap.h"
#include "file.h"
#include "inputstream.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

class GDIFileStream : public InputStream
{
public:
    GDIFileStream(File *file) : m_file(file) {}
    int Read(void *dst, int size) override { return m_file->Read(dst, size); }

private:
    File *m_file;
};

unsigned char *WorldHeightMap::Get_Pointer_To_Tile_Data(int x, int y, int width)
{
#ifdef GAME_DLL
    return Call_Method<unsigned char *, WorldHeightMap, int, int, int>(
        PICK_ADDRESS(0x007482F0, 0x005F198B), this, x, y, width);
#else
    return nullptr;
#endif
}
