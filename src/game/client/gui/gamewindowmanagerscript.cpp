/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game Window Manager Script
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorspace.h"
#include "display.h"
#include "filesystem.h"
#include "functionlexicon.h"
#include "gadgetcheckbox.h"
#include "gadgetcombobox.h"
#include "gadgetlistbox.h"
#include "gadgetprogressbar.h"
#include "gadgetpushbutton.h"
#include "gadgetradiobutton.h"
#include "gadgetslider.h"
#include "gadgetstatictext.h"
#include "gadgettabcontrol.h"
#include "gadgettextentry.h"
#include "gamefont.h"
#include "gametext.h"
#include "gamewindowmanager.h"
#include "headertemplate.h"
#include "image.h"
#include "namekeygenerator.h"

typedef bool(__cdecl *GameWindowParseFunc)(char *, WinInstanceData *, char *, void *);
typedef bool(__cdecl *LayoutScriptParseFunc)(char *, char *, unsigned int, WindowLayoutInfo *);

struct GameWindowParse
{
    const char *name;
    GameWindowParseFunc parse;
};

struct LayoutScriptParse
{
    const char *name;
    LayoutScriptParseFunc parse;
};

static WinDrawData s_hiliteSliderThumbDrawData[9];
static WinDrawData s_disabledSliderThumbDrawData[9];
static WinDrawData s_enabledSliderThumbDrawData[9];
static WinDrawData s_hiliteSliderDrawData[9];
static WinDrawData s_disabledSliderDrawData[9];
static WinDrawData s_enabledSliderDrawData[9];
static WinDrawData s_hiliteDownButtonDrawData[9];
static WinDrawData s_disabledDownButtonDrawData[9];
static WinDrawData s_enabledDownButtonDrawData[9];
static WinDrawData s_hiliteUpButtonDrawData[9];
static WinDrawData s_disabledUpButtonDrawData[9];
static WinDrawData s_enabledUpButtonDrawData[9];
static WinDrawData s_hiliteListBoxDrawData[9];
static WinDrawData s_disabledListBoxDrawData[9];
static WinDrawData s_enabledListBoxDrawData[9];
static WinDrawData s_hiliteEditBoxDrawData[9];
static WinDrawData s_disabledEditBoxDrawData[9];
static WinDrawData s_enabledEditBoxDrawData[9];
static WinDrawData s_hiliteDropDownButtonDrawData[9];
static WinDrawData s_disabledDropDownButtonDrawData[9];
static WinDrawData s_enabledDropDownButtonDrawData[9];

static GameWindow **s_stackPtr;
static GameWindow *s_windowStack[10];
static Utf8String s_theDrawString;
static Utf8String s_theTooltipString;
static Utf8String s_theInputString;
static Utf8String s_theSystemString;
static WindowCallbackFunc s_systemFunc;
static WindowCallbackFunc s_inputFunc;
static WindowTooltipFunc s_tooltipFunc;
static WindowDrawFunc s_drawFunc;
static int s_defEnabledColor;
static int s_defDisabledColor;
static int s_defBackgroundColor;
static int s_defHiliteColor;
static int s_defSelectedColor;
static int s_defTextColor;
static GameFont *s_defFont;

int Scan_Int(const char *token, int &value)
{
    return sscanf(token, "%d", &value);
}

bool Parse_Tooltip(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    Utf16String str;
    str.Set(U_CHAR("Need tooltip translation"));
    inst_data->Set_Tooltip_Text(str);
    return true;
}

bool Parse_Image_Offset(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    inst_data->m_imageOffset.x = atoi(strtok(buffer, " \t\n\r"));
    inst_data->m_imageOffset.y = atoi(strtok(nullptr, " \t\n\r"));
    return true;
}

bool Parse_Font(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    strtok(buffer, " ,\n\r\t");
    char *str;

    for (str = buffer; *str != '"'; str++) {
    }

    char font_name[256];
    str++;
    strcpy(font_name, strtok(str, ":,\n\r\t\""));
    strtok(nullptr, " ,\n\r\t");
    int font_size;
    int font_bold;
    Scan_Int(strtok(nullptr, " ,\n\r\t"), font_size);
    strtok(nullptr, " ,\n\r\t");
    Scan_Int(strtok(nullptr, " ,\n\r\t"), font_bold);

    if (g_theFontLibrary != nullptr) {
        GameFont *font = g_theFontLibrary->Get_Font(font_name, font_size, font_bold != 0);

        if (font != nullptr) {
            inst_data->m_font = font;
        }
    }

    return true;
}

bool Parse_Name(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    inst_data->m_decoratedNameString = strtok(buffer + 1, "\"");

    if (g_theNameKeyGenerator != nullptr) {
        inst_data->m_id = g_theNameKeyGenerator->Name_To_Key(inst_data->m_decoratedNameString.Str());
    }

    return true;
}

bool Parse_Bit_Flag(const char *flag_string, unsigned int *bits, const char **flag_list)
{
    unsigned char bit = 0;

    while (*flag_list != nullptr) {
        if (!strcasecmp(*flag_list, flag_string)) {
            *bits |= 1 << bit;
            return true;
        }

        bit++;
        flag_list++;
    }

    return 0;
}

void Parse_Bit_String(const char *in_buffer, unsigned int *bits, const char **flag_list)
{
    char buffer[256];

    strcpy(buffer, in_buffer);

    if (strncmp(buffer, "NULL", 4) != 0) {
        for (const char *str = strtok(buffer, "+"); str != nullptr; str = strtok(nullptr, "+")) {
            if (!Parse_Bit_Flag(str, bits, flag_list)) {
                captainslog_debug("Parse_Bit_String: Invalid flag '%s'.", str);
            }
        }
    }
}

static const char *s_windowStatusNames[] = { "ACTIVE",
    "TOGGLE",
    "DRAGABLE",
    "ENABLED",
    "HIDDEN",
    "ABOVE",
    "BELOW",
    "IMAGE",
    "TABSTOP",
    "NOINPUT",
    "NOFOCUS",
    "DESTROYED",
    "BORDER",
    "SMOOTH_TEXT",
    "ONE_LINE",
    "NO_FLUSH",
    "SEE_THRU",
    "RIGHT_CLICK",
    "WRAP_CENTERED",
    "CHECK_LIKE",
    "HOTKEY_TEXT",
    "USE_OVERLAY_STATES",
    "NOT_READY",
    "FLASHING",
    "ALWAYS_COLOR",
    "ON_MOUSE_DOWN",
    nullptr };

bool Parse_Status(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    inst_data->m_status = 0;
    Parse_Bit_String(buffer, &inst_data->m_status, s_windowStatusNames);
    return true;
}

static const char *s_windowStyleNames[] = { "PUSHBUTTON",
    "RADIOBUTTON",
    "CHECKBOX",
    "VERTSLIDER",
    "HORZSLIDER",
    "SCROLLLISTBOX",
    "ENTRYFIELD",
    "STATICTEXT",
    "PROGRESSBAR",
    "USER",
    "MOUSETRACK",
    "ANIMATED",
    "TABSTOP",
    "TABCONTROL",
    "TABPANE",
    "COMBOBOX",
    nullptr };

bool Parse_Style(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    inst_data->m_style = 0;
    Parse_Bit_String(buffer, &inst_data->m_style, s_windowStyleNames);
    return true;
}

bool Parse_System_Callback(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    const char *str = strtok(buffer + 1, "\"");
    captainslog_dbgassert(g_theNameKeyGenerator && g_theFunctionLexicon, "Invalid singletons");
    s_theSystemString = str;
    s_systemFunc = g_theFunctionLexicon->Game_Win_System_Func(
        g_theNameKeyGenerator->Name_To_Key(s_theSystemString.Str()), FunctionLexicon::TABLE_GAME_WIN_SYSTEM);
    return true;
}

bool Parse_Input_Callback(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    const char *str = strtok(buffer + 1, "\"");
    captainslog_dbgassert(g_theNameKeyGenerator && g_theFunctionLexicon, "Invalid singletons");
    s_theInputString = str;
    s_inputFunc = g_theFunctionLexicon->Game_Win_Input_Func(
        g_theNameKeyGenerator->Name_To_Key(s_theInputString.Str()), FunctionLexicon::TABLE_GAME_WIN_INPUT);
    return true;
}

bool Parse_Tooltip_Callback(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    const char *str = strtok(buffer + 1, "\"");
    captainslog_dbgassert(g_theNameKeyGenerator && g_theFunctionLexicon, "Invalid singletons");
    s_theTooltipString = str;
    s_tooltipFunc = g_theFunctionLexicon->Game_Win_Tooltip_Func(
        g_theNameKeyGenerator->Name_To_Key(s_theTooltipString.Str()), FunctionLexicon::TABLE_GAME_WIN_TOOLTIP);
    return true;
}

bool Parse_Draw_Callback(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    const char *str = strtok(buffer + 1, "\"");
    captainslog_dbgassert(g_theNameKeyGenerator && g_theFunctionLexicon, "Invalid singletons");
    s_theDrawString = str;
    s_drawFunc = g_theFunctionLexicon->Game_Win_Draw_Func(
        g_theNameKeyGenerator->Name_To_Key(s_theDrawString.Str()), FunctionLexicon::TABLE_ANY);
    return true;
}

bool Parse_Header_Template(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    const char *str = strtok(buffer + 1, "\"");
    captainslog_dbgassert(g_theNameKeyGenerator && g_theFunctionLexicon, "Invalid singletons");
    inst_data->m_headerTemplateString = str;
    return true;
}

int Scan_Bool(const char *token, bool &value)
{
    int data;
    int ret = sscanf(token, "%d", &data);
    value = data != 0;
    return ret;
}

int Scan_Short(const char *token, short &value)
{
    int data;
    int ret = sscanf(token, "%d", &data);
    value = data;
    return ret;
}

bool Parse_Listbox_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _ListboxData *lb_data = static_cast<_ListboxData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Short(strtok(nullptr, " :,\n\r\t"), lb_data->m_listLength);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), lb_data->m_autoScroll);
    char *str = strtok(nullptr, " :,\n\r\t");

    if (strcasecmp(str, "ScrollIfAtEnd") == 0) {
        Scan_Bool(strtok(nullptr, " :,\n\r\t"), lb_data->m_scrollIfAtEnd);
        strtok(nullptr, " :,\n\r\t");
    } else {
        lb_data->m_scrollIfAtEnd = false;
    }

    Scan_Bool(strtok(nullptr, " :,\n\r\t"), lb_data->m_autoPurge);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), lb_data->m_scrollBar);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), lb_data->m_multiSelect);
    strtok(nullptr, " :,\n\r\t");
    Scan_Short(strtok(nullptr, " :,\n\r\t"), lb_data->m_columns);

    if (lb_data->m_columns <= 1) {
        lb_data->m_columnWidthPercentage = nullptr;
    } else {
        lb_data->m_columnWidthPercentage = new int[lb_data->m_columns];

        for (int i = 0; i < lb_data->m_columns; i++) {
            strtok(nullptr, " :,\n\r\t");
            Scan_Int(strtok(nullptr, " :,\n\r\t"), lb_data->m_columnWidthPercentage[i]);
        }
    }

    lb_data->m_columnWidth = nullptr;
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), lb_data->m_forceSelect);
    return true;
}

bool Parse_Combo_Box_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _ComboBoxData *cb_data = static_cast<_ComboBoxData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), cb_data->m_isEditable);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), cb_data->m_maxChars);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), cb_data->m_maxDisplay);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), cb_data->m_asciiOnly);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), cb_data->m_lettersAndNumbersOnly);
    return true;
}

bool Parse_Slider_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _SliderData *sl_data = static_cast<_SliderData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), sl_data->m_minVal);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), sl_data->m_maxVal);
    return true;
}

bool Parse_Radio_Button_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _RadioButtonData *rb_data = static_cast<_RadioButtonData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), rb_data->m_group);
    return true;
}

bool Parse_Tooltip_Text(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    buffer++;

    if (strlen(buffer) == 1) {
        return true;
    } else {
        char *str = strtok(buffer, "\n\r\t\"");

        if (strlen(str) < 128) {
            inst_data->m_tooltipString.Set(str);
            inst_data->Set_Tooltip_Text(g_theGameText->Fetch(str));
            return true;
        } else {
            captainslog_debug("TextTooltip label '%s' is too long, max is '%d'", str, 128);
            return false;
        }
    }
}

bool Parse_Tooltip_Delay(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    Scan_Int(strtok(buffer, " :,\n\r\t"), inst_data->m_tooltipDelay);
    return true;
}

bool Parse_Text(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    while (*buffer != '"') {
        buffer++;
    }

    char *str = strtok(buffer + 1, "\n\r\t\"");

    if (strlen(str) < 128) {
        inst_data->m_textLabelString = str;
        return true;
    } else {
        captainslog_debug("Text label '%s' is too long, max is '%d'", str, 128);
        return false;
    }
}

int Scan_Unsigned_Int(const char *token, unsigned int &value)
{
    return sscanf(token, "%d", &value);
}

bool Parse_Text_Color(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    bool first_time = true;
    TextDrawData *color;

    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            color = &inst_data->m_enabledText;
        } else if (i == 1) {
            color = &inst_data->m_disabledText;
        } else if (i == 2) {
            color = &inst_data->m_hiliteText;
        } else {
            captainslog_debug("Undefined state for text color");
            return false;
        }

        if (first_time) {
            strtok(buffer, " :,\n\r\t");
        } else {
            strtok(nullptr, " :,\n\r\t");
        }

        first_time = false;

        unsigned int red;
        unsigned int green;
        unsigned int blue;
        unsigned int alpha;
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), red);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), green);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), blue);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), alpha);
        color->color = Make_Color(red, green, blue, alpha);

        strtok(nullptr, " :,\n\r\t");
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), red);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), green);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), blue);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), alpha);
        color->borderColor = Make_Color(red, green, blue, alpha);
    }

    return true;
}

bool Parse_Static_Text_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _TextData *st_data = static_cast<_TextData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), st_data->m_centered);
    st_data->m_vertCentered = 1;
    st_data->m_xOffset = 7;
    st_data->m_yOffset = 7;
    return true;
}

bool Parse_Text_Entry_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _EntryData *te_data = static_cast<_EntryData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Short(strtok(nullptr, " :,\n\r\t"), te_data->m_maxTextLen);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), te_data->m_secretText);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), te_data->m_numericalOnly);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), te_data->m_alphaNumericalOnly);
    strtok(nullptr, " :,\n\r\t");
    Scan_Bool(strtok(nullptr, " :,\n\r\t"), te_data->m_asciiOnly);
    return true;
}

bool Parse_Tab_Control_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    _TabControlData *tc_data = static_cast<_TabControlData *>(data);
    strtok(buffer, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), tc_data->m_tabOrientation);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), tc_data->m_tabEdge);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), tc_data->m_tabWidth);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), tc_data->m_tabHeight);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), tc_data->m_tabCount);
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), tc_data->m_paneBorder);
    int count = 0;
    strtok(nullptr, " :,\n\r\t");
    Scan_Int(strtok(nullptr, " :,\n\r\t"), count);

    for (int i = 0; i < count; i++) {
        Scan_Bool(strtok(nullptr, " :,\n\r\t"), tc_data->m_subPaneDisabled[i]);
    }

    return true;
}

bool Parse_Draw_Data(char *token, class WinInstanceData *inst_data, char *buffer, void *data)
{
    bool first_time = true;

    for (int i = 0; i < 9; i++) {
        WinDrawData *draw_data;

        if (strcmp(token, "ENABLEDDRAWDATA") == 0) {
            draw_data = &inst_data->m_enabledDrawData[i];
        } else if (strcmp(token, "DISABLEDDRAWDATA") == 0) {
            draw_data = &inst_data->m_disabledDrawData[i];
        } else if (strcmp(token, "HILITEDRAWDATA") == 0) {
            draw_data = &inst_data->m_hiliteDrawData[i];
        } else if (strcmp(token, "LISTBOXENABLEDUPBUTTONDRAWDATA") == 0) {
            draw_data = &s_enabledUpButtonDrawData[i];
        } else if (strcmp(token, "LISTBOXDISABLEDUPBUTTONDRAWDATA") == 0) {
            draw_data = &s_disabledUpButtonDrawData[i];
        } else if (strcmp(token, "LISTBOXHILITEUPBUTTONDRAWDATA") == 0) {
            draw_data = &s_hiliteUpButtonDrawData[i];
        } else if (strcmp(token, "LISTBOXENABLEDDOWNBUTTONDRAWDATA") == 0) {
            draw_data = &s_enabledDownButtonDrawData[i];
        } else if (strcmp(token, "LISTBOXDISABLEDDOWNBUTTONDRAWDATA") == 0) {
            draw_data = &s_disabledDownButtonDrawData[i];
        } else if (strcmp(token, "LISTBOXHILITEDOWNBUTTONDRAWDATA") == 0) {
            draw_data = &s_hiliteDownButtonDrawData[i];
        } else if (strcmp(token, "LISTBOXENABLEDSLIDERDRAWDATA") == 0) {
            draw_data = &s_enabledSliderDrawData[i];
        } else if (strcmp(token, "LISTBOXDISABLEDSLIDERDRAWDATA") == 0) {
            draw_data = &s_disabledSliderDrawData[i];
        } else if (strcmp(token, "LISTBOXHILITESLIDERDRAWDATA") == 0) {
            draw_data = &s_hiliteSliderDrawData[i];
        } else if (strcmp(token, "SLIDERTHUMBENABLEDDRAWDATA") == 0) {
            draw_data = &s_enabledSliderThumbDrawData[i];
        } else if (strcmp(token, "SLIDERTHUMBDISABLEDDRAWDATA") == 0) {
            draw_data = &s_disabledSliderThumbDrawData[i];
        } else if (strcmp(token, "SLIDERTHUMBHILITEDRAWDATA") == 0) {
            draw_data = &s_hiliteSliderThumbDrawData[i];
        } else if (strcmp(token, "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA") == 0) {
            draw_data = &s_enabledDropDownButtonDrawData[i];
        } else if (strcmp(token, "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA") == 0) {
            draw_data = &s_disabledDropDownButtonDrawData[i];
        } else if (strcmp(token, "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA") == 0) {
            draw_data = &s_hiliteDropDownButtonDrawData[i];
        } else if (strcmp(token, "COMBOBOXEDITBOXENABLEDDRAWDATA") == 0) {
            draw_data = &s_enabledEditBoxDrawData[i];
        } else if (strcmp(token, "COMBOBOXEDITBOXDISABLEDDRAWDATA") == 0) {
            draw_data = &s_disabledEditBoxDrawData[i];
        } else if (strcmp(token, "COMBOBOXEDITBOXHILITEDRAWDATA") == 0) {
            draw_data = &s_hiliteEditBoxDrawData[i];
        } else if (strcmp(token, "COMBOBOXLISTBOXENABLEDDRAWDATA") == 0) {
            draw_data = &s_enabledListBoxDrawData[i];
        } else if (strcmp(token, "COMBOBOXLISTBOXDISABLEDDRAWDATA") == 0) {
            draw_data = &s_disabledListBoxDrawData[i];
        } else if (strcmp(token, "COMBOBOXLISTBOXHILITEDRAWDATA") == 0) {
            draw_data = &s_hiliteListBoxDrawData[i];
        } else {
            captainslog_debug("ParseDrawData, undefined token '%s'", token);
            return 0;
        }

        if (first_time) {
            strtok(buffer, " :,\n\r\t");
        } else {
            strtok(nullptr, " :,\n\r\t");
        }

        first_time = false;
        char *str = strtok(nullptr, " :,\n\r\t");

        if (strcmp(str, "NoImage") == 0) {
            draw_data->image = nullptr;
        } else {
            draw_data->image = g_theMappedImageCollection->Find_Image_By_Name(str);
        }

        unsigned int red;
        unsigned int green;
        unsigned int blue;
        unsigned int alpha;
        strtok(nullptr, " :,\n\r\t");
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), red);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), green);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), blue);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), alpha);
        draw_data->color = Make_Color(red, green, blue, alpha);

        strtok(nullptr, " :,\n\r\t");
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), red);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), green);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), blue);
        Scan_Unsigned_Int(strtok(nullptr, " :,\n\r\t"), alpha);
        draw_data->borderColor = Make_Color(red, green, blue, alpha);
    }

    return true;
}

void *Get_Data_Template(char *type)
{
    if (strcmp(type, "VERTSLIDER") == 0 || strcmp(type, "HORZSLIDER") == 0) {
        static _SliderData sData;
        memset(&sData, 0, sizeof(_SliderData));
        return &sData;
    } else if (strcmp(type, "SCROLLLISTBOX") == 0) {
        static _ListboxData lData;
        memset(&lData, 0, sizeof(_ListboxData));
        return &lData;
    } else if (strcmp(type, "TABCONTROL") == 0) {
        static _TabControlData tcData;
        memset(&tcData, 0, sizeof(_TabControlData));
        return &tcData;
    } else if (strcmp(type, "ENTRYFIELD") == 0) {
        static _EntryData eData;
        memset(&eData, 0, sizeof(_EntryData));
        return &eData;
    } else if (strcmp(type, "STATICTEXT") == 0) {
        static _TextData tData;
        memset(&tData, 0, sizeof(_TextData));
        return &tData;
    } else if (strcmp(type, "RADIOBUTTON") == 0) {
        static _RadioButtonData rData;
        memset(&rData, 0, sizeof(_RadioButtonData));
        return &rData;
    } else if (strcmp(type, "COMBOBOX") == 0) {
        static _ComboBoxData cData;
        memset(&cData, 0, sizeof(_ComboBoxData));
        return &cData;
    } else {
        return nullptr;
    }
}

bool Parse_Init(char *token, char *buffer, unsigned int version, WindowLayoutInfo *info)
{
    info->m_initNameString = strtok(buffer, " \n\r\t");
    info->m_initFunc = g_theFunctionLexicon->Win_Layout_Init_Func(
        g_theNameKeyGenerator->Name_To_Key(info->m_initNameString.Str()), FunctionLexicon::TABLE_ANY);
    return true;
}

bool Parse_Update(char *token, char *buffer, unsigned int version, WindowLayoutInfo *info)
{
    info->m_updateNameString = strtok(buffer, " \n\r\t");
    info->m_updateFunc = g_theFunctionLexicon->Win_Layout_Init_Func(
        g_theNameKeyGenerator->Name_To_Key(info->m_updateNameString.Str()), FunctionLexicon::TABLE_WIN_LAYOUT_UPDATE);
    return true;
}

bool Parse_Shutdown(char *token, char *buffer, unsigned int version, WindowLayoutInfo *info)
{
    info->m_shutdownNameString = strtok(buffer, " \n\r\t");
    info->m_shutdownFunc = g_theFunctionLexicon->Win_Layout_Init_Func(
        g_theNameKeyGenerator->Name_To_Key(info->m_shutdownNameString.Str()), FunctionLexicon::TABLE_WIN_LAYOUT_SHUTDOWN);
    return true;
}

void Read_Until_Semicolon(File *file, char *buffer, int max_buf_len)
{
    int i = 0;
    bool is_before_word = true;

    while (i < max_buf_len) {
        file->Read(&buffer[i], 1);

        if (isspace(buffer[i])) {
            if (!is_before_word) {
                buffer[i++] = ' ';
            }
        } else {
            is_before_word = false;

            if (buffer[i] == ';') {
                buffer[i] = '\0';
                return;
            }

            i++;
        }
    }

    captainslog_debug("Read_Until_Semicolon: ERROR - Read buffer overflow - input truncated.");
    buffer[max_buf_len - 1] = '\0';
}

static LayoutScriptParse s_layoutScriptTable[] = { { "LAYOUTINIT", Parse_Init },
    { "LAYOUTUPDATE", Parse_Update },
    { "LAYOUTSHUTDOWN", Parse_Shutdown },
    { nullptr, nullptr } };

bool Parse_Layout_Block(File *in_file, char *buffer, unsigned int version, WindowLayoutInfo *info)
{
    Utf8String str;

    if (!in_file->Scan_String(str) || str.Compare("STARTLAYOUTBLOCK") != 0) {
        return false;
    } else {
        for (;;) {
            in_file->Scan_String(str);

            if (str.Compare("ENDLAYOUTBLOCK") == 0) {
                break;
            }

            for (LayoutScriptParse *parse = s_layoutScriptTable; parse != nullptr && parse->name != nullptr; parse++) {
                if (str.Compare(parse->name) == 0) {
                    Read_Until_Semicolon(in_file, buffer, 2048);
                    char buf[256];

                    if (parse->parse(buf, strtok(buffer, " ="), version, info)) {
                        break;
                    }

                    return false;
                }
            }
        }

        return true;
    }
}

WindowLayout *GameWindowManager::Win_Create_Layout(Utf8String filename)
{
    WindowLayout *layout = new WindowLayout();

    if (layout->Load(filename)) {
        return layout;
    } else {
        layout->Delete_Instance();
        return nullptr;
    }
}

void GameWindowManager::Free_Static_Strings()
{
    s_theSystemString.Clear();
    s_theInputString.Clear();
    s_theTooltipString.Clear();
    s_theDrawString.Clear();
}

void Reset_Window_Stack()
{
    memset(s_windowStack, 0, sizeof(s_windowStack));
    s_stackPtr = s_windowStack;
}

void Reset_Window_Defaults()
{
    s_defEnabledColor = 0;
    s_defDisabledColor = 0;
    s_defBackgroundColor = 0;
    s_defHiliteColor = 0;
    s_defSelectedColor = 0;
    s_defTextColor = 0;
    s_defFont = nullptr;
}

bool Parse_Color(int *color, char *buffer)
{
    int red = atoi(strtok(buffer, " \t\n\r"));
    int green = atoi(strtok(nullptr, " \t\n\r"));
    int blue = atoi(strtok(nullptr, " \t\n\r"));
    *color = g_theWindowManager->Win_Make_Color(red, green, blue, 255);
    return true;
}

bool Parse_Default_Color(int *color, File *in_file, char *buffer)
{
    Utf8String str;
    in_file->Scan_String(str);
    Read_Until_Semicolon(in_file, buffer, 2048);

    if (strcmp(buffer, "TRANSPARENT") == 0) {
        *color = 0xFFFFFF;
    } else {
        Parse_Color(color, buffer);
    }

    return true;
}

bool Parse_Default_Font(GameFont *font, File *in_file, char *buffer)
{
    Utf8String str;
    in_file->Scan_String(str);
    Read_Until_Semicolon(in_file, buffer, 2048);
    return true;
}

GameWindow *Peek_Window()
{
    if (s_stackPtr == s_windowStack) {
        return nullptr;
    } else {
        return *(s_stackPtr - 1);
    }
}

bool Parse_Screen_Rect(char *token, char *buffer, int *x, int *y, int *width, int *height)
{
    GameWindow *window = Peek_Window();
    int upper;
    int left;
    int bottom;
    int right;
    int creation_res_x;
    int creation_res_y;

    strtok(nullptr, " ,:=\n\r\t");
    Scan_Int(strtok(nullptr, " ,:=\n\r\t"), upper);
    Scan_Int(strtok(nullptr, " ,:=\n\r\t"), left);
    strtok(nullptr, " ,:=\n\r\t");
    Scan_Int(strtok(nullptr, " ,:=\n\r\t"), bottom);
    Scan_Int(strtok(nullptr, " ,:=\n\r\t"), right);
    strtok(nullptr, " ,:=\n\r\t");
    Scan_Int(strtok(nullptr, " ,:=\n\r\t"), creation_res_x);
    Scan_Int(strtok(nullptr, " ,:=\n\r\t"), creation_res_y);

    float f1 = (float)g_theDisplay->Get_Width() / (float)creation_res_x;
    float f2 = (float)g_theDisplay->Get_Height() / (float)creation_res_y;
    upper *= f1;
    left *= f2;
    bottom *= f1;
    right *= f2;

    if (window != nullptr) {
        int screen_x;
        int screen_y;
        window->Win_Get_Screen_Position(&screen_x, &screen_y);
        *x = upper - screen_x;
        *y = left - screen_y;
    } else {
        *x = upper;
        *y = left;
    }

    *width = bottom - upper;
    *height = right - left;
    return true;
}

bool Parse_Data(void **data, char *type, char *buffer)
{
    if (strcmp(type, "VERTSLIDER") == 0 || strcmp(type, "HORZSLIDER") == 0) {
        static _SliderData sData;
        memset(&sData, 0, sizeof(_SliderData));
        sData.m_minVal = atoi(strtok(buffer, " \t\n\r"));
        sData.m_maxVal = atoi(strtok(nullptr, " \t\n\r"));
        *data = &sData;
    } else if (strcmp(type, "SCROLLLISTBOX") == 0) {
        static _ListboxData lData;
        memset(&lData, 0, sizeof(_ListboxData));
        lData.m_listLength = atoi(strtok(buffer, " \t\n\r"));
        lData.m_autoScroll = atoi(strtok(nullptr, " \t\n\r")) != 0;
        lData.m_autoPurge = atoi(strtok(nullptr, " \t\n\r")) != 0;
        lData.m_scrollBar = atoi(strtok(nullptr, " \t\n\r")) != 0;
        lData.m_multiSelect = atoi(strtok(nullptr, " \t\n\r")) != 0;
        lData.m_forceSelect = atoi(strtok(nullptr, " \t\n\r")) != 0;
        *data = &lData;
    } else if (strcmp(type, "ENTRYFIELD") == 0) {
        static _EntryData eData;
        memset(&eData, 0, sizeof(_EntryData));
        eData.m_maxTextLen = atoi(strtok(buffer, " \t\n\r"));
        strtok(nullptr, " \t\n\r");
        const char *str = strtok(nullptr, " \t\n\r");

        if (str != nullptr) {
            int i = atoi(str);
            eData.m_secretText = i != 0;

            if (i) {
                eData.m_secretText = true;
            }
        } else {
            eData.m_secretText = false;
        }

        const char *str2 = strtok(nullptr, " \t\n\r");

        if (str2 != nullptr) {
            eData.m_numericalOnly = atoi(str2) == 1;
            eData.m_alphaNumericalOnly = atoi(str2) == 2;
            eData.m_asciiOnly = atoi(str2) == 3;
        } else {
            eData.m_numericalOnly = false;
            eData.m_alphaNumericalOnly = false;
            eData.m_asciiOnly = false;
        }

        *data = &eData;
    } else if (strcmp(type, "STATICTEXT") == 0) {
        static _TextData tData;
        int i = atoi(strtok(buffer, " \t\n\r"));
        tData.m_centered = i != 0;

        if (i) {
            tData.m_centered = true;
        }

        strtok(nullptr, " \t\n\r");
        *data = &tData;
    } else if (strcmp(type, "RADIOBUTTON") == 0) {
        static _RadioButtonData rData;
        rData.m_group = atoi(strtok(buffer, " \t\n\r"));
        *data = &rData;
    } else {
        *data = nullptr;
    }

    return true;
}

GameWindow *Create_Gadget(
    char *type, GameWindow *parent, int status, int x, int y, int width, int height, WinInstanceData *inst_data, void *data)
{
    inst_data->m_owner = parent;
    GameWindow *window = nullptr;

    if (strcmp(type, "PUSHBUTTON") == 0) {
        inst_data->m_style |= GWS_PUSH_BUTTON;
        return g_theWindowManager->Go_Go_Gadget_Push_Button(
            parent, status, x, y, width, height, inst_data, inst_data->m_font, false);
    } else if (strcmp(type, "RADIOBUTTON") == 0) {
        _RadioButtonData *rb_data = static_cast<_RadioButtonData *>(data);
        char buf[64];
        strcpy(buf, inst_data->m_decoratedNameString.Str());
        char *c = strrchr(buf, ':');

        if (c != nullptr) {
            *c = '\0';
        }

        if (g_theNameKeyGenerator != nullptr) {
            rb_data->m_screen = g_theNameKeyGenerator->Name_To_Key(buf);
        }

        inst_data->m_style |= GWS_RADIO_BUTTON;
        return g_theWindowManager->Go_Go_Gadget_Radio_Button(
            parent, status, x, y, width, height, inst_data, rb_data, inst_data->m_font, false);
    } else if (strcmp(type, "CHECKBOX") == 0) {
        inst_data->m_style |= GWS_CHECK_BOX;
        return g_theWindowManager->Go_Go_Gadget_Checkbox(
            parent, status, x, y, width, height, inst_data, inst_data->m_font, false);
    } else if (strcmp(type, "TABCONTROL") == 0) {
        _TabControlData *tc_data = static_cast<_TabControlData *>(data);
        inst_data->m_style |= GWS_TAB_CONTROL;
        return g_theWindowManager->Go_Go_Gadget_Tab_Control(
            parent, status, x, y, width, height, inst_data, tc_data, inst_data->m_font, false);
    } else if (strcmp(type, "VERTSLIDER") == 0) {
        _SliderData *sl_data = static_cast<_SliderData *>(data);
        inst_data->m_style |= GWS_VERT_SLIDER;
        window = g_theWindowManager->Go_Go_Gadget_Slider(
            parent, status, x, y, width, height, inst_data, sl_data, inst_data->m_font, false);
        GameWindow *child = window->Win_Get_Child();

        if (child != nullptr) {
            WinInstanceData *child_data = child->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledSliderThumbDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledSliderThumbDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteSliderThumbDrawData[i];
            }
        }
    } else if (strcmp(type, "HORZSLIDER") == 0) {
        _SliderData *sl_data = static_cast<_SliderData *>(data);
        inst_data->m_style |= GWS_HORZ_SLIDER;
        window = g_theWindowManager->Go_Go_Gadget_Slider(
            parent, status, x, y, width, height, inst_data, sl_data, inst_data->m_font, false);
        GameWindow *child = window->Win_Get_Child();

        if (child != nullptr) {
            WinInstanceData *child_data = child->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledSliderThumbDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledSliderThumbDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteSliderThumbDrawData[i];
            }
        }
    } else if (strcmp(type, "SCROLLLISTBOX") == 0) {
        _ListboxData *lb_data = static_cast<_ListboxData *>(data);
        inst_data->m_style |= GWS_SCROLL_LISTBOX;
        window = g_theWindowManager->Go_Go_Gadget_List_Box(
            parent, status, x, y, width, height, inst_data, lb_data, inst_data->m_font, false);
        GameWindow *up_button = Gadget_List_Box_Get_Up_Button(window);

        if (up_button != nullptr) {
            WinInstanceData *child_data = up_button->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledUpButtonDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledUpButtonDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteUpButtonDrawData[i];
            }
        }

        GameWindow *down_button = Gadget_List_Box_Get_Down_Button(window);

        if (down_button != nullptr) {
            WinInstanceData *child_data = down_button->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledDownButtonDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledDownButtonDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteDownButtonDrawData[i];
            }
        }

        GameWindow *slider = Gadget_List_Box_Get_Slider(window);

        if (slider != nullptr) {
            WinInstanceData *child_data = slider->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledSliderDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledSliderDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteSliderDrawData[i];
            }

            GameWindow *child = slider->Win_Get_Child();

            if (child != nullptr) {
                child_data = child->Win_Get_Instance_Data();

                for (int i = 0; i < 9; i++) {
                    child_data->m_enabledDrawData[i] = s_enabledSliderThumbDrawData[i];
                    child_data->m_disabledDrawData[i] = s_disabledSliderThumbDrawData[i];
                    child_data->m_hiliteDrawData[i] = s_hiliteSliderThumbDrawData[i];
                }
            }
        }
    } else if (strcmp(type, "COMBOBOX") == 0) {
        _ComboBoxData *cb_data = static_cast<_ComboBoxData *>(data);
        cb_data->m_entryData = new _EntryData;
        memset(cb_data->m_entryData, 0, sizeof(_EntryData));
        cb_data->m_listboxData = new _ListboxData;
        memset(cb_data->m_listboxData, 0, sizeof(_ListboxData));
        cb_data->m_entryCount = 0;
        cb_data->m_entryData->m_asciiOnly = cb_data->m_asciiOnly;
        cb_data->m_entryData->m_alphaNumericalOnly = cb_data->m_lettersAndNumbersOnly;
        cb_data->m_entryData->m_maxTextLen = cb_data->m_maxChars;
        cb_data->m_listboxData->m_listLength = 10;
        cb_data->m_listboxData->m_autoScroll = false;
        cb_data->m_listboxData->m_scrollIfAtEnd = false;
        cb_data->m_listboxData->m_autoPurge = false;
        cb_data->m_listboxData->m_scrollBar = true;
        cb_data->m_listboxData->m_multiSelect = false;
        cb_data->m_listboxData->m_forceSelect = true;
        cb_data->m_listboxData->m_columns = 1;
        cb_data->m_listboxData->m_columnWidth = nullptr;
        cb_data->m_listboxData->m_columnWidthPercentage = nullptr;
        inst_data->m_style |= GWS_COMBO_BOX;
        window = g_theWindowManager->Go_Go_Gadget_Combo_Box(
            parent, status, x, y, width, height, inst_data, cb_data, inst_data->m_font, 0);
        GameWindow *button = Gadget_Combo_Box_Get_Drop_Down_Button(window);

        if (button != nullptr) {
            WinInstanceData *child_data = button->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledDropDownButtonDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledDropDownButtonDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteDropDownButtonDrawData[i];
            }
        }

        GameWindow *edit = Gadget_Combo_Box_Get_Edit_Box(window);

        if (edit != nullptr) {
            WinInstanceData *child_data = edit->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledEditBoxDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledEditBoxDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteEditBoxDrawData[i];
            }
        }

        GameWindow *list = Gadget_Combo_Box_Get_List_Box(window);

        if (list != nullptr) {
            WinInstanceData *child_data = list->Win_Get_Instance_Data();

            for (int i = 0; i < 9; i++) {
                child_data->m_enabledDrawData[i] = s_enabledListBoxDrawData[i];
                child_data->m_disabledDrawData[i] = s_disabledListBoxDrawData[i];
                child_data->m_hiliteDrawData[i] = s_hiliteListBoxDrawData[i];
            }

            GameWindow *up_button = Gadget_List_Box_Get_Up_Button(list);

            if (up_button != nullptr) {
                child_data = up_button->Win_Get_Instance_Data();

                for (int i = 0; i < 9; i++) {
                    child_data->m_enabledDrawData[i] = s_enabledUpButtonDrawData[i];
                    child_data->m_disabledDrawData[i] = s_disabledUpButtonDrawData[i];
                    child_data->m_hiliteDrawData[i] = s_hiliteUpButtonDrawData[i];
                }
            }

            GameWindow *down_button = Gadget_List_Box_Get_Down_Button(list);

            if (down_button != nullptr) {
                child_data = down_button->Win_Get_Instance_Data();

                for (int i = 0; i < 9; i++) {
                    child_data->m_enabledDrawData[i] = s_enabledDownButtonDrawData[i];
                    child_data->m_disabledDrawData[i] = s_disabledDownButtonDrawData[i];
                    child_data->m_hiliteDrawData[i] = s_hiliteDownButtonDrawData[i];
                }
            }

            GameWindow *slider = Gadget_List_Box_Get_Slider(list);

            if (slider != nullptr) {
                child_data = slider->Win_Get_Instance_Data();

                for (int i = 0; i < 9; i++) {
                    child_data->m_enabledDrawData[i] = s_enabledSliderDrawData[i];
                    child_data->m_disabledDrawData[i] = s_disabledSliderDrawData[i];
                    child_data->m_hiliteDrawData[i] = s_hiliteSliderDrawData[i];
                }

                GameWindow *child = slider->Win_Get_Child();

                if (child != nullptr) {
                    child_data = child->Win_Get_Instance_Data();

                    for (int i = 0; i < 9; i++) {
                        child_data->m_enabledDrawData[i] = s_enabledSliderThumbDrawData[i];
                        child_data->m_disabledDrawData[i] = s_disabledSliderThumbDrawData[i];
                        child_data->m_hiliteDrawData[i] = s_hiliteSliderThumbDrawData[i];
                    }
                }
            }
        }
    } else if (strcmp(type, "ENTRYFIELD") == 0) {
        _EntryData *te_data = static_cast<_EntryData *>(data);
        inst_data->m_style |= GWS_ENTRY_FIELD;
        return g_theWindowManager->Go_Go_Gadget_Text_Entry(
            parent, status, x, y, width, height, inst_data, te_data, inst_data->m_font, false);
    } else if (strcmp(type, "STATICTEXT") == 0) {
        _TextData *st_data = static_cast<_TextData *>(data);
        inst_data->m_style |= GWS_STATIC_TEXT;
        return g_theWindowManager->Go_Go_Gadget_Static_Text(
            parent, status, x, y, width, height, inst_data, st_data, inst_data->m_font, false);
    } else if (strcmp(type, "PROGRESSBAR") == 0) {
        inst_data->m_style |= GWS_PROGRESS_BAR;
        return g_theWindowManager->Go_Go_Gadget_Progress_Bar(
            parent, status, x, y, width, height, inst_data, inst_data->m_font, false);
    }

    return window;
}

void Set_Window_Text(GameWindow *window, Utf8String text_label)
{
    if (text_label.Is_Empty()) {
        return;
    } else {
        Utf16String the_text;
        Utf16String entry_text;
        the_text = g_theGameText->Fetch(text_label.Str());

        if ((window->Win_Get_Style() & GWS_PUSH_BUTTON) != 0) {
            Gadget_Button_Set_Text(window, the_text);
        } else if ((window->Win_Get_Style() & GWS_RADIO_BUTTON) != 0) {
            Gadget_Radio_Set_Text(window, the_text);
        } else if ((window->Win_Get_Style() & GWS_CHECK_BOX) != 0) {
            Gadget_Check_Box_Set_Text(window, the_text);
        } else if ((window->Win_Get_Style() & GWS_STATIC_TEXT) != 0) {
            Gadget_Static_Text_Set_Text(window, the_text);
        } else if ((window->Win_Get_Style() & GWS_ENTRY_FIELD) != 0) {
            entry_text.Translate(text_label);
            Gadget_Text_Entry_Set_Text(window, entry_text);
        } else {
            window->Win_Set_Text(the_text);
        }
    }
}

GameWindow *Create_Window(char *type,
    int id,
    int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    void *data,
    WindowCallbackFunc system,
    WindowCallbackFunc input,
    WindowTooltipFunc tooltip,
    WindowDrawFunc draw)
{
    GameWindow *parent = Peek_Window();
    GameWindow *window;

    if (strcmp(type, "USER") == 0) {
        window = g_theWindowManager->Win_Create(parent, status, x, y, width, height, system, nullptr);

        if (window != nullptr) {
            inst_data->m_style |= GWS_USER_WINDOW;
            window->Win_Set_Instance_Data(inst_data);
            window->Win_Set_Window_Id(id);
        }
    } else if (strcmp(type, "TABPANE") == 0) {
        window = g_theWindowManager->Win_Create(parent, status, x, y, width, height, system, nullptr);

        if (window != nullptr) {
            inst_data->m_style |= GWS_TAB_PANE;
            window->Win_Set_Instance_Data(inst_data);
            window->Win_Set_Window_Id(id);
        }
    } else {
        window = Create_Gadget(type, parent, status, x, y, width, height, inst_data, data);

        if (window != nullptr) {
            window->Win_Set_Window_Id(id);
        }
    }

    if (window != nullptr) {
        if (system != nullptr) {
            window->Win_Set_System_Func(system);
        }

        if (input != nullptr) {
            window->Win_Set_Input_Func(input);
        }

        if (tooltip != nullptr) {
            window->Win_Set_Tooltip_Func(tooltip);
        }

        if (draw != nullptr) {
            window->Win_Set_Draw_Func(draw);
        }

        GameWindowEditData *edit_data = window->Win_Get_Edit_Data();

        if (edit_data != nullptr) {
            edit_data->system_callback_string = s_theSystemString;
            edit_data->input_callback_string = s_theInputString;
            edit_data->tooltip_callback_string = s_theTooltipString;
            edit_data->draw_callback_string = s_theDrawString;
        }
    }

    if (window != nullptr) {
        Set_Window_Text(window, inst_data->m_textLabelString);
    }

    if (window != nullptr && parent != nullptr) {
        g_theWindowManager->Win_Send_Input_Msg(parent, GWM_SCRIPT_CREATE, id, 0);
    }

    return window;
}

GameWindow *Pop_Window()
{
    if (s_stackPtr == s_windowStack) {
        return nullptr;
    }

    s_stackPtr--;
    return *s_stackPtr;
}

void Push_Window(GameWindow *window)
{
    if (s_stackPtr == &s_windowStack[9]) {
        captainslog_debug("Push_Window: Warning, stack overflow");
    } else {
        *s_stackPtr = window;
        s_stackPtr++;
    }
}

GameWindow *Parse_Window(File *in_file, char *buffer);

bool Parse_Child_Windows(GameWindow *window, File *in_file, char *buffer)
{
    Utf8String str;

    if ((window->Win_Get_Style() & GWS_TAB_CONTROL) != 0) {
        GameWindow *next;
        for (GameWindow *child = window->Win_Get_Child(); child != nullptr; child = next) {
            next = child->Win_Get_Next();
            g_theWindowManager->Win_Destroy(child);
        }
    }

    Push_Window(window);

    while (in_file->Scan_String(str) && str.Compare("ENDALLCHILDREN") != 0 && str.Compare("END") != 0) {
        if (str.Compare("ENABLEDCOLOR") != 0) {
            if (str.Compare("DISABLEDCOLOR") != 0) {
                if (str.Compare("HILITECOLOR") != 0) {
                    if (str.Compare("SELECTEDCOLOR") != 0) {
                        if (str.Compare("TEXTCOLOR") != 0) {
                            if (str.Compare("WINDOW") == 0 && !Parse_Window(in_file, buffer)) {
                                return false;
                            }
                        } else if (!Parse_Default_Color(&s_defTextColor, in_file, buffer)) {
                            return false;
                        }
                    } else if (!Parse_Default_Color(&s_defSelectedColor, in_file, buffer)) {
                        return false;
                    }
                } else if (!Parse_Default_Color(&s_defHiliteColor, in_file, buffer)) {
                    return false;
                }
            } else if (!Parse_Default_Color(&s_defDisabledColor, in_file, buffer)) {
                return false;
            }
        } else if (!Parse_Default_Color(&s_defEnabledColor, in_file, buffer)) {
            return false;
        }
    }

    if (Pop_Window() == window) {
        if ((window->Win_Get_Style() & GWS_TAB_CONTROL) != 0) {
            Gadget_Tab_Control_Fixup_Sub_Pane_List(window);
        }

        return true;
    } else {
        captainslog_debug("Parse_Child_Windows: unmatched window on stack.  Corrupt stack or bad source");
        return false;
    }
}

static GameWindowParse s_gameWindowFieldList[] = { { "NAME", Parse_Name },
    { "STATUS", Parse_Status },
    { "STYLE", Parse_Style },
    { "SYSTEMCALLBACK", Parse_System_Callback },
    { "INPUTCALLBACK", Parse_Input_Callback },
    { "TOOLTIPCALLBACK", Parse_Tooltip_Callback },
    { "DRAWCALLBACK", Parse_Draw_Callback },
    { "FONT", Parse_Font },
    { "HEADERTEMPLATE", Parse_Header_Template },
    { "LISTBOXDATA", Parse_Listbox_Data },
    { "COMBOBOXDATA", Parse_Combo_Box_Data },
    { "SLIDERDATA", Parse_Slider_Data },
    { "RADIOBUTTONDATA", Parse_Radio_Button_Data },
    { "TOOLTIPTEXT", Parse_Tooltip_Text },
    { "TOOLTIPDELAY", Parse_Tooltip_Delay },
    { "TEXT", Parse_Text },
    { "TEXTCOLOR", Parse_Text_Color },
    { "STATICTEXTDATA", Parse_Static_Text_Data },
    { "TEXTENTRYDATA", Parse_Text_Entry_Data },
    { "TABCONTROLDATA", Parse_Tab_Control_Data },
    { "ENABLEDDRAWDATA", Parse_Draw_Data },
    { "DISABLEDDRAWDATA", Parse_Draw_Data },
    { "HILITEDRAWDATA", Parse_Draw_Data },
    { "LISTBOXENABLEDUPBUTTONDRAWDATA", Parse_Draw_Data },
    { "LISTBOXENABLEDDOWNBUTTONDRAWDATA", Parse_Draw_Data },
    { "LISTBOXENABLEDSLIDERDRAWDATA", Parse_Draw_Data },
    { "LISTBOXDISABLEDUPBUTTONDRAWDATA", Parse_Draw_Data },
    { "LISTBOXDISABLEDDOWNBUTTONDRAWDATA", Parse_Draw_Data },
    { "LISTBOXDISABLEDSLIDERDRAWDATA", Parse_Draw_Data },
    { "LISTBOXHILITEUPBUTTONDRAWDATA", Parse_Draw_Data },
    { "LISTBOXHILITEDOWNBUTTONDRAWDATA", Parse_Draw_Data },
    { "LISTBOXHILITESLIDERDRAWDATA", Parse_Draw_Data },
    { "SLIDERTHUMBENABLEDDRAWDATA", Parse_Draw_Data },
    { "SLIDERTHUMBDISABLEDDRAWDATA", Parse_Draw_Data },
    { "SLIDERTHUMBHILITEDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXEDITBOXENABLEDDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXEDITBOXDISABLEDDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXEDITBOXHILITEDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXLISTBOXENABLEDDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXLISTBOXDISABLEDDRAWDATA", Parse_Draw_Data },
    { "COMBOBOXLISTBOXHILITEDRAWDATA", Parse_Draw_Data },
    { "IMAGEOFFSET", Parse_Image_Offset },
    { "TOOLTIP", Parse_Tooltip } };

GameWindow *Parse_Window(File *in_file, char *buffer)
{
    GameWindow *new_window = nullptr;
    GameWindow *window = Peek_Window();
    WinInstanceData inst_data;
    void *data = nullptr;
    Utf8String str;
    s_systemFunc = nullptr;
    s_inputFunc = nullptr;
    s_tooltipFunc = nullptr;
    s_drawFunc = nullptr;
    s_theSystemString.Clear();
    s_theInputString.Clear();
    s_theTooltipString.Clear();
    s_theDrawString.Clear();
    int width;
    int height;

    if (window != nullptr) {
        window->Win_Get_Size(&width, &height);
    } else {
        width = g_theDisplay->Get_Width();
        height = g_theDisplay->Get_Height();
    }

    inst_data.Init();
    inst_data.m_enabledText.color = s_defTextColor;
    inst_data.m_enabledText.borderColor = s_defTextColor;
    inst_data.m_disabledText.color = s_defTextColor;
    inst_data.m_disabledText.borderColor = s_defTextColor;
    inst_data.m_hiliteText.color = s_defTextColor;
    inst_data.m_hiliteText.borderColor = s_defTextColor;
    inst_data.m_font = s_defFont;
    Read_Until_Semicolon(in_file, buffer, 2048);
    strtok(buffer, " =;\n\r\t");
    char type[64];
    strcpy(type, strtok(nullptr, " =;\n\r\t"));
    data = Get_Data_Template(type);
    Read_Until_Semicolon(in_file, buffer, 2048);

    int screen_x;
    int screen_y;
    int screen_width;
    int screen_height;

    if (Parse_Screen_Rect(strtok(buffer, " =;\n\r\t"), buffer, &screen_x, &screen_y, &screen_width, &screen_height)) {
        for (;;) {
            for (;;) {
                GameWindowParse *parse;

                do {
                l1:
                    in_file->Scan_String(str);

                    for (parse = s_gameWindowFieldList; parse->parse != nullptr; parse++) {
                        if (str.Compare(parse->name) == 0) {
                            char token[256];
                            strcpy(token, str.Str());
                            in_file->Scan_String(str);
                            Read_Until_Semicolon(in_file, buffer, 2048);

                            if (!parse->parse(token, &inst_data, buffer, data)) {
                                captainslog_debug("Parse_Window: Error parsing %s", parse->name);
                                return new_window;
                            }

                            break;
                        }
                    }
                } while (parse->parse != nullptr);

                if (str.Compare("DATA") != 0) {
                    break;
                }

                in_file->Scan_String(str);
                Read_Until_Semicolon(in_file, buffer, 2048);

                if (!Parse_Data(&data, type, buffer)) {
                    captainslog_debug("Parse_Window: Error parsing %s", parse->name);
                    return new_window;
                }
            }

            if (str.Compare("END") == 0) {
                break;
            }

            if (str.Compare("CHILD") != 0) {
                Read_Until_Semicolon(in_file, buffer, 2048);
                goto l1;
            }

            new_window = Create_Window(type,
                inst_data.m_id,
                inst_data.Get_Status(),
                screen_x,
                screen_y,
                screen_width,
                screen_height,
                &inst_data,
                data,
                s_systemFunc,
                s_inputFunc,
                s_tooltipFunc,
                s_drawFunc);

            if (new_window == nullptr) {
                return new_window;
            }

            if (!Parse_Child_Windows(new_window, in_file, buffer)) {
                g_theWindowManager->Win_Destroy(new_window);
                new_window = nullptr;
                return new_window;
            }
        }

        if (g_theHeaderTemplateManager->Get_Font_From_Template(inst_data.m_headerTemplateString) != nullptr) {
            inst_data.m_font = g_theHeaderTemplateManager->Get_Font_From_Template(inst_data.m_headerTemplateString);
        }

        if (new_window == nullptr) {
            new_window = Create_Window(type,
                inst_data.m_id,
                inst_data.Get_Status(),
                screen_x,
                screen_y,
                screen_width,
                screen_height,
                &inst_data,
                data,
                s_systemFunc,
                s_inputFunc,
                s_tooltipFunc,
                s_drawFunc);
        }
    }

    return new_window;
}

GameWindow *GameWindowManager::Win_Create_From_Script(Utf8String filename, WindowLayoutInfo *info)
{
    static char buffer[2096];
    char path[PATH_MAX];
    const char *fname = filename.Str();
    GameWindow *window = nullptr;
    memset(path, 0, sizeof(path));
    strcpy(path, "Window\\");
    WindowLayoutInfo new_info;
    Utf8String str;
    Reset_Window_Stack();
    Reset_Window_Defaults();

    if (strchr(fname, '\\') != nullptr) {
        strcpy(path, fname);
    } else {
        sprintf(path, "Window\\%s", fname);
    }

    File *in_file = g_theFileSystem->Open_File(path, File::READ);

    if (in_file != nullptr) {
        in_file = in_file->Convert_To_RAM_File();
        in_file->Read(nullptr, strlen("FILE_VERSION = "));
        int version;
        in_file->Scan_Int(version);
        in_file->Next_Line(nullptr, 0);

        if (version < 2) {
            new_info.m_initNameString = "[None]";
            new_info.m_updateNameString = "[None]";
            new_info.m_shutdownNameString = "[None]";
            goto l1;
        }

        if (Parse_Layout_Block(in_file, buffer, version, &new_info)) {
        l1:
            while (in_file->Scan_String(str)) {
                if (str.Compare("END") != 0) {
                    if (str.Compare("ENABLEDCOLOR") != 0) {
                        if (str.Compare("DISABLEDCOLOR") != 0) {
                            if (str.Compare("HILITECOLOR") != 0) {
                                if (str.Compare("SELECTEDCOLOR") != 0) {
                                    if (str.Compare("TEXTCOLOR") != 0) {
                                        if (str.Compare("BACKGROUNDCOLOR") != 0) {
                                            if (str.Compare("FONT") != 0) {
                                                if (str.Compare("WINDOW") == 0) {
                                                    GameWindow *new_window = Parse_Window(in_file, buffer);

                                                    if (window == nullptr) {
                                                        window = new_window;
                                                    }

                                                    new_info.m_windowList.push_back(new_window);
                                                }
                                            } else if (!Parse_Default_Font(s_defFont, in_file, buffer)) {
                                                in_file->Close();
                                                in_file = nullptr;
                                                return nullptr;
                                            }
                                        } else if (!Parse_Default_Color(&s_defBackgroundColor, in_file, buffer)) {
                                            in_file->Close();
                                            in_file = nullptr;
                                            return nullptr;
                                        }
                                    } else if (!Parse_Default_Color(&s_defTextColor, in_file, buffer)) {
                                        in_file->Close();
                                        in_file = nullptr;
                                        return nullptr;
                                    }
                                } else if (!Parse_Default_Color(&s_defSelectedColor, in_file, buffer)) {
                                    in_file->Close();
                                    in_file = nullptr;
                                    return nullptr;
                                }
                            } else if (!Parse_Default_Color(&s_defHiliteColor, in_file, buffer)) {
                                in_file->Close();
                                in_file = nullptr;
                                return nullptr;
                            }
                        } else if (!Parse_Default_Color(&s_defDisabledColor, in_file, buffer)) {
                            in_file->Close();
                            in_file = nullptr;
                            return nullptr;
                        }
                    } else if (!Parse_Default_Color(&s_defEnabledColor, in_file, buffer)) {
                        in_file->Close();
                        in_file = nullptr;
                        return nullptr;
                    }
                }
            }

            in_file->Close();
            in_file = nullptr;

            if (info != nullptr) {
                *info = new_info;
            }

            return window;
        } else {
            captainslog_debug("Win_Create_From_Script: Error parsing layout block");
            return nullptr;
        }
    } else {
        captainslog_debug("Win_Create_From_Script: Cannot access file '%s'.", fname);
        return nullptr;
    }
}
