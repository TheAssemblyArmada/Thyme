/**
 * @file
 *
 * @author tomsons26
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
#include <cstdint>

int32_t MAX_FRAMES_AHEAD = 128;
int32_t MIN_RUNAHEAD = 10;
int32_t FRAMES_TO_KEEP = MAX_FRAMES_AHEAD / 2 + 1;
int32_t FRAME_DATA_LENGTH = 2 * MAX_FRAMES_AHEAD + 2;
