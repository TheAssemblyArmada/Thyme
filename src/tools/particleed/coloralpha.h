#pragma once

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/clrpicker.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/xrc/xmlres.h>

#include "wxpartedui.h"

class ColorAndAlphaDialog : public ColorAlphaBase
{
public:
    ColorAndAlphaDialog(wxWindow *parent);

    // Event handlers
    void On_Changed(wxCommandEvent &event);

    // Entry handling.
    void Init_Entries();
    void Update(bool update_dialogs);

private:
    struct ColorPointers
    {
        wxColourPickerCtrl *color_picker;
        wxTextCtrl *color_frame;
        wxTextCtrl *alpha_min;
        wxTextCtrl *alpha_max;
        wxTextCtrl *alpha_frame;
    };
    
    ColorPointers m_controls[8];
};
