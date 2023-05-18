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
#include "anim2d.h"
#include "colorspace.h"
#include "gamelogic.h"
#include "ini.h"
#include "randomvalue.h"
#include "w3ddisplay.h"

#ifndef GAME_DLL
Anim2DCollection *g_theAnim2DCollection = nullptr;
#endif

static const char *s_Anim2DModeNames[] = {
    "NONE", "ONCE", "ONCE_BACKWARDS", "LOOP", "LOOP_BACKWARDS", "PING_PONG", "PING_PONG_BACKWARDS", nullptr
};

const FieldParse Anim2DTemplate::s_anim2dFieldParseTable[] = { { "NumberImages", &Parse_Num_Images, nullptr, 0 },
    { "Image", &Parse_Image, nullptr, 0 },
    { "Image_Sequence", &Parse_Image_Sequence, nullptr, 0 },
    { "AnimationMode", &INI::Parse_Index_List, s_Anim2DModeNames, offsetof(Anim2DTemplate, m_animMode) },
    { "AnimationDelay", &INI::Parse_Duration_Unsigned_Short, nullptr, offsetof(Anim2DTemplate, m_animDelay) },
    { "RandomizeStartFrame", &INI::Parse_Bool, nullptr, offsetof(Anim2DTemplate, m_randomizeStartFrame) },
    { nullptr, nullptr, nullptr, 0 } };

Anim2DTemplate::Anim2DTemplate(Utf8String name) :
    m_next(nullptr),
    m_name(name),
    m_images(0),
    m_numFrames(0),
    m_animDelay(0),
    m_animMode(ANIM2D_MODE_LOOP),
    m_randomizeStartFrame(false)
{
}

Anim2DTemplate::~Anim2DTemplate()
{
    if (m_images) {
        delete[] m_images;
    }
}

void Anim2DTemplate::Allocate_Images(unsigned short count)
{
    m_numFrames = count;
    m_images = new Image *[m_numFrames];

    for (int i = 0; i < m_numFrames; i++) {
        m_images[i] = nullptr;
    }
}

Image *Anim2DTemplate::Get_Frame(unsigned short frame)
{
    captainslog_dbgassert(m_images, "Anim2DTemplate::Get_Frame - Image data is NULL for animation '%s'", Get_Name().Str());

    if (frame < m_numFrames) {
        return m_images[frame];
    }

    captainslog_dbgassert(
        frame < m_numFrames, "Anim2DTemplate::Get_Frame - Illegal frame number '%d' for animation '%s'", Get_Name().Str());
    return nullptr;
}

void Anim2DTemplate::Store_Image(Image *image)
{
    if (image) {
        for (int i = 0; i < m_numFrames; i++) {
            if (m_images[i] == nullptr) {
                m_images[i] = image;
                return;
            }
        }

        captainslog_relassert(0,
            CODE_06,
            "Anim2DTemplate::Store_Image - Unable to store image '%s' into animation '%s' because the animation is setup to "
            "only support '%d' image frames",
            image->Get_Name().Str(),
            Get_Name().Str(),
            m_numFrames);
    }
}

void Anim2DTemplate::Parse_Image(INI *ini, void *instance, void *store, const void *user_data)
{
    Image *image;
    ImageCollection::Parse_Mapped_Image(ini, instance, &image, user_data);
    (static_cast<Anim2DTemplate *>(instance))->Store_Image(image);
}

void Anim2DTemplate::Parse_Image_Sequence(INI *ini, void *instance, void *store, const void *user_data)
{
    Anim2DTemplate *tmplate = static_cast<Anim2DTemplate *>(instance);
    captainslog_relassert(tmplate->Get_Num_Frames(),
        CODE_06,
        "Anim2DTemplate::Parse_Image_Sequence - You must specify the number of animation frames for animation '%s' *BEFORE* "
        "specifying the image sequence name",
        tmplate->Get_Name().Str());
    Utf8String str = ini->Get_Next_Ascii_String();
    Utf8String str2;

    for (int i = 0; i < tmplate->Get_Num_Frames(); i++) {
        str2.Format("%s%03d", str.Str(), i);
        Image *image = g_theMappedImageCollection->Find_Image_By_Name(str2);
        captainslog_relassert(image != nullptr,
            CODE_06,
            "Anim2DTemplate::Parse_Image_Sequence - Image '%s' not found for animation '%s'.  Check the number of images "
            "specified in INI and also make sure all the actual images exist.",
            str2.Str(),
            tmplate->Get_Name().Str());
        tmplate->Store_Image(image);
    }
}

void Anim2DTemplate::Parse_Num_Images(INI *ini, void *instance, void *store, const void *user_data)
{
    Anim2DTemplate *tmplate = static_cast<Anim2DTemplate *>(instance);
    unsigned int count;
    ini->Parse_Unsigned_Int(ini, instance, &count, user_data);
    captainslog_relassert(count,
        CODE_06,
        "Anim2DTemplate::Parse_Num_Images - Invalid animation '%s', animations must have '%d' or more frames defined",
        tmplate->Get_Name().Str(),
        1);
    tmplate->Allocate_Images(count);
}

Anim2D::Anim2D(Anim2DTemplate *tmplate, Anim2DCollection *collection) :
    m_currentFrame(0),
    m_gameFrame(0),
    m_template(tmplate),
    m_status(0),
    m_firstFrame(0),
    m_alpha(1.0f),
    m_collection(collection),
    m_collectionSystemNext(nullptr),
    m_collectionSystemPrev(nullptr)
{
    captainslog_dbgassert(tmplate, "Anim2D::Anim2D - NULL template");

    if (m_template->Get_Randomize_Start_Frame()) {
        Randomize_Current_Frame();
    } else {
        Reset();
    }

    m_lastFrame = m_template->Get_Num_Frames() - 1;
    m_animDelay = m_template->Get_Anim_Delay();

    if (m_collection) {
        m_collection->Register_Animation(this);
    }
}

Anim2D::~Anim2D()
{
    if (m_collection) {
        m_collection->Unregister_Animation(this);
    }
}

void Anim2D::Set_Current_Frame(unsigned short frame)
{
    captainslog_dbgassert(m_template, "Anim2D::Set_Current_Frame - No template for animation");
    captainslog_dbgassert(g_theGameLogic,
        "Anim2D::Set_Current_Frame - g_theGameLogic must exist to use animation instances (%s)",
        m_template->Get_Name().Str());
    captainslog_dbgassert(frame < m_template->Get_Num_Frames(),
        "Anim2D::Set_Current_Frame - Illegal frame number '%d' in animation %d",
        frame,
        m_template->Get_Name().Str());
    m_currentFrame = frame;
    m_gameFrame = g_theGameLogic->Get_Frame();
}

void Anim2D::Randomize_Current_Frame()
{
    captainslog_dbgassert(m_template, "Anim2D::Randomize_Current_Frame - No template for animation");
    Set_Current_Frame(Get_Client_Random_Value(0, m_template->Get_Num_Frames() - 1));
}

void Anim2D::Reset()
{
    captainslog_dbgassert(m_template, "Anim2D::Reset - No template for animation");

    switch (m_template->Get_Anim_Mode()) {
        case ANIM2D_MODE_ONCE:
        case ANIM2D_MODE_LOOP:
        case ANIM2D_MODE_PING_PONG:
            Set_Current_Frame(m_firstFrame);
            break;
        case ANIM2D_MODE_ONCE_BACKWARDS:
        case ANIM2D_MODE_LOOP_BACKWARDS:
        case ANIM2D_MODE_PING_PONG_BACKWARDS:
            Set_Current_Frame(m_lastFrame);
            break;
        default:
            captainslog_debug("Anim2D::Reset - Unknown animation mode '%d' for '%s'",
                m_template->Get_Anim_Mode(),
                m_template->Get_Name().Str());
            break;
    }
}

void Anim2D::Try_Next_Frame()
{
    captainslog_dbgassert(g_theGameLogic,
        "Anim2D::Try_Next_Frame - g_theGameLogic must exist to use animation instances (%s)",
        m_template->Get_Name().Str());

    if (g_theGameLogic->Get_Frame() - m_gameFrame >= m_animDelay) {
        switch (m_template->Get_Anim_Mode()) {
            case ANIM2D_MODE_ONCE:
                if (m_currentFrame < m_lastFrame) {
                    Set_Current_Frame(m_currentFrame + 1);
                } else {
                    Set_Status(STATUS_ANIM_COMPLETE);
                }

                break;
            case ANIM2D_MODE_ONCE_BACKWARDS:
                if (m_currentFrame <= m_firstFrame) {
                    Set_Status(STATUS_ANIM_COMPLETE);
                } else {
                    Set_Current_Frame(m_currentFrame - 1);
                }

                break;
            case ANIM2D_MODE_LOOP:
                if (m_currentFrame == m_lastFrame) {
                    Set_Current_Frame(m_firstFrame);
                } else {
                    Set_Current_Frame(m_currentFrame + 1);
                }

                break;
            case ANIM2D_MODE_LOOP_BACKWARDS:
                if (m_currentFrame <= m_firstFrame) {
                    Set_Current_Frame(m_lastFrame);
                } else {
                    Set_Current_Frame(m_currentFrame - 1);
                }

                break;
            case ANIM2D_MODE_PING_PONG:
            case ANIM2D_MODE_PING_PONG_BACKWARDS:
                if ((m_status & STATUS_ANIM_PING_PONG_DECREASING) != 0) {
                    if (m_currentFrame == m_firstFrame) {
                        Set_Current_Frame(m_currentFrame + 1);
                        Clear_Status(STATUS_ANIM_PING_PONG_DECREASING);
                    } else {
                        Set_Current_Frame(m_currentFrame - 1);
                    }
                } else if (m_currentFrame == m_lastFrame) {
                    Set_Current_Frame(m_currentFrame - 1);
                    Set_Status(STATUS_ANIM_PING_PONG_DECREASING);
                } else {
                    Set_Current_Frame(m_currentFrame + 1);
                }

                break;
            default:
                captainslog_debug("Anim2D::Try_Next_Frame - Unknown animation mode '%d' for '%s'",
                    m_template->Get_Anim_Mode(),
                    m_template->Get_Name().Str());
                break;
        };
    }
}

int Anim2D::Get_Current_Frame_Width()
{
    Image *image = m_template->Get_Frame(m_currentFrame);

    if (image != nullptr) {
        return image->Get_Image_Width();
    } else {
        return 0;
    }
}

int Anim2D::Get_Current_Frame_Height()
{
    Image *image = m_template->Get_Frame(m_currentFrame);

    if (image != nullptr) {
        return image->Get_Image_Height();
    } else {
        return 0;
    }
}

void Anim2D::Draw(int x, int y, int width, int height)
{
    Image *image = m_template->Get_Frame(m_currentFrame);
    captainslog_dbgassert(image,
        "Anim2D::draw - Image not found for frame '%d' on animation '%s'",
        m_currentFrame,
        m_template->Get_Name().Str());
    int color = Make_Color(0xFF, 0xFF, 0xFF, (255.0f * m_alpha));
    g_theDisplay->Draw_Image(image, x, y, width + x, height + y, color, Display::DRAWIMAGE_ADDITIVE);

    if (m_collection == nullptr && (m_status & STATUS_NO_UPDATE) == 0) {
        Try_Next_Frame();
    }
}

void Anim2D::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUnsignedShort(&m_currentFrame);
    xfer->xferUnsignedInt(&m_gameFrame);
    xfer->xferUnsignedByte(&m_status);
    xfer->xferUnsignedShort(&m_firstFrame);
    xfer->xferUnsignedShort(&m_lastFrame);
    xfer->xferUnsignedInt(&m_animDelay);
    xfer->xferReal(&m_alpha);
}

Anim2DCollection::Anim2DCollection() : m_templateList(nullptr), m_instanceList(nullptr) {}

Anim2DCollection::~Anim2DCollection()
{
    captainslog_dbgassert(!m_instanceList, "Anim2DCollection - instance list is not NULL");

    while (m_templateList) {
        Anim2DTemplate *next = m_templateList->Get_Next();
        m_templateList->Delete_Instance();
        m_templateList = next;
    }
}

void Anim2DCollection::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Animation2D.ini", INI_LOAD_OVERWRITE, nullptr);
}

void Anim2DCollection::Update()
{
    for (Anim2D *anim = m_instanceList; anim != nullptr; anim = anim->m_collectionSystemNext) {
        if ((anim->Get_Status() & Anim2D::STATUS_NO_UPDATE) == 0) {
            anim->Try_Next_Frame();
        }
    }
}

Anim2DTemplate *Anim2DCollection::Find_Template(Utf8String const &name)
{
    for (Anim2DTemplate *tmplate = m_templateList; tmplate != nullptr; tmplate = tmplate->Get_Next()) {
        if (tmplate->Get_Name() == name) {
            return tmplate;
        }
    }

    return nullptr;
}

Anim2DTemplate *Anim2DCollection::Get_Next_Template(Anim2DTemplate *tmplate)
{
    if (tmplate) {
        return tmplate->Get_Next();
    }

    return nullptr;
}

Anim2DTemplate *Anim2DCollection::New_Template(Utf8String const &name)
{
    Anim2DTemplate *tmplate = new Anim2DTemplate(name);
    tmplate->Set_Next(m_templateList);
    m_templateList = tmplate;
    return tmplate;
}

void Anim2DCollection::Register_Animation(Anim2D *animation)
{
    if (animation != nullptr) {
        captainslog_dbgassert(animation->m_collectionSystemNext == nullptr && animation->m_collectionSystemPrev == nullptr,
            "Registering animation instance, instance '%s' is already in a system",
            animation->Get_Template()->Get_Name().Str());
        animation->m_collectionSystemPrev = nullptr;
        animation->m_collectionSystemNext = m_instanceList;

        if (m_instanceList != nullptr) {
            m_instanceList->m_collectionSystemPrev = animation;
        }

        m_instanceList = animation;
    }
}

void Anim2DCollection::Unregister_Animation(Anim2D *animation)
{
    if (animation != nullptr && animation->m_collection == this) {
        if (animation->m_collectionSystemNext != nullptr) {
            animation->m_collectionSystemNext->m_collectionSystemPrev = animation->m_collectionSystemPrev;
        }
        if (animation->m_collectionSystemPrev != nullptr) {
            animation->m_collectionSystemPrev->m_collectionSystemNext = animation->m_collectionSystemNext;
        } else {
            m_instanceList = animation->m_collectionSystemNext;
        }
    }
}

void Anim2DCollection::Parse_Anim2D_Definition(INI *ini)
{
    Utf8String str(ini->Get_Next_Token());

    if (g_theAnim2DCollection != nullptr) {
        Anim2DTemplate *tmplate = g_theAnim2DCollection->Find_Template(str);
        captainslog_dbgassert(tmplate == nullptr,
            "Anim2DCollection::Parse_Anim2D_Definition - Animation template '%s' already exists",
            tmplate->Get_Name().Str());

        if (tmplate == nullptr) {
            tmplate = g_theAnim2DCollection->New_Template(str);
            captainslog_dbgassert(tmplate != nullptr,
                "Anim2DCollection::Parse_Anim2D_Definition -  unable to allocate animation template for '%s'",
                str.Str());
        }

        ini->Init_From_INI(tmplate, Anim2DTemplate::Get_Field_Parse());
    }
}

void Anim2DCollection::Parse_Anim2D_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String str = ini->Get_Next_Token();
    captainslog_dbgassert(
        g_theAnim2DCollection != nullptr, "Anim2DCollection::Parse_Anim2D_Template - TheAnim2DCollection is NULL");

    if (g_theAnim2DCollection != nullptr) {
        *(static_cast<Anim2DTemplate **>(store)) = g_theAnim2DCollection->Find_Template(str);
    }
}
