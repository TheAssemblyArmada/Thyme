/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Virtual interface for video streams.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "videostream.h"
#include "videoplayer.h"

/**
 * 0x0051B1F0
 */
VideoStream::~VideoStream()
{
    if (m_player != nullptr) {
        m_player->Remove_Stream(this);
        m_player = nullptr;
    }
}
