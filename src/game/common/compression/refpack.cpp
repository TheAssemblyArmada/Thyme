/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief RefPack LZ compression.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "refpack.h"

/**
* @brief Decompresses EA's proprietary "RefPack" format.
*/
int RefPack_Uncompress(void *dst, const void *src, int *size)
{
    const uint8_t *getp;
    uint8_t *ref;
    uint8_t *putp;
    uint8_t first;
    uint8_t second;
    uint8_t third;
    uint8_t forth;
    uint16_t flags;
    uint32_t run;
    int out_length = 0;

    if (src == nullptr) {
        if (size != nullptr) {
            *size = 0;
        }

        return 0;
    }

    getp = static_cast<const uint8_t *>(src);

    // This flag and size reading section appears to differe between different RefPack versions.
    flags = (getp[0] << 8) | getp[1];
    getp += 2;

    if (flags & 0x8000) {
        if (flags & 0x0100) {
            getp += 6;
        }

        out_length = (getp[0] << 24) | (getp[1] << 16) | (getp[2] << 8) | getp[3];
        getp += 4;
    } else {
        if (flags & 0x0100) {
            getp += 5;
        }

        out_length = (getp[0] << 16) | (getp[1] << 8) | getp[2];
        getp += 3;
    }

    putp = static_cast<uint8_t *>(dst);

    while (true) {
        first = *getp++;

        // Short command.
        if (!(first & 0x80)) {
            second = *getp++;
            run = first & 3;

            while (run--) {
                *putp++ = *getp++;
            }

            ref = putp - 1 - (((first & 0x60) << 3) + second);
            run = ((first & 0x1c) >> 2) + 3 - 1;

            do {
                *putp++ = *ref++;
            } while (run--);

            continue;
        }

        // Medium command.
        if (!(first & 0x40)) {
            second = *getp++;
            third = *getp++;
            run = second >> 6;

            while (run--) {
                *putp++ = *getp++;
            }

            ref = putp - 1 - (((second & 0x3f) << 8) + third);

            run = (first & 0x3f) + 4 - 1;

            do {
                *putp++ = *ref++;
            } while (run--);

            continue;
        }

        // Long command.
        if (!(first & 0x20)) {
            second = *getp++;
            third = *getp++;
            forth = *getp++;
            run = first & 3;

            while (run--) {
                *putp++ = *getp++;
            }

            ref = putp - 1 - (((first & 0x10) >> 4 << 16) + (second << 8) + third);

            run = ((first & 0x0c) >> 2 << 8) + forth + 5 - 1;

            do {
                *putp++ = *ref++;
            } while (run--);

            continue;
        }

        // Byte command.
        run = ((first & 0x1f) << 2) + 4;

        if (run <= 112) {
            while (run--) {
                *putp++ = *getp++;
            }

            continue;
        }

        // End marker and possibly run of up to 3 bytes.
        run = first & 3;

        while (run--) {
            *putp++ = *getp++;
        }

        break;
    }

    if (size != nullptr) {
        *size = getp - static_cast<const uint8_t *>(src);
    }

    return out_length;
}