/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief SHA1 hash algorithm implementation.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <stdint.h>

class SHAEngine
{
    enum
    {
        SHA_DIGEST_LENGTH = 20,
        SHA_BLOCK_LENGTH = 64,
    };
public:
    struct SHADigest
    {
        uint32_t H0;
        uint32_t H1;
        uint32_t H2;
        uint32_t H3;
        uint32_t H4;
    };

    SHAEngine();
    ~SHAEngine();

    void Hash(const void *input, int length);
    int Result(void *output);
    const char *Print_Result();

private:
    void Process_Partial(const void *&data, int &length);
    void Process_Block(const void *data, SHADigest &digest) const;

private:
    bool m_computed;
    SHADigest m_finalDigest;
    SHADigest m_digest;
    uint32_t m_processedBytes;
    uint32_t m_unprocessedBytes;
    char m_messageBlock[SHA_BLOCK_LENGTH];
    char m_asciiDigest[SHA_DIGEST_LENGTH * 2 + 1];
};
