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
#include "sha1.h"
#include "endiantype.h"
#include "intrinsics.h"
#include <cstring>
#include <cstdio>

SHAEngine::SHAEngine() : m_computed(false), m_processedBytes(0), m_unprocessedBytes(0)
{
    // make sure that the data type is the right size
    static_assert((sizeof(uint32_t) * 5) == SHA_DIGEST_LENGTH, "Data type used for holding the hash is not expected size.");

    // initialize hash constants
    m_digest.H0 = 0x67452301;
    m_digest.H1 = 0xEFCDAB89;
    m_digest.H2 = 0x98BADCFE;
    m_digest.H3 = 0x10325476;
    m_digest.H4 = 0xC3D2E1F0;
}

SHAEngine::~SHAEngine()
{
    // clear hash constants
    m_digest.H0 = 0;
    m_digest.H1 = 0;
    m_digest.H2 = 0;
    m_digest.H3 = 0;
    m_digest.H4 = 0;
}

/**
 * @brief Process data for the hash that doesn't fit into the 64bit processing block..
 */
void SHAEngine::Process_Partial(const void *&data, int &length)
{
    unsigned int bytestoprocess;

    if (length && data) {
        if (m_unprocessedBytes || length < SHA_BLOCK_LENGTH) {
            bytestoprocess = SHA_BLOCK_LENGTH - m_unprocessedBytes;

            if (length < (int)(SHA_BLOCK_LENGTH - m_unprocessedBytes)) {
                bytestoprocess = length;
            }

            std::memcpy(m_messageBlock + m_unprocessedBytes, data, bytestoprocess);
            data = static_cast<const char *>(data) + bytestoprocess;
            m_unprocessedBytes += bytestoprocess;
            length -= bytestoprocess;

            if (m_unprocessedBytes == SHA_BLOCK_LENGTH) {
                SHAEngine::Process_Block(m_messageBlock, m_digest);
                m_processedBytes += SHA_BLOCK_LENGTH;
                m_unprocessedBytes = 0;
            }
        }
    }
}

/**
 * @brief Process data for the hash.
 */
void SHAEngine::Hash(const void *data, int length)
{
    m_computed = false;
    Process_Partial(data, length);

    if (length) {
        if (length / SHA_BLOCK_LENGTH > 0) {
            int datablocks = length / SHA_BLOCK_LENGTH;

            for (int i = 0; i < datablocks; ++i) {
                SHAEngine::Process_Block(data, m_digest);
                data = static_cast<const char *>(data) + SHA_BLOCK_LENGTH;
                m_processedBytes += SHA_BLOCK_LENGTH;
                length -= SHA_BLOCK_LENGTH;
            }
        }

        Process_Partial(data, length);
    }
}

/**
 * @brief Finalise the hash and outputs it to the provided buffer.
 */
int SHAEngine::Result(void *output)
{
    int bytesremaining;
    uint32_t *finalp;
    SHADigest digest;
    char msgblock[SHA_BLOCK_LENGTH];
    char *mblockp;

    // If we already computed the hash, don't waste time doing it again
    if (m_computed) {
        std::memcpy(output, &m_finalDigest, SHA_DIGEST_LENGTH);
    } else {
        // These copies allow us to continue hashing data if we want
        std::memcpy(msgblock, m_messageBlock, SHA_BLOCK_LENGTH);
        uint32_t totalbytes = m_unprocessedBytes + m_processedBytes;

        // start padding with first bit set
        msgblock[m_unprocessedBytes++] = -128;

        // These copies allow us to continue hashing data if we want
        std::memcpy(&digest, &m_digest, sizeof(digest));
        bytesremaining = m_unprocessedBytes;

        // Message block must have at least 8bytes after padding
        // If it won't have enough space, pad with 0 and process
        if ((SHA_BLOCK_LENGTH - m_unprocessedBytes) < 8) {
            if (m_unprocessedBytes < SHA_BLOCK_LENGTH) {
                mblockp = msgblock;
                std::memset(mblockp + bytesremaining, 0, SHA_BLOCK_LENGTH - bytesremaining);
                m_unprocessedBytes = SHA_BLOCK_LENGTH;
            }

            SHAEngine::Process_Block(msgblock, digest);
            bytesremaining = 0;
            m_unprocessedBytes = 0;
        }

        std::memset(msgblock + bytesremaining, 0, 56 - bytesremaining);
        mblockp = &msgblock[56];
        // pad total length of data into last 8 bytes
        *reinterpret_cast<uint32_t *>(msgblock + 56) = htobe32(totalbytes >> 29);
        *reinterpret_cast<uint32_t *>(msgblock + 60) = htobe32(totalbytes << 3);

        m_unprocessedBytes = SHA_BLOCK_LENGTH;
        SHAEngine::Process_Block(&msgblock[0], digest);
        m_unprocessedBytes = 0;

        // format the digest into the correct byte order, big endian
        std::memcpy(&m_finalDigest, &digest, sizeof(m_finalDigest));
        finalp = reinterpret_cast<uint32_t *>(&m_finalDigest.H0);

        for (int i = 0; i < 5; ++i) {
            finalp[i] = htobe32(finalp[i]);
        }

        m_computed = true;
        std::memcpy(output, &m_finalDigest, SHA_DIGEST_LENGTH);
    }

    return SHA_DIGEST_LENGTH;
}

/**
 * @brief Processes a 64 byte block into the hash..
 */
void SHAEngine::Process_Block(const void *input, SHADigest &digest) const
{
    const uint8_t *data = static_cast<const uint8_t *>(input);
    int t;
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t K;
    uint32_t f;
    uint32_t W[80];

    // Copy and expand the message block (Rounds 0 to 15?)
    for (t = 0; t < 16; ++t) {
        W[t] = (data[t * 4] << 24) + (data[t * 4 + 1] << 16) + (data[t * 4 + 2] << 8) + data[t * 4 + 3];
    }

    for (; t < 80; ++t) {
        W[t] = __rotl32(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
    }

    // Initialize registers with the previous intermediate value.
    a = digest.H0;
    b = digest.H1;
    c = digest.H2;
    d = digest.H3;
    e = digest.H4;

    // main loop
    for (t = 0; t < 80; ++t) {
        if (t < 20) {
            K = 0x5A827999;
            f = (b & c) | ((~b) & d);

        } else if (t < 40) {
            K = 0x6ED9EBA1;
            f = b ^ c ^ d;

        } else if (t < 60) {
            K = 0x8F1BBCDC;
            f = (b & c) | (b & d) | (c & d);

        } else {
            K = 0xCA62C1D6;
            f = b ^ c ^ d;
        }

        uint32_t temp = __rotl32(a, 5) + f + e + W[t] + K;
        e = d;
        d = c;
        c = __rotl32(b, 30);
        b = a;
        a = temp;
    }

    // Compute the current intermediate hash value.
    digest.H0 += a;
    digest.H1 += b;
    digest.H2 += c;
    digest.H3 += d;
    digest.H4 += e;
}

/**
 * @brief Returns the SHA1 hash as a C string.
 *
 * @warning Multiple calls share a single buffer.
 */
const char *SHAEngine::Print_Result()
{
    uint8_t buffer[SHA_DIGEST_LENGTH];

    Result(buffer);

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        std::sprintf(&m_asciiDigest[2 * i], "%02x", buffer[i]);
    }

    return m_asciiDigest;
}
