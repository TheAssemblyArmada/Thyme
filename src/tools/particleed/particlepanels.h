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

#include "updatable.h"

class _2DParticleParametersDialog : public Particle2DParamBase, public Updatable
{
public:
    _2DParticleParametersDialog(wxWindow *parent) : Particle2DParamBase(parent) {}
    virtual void Update(bool update_dialogs) override;
};

class _3DParticleParametersDialog : public Particle3DParamBase, public Updatable
{
public:
    _3DParticleParametersDialog(wxWindow *parent) : Particle3DParamBase(parent) {}

    void On_Change();
    virtual void Update(bool update_dialogs) override;
};

class StreakParticleParametersDialog : public StreakParticleParamBase, public Updatable
{
public:
    StreakParticleParametersDialog(wxWindow *parent) : StreakParticleParamBase(parent) {}
    virtual void Update(bool update_dialogs) override;
};

class EmptyPanel : public wxPanel, public Updatable
{
    EmptyPanel(wxWindow *parent) : wxPanel(parent) {}
    virtual void Update(bool update_dialogs) override {}
};
