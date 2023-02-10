/**
 * @file
 *
 * @author feliwir
 *
 * @brief A software implementation for video buffers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "videobuffer.h"

namespace Thyme
{
class SWVideoBuffer : public VideoBuffer
{
public:
    SWVideoBuffer(Type type);
    virtual ~SWVideoBuffer() override;
    virtual bool Allocate(unsigned width, unsigned height) override;
    virtual void Free() override;
    virtual void *Lock() override;
    virtual void Unlock() override;
    virtual bool Valid() override;

private:
    bool m_locked;
    uint8_t *m_data;
};

} // namespace Thyme
