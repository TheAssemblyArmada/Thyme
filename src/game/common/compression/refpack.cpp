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
#include <algorithm>
#include <cstdlib>
#include <cstring>

using std::malloc;
using std::max;
using std::memcpy;
using std::memset;
using std::min;

/**
 * Utility function to quickly calculate a pseudo-hash.
 */
static int32_t RefPack_Hash(const uint8_t *s)
{
    return ((s[0] << 8 | s[2]) ^ s[1] << 4);
}

/**
 * Utility function for compression for checking length of matching data.
 */
static uint32_t RefPack_Matchlen(const uint8_t *s, const uint8_t *d, uint32_t maxmatch)
{
    uint32_t current;

    for (current = 0; current < maxmatch && *s++ == *d++; ++current)
        ;

    return current;
}

/**
 * Compresses data using refpack LZ method
 */
static int RefPack_Encode(const void *src, int src_len, void *dst, int dst_len, bool quick)
{
    int32_t len;
    uint32_t tlen;
    uint32_t tcost;
    uint32_t run;
    uint32_t toffset;
    uint32_t boffset;
    uint32_t blen;
    uint32_t bcost;
    uint32_t mlen;
    const uint8_t *tptr;
    const uint8_t *getp;
    const uint8_t *runp;
    uint8_t *putp;
    int32_t hash;
    int32_t hoffset;
    int32_t minhoffset;
    int i;
    int32_t *link;
    int32_t *hashtbl;

    len = src_len;
    putp = static_cast<uint8_t *>(dst);
    run = 0;
    getp = runp = static_cast<const uint8_t *>(src);

    hashtbl = static_cast<int32_t *>(malloc(sizeof(int32_t) * 0x10000));
    link = static_cast<int32_t *>(malloc(sizeof(int32_t) * 0x20000));
    memset(hashtbl, 0xFF, sizeof(int32_t) * 0x10000);

    len -= 4;
    while (len >= 0) {
        boffset = 0;
        blen = bcost = 2;
        mlen = min(len, 1028);
        tptr = getp - 1;
        hash = RefPack_Hash(getp);
        hoffset = hashtbl[hash];
        minhoffset = max(intptr_t(getp - static_cast<const uint8_t *>(src) - 131071), intptr_t(0));

        if (hoffset >= minhoffset) {
            do {
                tptr = static_cast<const uint8_t *>(src) + hoffset;

                if (getp[blen] == tptr[blen]) {
                    tlen = RefPack_Matchlen(getp, tptr, mlen);

                    if (tlen > blen) {
                        toffset = (getp - 1) - tptr;

                        if (toffset < 1024 && tlen <= 10) { // two byte long form
                            tcost = 2;
                        } else if (toffset < 16384 && tlen <= 67) { // three byte long form
                            tcost = 3;
                        } else { // four byte very long form
                            tcost = 4;
                        }

                        if (tlen - tcost + 4 > blen - bcost + 4) {
                            blen = tlen;
                            bcost = tcost;
                            boffset = toffset;

                            if (blen >= 1028) {
                                break;
                            }
                        }
                    }
                }
            } while ((hoffset = link[hoffset & 131071]) >= minhoffset);
        }

        if (bcost >= blen || len < 4) {
            hoffset = (getp - static_cast<const uint8_t *>(src));
            link[hoffset & 131071] = hashtbl[hash];
            hashtbl[hash] = hoffset;

            ++run;
            ++getp;
            --len;
        } else {
            while (run > 3) { // literal block of data
                tlen = min((uint32_t)112, run & ~3);
                run -= tlen;
                *putp++ = (unsigned char)(0xe0 + (tlen >> 2) - 1);
                memcpy(putp, runp, tlen);
                runp += tlen;
                putp += tlen;
            }

            if (bcost == 2) // two byte long form
            {
                *putp++ = (unsigned char)(((boffset >> 8) << 5) + ((blen - 3) << 2) + run);
                *putp++ = (unsigned char)boffset;
            } else if (bcost == 3) { // three byte long form
                *putp++ = (unsigned char)(0x80 + (blen - 4));
                *putp++ = (unsigned char)((run << 6) + (boffset >> 8));
                *putp++ = (unsigned char)boffset;
            } else { // four byte very long form
                *putp++ = (unsigned char)(0xc0 + ((boffset >> 16) << 4) + (((blen - 5) >> 8) << 2) + run);
                *putp++ = (unsigned char)(boffset >> 8);
                *putp++ = (unsigned char)(boffset);
                *putp++ = (unsigned char)(blen - 5);
            }

            if (run) {
                memcpy(putp, runp, run);
                putp += run;
                run = 0;
            }

            if (quick) {
                hoffset = (getp - static_cast<const uint8_t *>(src));
                link[hoffset & 131071] = hashtbl[hash];
                hashtbl[hash] = hoffset;
                getp += blen;
            } else {
                for (i = 0; i < (int)blen; ++i) {
                    hash = RefPack_Hash(getp);
                    hoffset = (getp - static_cast<const uint8_t *>(src));
                    link[hoffset & 131071] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                    ++getp;
                }
            }

            runp = getp;
            len -= blen;
        }
    }
    len += 4;
    run += len;

    while (run > 3) // no match at end, use literal
    {
        tlen = min((uint32_t)112, run & ~3);
        run -= tlen;
        *putp++ = (unsigned char)(0xe0 + (tlen >> 2) - 1);
        memcpy(putp, runp, tlen);
        runp += tlen;
        putp += tlen;
    }

    *putp++ = (unsigned char)(0xFC + run); // end of stream command + 0..3 literal
    if (run) {
        memcpy(putp, runp, run);
        putp += run;
    }

    free(link);
    free(hashtbl);

    return putp - static_cast<uint8_t *>(dst);
}

/**
 * Decompresses EA's proprietary "RefPack" format.
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
            getp += 4;
        }

        out_length = (getp[0] << 24) | (getp[1] << 16) | (getp[2] << 8) | getp[3];
        getp += 4;
    } else {
        if (flags & 0x0100) {
            getp += 3;
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

/**
 * Compresses EA's proprietary "RefPack" format.
 */
int RefPack_Compress(void *dst, const void *src, int size, int *opts)
{
    uint8_t *putp = static_cast<uint8_t *>(dst);
    int header_len = 0;

    if (size < 0xFFFFFF) {
        putp[0] = 0x10;
        putp[1] = 0xFB;
        putp[2] = (unsigned)(size & 0xFF0000) >> 16;
        putp[3] = (unsigned)(size & 0xFF00) >> 8;
        putp[4] = (unsigned)(size & 0xFF);
        header_len = 5;
    } else {
        putp[0] = 0x90;
        putp[1] = 0xFB;
        putp[2] = (unsigned)(size & 0xFF000000) >> 24;
        putp[3] = (unsigned)(size & 0xFF0000) >> 16;
        putp[4] = (unsigned)(size & 0xFF00) >> 8;
        putp[5] = (unsigned)(size & 0xFF);
        header_len = 6;
    }

    return header_len + RefPack_Encode(src, size, &putp[header_len], 0x20000, false);
}
