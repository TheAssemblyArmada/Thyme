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

class OrthoVelocityPropertiesDialog : public OrthoVelocityBasePanel, public Updatable
{
public:
    OrthoVelocityPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class SphericalVelocityPropertiesDialog : public SphericalVelocityBasePanel, public Updatable
{
public:
    SphericalVelocityPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class HemisphericalVelocityPropertiesDialog : public HemisphericalVelocityBasePanel, public Updatable
{
public:
    HemisphericalVelocityPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class CylindricalVelocityPropertiesDialog : public CylindricalVelocityBasePanel, public Updatable
{
public:
    CylindricalVelocityPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class OutwardVelocityPropertiesDialog : public OutwardVelocityBasePanel, public Updatable
{
public:
    OutwardVelocityPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};
