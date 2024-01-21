/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Anim2d
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
#include "image.h"
#include "snapshot.h"

class Anim2DCollection;

enum Anim2DMode
{
    ANIM2D_MODE_NONE,
    ANIM2D_MODE_ONCE,
    ANIM2D_MODE_ONCE_BACKWARDS,
    ANIM2D_MODE_LOOP,
    ANIM2D_MODE_LOOP_BACKWARDS,
    ANIM2D_MODE_PING_PONG,
    ANIM2D_MODE_PING_PONG_BACKWARDS,
};

class Anim2DTemplate : public MemoryPoolObject
{
    IMPLEMENT_POOL(Anim2DTemplate)

public:
    Anim2DTemplate(Utf8String name);
    virtual ~Anim2DTemplate() override;

    void Allocate_Images(unsigned short count);
    Image *Get_Frame(unsigned short frame);
    void Store_Image(Image *image);

    static void Parse_Image(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Image_Sequence(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Num_Images(INI *ini, void *instance, void *store, const void *user_data);

    static const FieldParse *Get_Field_Parse() { return s_anim2dFieldParseTable; }

    unsigned short Get_Anim_Delay() const { return m_animDelay; }
    Anim2DMode Get_Anim_Mode() const { return m_animMode; }
    Utf8String Get_Name() const { return m_name; }
    Anim2DTemplate *Get_Next() const { return m_next; }
    unsigned short Get_Num_Frames() const { return m_numFrames; }
    bool Get_Randomize_Start_Frame() const { return m_randomizeStartFrame; }
    void Set_Next(Anim2DTemplate *next) { m_next = next; }

private:
    Anim2DTemplate *m_next;
    Utf8String m_name;
    Image **m_images;
    unsigned short m_numFrames;
    unsigned short m_animDelay;
    Anim2DMode m_animMode;
    bool m_randomizeStartFrame;

    static const FieldParse s_anim2dFieldParseTable[];
};

class Anim2D : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Anim2D);

    enum Anim2DStatus
    {
        STATUS_NO_UPDATE = 1,
        STATUS_ANIM_PING_PONG_DECREASING = 2,
        STATUS_ANIM_COMPLETE = 4,
    };

public:
    Anim2D(Anim2DTemplate *tmplate, Anim2DCollection *collection);
    virtual ~Anim2D() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Draw(int x, int y, int width, int height);
    int Get_Current_Frame_Height();
    int Get_Current_Frame_Width();
    void Randomize_Current_Frame();
    void Reset();
    void Set_Current_Frame(unsigned short frame);
    void Try_Next_Frame();

    void Clear_Status(unsigned char status) { m_status &= ~status; }
    unsigned char Get_Status() const { return m_status; }
    void Set_Status(unsigned char status) { m_status |= status; }
    void Set_First_Frame(unsigned short frame) { m_firstFrame = frame; }
    void Set_Alpha(float alpha) { m_alpha = alpha; }
    const Anim2DTemplate *Get_Template() const { return m_template; }

#ifdef GAME_DLL
    Anim2D *Hook_Ctor(Anim2DTemplate *tmplate, Anim2DCollection *collection)
    {
        return new (this) Anim2D(tmplate, collection);
    }
#endif

private:
    unsigned short m_currentFrame;
    unsigned int m_gameFrame;
    Anim2DTemplate *m_template;
    unsigned char m_status;
    unsigned short m_firstFrame;
    unsigned short m_lastFrame;
    unsigned int m_animDelay;
    float m_alpha;
    Anim2DCollection *m_collection;
    Anim2D *m_collectionSystemNext;
    Anim2D *m_collectionSystemPrev;
    friend class Anim2DCollection;
};

class Anim2DCollection : public SubsystemInterface
{
public:
    Anim2DCollection();

    virtual ~Anim2DCollection() override;
    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override;

    Anim2DTemplate *Find_Template(Utf8String const &name);
    Anim2DTemplate *Get_Next_Template(Anim2DTemplate *tmplate);
    Anim2DTemplate *New_Template(Utf8String const &name);
    void Register_Animation(Anim2D *animation);
    void Unregister_Animation(Anim2D *animation);
    static void Parse_Anim2D_Definition(INI *ini);
    static void Parse_Anim2D_Template(INI *ini, void *formal, void *store, const void *user_data);

    Anim2DTemplate *Get_Templates() const { return m_templateList; }

#ifdef GAME_DLL
    Anim2DCollection *Hook_Ctor() { return new (this) Anim2DCollection(); }
#endif

private:
    Anim2DTemplate *m_templateList;
    Anim2D *m_instanceList;
};

#ifdef GAME_DLL
extern Anim2DCollection *&g_theAnim2DCollection;
#else
extern Anim2DCollection *g_theAnim2DCollection;
#endif
