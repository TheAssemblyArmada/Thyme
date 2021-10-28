/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief loaders
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "loaders.h"
#include <captainslog.h>

PrototypeClass *AggregateLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_assert("Aggregate not suported");
    return nullptr;
}

PrototypeClass *RingLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_assert("Ring not suported");
    return nullptr;
}

PrototypeClass *SphereLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    captainslog_assert("Sphere not suported");
    return nullptr;
}
