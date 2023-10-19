/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar Scheme
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
#include "coord.h"
#include "gametype.h"
#include "ini.h"
#include <list>

class Image;
class Player;
class PlayerTemplate;

class ControlBarSchemeImage
{
public:
    ControlBarSchemeImage();
    ~ControlBarSchemeImage();

    Utf8String m_name;
    ICoord2D m_position;
    ICoord2D m_size;
    Image *m_image;
    int m_layer;
};

class ControlBarSchemeAnimation
{
public:
    ControlBarSchemeAnimation();
    ~ControlBarSchemeAnimation();

    unsigned int Get_Current_Frame() const { return m_currentFrame; }
    ICoord2D Get_Start_Pos() const { return m_startPos; }
    void Set_Current_Frame(unsigned int frame) { m_currentFrame = frame; }
    void Set_Start_Pos(ICoord2D pos) { m_startPos = pos; }

    enum
    {
        CB_ANIM_SLIDE_RIGHT,
        CB_ANIM_MAX,
    };

    Utf8String m_name;
    int m_animType;
    ControlBarSchemeImage *m_animImage;
    unsigned int m_animDuration;
    ICoord2D m_finalPos;
    ICoord2D m_startPos;
    unsigned int m_currentFrame;
};

class ControlBarScheme
{
public:
    ControlBarScheme();
    ~ControlBarScheme();
    void Add_Animation(ControlBarSchemeAnimation *scheme_anim);
    void Add_Image(ControlBarSchemeImage *scheme_image);
    void Draw_Background(Coord2D multi, ICoord2D offset);
    void Draw_Foreground(Coord2D multi, ICoord2D offset);
    void Init();
    void Reset();
    void Update();
    void Update_Anim(ControlBarSchemeAnimation *anim);

    enum
    {
        LAYER_COUNT = 6,
    };

private:
    Utf8String m_name;
    ICoord2D m_screenCreationRes;
    Utf8String m_side;
    Image *m_buttonQueueImage;
    Image *m_rightHUDImage;
    int m_buildUpClockColor;
    int m_borderBuildColor;
    int m_borderActionColor;
    int m_borderUpgradeColor;
    int m_borderSystemColor;
    int m_commandBarBorderColor;
    Image *m_optionsButtonEnableImage;
    Image *m_optionsButtonHilitedImage;
    Image *m_optionsButtonPushedImage;
    Image *m_optionsButtonDisabledImage;
    Image *m_idleWorkerButtonEnableImage;
    Image *m_idleWorkerButtonHilitedImage;
    Image *m_idleWorkerButtonPushedImage;
    Image *m_idleWorkerButtonDisabledImage;
    Image *m_buddyButtonEnableImage;
    Image *m_buddyButtonHilitedImage;
    Image *m_buddyButtonPushedImage;
    Image *m_buddyButtonDisabledImage;
    Image *m_beaconButtonEnableImage;
    Image *m_beaconButtonHilitedImage;
    Image *m_beaconButtonPushedImage;
    Image *m_beaconButtonDisabledImage;
    Image *m_genBarButtonInImage;
    Image *m_genBarButtonOnImage;
    Image *m_toggleButtonUpInImage;
    Image *m_toggleButtonUpOnImage;
    Image *m_toggleButtonUpPushedImage;
    Image *m_toggleButtonDownInImage;
    Image *m_toggleButtonDownOnImage;
    Image *m_toggleButtonDownPushedImage;
    Image *m_generalButtonEnableImage;
    Image *m_generalButtonHilitedImage;
    Image *m_generalButtonPushedImage;
    Image *m_generalButtonDisabledImage;
    Image *m_uAttackButtonEnableImage;
    Image *m_uAttackButtonHilitedImage;
    Image *m_uAttackButtonPushedImage;
    Image *m_minMaxButtonEnableImage;
    Image *m_minMaxButtonHilitedImage;
    Image *m_minMaxButtonPushedImage;
    Image *m_genArrowImage;
    ICoord2D m_moneyUL;
    ICoord2D m_moneyLR;
    ICoord2D m_minMaxUL;
    ICoord2D m_minMaxLR;
    ICoord2D m_generalUL;
    ICoord2D m_generalLR;
    ICoord2D m_uAttackUL;
    ICoord2D m_uAttackLR;
    ICoord2D m_optionsUL;
    ICoord2D m_optionsLR;
    ICoord2D m_workerUL;
    ICoord2D m_workerLR;
    ICoord2D m_chatUL;
    ICoord2D m_chatLR;
    ICoord2D m_beaconUL;
    ICoord2D m_beaconLR;
    ICoord2D m_powerBarUL;
    ICoord2D m_powerBarLR;
    Image *m_expBarForegroundImage;
    Image *m_commandMarkerImage;
    Image *m_powerPurchaseImage;
    std::list<ControlBarSchemeImage *> m_layer[LAYER_COUNT];
    std::list<ControlBarSchemeAnimation *> m_animations;
    friend class ControlBarSchemeManager;
};

class ControlBarSchemeManager
{
public:
    ControlBarSchemeManager();
    ~ControlBarSchemeManager();
    void Init();
    void Update();
    void Draw_Background(ICoord2D offset);
    void Draw_Foreground(ICoord2D offset);
    void Set_Control_Bar_Scheme(Utf8String scheme_name);
    void Set_Control_Bar_Scheme_By_Player(Player *player);
    void Set_Control_Bar_Scheme_By_Player_Template(const PlayerTemplate *player_template, bool small);
    ControlBarScheme *Find_Control_Bar_Scheme(Utf8String name);
    ControlBarScheme *New_Control_Bar_Scheme(Utf8String name);
    void Preload_Assets(TimeOfDayType time_of_day);

    static FieldParse *Get_Field_Parse() { return m_controlBarSchemeFieldParseTable; }
    static void Parse_Animating_Part(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Animating_Part_Image(INI *ini, void *instance, void *store, const void *user_data);
    static void Parse_Image_Part(INI *ini, void *instance, void *store, const void *user_data);

private:
    ControlBarScheme *m_currentScheme;
    Coord2D m_multiplyer;
    std::list<ControlBarScheme *> m_schemeList;
    static FieldParse m_controlBarSchemeFieldParseTable[];
};
