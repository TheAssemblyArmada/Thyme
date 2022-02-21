/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Buffer Straw Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "xstraw.h"
#include <cstring>

int BufferStraw::Get(void *source, int slen)
{
    int total = 0;

    if (Is_Valid() && source != NULL && slen > 0) {
        int len = slen;

        if (m_bufferPtr.Get_Size() != 0) {
            int theoretical_max = m_bufferPtr.Get_Size() - m_index;
            len = (slen < theoretical_max) ? slen : theoretical_max;
        }

        if (len > 0) {
            memmove(source, static_cast<char *>(m_bufferPtr.Get_Buffer()) + m_index, len);
        }

        m_index += len;
        total += len;
    }

    return total;
}
