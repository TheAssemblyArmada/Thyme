/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Post Loadable class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

class PostLoadableClass
{
public:
    PostLoadableClass() : m_isPostLoadRegistered(false) {}
    virtual ~PostLoadableClass() {}
    virtual void On_Post_Load() {}
    bool Is_Post_Load_Registered() const { return m_isPostLoadRegistered; }
    void Set_Post_Load_Registered(bool state) { m_isPostLoadRegistered = state; }

private:
    bool m_isPostLoadRegistered;
};
