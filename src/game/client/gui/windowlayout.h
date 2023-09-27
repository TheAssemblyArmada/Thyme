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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "mempoolobj.h"
#include <list>

class GameWindow;
class WindowLayout;

typedef void(__cdecl *WindowLayoutCallbackFunc)(WindowLayout *layout, void *user_data);

class WindowLayout : public MemoryPoolObject
{
    IMPLEMENT_NAMED_POOL(WindowLayout, WindowLayoutPool);

public:
    WindowLayout();

protected:
    virtual ~WindowLayout() override;

public:
    void Hide(bool hide);

    void Add_Window(GameWindow *window);
    void Remove_Window(GameWindow *window);
    void Destroy_Windows();

    bool Load(Utf8String filename);

    void Bring_Forward();

    GameWindow *Find_Window(GameWindow *window);

    Utf8String Get_Filename() const { return m_filenameString; }
    GameWindow *Get_Window_List() const { return m_windowList; }

    void Set_Init(WindowLayoutCallbackFunc func) { m_initFunc = func; }
    void Set_Update(WindowLayoutCallbackFunc func) { m_updateFunc = func; }
    void Set_Shutdown(WindowLayoutCallbackFunc func) { m_shutdownFunc = func; }

    bool Is_Hidden() { return m_hidden; }
    Utf8String Get_Filename() { return m_filenameString; }

    void Run_Init(void *user_data)
    {
        if (m_initFunc != nullptr) {
            m_initFunc(this, user_data);
        }
    }

    void Run_Update(void *user_data)
    {
        if (m_updateFunc != nullptr) {
            m_updateFunc(this, user_data);
        }
    }

    void Run_Shutdown(void *user_data)
    {
        if (m_shutdownFunc != nullptr) {
            m_shutdownFunc(this, user_data);
        }
    }

protected:
    Utf8String m_filenameString;
    GameWindow *m_windowList;
    GameWindow *m_windowTail;
    int m_windowCount;
    bool m_hidden;
    WindowLayoutCallbackFunc m_initFunc;
    WindowLayoutCallbackFunc m_updateFunc;
    WindowLayoutCallbackFunc m_shutdownFunc;
};

class WindowLayoutInfo
{
public:
    WindowLayoutInfo() : m_version(0), m_initFunc(nullptr), m_updateFunc(nullptr), m_shutdownFunc(nullptr) {}
    ~WindowLayoutInfo() {}

public:
    unsigned int m_version;
    WindowLayoutCallbackFunc m_initFunc;
    WindowLayoutCallbackFunc m_updateFunc;
    WindowLayoutCallbackFunc m_shutdownFunc;
    Utf8String m_initNameString;
    Utf8String m_updateNameString;
    Utf8String m_shutdownNameString;
    std::list<GameWindow *> m_windowList;
};
