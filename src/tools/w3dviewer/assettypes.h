/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View asset types
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "w3dafx.h"

enum AssetType
{
    ASSET_TYPE_NONE = -1,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MESH,
    ASSET_TYPE_HIERARCHY,
    ASSET_TYPE_AGGREGATE,
    ASSET_TYPE_HLOD,
    ASSET_TYPE_ANIMATION,
    ASSET_TYPE_UNK1,
    ASSET_TYPE_PARTICLEEMITTER,
    ASSET_TYPE_MORPH,
    ASSET_TYPE_PRIMITIVE,
    ASSET_TYPE_SOUND,
};
