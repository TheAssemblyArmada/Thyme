/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Base interface for fetching a stream of data.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "straw.h"

Straw::~Straw()
{
    if (m_chainTo != nullptr) {
        m_chainTo->m_chainFrom = m_chainFrom;
    }

    if (m_chainFrom != nullptr) {
        m_chainFrom->Get_From(m_chainTo);
    }

    m_chainTo = nullptr;
    m_chainFrom = nullptr;
}

void Straw::Get_From(Straw *straw)
{
    if (m_chainTo != straw) {
        if (straw != nullptr && straw->m_chainFrom != nullptr) {
            straw->m_chainFrom->Get_From(nullptr);
            straw->m_chainFrom = nullptr;
        }

        if (m_chainTo != nullptr) {
            m_chainTo->m_chainFrom = nullptr;
        }

        m_chainTo = straw;

        if (straw) {
            straw->m_chainFrom = this;
        }
    }
}

int Straw::Get(void *source, int length)
{
    if (m_chainTo != nullptr) {
        return m_chainTo->Get(source, length);
    }

    return 0;
}
